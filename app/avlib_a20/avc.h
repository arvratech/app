#ifndef _AVC_H_
#define _AVC_H_

#include "avcodec.h"


int  avcDecOpen(AV_CODEC *codec);
void avcDecClose(AV_CODEC *codec);
int  avcDecStart(AV_CODEC *codec);
void avcDecStop(AV_CODEC *codec);
int  avcDecPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer);
int  avcDecGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer);
int  avcDecDequeueOutputBuffer(AV_CODEC *codec);
int  avcDecEnqueueOutputBuffer(AV_CODEC *codec, int index);

int  avcEncOpen(AV_CODEC *codec);
void avcEncClose(AV_CODEC *codec);
int  avcEncStart(AV_CODEC *codec);
void avcEncStop(AV_CODEC *codec);
int  avcEncPutInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer);
int  avcEncGetOutputBuffer(AV_CODEC *codec, int index, AV_BUFFER *buffer);
int  avcEncDequeueOutputBuffer(AV_CODEC *codec);
int  avcEncEnqueueOutputBuffer(AV_CODEC *codec, int index);

#endif

