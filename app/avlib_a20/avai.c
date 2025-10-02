#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include "avdevice.h"

extern char		*card;

static snd_pcm_t			*pcm;
static snd_pcm_hw_params_t	*hw_params;
static snd_pcm_sw_params_t	*sw_params;
static snd_pcm_uframes_t	bufferSize;
static snd_pcm_uframes_t	periodSize;
static int		sampleBytes;


int avAudioInputOpen(void)
{
	int		rval;

	rval = snd_pcm_open(&pcm, card, SND_PCM_STREAM_CAPTURE, 0);
	if(rval < 0) {
		printf("snd_pcm_open() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_hw_params_malloc(&hw_params);
	if(rval < 0) {
		printf("snd_pcm_hw_params_malloc() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_sw_params_malloc(&sw_params);
	if(rval < 0) {
		printf("snd_pcm_sw_params_malloc() error: %s\n", snd_strerror(rval));
		return -1;
	}
//printf("avAudioInputOpen: OK\n");
	return 0;
}

void avAudioInputClose(void)
{
	snd_pcm_sw_params_free(sw_params);
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_close(pcm);
//printf("avAudioInputClose\n");
}

void audio_input_set(snd_pcm_t *pcm, snd_pcm_hw_params_t *hw_params, snd_pcm_sw_params_t *sw_params);

void avAudioInputSet(void)
{
	audio_input_set(pcm, hw_params, sw_params);
}

int avAudioInputStart(AV_STREAM_TRACK *track)
{
	return 0;
}

void avAudioInputStop(void)
{
}

