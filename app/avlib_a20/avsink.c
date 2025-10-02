#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include "rtc.h"
#include "uv.h"
#include "avdevice.h"
#include "avcodecs.h"
#include "avsink.h"

void *MainLoop(void);

#define MAX_AOBUF_SZ		15384
unsigned char	codecBuf[409600];	// 32000 

static uv_timer_t	*timerSinkAudio, _timerSinkAudio;
static uv_timer_t	*timerSinkVideo, _timerSinkVideo;
void _OnSinkAudioTimer(uv_timer_t *handle);
void _OnSinkVideoTimer(uv_timer_t *handle);

#define _SinkAudioTimerStart(timeout)	uv_timer_start(timerSinkAudio, _OnSinkAudioTimer, timeout, 0)
#define _SinkVideoTimerStart(timeout)	uv_timer_start(timerSinkVideo, _OnSinkVideoTimer, timeout, 0)


int avSinkOpen(AV_SINK *sink)
{
	int		rval;

	memset(sink, 0, sizeof(AV_SINK));
	sink->audioState = sink->videoState = AV_STATE_NULL;
	avStreamInit(sink, MAX_AV_TRACK_SZ);
	rval = avAudioOutputOpen();
	if(rval) return -1;
	rval = avVideoOutputOpen();
	if(rval) {
		avAudioOutputClose();
		rval = -1;
	}
	timerSinkAudio = &_timerSinkAudio;
	uv_timer_init((uv_loop_t *)MainLoop(), timerSinkAudio);
	timerSinkAudio->data = sink;
	timerSinkVideo = &_timerSinkVideo;
	uv_timer_init((uv_loop_t *)MainLoop(), timerSinkVideo);
	timerSinkVideo->data = sink;
	sink->audioState = sink->videoState = AV_STATE_IDLE;
	rval = 0;
	return rval;
}

void avSinkClose(AV_SINK *sink)
{
	if(avSinkState(sink) >= AV_STATE_IDLE) {
		avVideoOutputClose();
		avAudioOutputClose();
		sink->audioState = sink->videoState = AV_STATE_NULL;
	}
}

int avSinkStart(AV_SINK *sink)
{
	AV_STREAM_TRACK	*audioTrk, *videoTrk;
	AV_CODEC	*codec;
	int		rval, layerHandle;

	audioTrk = avStreamSrcTrack(sink, AV_MEDIA_TYPE_AUDIO);
	videoTrk = avStreamSrcTrack(sink, AV_MEDIA_TYPE_VIDEO);
	if(!audioTrk && !videoTrk) {
		printf("avSinkStart: no audio and video\n");
		return -1;
	}
	rval = 0;
	if(audioTrk) {
		avAudioMixerSetVolume((int)sink->volume);
		sink->audioCodec = codec = avCodecsFindDecoder(audioTrk->codecId);
		avCodecConfigure(codec, audioTrk);
		rval = avCodecStart(codec);
		if(rval < 0) return -1;
		audioTrk = avCodecOutputTrack(codec);
		rval = avAudioOutputStart(audioTrk);
		if(rval < 0) {
			avCodecStop(codec);
			return -1;
		}
		codec->periodSize = avAudioOutputPeriodSize();
	}
	if(videoTrk) {
		sink->videoCodec = codec = avCodecsFindDecoder(videoTrk->codecId);
		avCodecConfigure(codec, videoTrk);
		rval = avCodecStart(codec);
		if(rval < 0) {
			if(audioTrk) {
				avCodecStop(sink->audioCodec);
				avAudioOutputStop();
			}
			return -1;
		}
		videoTrk = avCodecOutputTrack(codec);
		layerHandle = avVideoOutputStart(videoTrk);
		if(layerHandle < 0) {
			avCodecStop(codec);
			if(audioTrk) {
				avCodecStop(sink->audioCodec);
				avAudioOutputStop();
			}
			return -1;
		}
		sink->layerHandle = layerHandle;
	}
	if(audioTrk) {
		_SinkAudioTimerStart(1);
		sink->audioState = AV_STATE_BUSY;
	}
	if(videoTrk) {
		_SinkVideoTimerStart(1);
		sink->videoState = AV_STATE_BUSY;
	}
	return 0;
}

