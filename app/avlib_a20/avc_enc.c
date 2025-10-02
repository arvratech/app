#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include "v4l.h"
#include "vencoder.h"
#include "avstream.h"
#include "avc.h"

static VideoEncoder			*pVideoEnc;
static VencBaseConfig		_baseConfig;
static VencAllocateBufferParam	_bufParam;
static VencH264Param		_h264Param;
static VencH264FixQP		_fixQP;
static VencHeaderData		_headData;
static VencOutputBuffer		*outBuffer, _outBuffer;


int avcEncOpen(AV_CODEC *codec)
{
	codec->encoder = 1;	
	codec->codecId = AV_CODEC_AVC;
	strcpy(codec->codecName, "AVC(H.264)");
	codec->state = 1;	
	return 0;
}

void avcEncClose(AV_CODEC *codec)
{
	if(codec->state > 1) avcEncStop(codec);
	codec->state = 0;	
}

int avcEncStart(AV_CODEC *codec)
{
	AV_STREAM_TRACK	*trk;
	unsigned char	*p;
	int		rval, temp, value, codecType;
	
printf("avcEncStart...\n");
	codecType = VENC_CODEC_H264;
	pVideoEnc = VideoEncCreate(codecType);
	if(!pVideoEnc) {
		perror("avcEncStart: VideoEncCreate() error ");
		return -1;
	}
	trk = avCodecOutputTrack(codec);
	_h264Param.bEntropyCodingCABAC	= 1;
	_h264Param.nBitrate				= trk->bitRate;
	_h264Param.nFramerate			= trk->frameRateNum;	// fps
	_h264Param.nCodingMode			= VENC_FRAME_CODING;	// VENC_FIELD_CODING;
	_h264Param.nMaxKeyInterval		= 30;
	_h264Param.sProfileLevel.nProfile= VENC_H264ProfileBaseline;	// VENC_H264ProfileMain;
	_h264Param.sProfileLevel.nLevel	= VENC_H264Level2;				// VENC_H264Level31;
	_h264Param.sQPRange.nMinqp		= 10;
	_h264Param.sQPRange.nMaxqp		= 40;
	VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264Param, &_h264Param);
	value = 0;
	VideoEncSetParameter(pVideoEnc, VENC_IndexParamIfilter, &value);
	value = 0;	// degree
	VideoEncSetParameter(pVideoEnc, VENC_IndexParamRotation, &value);
	//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264FixQP, &fixQP);
	//VideoEncSetParameter(pVideoEnc, VENC_IndexParamH264CyclicIntraRefresh, &sIntraRefresh);
	memset(&_baseConfig, 0, sizeof(VencBaseConfig));
	_baseConfig.nInputWidth			= trk->codedWidth;
	_baseConfig.nInputHeight		= trk->codedHeight;
	_baseConfig.nStride				= trk->codedWidth;
	_baseConfig.nDstWidth			= trk->width;
	_baseConfig.nDstHeight			= trk->height;
	_baseConfig.eInputFormat		= VENC_PIXEL_YUV420SP;
	VideoEncInit(pVideoEnc, &_baseConfig);
	VideoEncGetParameter(pVideoEnc, VENC_IndexParamH264SPSPPS, &_headData);
printf("avcEncStart: SPS_PPS: [%02x", _headData.pBuffer[0]);
for(rval = 1;rval < _headData.nLength;rval++) printf("-%02x", _headData.pBuffer[rval]); printf("]\n");
	trk->extradataLen = _headData.nLength;
	memcpy(trk->extradata, _headData.pBuffer, trk->extradataLen);
	memset(&_bufParam, 0 ,sizeof(VencAllocateBufferParam));
	_bufParam.nSizeY				= _baseConfig.nInputWidth * _baseConfig.nInputHeight;
	_bufParam.nSizeC				= _baseConfig.nInputWidth * _baseConfig.nInputHeight / 2;
	_bufParam.nBufferNum			= 4;
	AllocInputBuffer(pVideoEnc, &_bufParam);
	codec->state = 2;
	outBuffer = NULL;
	return 0;
}

void avcEncStop(AV_CODEC *codec)
{
	if(codec->state > 1) {
printf("avcEncStop...\n");
		VideoEncUnInit(pVideoEnc);
		VideoEncDestroy(pVideoEnc);
		codec->state = 1;	
	}
}

int		encloop;

int avcEncPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer)
{
	VencInputBuffer		*inBuffer, _inBuffer;
	AV_STREAM_TRACK	*trk;
	int		rval, offset;

	trk = avCodecOutputTrack(codec);
	inBuffer = &_inBuffer;
	memset(inBuffer, 0, sizeof(VencInputBuffer));
	offset = trk->width * trk->height;
	inBuffer->pAddrPhyY			= (unsigned char *)buffer->paddrY;
	inBuffer->pAddrPhyC			= (unsigned char *)(inBuffer->pAddrPhyY + offset);
	inBuffer->pAddrVirY			= buffer->data;
	inBuffer->pAddrVirC			= inBuffer->pAddrVirY + offset;
	inBuffer->bEnableCorp		= 0;
	inBuffer->sCropInfo.nLeft	= 0;
	inBuffer->sCropInfo.nTop	= 0;
	inBuffer->sCropInfo.nWidth	= trk->width;
	inBuffer->sCropInfo.nHeight	= trk->height;
if(!(encloop & 0x03)) inBuffer->nFlag = VENC_BUFFERFLAG_KEYFRAME;
else	inBuffer->nFlag = 0;
encloop++;
	FlushCacheAllocInputBuffer(pVideoEnc, inBuffer);
	AddOneInputBuffer(pVideoEnc, inBuffer);
	rval = VideoEncodeOneFrame(pVideoEnc);
//printf("avcEncPutInputBuffer: VideoEncodeOneFrame=%d...\n", rval);
	AlreadyUsedInputBuffer(pVideoEnc, inBuffer);
	return 0;
}

int avcEncGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer)
{
	unsigned char	*p;
	int		size;

	if(!outBuffer) return -1;
	buffer->data	= outBuffer->pData0;
	buffer->size	= outBuffer->nSize0;
	buffer->flags	= 0;
	buffer->dts		= buffer->pts = outBuffer->nPts;
	size = buffer->size - 4;
	p = buffer->data;
//printf("[%02x-%02x-%02x-%02x-%02x..]\n", (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4]);
	p[0] = size >> 24;p[1] = size >> 16; p[2] = size >> 8; p[3] = size;	
	if((p[4] & 0x1f) == 5) buffer->flags |= AV_FLAG_KEY_FRAME;
	return 0;
}

int avcEncDequeueOutputBuffer(AV_CODEC *codec)
{
	int		rval, index;

	rval = GetOneBitstreamFrame(pVideoEnc, &_outBuffer);
//printf("avcEncDequeueOutputBuffer: GetOneBitStreamFrame=%d\n", rval);
	if(rval < 0) index = -1;
	else {
		outBuffer = &_outBuffer; index = 0;
	}
	return index;
}

int avcEncEnqueueOutputBuffer(AV_CODEC *codec, int index)
{
	int		rval;

	if(!outBuffer) return -1;
	rval = FreeOneBitStreamFrame(pVideoEnc, outBuffer);
//printf("avcEncQnqueueOutputBuffer: FreeOneBitStreamFrame=%d\n", rval);
	outBuffer = NULL;
	return 0;
}

