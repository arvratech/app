#ifndef _AV_SINK_H_
#define _AV_SINK_H_

#include "avstream.h"
#include "avcodec.h"


typedef struct _AV_SINK {
	AV_STREAM		_stream;
	unsigned char	state;
	unsigned char	audioState;
	unsigned char	videoState;
	unsigned char	volume;
	AV_CODEC		*audioCodec;
	AV_CODEC		*videoCodec;
	unsigned long long	startTimer;
	int				layerHandle;
	CGRect			_screen;
} AV_SINK;


int  avSinkOpen(AV_SINK *sink);
void avSinkClose(AV_SINK *sink);
int  avSinkStart(AV_SINK *sink);
void avSinkStop(AV_SINK *sink);
void avSinkSetVolume(AV_SINK *sink, int volume);
void avSinkSetScreen(AV_SINK *sink, CGRect *screen);
int  avSinkState(AV_SINK *sink);


#endif