static void _AvSinkAudioStopped(AV_SINK *sink)
{
	avAudioOutputStop();
	uv_timer_stop(timerSinkAudio);
	avCodecStop(sink->audioCodec);
	sink->audioState = AV_STATE_IDLE;
}

static void _AvSinkVideoStopped(AV_SINK *sink)
{
	avVideoOutputStop(sink->layerHandle);
	uv_timer_stop(timerSinkVideo);
	avCodecStop(sink->videoCodec);
	sink->videoState = AV_STATE_IDLE;
}

void avSinkStop(AV_SINK *sink)
{
	if(sink->audioState >= AV_STATE_BUSY) {
		_AvSinkAudioStopped(sink);
	}
	if(sink->videoState >= AV_STATE_BUSY) {
		_AvSinkVideoStopped(sink);
	}
}

void _OnSinkAudioTimer(uv_timer_t *handle)
{
    AV_SINK			*sink;
	AV_STREAM_TRACK	*trk;
	AV_CODEC		*codec;
	AV_BUFFER		*buffer, _buffer;
	int		rval, size, index, periodSize, loop;

    sink = (AV_SINK *)handle->data;
	codec = sink->audioCodec;
	trk = avCodecOutputTrack(codec);
	buffer = &_buffer;
	periodSize = avAudioOutputPeriodSize();
//printf("%lu ### OnSinkAudioTimer... ###\n", MS_TIMER);
	while(sink->audioState == AV_STATE_BUSY) {
		avAudioOutputRecover();
		size = avAudioOutputFreeSize();
		if(size < periodSize) {
			size = (avAudioOutputBufferSize() >> 1) - size;
			size = size * 1000 / trk->sampleRate + 1;
			_SinkAudioTimerStart(size);
			break;
		}
loop = 0;
		while(1) {
			rval = avCodecGetOutputBuffer(codec, 0, buffer);
loop++;
//printf("%lu avCodecGetOutputBuffer(): loop=%d rval=%d flags=%x\n", MS_TIMER, loop, rval, buffer->flags);
			if(!rval) {
				if(buffer->size > 0) {
					rval = avAudioOutputWrite(buffer->data);
					if(!rval) {
						codec->sampleCount += periodSize;
						avCodecPutOutputBuffer(codec, index);
					} else if(rval == 2) avAudioOutputRecover();
				}
				if(buffer->flags & AV_FLAG_END_OF_STREAM) rval = -1;
				break;
			}
			rval = avCodecGetInputBuffer(codec, buffer);
//printf("%lu avCodecGetInpuBuffer(): loop=%d rval=%d\n", MS_TIMER, loop, rval);
			if(!rval) { 
				buffer->size = periodSize << 2;
				avStreamPull(sink, buffer, AV_MEDIA_TYPE_AUDIO);
				if(buffer->status == 0 && buffer->size > 0) {
					buffer->flags = 0;
					rval = 0;
				} else if(buffer->status == 1) {
					buffer->size = 0; buffer->flags = AV_FLAG_END_OF_STREAM;
					rval = 0;
				} else {
					rval = -1;
				}
				if(!rval) avCodecPutInputBuffer(codec, buffer);
			} else {
				rval = 0;
			}
			if(rval < 0) break;
		}
		if(rval < 0) {
			sink->audioState = AV_STATE_DONE;
			avAudioOutputStopped();
		}
	}
	if(sink->audioState == AV_STATE_DONE) {
		if(avAudioOutputIsStopped()) {
			_AvSinkAudioStopped(sink);
			avStreamOnStopped(sink, AV_MEDIA_TYPE_AUDIO);
		} else {
			size = avAudioOutputBufferSize() - avAudioOutputFreeSize();
			size = size * 1000 / trk->sampleRate + 1;
			_SinkAudioTimerStart(size);
		}
	}
}

extern int		spibufintr;

