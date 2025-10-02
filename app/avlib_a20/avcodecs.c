#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "rtc.h"
#include "wav.h"
#include "avc.h"
#include "aac.h"
#include "avcodec.h"

#define MAX_CODEC_SZ		3

AV_CODEC	decCodecs[MAX_CODEC_SZ];
AV_CODEC	encCodecs[MAX_CODEC_SZ];


int avCodecsInit(void)
{
	AV_CODEC	*codec;
	int		rval;

	codec = &decCodecs[0];
	rval = wavDecOpen(codec);
	codec = &decCodecs[1];
	rval = avcDecOpen(codec);
	codec = &decCodecs[2];
	rval = aacDecOpen(codec);

	codec = &encCodecs[0];
	rval = wavEncOpen(codec);
	codec = &encCodecs[1];
	rval = avcEncOpen(codec);
	codec = &encCodecs[2];
	rval = aacEncOpen(codec);
	return 0;
}

void avCodecsExit(void)
{
	AV_CODEC	*codec;
	int		rval;

	codec = &decCodecs[0];
	wavDecClose(codec);
	codec = &decCodecs[1];
	avcDecClose(codec);
	codec = &decCodecs[2];
	aacDecClose(codec);

	codec = &encCodecs[0];
	wavEncClose(codec);
	codec = &encCodecs[1];
	avcEncClose(codec);
	codec = &encCodecs[2];
	aacEncClose(codec);
}

AV_CODEC *avCodecsFindDecoder(int codecId)
{
	AV_CODEC	*codec;
	int		i;
	
	for(i = 0, codec = decCodecs;i < MAX_CODEC_SZ;i++, codec++)
		if(codec->codecId == codecId) break;
	if(i >= MAX_CODEC_SZ) codec = NULL;
	return  codec;
}

AV_CODEC *avCodecsFindEncoder(int codecId)
{
	AV_CODEC	*codec;
	int		i;
	
	for(i = 0, codec = encCodecs;i < MAX_CODEC_SZ;i++, codec++)
		if(codec->codecId == codecId) break;
	if(i >= MAX_CODEC_SZ) codec = NULL;
	return  codec;
}

