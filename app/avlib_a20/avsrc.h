#ifndef _AV_SRC_H_
#define _AV_SRC_H_

#include "cgrect.h"
#include "avstream.h"
#include "avcodec.h"


typedef struct _AV_SOURCE {
	AV_STREAM		_stream;
	unsigned char	state;
	unsigned char	started;
	unsigned char	reserve[2];
	AV_CODEC		*codec;
	unsigned long long startTimer;
	CGRect			_preview;
	int				previewState;
	int				layerHandle;
} AV_SOURCE;


int  avSourceOpen(AV_SOURCE *src);
void avSourceClose(AV_SOURCE *src);
int  avSourceStart(AV_SOURCE *src);
void avSourceStop(AV_SOURCE *src);
void avSourceStartPreview(AV_SOURCE *src, CGRect *preview);
void avSourceStopPreview(AV_SOURCE *src);
int  avSourceConnect(AV_SOURCE *src, void *sink, int mediaType);
int  avSourceDisconnect(AV_SOURCE *src, void *sink, int mediaType);
int  avSourceState(AV_SOURCE *src);
void avSourceSaveJpeg(char *fileName);


#endif

