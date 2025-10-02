#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "v4l_a20.h"
#include "uv.h"
#include "avdevice.h"
#include "avcodecs.h"
#include "avsrc.h"

void *MainLoop(void);
static uv_timer_t	*timerVideoSrc, _timerVideoSrc;
static uv_poll_t	*pollVideoSrc, _pollVideoSrc;
static V4L_PICT		_pict;
static int			jpegSave;	
static char			jpegFileName[80];	

#define _VideoSrcTimerStart(timeout)	uv_timer_start(timerVideoSrc, _OnVideoSrcTimer, timeout, 0)

void _OnVideoSrcTimer(uv_timer_t *handle);


int avSourceOpen(AV_SOURCE *src)
{
	int		rval, fd;

	memset(src, 0, sizeof(AV_SOURCE));
	src->state = AV_STATE_NULL;
	avStreamInit(src, MAX_AV_TRACK_SZ);
	rval = avVideoInputOpen();
	if(rval) { 
		rval = -1;
	} else {
		timerVideoSrc = &_timerVideoSrc;
		uv_timer_init((uv_loop_t *)MainLoop(), timerVideoSrc);
		timerVideoSrc->data = src;
		pollVideoSrc = &_pollVideoSrc;
		fd = avVideoInputGet();
		uv_poll_init((uv_loop_t *)MainLoop(), pollVideoSrc, fd);
		pollVideoSrc->data = src;
		src->state = AV_STATE_IDLE;
		rval = 0;
	}
	jpegSave = 0;
	return rval;
}

void avSourceClose(AV_SOURCE *src)
{
	if(src->state >= AV_STATE_IDLE) {
		avVideoInputClose();
		src->state = AV_STATE_NULL;
	}
}

void _OnVideoSrcPoll(uv_poll_t *handle, int status, int events);

int avSourceStart(AV_SOURCE *src)
{
	AV_STREAM_TRACK	*trk, _trk, *trk2;
	AV_CODEC	*codec;
	int		rval, fd;

	if(src->state == AV_STATE_NULL) {
		printf("avSourceStart() error: camera not exist\n");
		return -1;
	}
	trk = &_trk;
	memset(trk, 0, sizeof(AV_STREAM_TRACK)); 
	trk->index			= 0;
	trk->trackId		= 1;
	trk->mediaType		= AV_MEDIA_TYPE_VIDEO;
	trk->mediaFormat	= AV_VIDEO_NV12;
	trk->codecId		= AV_CODEC_AVC;
	trk->codedWidth		= 640;
	trk->codedHeight	= 480;
	trk->width			= 640;
	trk->height			= 480;
	trk->frameRateNum	= 15;
	trk->frameRateDen	= 1;
	trk->bitRate		= 1*1024*1024;		// bps
	avStreamInit(src, MAX_AV_TRACK_SZ);
	avStreamAddTrack(src, trk);
	src->codec = codec = avCodecsFindEncoder(trk->codecId);
	avCodecConfigure(codec, trk);
	rval = avCodecStart(codec);
	if(rval < 0) return -1;
	rval = avVideoInputStart(trk);
	if(rval < 0) {
		avCodecStop(codec);
		return -1;
	}
	trk = avCodecOutputTrack(codec);
printf("avSourceStart: extraLen=%d\n", trk->extradataLen);
	trk2 = avStreamTrack(src, AV_MEDIA_TYPE_VIDEO);
	memcpy(trk2->extradata, trk->extradata, trk->extradataLen);
	trk2->extradataLen = trk->extradataLen;
	src->started = 0;
	_pict.v4lbuf.type = 0;
	uv_poll_start(pollVideoSrc, UV_READABLE, _OnVideoSrcPoll);
	src->state = AV_STATE_BUSY;
	return 0;
}

static void _AvSourceVideoStopped(AV_SOURCE *src)
{
	uv_poll_stop(pollVideoSrc);
	uv_timer_stop(timerVideoSrc);
	avCodecStop(src->codec);
	src->state = AV_STATE_IDLE;
}

void avSourceStop(AV_SOURCE *src)
{
	if(src->state >= AV_STATE_BUSY) {
		_AvSourceVideoStopped(src);
	}
}

