#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "rtc.h"
#include "cbuf.h"
#include "lang.h"
#include "msg.h"
#include "lcdc.h"
#include "av.h"
#include "audio.h"

#ifdef _A20
#else
extern AV_CONTEXT	*av;
#endif

static unsigned char	cbuf[8+32];
static int		_audioAlarm;


void audioInit(void)
{
	cbufInit(cbuf, 31);
	_audioAlarm = 0;
}

void audioExit(void)
{
	audioFlush();
}

#ifdef _A20
static void _OnAudioStopped(AV_PLAYER *player)
#else
static void _OnAudioStopped(AV_CONTEXT *av)
#endif
{
	char	fileName[64];
	unsigned char	msg[4];
	int		rval, no;

	rval = cbufGet(cbuf, msg, 3);
//printf("_OnAudipStopped: %d\n", rval);
	if(rval == 3) {
		no = msg[0];
		if(no > 250) sprintf(fileName, "ment/100_%d.wav", no-250);
		else	sprintf(fileName, "ment/%d.wav", no);
#ifdef _A20
		AvStoppedPlayWave();
		AvPlayWave(fileName, (int)msg[1], _OnAudioStopped);
#else
		adSetVolume(av->ad, (int)msg[1]);
		avPlayWave(av, fileName, _OnAudioStopped);
#endif
		if(msg[2]) audioPost(msg);
	} else {
		//adSetVolume(av->ad, 0);
#ifdef _A20
		AvStoppedPlayWave();		// bug fix at 2020.9.14
#endif
		_audioAlarm = 0;
	}
}

int audioPost(unsigned char *msg)
{
	char	fileName[64];
	int		rval, no, state;

	rval = cbufFree(cbuf);
	if(rval < 3) rval = -1;
	else {
#ifdef _A20
		state = AvPlayState();
		if(state > AV_STATE_IDLE) {
#else
		state = adState(av->ad);
		if(state > AO_STATE_IDLE) {
#endif
			cbufPut(cbuf, msg, 3);
		} else {
			no = msg[0];
			if(no > 250) sprintf(fileName, "ment/100_%d.wav", no-250);
			else	sprintf(fileName, "ment/%d.wav", no);
#ifdef _A20
			AvPlayWave(fileName, (int)msg[1], _OnAudioStopped);
#else
			adSetVolume(av->ad, (int)msg[1]);
			avPlayWave(av, fileName, _OnAudioStopped);
#endif
			if(msg[2]) audioPost(msg);
		}
		rval = 1;
	}
	return rval;
}

void audioContinue(void)
{
	_OnAudioStopped(NULL);
}

void audioFlush(void)
{
//printf("audioFlush...\n");
#ifdef _A20
	if(AvIsPlayingWave()) AvStopPlayWave();
#else
	avStopPlay(av);
#endif
	//avPlaySetVolume(play, 0);
	cbufFlush(cbuf);
	_audioAlarm = 0;
}

int audioActive(void)
{
	int		state;

#ifdef _A20
	state = AvPlayState();
	if(state > AV_STATE_IDLE) return 1;
#else
	state = adState(av->ad);
	if(state > AO_STATE_IDLE) return 1;
#endif
	else	return 0;
}

void audioSetAlarm(void)
{
	_audioAlarm = 1;
}

void audioFlushAlarm(void)
{
	if(_audioAlarm) audioFlush();
}

