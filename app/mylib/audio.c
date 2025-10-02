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


extern AV_CONTEXT	*av;
static pthread_mutex_t	_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t	_cond  = PTHREAD_COND_INITIALIZER;
static pthread_t		pthread;
static pthread_attr_t	pt_attr;
//static pthread_t	_thread;
//static int		threadid;
static unsigned char	cbuf[8+32];
static unsigned char	stopReq;


static int _GetAudioMessage(unsigned char *msg)
{
	int		rval;

	rval = pthread_mutex_lock(&_mutex);
	while(cbuf_empty(cbuf)) {
//printf("%ld [Thread-%d sleep]\n", GetCurrentTime(), threadid);
		rval = pthread_cond_wait(&_cond, &_mutex);
//printf("%ld [Thread-%d wakeup]\n", GetCurrentTime(), threadid);
	}
	rval = cbuf_get(cbuf, msg, 3);
	pthread_mutex_unlock(&_mutex);
//printf("%ld [Thread-%d handled=%d]\n", GetCurrentTime(), threadid, rval);
	if(rval != 3) return -1;
	return 1;
}

static void _DispatchAudioMessage(unsigned char *msg)
{
	char	fileName[64];

	sprintf(fileName, "ment/%d.wav", (int)msg[0]);
	adSetVolume(av->ad, (int)msg[1]);
	avPlayWave(av, fileName, audioGet);
	if(msg[2] && !stopReq) audioPost(msg);
	stopReq = 0;
}

void *AudioMain(void *arg)
{
	unsigned char	msg[8];
	int		rval;

	while(1) {
		rval = _GetAudioMessage(msg);
		if(rval > 0) _DispatchAudioMessage(msg);
	}
	return arg;
}

void audioInit(void)
{
printf("audioInit...\n");
	cbuf_init(cbuf, 31);
//	pthread_create(&_thread, &attr, WinMain, &thread_id);
	pthread_attr_init(&pt_attr);
	//pthread_attr_setstacksize(&attr, stack_size);
	pthread_create(&pthread, &pt_attr, AudioMain, NULL);
	stopReq = 0;
}

void audioExit(void)
{
//	pthread_attr_destroy(&pt_attr);
//	pthread_join(pt_tinfo.thread_id, &res);
}

int audioPost(unsigned char *msg)
{
	int		rval;

	pthread_mutex_lock(&_mutex);
	rval = cbuf_free(cbuf);
	if(rval < 3) rval = -1;
	else {
		cbuf_put(cbuf, msg, 3);
		pthread_cond_signal(&_cond);
		rval = 1;
	}
	pthread_mutex_unlock(&_mutex);
	stopReq = 0;
	return rval;
}

void audioFlush(void)
{
//printf("audioFlush...\n");
	stopReq = 1;
	adStop(ad);
	pthread_mutex_lock(&_mutex);
	cbuf_flush(cbuf);
	pthread_mutex_unlock(&_mutex);
}

