#ifndef _WAVE_H_
#define _WAVE_H_


#include "avcodec.h"
#include "avplayer.h"


int  wavProbe(unsigned char *buf);
int  wavReadHeader(AV_PLAYER *player);
int  waveReadData(AV_PLAYER *player, unsigned char *buf, int size);

int  wavDecOpen(AV_CODEC *codec);
void wavDecClose(AV_CODEC *codec);
int  wavDecStart(AV_CODEC *codec);
void wavDecStop(AV_CODEC *codec);
int  wavDecDecodeInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer, unsigned char *outBuf);

int  wavEncOpen(AV_CODEC *codec);
void wavEncClose(AV_CODEC *codec);
int  wavEncStart(AV_CODEC *codec);
void wavEncStop(AV_CODEC *codec);


#endif

