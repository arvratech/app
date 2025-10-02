#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <errno.h>
#include "../mylib/rtc.h"
#include "avdevice.h"

extern char		*card;

static snd_pcm_t			*pcm;
static snd_pcm_hw_params_t	*hw_params;
static snd_pcm_sw_params_t	*sw_params;
static snd_pcm_uframes_t	bufferSize;
static snd_pcm_uframes_t	periodSize;
static int		sampleBytes;


int avAudioOutputOpen(void)
{
	int		rval;

//	rval = snd_pcm_open(&pcm, card, SND_PCM_STREAM_PLAYBACK | SND_PCM_NONBLOCK, 0);
	rval = snd_pcm_open(&pcm, card, SND_PCM_STREAM_PLAYBACK, 0);
	if(rval < 0) {
		printf("snd_pcm_open() error: %s\n", snd_strerror(rval));
		return -1;
	}
	snd_pcm_nonblock(pcm, 1);
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
//printf("avAudioOutputOpen: OK\n");
	return 0;
}

void avAudioOutputClose(void)
{
	snd_pcm_sw_params_free(sw_params);
	snd_pcm_hw_params_free(hw_params);
	snd_pcm_close(pcm);
//printf("avAudioOutputClose\n");
}

void audio_output_set(snd_pcm_t *pcm, snd_pcm_hw_params_t *hw_params, snd_pcm_sw_params_t *sw_params);

void avAudioOutputSet(void)
{
	audio_output_set(pcm, hw_params, sw_params);
}

int avAudioOutputStart(AV_STREAM_TRACK *track)
{
	int		rval, rate;

    rval = snd_pcm_hw_params_any(pcm, hw_params);
	if(rval < 0) {
		printf("snd_pcm_hw_params_any() error: %s\n", snd_strerror(rval));
		return -1;
	}
    rval = snd_pcm_hw_params_set_access(pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED);
	if(rval < 0) {
		printf("snd_pcm_hw_params_set_access() error: %s\n", snd_strerror(rval));
		return -1;
	}
    rval = snd_pcm_hw_params_set_format(pcm, hw_params, SND_PCM_FORMAT_S16_LE);
	rate = track->sampleRate;
	rval = snd_pcm_hw_params_set_rate_near(pcm, hw_params, &rate, 0);
	rval = snd_pcm_hw_params_set_channels(pcm, hw_params, 2);
	bufferSize = 8192; periodSize = 1024;
	snd_pcm_hw_params_set_buffer_size_near(pcm, hw_params, &bufferSize);
	snd_pcm_hw_params_set_period_size_near(pcm, hw_params, &periodSize, NULL);
	// bufferSize=8192 periodSize=1024
	// buffer(bufferSize=8192) = 8 * fragment(periodSize=1024)
//printf("avAudioOutputStart: bufferSize=%d periodSize=%d\n", bufferSize, periodSize);
	rval = snd_pcm_hw_params(pcm, hw_params);
	if(rval < 0) {
		printf("snd_pcm_hw_params() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_sw_params_current(pcm, sw_params);
	if(rval < 0) {
		printf("snd_pcm_sw_params_current() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_sw_params_set_avail_min(pcm, sw_params, periodSize);
	if(rval < 0) {
		printf("snd_pcm_sw_params_set_avaliable() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_sw_params_set_start_threshold(pcm, sw_params, bufferSize-periodSize);
	if(rval < 0) {
		printf("snd_pcm_sw_params_set_start_threshold() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_pcm_sw_params(pcm, sw_params);
	if(rval < 0) {
		printf("snd_pcm_sw_params() error: %s\n", snd_strerror(rval));
		return -1;
	}
    rval = snd_pcm_prepare(pcm);
	if(rval < 0) {
		printf("snd_pcm_prepare() error: %s\n", snd_strerror(rval));
		return -1;
	}
	sampleBytes = 0;
//printf("avAudioOutputStart...\n");
	return 0;
}

void avAudioOutputStop(void)
{
	int		state;

	state = snd_pcm_state(pcm);
//printf("avAudioOutputStop...%d\n", state);
	if(state > SND_PCM_STATE_SETUP) {
		snd_pcm_drop(pcm);
	}
}

int avAudioOutputPeriodSize(void)
{
	return periodSize;
}

int avAudioOutputBufferSize(void)
{
	return bufferSize;
}

int avAudioOutputFreeSize(void)
{
	return snd_pcm_avail_update(pcm);
}

int avAudioOutputUsedSize(void)
{
	return bufferSize - snd_pcm_avail_update(pcm);
}

// -1: error
//  0:Success
//  1:buffer full - EAGAIN
//  2:underrun - EPIPE
int avAudioOutputWrite(void *buf)
{
	int		rval;

	rval = snd_pcm_writei(pcm, buf, periodSize);
	if(rval < 0) {
		printf("avAudioOutputWrite: %s\n", snd_strerror(errno));
		if(errno == EAGAIN) rval = 1;
		else if(errno == EPIPE) rval = 2;
	} else {
		rval = 0;
	}
	//sampleBytes += periodSize;
	//state = snd_pcm_state(pcm);
	//if(state != SND_PCM_STATE_RUNNING) snd_pcm_start(pcm);
	return rval;
}

void avAudioOutputRecover(void)
{
	int		state;

	state = snd_pcm_state(pcm);
	if(state == SND_PCM_STATE_XRUN) {
printf("%lu ### underrun recover ###\n", MS_TIMER);
		snd_pcm_prepare(pcm);
	}
}

void avAudioOutputStopped(void)
{
	int		rval;

	rval = snd_pcm_drain(pcm);
//if(rval < 0) {
//	printf("snd_pcm_drain() error: %s\n", snd_strerror(rval));
//}
	//snd_pcm_nonblock(pcm, 0);
}

int avAudioOutputIsStopped(void)
{
	int		rval, state;

	state = snd_pcm_state(pcm);
//printf("onAdPlay: done state=%d\n", state);
	if(state <= SND_PCM_STATE_SETUP) rval = 1;
	else	rval = 0;
	return rval;
}

