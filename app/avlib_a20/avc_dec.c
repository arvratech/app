#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include "rtc.h"
#include "vdecoder.h"
#include "avstream.h"
#include "avc.h"

static VideoDecoder		*pVideoDec;
static VConfig			_vconfig;
static VideoStreamInfo	_vsi;


int avcDecOpen(AV_CODEC *codec)
{
	codec->encoder = 0;	
	codec->codecId = AV_CODEC_AVC;
	strcpy(codec->codecName, "AVC(H.264)");
	codec->state = 1;	
	return 0;
}

void avcDecClose(AV_CODEC *codec)
{
	if(codec->state > 1) avcDecStop(codec);
	codec->state = 0;	
}

int avcDecStart(AV_CODEC *codec)
{
	AV_STREAM_TRACK	*trk;
	int		rval;

	pVideoDec = CreateVideoDecoder();
	if(!pVideoDec) {
		perror("avcDecOpen: VideoDecCreate() error ");
		return -1;
	}
	trk = avCodecOutputTrack(codec);
	_vconfig.eOutputPixelFormat = PIXEL_FORMAT_YUV_PLANER_420;
	_vconfig.nVbvBufferSize		= 0;
	_vsi.eCodecFormat			= VIDEO_CODEC_FORMAT_H264;
	_vsi.nWidth					= trk->width;
	_vsi.nHeight				= trk->height;
	_vsi.nFrameRateNum			= trk->frameRateNum;
	_vsi.nFrameRateDen			= trk->frameRateDen;
	_vsi.nFrameDuration			= 0;
	_vsi.nAspectRatio			= 1000;
	_vsi.pCodecSpecificData		= trk->extradata;
	_vsi.nCodecSpecificDataLen	= trk->extradataLen;
	InitializeVideoDecoder(pVideoDec, &_vsi, &_vconfig);
	codec->state = 2;	
	return 0;
}

void avcDecStop(AV_CODEC *codec)
{
	if(codec->state > 1) {
		DestroyVideoDecoder(pVideoDec);
		codec->state = 1;	
	}
}

void _AvcDecDecodeSlice(AV_CODEC *codec, unsigned char *buf, int len)
{
	VideoStreamDataInfo	*vsdi, _vsdi;
	AV_STREAM_TRACK	*trk;
	char	*vbuf, *ringBuf;
	unsigned char	*p;
	int		i, type, rval, bufSize, ringBufSize, streamBufIndex; 

	type = buf[0] & 0x1f;
//printf("vdDecodeFrame: type=%d len=%d\n", type, len);
	rval = RequestVideoStreamBuffer(pVideoDec, len+4, &vbuf, &bufSize, &ringBuf, &ringBufSize, 0);
	p = (unsigned char *)vbuf;
	for(i = 0;i < 4;i++) {
		*p++ = len >> (24 - 8*i); bufSize--;
		if(bufSize <= 0) {
			p = (unsigned char *)ringBuf;
			bufSize = ringBufSize;
		}
	}
	if(len <= bufSize) memcpy(p, buf, len);
	else {
//printf("ringbuf back =======================\n");
		memcpy(p, buf, bufSize);	
		memcpy(ringBuf, buf+bufSize, len-bufSize);	
	}
	trk = avCodecOutputTrack(codec);
	vsdi = &_vsdi;
	vsdi->pData			= vbuf;
	vsdi->nLength		= 4+len;
	vsdi->nPts			= 0;
	vsdi->nPcr			= 0;
	vsdi->bIsFirstPart	= 1;
	vsdi->bIsLastPart	= 1;
	vsdi->nID			= codec->sliceCount;
	vsdi->nStreamIndex	= 0;
	vsdi->bValid		= 1;
	rval = SubmitVideoStreamData(pVideoDec, vsdi, 0);
	rval = DecodeVideoStream(pVideoDec, 0, 0, 0, vsdi->nPts);
//printf("DecodeVideoStream=%d\n", rval);
//	ResetVideoStreamBuffer(pVideoDec);
}

// |--------------- size --------------------|       
// |----|--------|----|------- ... ----------|
// |len1| dada1  |len2|      data 2          |
// |----|--------|----|------- ... ----------|
// sizex=1774 len1=2 len2=1664  
int avcDecPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer)
{
	unsigned char	*p;
	int		i, count, len;

	if(buffer->status) {
printf("%lu ### avcDecPutInputBuffer: %d/%d status=%d ###\n", MS_TIMER,
				buffer->frameCount, buffer->frameSize, buffer->status);
		return -1;
	} 
	count = buffer->size;
	p = buffer->data;
	while(count > 4) {
		len = 0;
		for(i = 0;i < 4;i++) {
			len = (len << 8) | p[i];
		}
		p += 4; count -= 4;
		if(len > count) {
			p -= 4;
printf("%lu ### avcDecPutInputBuffer: %d/%d data error: [%02x%02x%02x%2x] > %d ###\n", MS_TIMER,
				buffer->frameCount, buffer->frameSize,
				(int)p[0], (int)p[1], (int)p[2], (int)p[3], count);
			break;
		} else { 
			_AvcDecDecodeSlice(codec, p, len);
			p += len; count -= len;
			codec->sliceCount++;
		}
	}
	return 0;
}

int avcDecGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer)
{
	VideoPicture	*pct;
	AV_STREAM_TRACK	*trk;
	int		rval;

	trk = avCodecOutputTrack(codec);
	pct = GetOutputBuffer(pVideoDec, index);
	if(pct) {
		trk->codedWidth		= pct->nWidth;
		trk->codedHeight	= pct->nHeight;
		//vpct->nLeftOffset, pct->nTopOffset;
		trk->width			= pct->nRightOffset - pct->nLeftOffset;
		trk->height			= pct->nBottomOffset - pct->nTopOffset;
		trk->frameRateNum	= pct->nFrameRateNum;
		trk->frameRateDen	= pct->nFrameRateDen;
		buffer->paddrY		= pct->phyYBufAddr;
		buffer->paddrC		= pct->phyCBufAddr;
		buffer->pts			= pct->nPts;
		buffer->size		= 1;
		rval = 0;
	} else {
		rval  -1;
	}
	return rval;
}

int avcDecDequeueOutputBuffer(AV_CODEC *codec)
{
	return DequeueOutputBuffer(pVideoDec);
}

int avcDecEnqueueOutputBuffer(AV_CODEC *codec, int index)
{
	return EnqueueOutputBuffer(pVideoDec, index);
}

