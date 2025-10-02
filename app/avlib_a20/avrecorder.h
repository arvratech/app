#ifndef _AV_RECORDER_H_
#define _AV_RECORDER_H_

#include "avstream.h"
#include <time.h>


typedef struct _AV_RECORDER {
	AV_STREAM		_stream;
	int				fd;
	unsigned char	state;
	unsigned char	frameStarted;
	unsigned char	reserve[2];
	void			*bio;
	void			*priv_data;
	int64_t			startPts;
	int64_t			startDts;
	time_t			timestamp;
} AV_RECORDER;


int  avREcorderOpen(AV_RECORDER *recorder, char *fileName);
void avRecorderClose(AV_RECORDER *recorder);
int  avRecorderStart(AV_RECORDER *recorder);
void avRecorderStop(AV_RECORDER *recorder);
int  avRecorderState(AV_RECORDER *recorder);


#endif

