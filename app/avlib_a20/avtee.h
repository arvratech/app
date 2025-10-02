#ifndef _AV_TEE_H_
#define _AV_TEE_H_

#include "avstream.h"


typedef struct _AV_TEE {
	AV_STREAM		_stream;
	unsigned char	state;
	unsigned char	reserve[3];
} AV_TEE;


int  avTeeOpen(AV_TEE *tee);
void avTeeClose(AV_TEE *tee);
AV_PORT *avTeeSinkPort(AV_TEE *tee);
AV_PORT *avTeeSrcPort(AV_TEE *tee, int index);
int  avTeeConnect(AV_TEE *tee, int index, AV_PORT *sink);
int  avTeeDisconnect(AV_TEE *tee, int index, AV_PORT *sink);
int  avTeeStart(AV_TEE *tee);
void avTeeStop(AV_TEE *tee);
int  avTeeState(AV_TEE *tee);


#endif

