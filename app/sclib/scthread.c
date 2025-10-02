#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "sctask.h"
#include "cbuf.h"

#ifdef _A20
extern unsigned char	cbufLcd[];
#endif

static pthread_t	_thread;


void lcdDoOperation(unsigned char *buf);

void *sc_thread_func(void *arg)
{
	unsigned char	buf[4];
	int		rval;

	while(1) {
		scTask();
		usleep(1000);
#ifdef _A20
		rval = cbufGet(cbufLcd, buf, 2);
		if(rval == 2) lcdDoOperation(buf);
#endif
	}
	return (void *)0;	
}

void scThreadCreate()
{
	pthread_create(&_thread, NULL, sc_thread_func, (void *)0);
}

