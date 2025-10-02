#ifndef _AV_CODEC_H_
#define _AV_CODEC_H_

#include "avstream.h"


#define AV_CODEC_DECODER	0
#define AV_CODEC_ENCODER	1

#define AV_CODEC_WAV		1
#define AV_CODEC_AVC		2
#define AV_CODEC_AAC		3


typedef struct _AV_CODEC {
	unsigned char	encoder;
	unsigned char	state;
	unsigned char	eos;
	unsigned char	reserve[1];	
	AV_STREAM_TRACK	_track;	
	int				codecId;
	char			codecName[32];
	int				periodSize;
	int				dFrameCount;
	int				pFrameCount;
	int				sliceCount;
	int				sampleCount;
} AV_CODEC;


int  avCodecOpen(AV_CODEC *codec, int encoder, int codecId);
void avCodecClose(AV_CODEC *codec);
int  avCodecConfigure(AV_CODEC *codec, AV_STREAM_TRACK *track);
int  avCodecStart(AV_CODEC *codec);
void avCodecStop(AV_CODEC *codec);
AV_STREAM_TRACK *avCodecOutputTrack(AV_CODEC *codec);
int  avCodecGetInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer);
int  avCodecPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer);
int  avCodecGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer);
int  avCodecPutOutputBuffer(AV_CODEC *codec, int index);
void avCodecGetPresentTimestamp(AV_CODEC *codec, unsigned long long *tval);


#endif

