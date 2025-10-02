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

static faacDecHandle faac_hdec;


int aacDecOpen(AV_CODEC *codec)
{
	int		rval;

	faac_hdec = faacDecOpen();
	if(faac_hdec) {
		codec->encoder = 0;	
		codec->codecId = AV_CODEC_AAC;
		strcpy(codec->codecName, "AAC");
		codec->state = 1;	
		rval = 0;
	} else {
		rval = 1;
		codec->state = 0;	
	}
	return  rval;
}

void aacDecClose(AV_CODEC *codec)
{
	if(codec->state > 1) aacDecStop(codec);
	if(codec->state == 1) {
		//faacDecClose(faac_hdec);
		faac_hdec = NULL;
		codec->state = 0;	
	}
}

static faacDecFrameInfo faac_finfo;

int aacDecStart(AV_CODEC *codec)
{
	AV_STREAM_TRACK	*trk;
	unsigned long faac_samplerate;
	unsigned char faac_channels;
	int		rval;

	trk = avCodecOutputTrack(codec);
printf("aacDecStart: extradata=%d\n", trk->extradataLen);
	if(trk->extradataLen > 0) {	// We have esds(Elementary Stream Description)
		faacDecConfigurationPtr faac_conf = faacDecGetCurrentConfiguration(faac_hdec);
		//if(audio_output_channels <= 2) {
		//	faac_conf->downMatrix = 1;
		//	faacDecSetConfiguration(faac_hdec, faac_conf);
		//}
		rval = faacDecInit2(faac_hdec, trk->extradata, trk->extradataLen,
										&faac_samplerate, &faac_channels);
	} else {
		rval = -1;
	}
	if(rval < 0) {
		printf("aacDecStart: FAAD Failed to initialize the decoder!\n");
	} else {
		// 8 channels is aac channel order #7.
		trk->channels		= faac_channels;
		trk->sampleRate		= faac_samplerate;
		trk->bitsPerSample	= 16;
printf("aacDecStart: ch=%d sr=%d\n", trk->channels, trk->sampleRate);
		codec->state = 2;	
	}
	return rval;
}

void aacDecStop(AV_CODEC *codec)
{
	codec->state = 1;
}

static int	prevOutSize	= 4096;
static unsigned char	tmpBuf[16384];

// return decoded bytes number
int aacDecDecodeInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer, unsigned char *outBuf)
{
	AV_STREAM_TRACK	*trk;
	unsigned char	*p;
	void	*sample;
	int		len;

	trk = avCodecOutputTrack(codec);
	if(trk->channels < 2) p = tmpBuf;
	else	p = outBuf;
	sample = faacDecDecode2(faac_hdec, &faac_finfo, buffer->data, buffer->size, (void **)&p, 12284);
//printf("FAAD: inLen=%d outLen=%d %d\n", buffer->size, faac_finfo.bytesconsumed, faac_finfo.samples);
//for (j=0;j<faac_finfo.channels;j++) printf("%d:%d\n", j, faac_finfo.channel_position[j]);
	if(faac_finfo.error > 0) {
		printf("%lu FAAD: Failed to decode frame: %d/%d status=%d %s\n", MS_TIMER,
					buffer->frameCount, buffer->frameSize, buffer->status,
					faacDecGetErrorMessage(faac_finfo.error));
		len = prevOutSize;
		memset(outBuf, 0, len);
	} else if(faac_finfo.samples == 0) {
		printf("%lu FAAD: Decoded zero samples: %d/%d status=%d\n", MS_TIMER,
					buffer->frameCount, buffer->frameSize, buffer->status);
		len = prevOutSize;
		memset(outBuf, 0, len);
	} else {
		// XXX: samples already multiplied by channels!
		len = faac_finfo.samples << 1;	// x (bitsPerSample/8)
//printf("FAAD: Successfully decoded frame (%ld Bytes)!\n", len);
//		ad->pts_bytes += len;
		if(trk->channels < 2) {
			pcmSingle2Stereo(p, len, outBuf);
			len <<= 1;
		}
		prevOutSize = len;
	}
	return len;
}

