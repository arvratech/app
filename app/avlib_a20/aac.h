#ifndef _AAC_H_
#define _AAC_H_

#include "avcodec.h"


int  aacDecOpen(AV_CODEC *codec);
void aacDecClose(AV_CODEC *codec);
int  aacDecStart(AV_CODEC *codec);
void aacDecStop(AV_CODEC *codec);
int  aacDecDecodeInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer, unsigned char *outBuf);

int  aacEncOpen(AV_CODEC *codec);
void aacEncClose(AV_CODEC *codec);
int  aacEncStart(AV_CODEC *codec);
void aacEncStop(AV_CODEC *codec);


#endif