void _OnSinkVideoTimer(uv_timer_t *handle)
{
    AV_SINK			*sink;
	AV_STREAM_TRACK	*trk;
	AV_CODEC		*codec;
	AV_BUFFER		*buffer, _buffer;
	int64_t		cts, pts;
	int		rval, index, val, putCount;

	sink = (AV_SINK *)handle->data;
	codec = sink->videoCodec;
	trk = avCodecOutputTrack(codec);
	buffer = &_buffer;
	putCount = 0;
	rval = -1;	
//printf("OnSinkVideoTimer...%d %d\n", codec->pFrameCount, codec->dFrameCount);
	while(sink->videoState == AV_STATE_BUSY) {
		index = avCodecDequeueOutputBuffer(codec);
		if(index >= 0) {
			avCodecGetOutputBuffer(codec, index, buffer);
//printf("GetOut: index=%d frameCount=%d size=%d\n", index, codec->pFrameCount, buffer->size);
			if(buffer->size > 0) {
				if(codec->pFrameCount == 1) {
					rtcGetMicroTimer(&sink->startTimer);
					adVideoOutputFirstFrame(sink->layerHandle, trk, buffer, &sink->_screen);
	//printf("pict: pts=%llu w=%d h=%d fps=%d/%d frame=%d\n", buffer->pts, pict->nWidth, pict->nHeight, pict->nFrameRateNum, pict->nFrameRateDen, pict->nFrameDuration);
				} else {
					adVideoOutputFrame(sink->layerHandle, buffer);
				}
				//audioFmt = avStreamOutputGetTrackFormat(stream, stream->audioIndex);
				//if(audioFmt && stream->audioState >= AV_STATE_BUSY) avCodecGetPresentTimestamp(audioFmt->codec, &cts);
				//else	cts = rtcMicroTimer() - sink->startTimer;
				rtcGetMicroTimer(&cts);
				cts -= sink->startTimer;
				avCodecGetPresentTimestamp(codec, &pts);	// pict->nFrameRateNum, pict->nFrameRateDen;
	//print_status(vd, a_pts, v_pts, a_pts - v_pts);
				val = (pts - cts) / 1000;
				if(val < 1) val = 1;
				_SinkVideoTimerStart(val);
				if(buffer->flags & AV_FLAG_END_OF_STREAM) { 
					sink->audioState = AV_STATE_DONE;
				}
				rval = 0;
			} else {
				//if(buffer->flags & AV_FLAG_END_OF_STREAM) { 
printf("### end....\n");
				rval = -1;
			}
			avCodecPutOutputBuffer(codec, index); 
			break;
		} else { 
			buffer->data = codecBuf; buffer->size = 0;
			avStreamPull(sink, buffer, AV_MEDIA_TYPE_VIDEO);
			if(buffer->status == 0 && buffer->size > 0) {
				buffer->flags = 0;
			} else {
				buffer->size = 0; buffer->flags = AV_FLAG_END_OF_STREAM;
			}
			rval = avCodecPutInputBuffer(codec, buffer);
			if(rval < 0) break;
			putCount++;
if(putCount > 1) printf("### acCodecPutInputBuffer(video): %d/%d count=%d\n", 
										buffer->frameCount, buffer->frameSize, putCount);
			codec->dFrameCount++;
			
		}
	}
	if(rval < 0) {
printf("video stopped...\n");
		_AvSinkVideoStopped(sink);
		avStreamOnStopped(sink, AV_MEDIA_TYPE_VIDEO);
	}
}

void avSinkSetVolume(AV_SINK *sink, int volume)
{
	sink->volume = volume;
}

void avSinkSetScreen(AV_SINK *sink, CGRect *screen)
{
	if(screen) {
		sink->_screen.x			= screen->x;
		sink->_screen.y			= screen->y;
		sink->_screen.width		= screen->width;
		sink->_screen.height	= screen->height;
	} else {
		sink->_screen.x = sink->_screen.y = 0;
		sink->_screen.width = sink->_screen.height = 0;
	}
}

int avSinkState(AV_SINK *sink)
{
	int		state;

//printf("audioState=%d videoState=%d\n", (int)sink->audioState, (int)sink->videoState);
	if(sink->audioState == AV_STATE_IDLE && sink->videoState >= AV_STATE_IDLE) state = AV_STATE_IDLE;
	else if(sink->audioState >= AV_STATE_BUSY || sink->videoState >= AV_STATE_BUSY) state = AV_STATE_BUSY;
	else	state = AV_STATE_NULL;
	return state;
}

