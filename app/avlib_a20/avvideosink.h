#ifndef _AV_VIDEO_SINK_H
#define _AV_VIDEO_SINK_H

#include "cgrect.h"
#include "avstream.h"
#include "avcodec.h"


typedef struct _AV_VIDEO_SINK {
	AV_STREAM		_stream;
	unsigned char	state;
	unsigned char	reserve[3];
	int64_t			startTimer;
	int				layerHandle;
	CGRect			_screen;
	AV_CODEC		*codec;
} AV_VIDEO_SINK;


int  avVideoSinkOpen(AV_VIDEO_SINK *videoSink);
void avVideoSinkClose(AV_VIDEO_SINK *videoSink);
int  avVideoSinkStart(AV_VIDEO_SINK *videoSink);
void avVideoSinkStop(AV_VIDEO_SINK *videoSink);
void avVideoSinkSetScreen(AV_VIDEO_SINK *videoSink, CGRect *screen);
int  avVideoSinkState(AV_VIDEO_SINK *videoSink);


#endif