void avSourceStartPreview(AV_SOURCE *src, CGRect *preview)
{
	AV_STREAM_TRACK	*trk;
	int		layerHandle;

	trk = avCodecOutputTrack(src->codec);	
	layerHandle = avVideoOutputStart(trk);
	if(layerHandle >= 0) src->layerHandle = layerHandle;
	if(preview) {
		src->_preview.x			= preview->x;
		src->_preview.y			= preview->y;
		src->_preview.width		= preview->width;
		src->_preview.height	= preview->height;
	} else {
		src->_preview.x  = src->_preview.y = 0;
		src->_preview.width = src->_preview.height = 0;
	}	
	src->previewState = 1;
}

void avSourceStopPreview(AV_SOURCE *src)
{
	avVideoOutputStop(src->layerHandle);
	src->previewState = 0;
}

int avSourceConnect(AV_SOURCE *src, void *sink, int mediaType)
{
	return avStreamConnect(src, sink, mediaType);
}

int avSourceDisconnect(AV_SOURCE *src, void *sink, int mediaType)
{
	return avStreamDisconnect(src, sink, mediaType);
}

unsigned long	camTimer;

void avSourceSaveJpeg(char *fileName)
{
	strcpy(jpegFileName, fileName);
	jpegSave = 1;
}

int  jpegSaveFile(char *fileName, int width, int height, void *yuvBuffer);

void _OnVideoSrcPoll(uv_poll_t *handle, int status, int events)
{
	AV_SOURCE	*src;
	AV_STREAM_TRACK	*trk;
	V4L_PICT	*pict;
	AV_BUFFER	*buffer, _buffer;
	unsigned long	paddr;
	int		fd, rval;

    src = (AV_SOURCE *)handle->data;
	trk = avCodecOutputTrack(src->codec);
	fd = handle->io_watcher.fd;
	pict = &_pict;
	rval = v4lDevCapture(fd, pict);
	if(rval) {
		printf("### capture error...\n");
		return;
	}
	if(jpegSave) {
		jpegSaveFile(jpegFileName, (int)trk->width, (int)trk->height, pict->addr);
		jpegSave = 0;
	}
	buffer = &_buffer;
	if(src->previewState) {
		buffer->paddrY		= paddr = pict->v4lbuf.m.offset;
		buffer->paddrC		= paddr + trk->width * trk->height;
		//buffer->pts			= pct->nPts;
		buffer->size		= 1;
		if(src->previewState == 1) {
			adVideoOutputFirstFrame(src->layerHandle, trk, buffer, &src->_preview);
			src->previewState++;
		} else {
			adVideoOutputFrame(src->layerHandle, buffer);
		}
	}
	if(!src->codec->pFrameCount) {
		rtcGetMicroTimer(&src->startTimer);
		_OnVideoSrcTimer(timerVideoSrc);
	}
//printf("%lu frame=%d %d\n", MS_TIMER-camTimer, pict->v4lbuf.index, pict->v4lbuf.type);
//camTimer = MS_TIMER;
}

void _OnVideoSrcTimer(uv_timer_t *handle)
{
	AV_SOURCE	*src;
	AV_CODEC	*codec;
	AV_BUFFER	*buffer, _buffer;
	unsigned long long	cts, pts;
	int		rval, val;

    src = (AV_SOURCE *)handle->data;
	codec = src->codec;
	buffer = &_buffer;
	buffer->paddrY	= _pict.v4lbuf.m.offset;
	buffer->data	= _pict.addr;
	buffer->size	= 1; 
	buffer->flags	= 0;
	rval = avCodecPutInputBuffer(codec, buffer);
	if(rval == 0) {
		while(1) {
			rval = avCodecDequeueOutputBuffer(codec);
			if(rval < 0) break;
			rval = avCodecGetOutputBuffer(codec, rval, buffer);
			buffer->pts = codec->pFrameCount - 1; buffer->dts = codec->dFrameCount - 1;
			buffer->duration = 1;
			avStreamPush(src, buffer, AV_MEDIA_TYPE_VIDEO);
			avCodecPutOutputBuffer(codec, rval);
		}
	}
	rtcGetMicroTimer(&cts);
	cts -= src->startTimer;
	avCodecGetPresentTimestamp(codec, &pts);
	val = (pts - cts) / 1000;
	if(val < 1) val = 1;
	_VideoSrcTimerStart(val);
}

int avSourceState(AV_SOURCE *src)
{
	return (int)src->state;
}

