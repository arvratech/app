#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <alsa/asoundlib.h>
#include "rtc.h"
#include "faad.h"
#include "avstream.h"
#include "aac.h"


int aacEncOpen(AV_CODEC *codec)
{
	codec->encoder = 1;	
	codec->codecId = AV_CODEC_AAC;
	strcpy(codec->codecName, "AAC");
	codec->state = 1;	
	return  0;
}

void aacEncClose(AV_CODEC *codec)
{
	if(codec->state > 1) aacEncStop(codec);
	codec->state = 0;	
}

int aacEncStart(AV_CODEC *codec)
{
	codec->state = 2;	
	return 0;
}

void aacEncStop(AV_CODEC *codec)
{
	codec->state = 1;	
}

