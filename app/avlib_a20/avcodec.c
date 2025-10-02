#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "rtc.h"
#include "avc.h"
#include "aac.h"
#include "wav.h"
#include "avcodec.h"

#define MAX_DEC_BUF_SZ		16384

unsigned char	decBuf[MAX_DEC_BUF_SZ];
int		rdIndex, wrIndex, wrCount;


int avCodecConfigure(AV_CODEC *codec, AV_STREAM_TRACK *track)
{
	AV_STREAM_TRACK	*trk;
	int		rval;

	trk = &codec->_track;
	memcpy(trk, track, sizeof(AV_STREAM_TRACK));
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			break;
		case AV_CODEC_AAC:
			break;
		case AV_CODEC_WAV:
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			break;
		case AV_CODEC_AAC:
			break;
		case AV_CODEC_WAV:
			break;
		}
	}
	return rval;
}

int avCodecStart(AV_CODEC *codec)
{
	int		rval;

	rval = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcEncStart(codec);
			break;
		case AV_CODEC_AAC:
			rval = aacEncStart(codec);
			break;
		case AV_CODEC_WAV:
			rval = wavEncStart(codec);
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcDecStart(codec);
			break;
		case AV_CODEC_AAC:
			rval = aacDecStart(codec);
			break;
		case AV_CODEC_WAV:
			rval = wavDecStart(codec);
			break;
		}
	}
	if(!rval) {
		codec->dFrameCount = codec->pFrameCount = 0;
		codec->sliceCount = 0;
		codec->sampleCount = 0;
		codec->eos = 0;
		rdIndex = wrIndex = wrCount = 0;
//printf("avCodecStart: %s\n", codec->codecName);
	}
	return rval;
}

void avCodecStop(AV_CODEC *codec)
{
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			avcEncStop(codec);
			break;
		case AV_CODEC_AAC:
			aacEncStop(codec);
			break;
		case AV_CODEC_WAV:
			wavEncStop(codec);
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			avcDecStop(codec);
			break;
		case AV_CODEC_AAC:
			aacDecStop(codec);
			break;
		case AV_CODEC_WAV:
			wavDecStop(codec);
			break;
		}
	}
}

AV_STREAM_TRACK *avCodecOutputTrack(AV_CODEC *codec)
{
	return &codec->_track;
}

extern unsigned char	codecBuf[];

int avCodecGetInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer)
{
	int		rval, len, size;

	rval = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			break;
		case AV_CODEC_AAC:
		case AV_CODEC_WAV:
			if(rdIndex >= wrIndex && !codec->eos) {
				size = codec->periodSize << 2;
				if(wrCount && wrIndex) memcpy(decBuf, decBuf+size*wrIndex, wrCount);
				rdIndex = 0; wrIndex = 0;
				buffer->data = codecBuf;
				rval = 0;
			}
			break;
		}
	}
	return rval;
}

int avCodecPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer)
{
	int		rval, len, size;

	rval = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcEncPutInputBuffer(codec, buffer);
			codec->pFrameCount++;
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcDecPutInputBuffer(codec, buffer);
			codec->dFrameCount++;
			if(buffer->flags & AV_FLAG_END_OF_STREAM) codec->eos = 1;
			break;
		case AV_CODEC_AAC:
		case AV_CODEC_WAV:
			size = codec->periodSize << 2;
			if(wrCount && wrIndex) memcpy(decBuf, decBuf+size*wrIndex, wrCount);
			len = wrCount;
			if(buffer->size > 0) {
				if(codec->codecId == AV_CODEC_AAC) rval = aacDecDecodeInputBuffer(codec, buffer, decBuf+wrCount);
				else	rval = wavDecDecodeInputBuffer(codec, buffer, decBuf+wrCount);
				len += rval;
			}
			rdIndex = 0; wrIndex = len / size; wrCount = len % size;
			if(buffer->flags & AV_FLAG_END_OF_STREAM) {
				if(wrCount) {				// |############|########0000|
					memset(decBuf+size*wrIndex+wrCount, 0, size-wrCount);
					wrIndex++;
				} else if(wrIndex > 0) {	// |############|############|
					wrCount = size;
				} else {					// |            |
					wrIndex = 1;
				}
				codec->eos = 1;
			}
			rval = 0;
			break;
		}
	}
	return rval;
}

int avCodecGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer)
{
	int		rval, size;

	rval = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcEncGetOutputBuffer(codec, index, buffer);
			if(rval == 0) {
				codec->dFrameCount++;
			}
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			if(codec->eos >= 2) {
				if(codec->eos >= 2) {
					buffer->size = 0; buffer->flags = AV_FLAG_END_OF_STREAM;
					rval = 0;
				}
			} else {
				rval = avcDecGetOutputBuffer(codec, index, buffer);
				if(rval == 0) {
					buffer->flags = 0; codec->pFrameCount++;
				}
			}
			break;
		case AV_CODEC_AAC:
		case AV_CODEC_WAV:
			if(rdIndex < wrIndex) {
				index = rdIndex;
				size = codec->periodSize << 2;
				buffer->data = decBuf + size * rdIndex;
				if(codec->eos && rdIndex == wrIndex-1) {
					buffer->size = wrCount; buffer->flags = AV_FLAG_END_OF_STREAM;
				} else {
					buffer->size = size; buffer->flags = 0;
				}
				rval = 0;
			}
			break;
		}
	}
	return rval;
}

int avCodecDequeueOutputBuffer(AV_CODEC *codec)
{
	int		index;

	index = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			index = avcEncDequeueOutputBuffer(codec);
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			index = avcDecDequeueOutputBuffer(codec);
			if(index < 0 && codec->eos == 1) {
				codec->eos = 2; index = 0;
			}
			break;
		case AV_CODEC_AAC:
		case AV_CODEC_WAV:
			if(rdIndex < wrIndex) index = rdIndex;
			break;
		}
	}
	return index;
}

int avCodecPutOutputBuffer(AV_CODEC *codec, int index)
{
	int		rval;

	rval = -1;
	if(codec->encoder) {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			rval = avcEncEnqueueOutputBuffer(codec, index);
			break;
		}
	} else {
		switch(codec->codecId) {
		case AV_CODEC_AVC:
			if(codec->eos >= 2) {
				if(codec->eos == 2) {
					codec->eos = 3; rval = 0;
				}
			} else {
				rval = avcDecEnqueueOutputBuffer(codec, index);
			}
			break;
		case AV_CODEC_AAC:
		case AV_CODEC_WAV:
			if(rdIndex < wrIndex) {
				rdIndex++; rval = 0;
			}
			break;
		}
	}
	return rval;
}

void avCodecGetPresentTimestamp(AV_CODEC *codec, unsigned long long *tval)
{
	AV_STREAM_TRACK	*trk;
	unsigned long long val;

	trk = avCodecOutputTrack(codec);
	if(codec->codecId == AV_CODEC_AVC) {
		//val = codec->frameCount * fmt->frameRateDen * 1000000; ==> right int32 caclulation	
		val = codec->pFrameCount;
		val = val * trk->frameRateDen * 1000000 / trk->frameRateNum;
	} else {
		val = codec->sampleCount - avAudioOutputUsedSize();
		val = val * 1000000 / trk->sampleRate;
	}
	*tval = val;
}

