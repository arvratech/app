#ifndef _AV_CODECS_H_
#define _AV_CODECS_H_


#include "avcodec.h"


int avCodecsInit(void);
void avCodecsExit(void);
AV_CODEC *avCodecsFindDecoder(int codecId);
AV_CODEC *avCodecsFindEncoder(int codecId);


#endif

