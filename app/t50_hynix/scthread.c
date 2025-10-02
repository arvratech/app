#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
////////////////////////////////
#include <pthread.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "msg.h"
#include "dev.h"
#include "unit.h"
#include "subdev.h"
#include "bio.h"
#include "ad.h"

static pthread_t		_thread;


void *sc_thread_func(void *arg);


void scThreadCreate()
{
	pthread_create(&_thread, NULL, sc_thread_func, (void *)0);
}


void *sc_thread_func(void *arg)
{
printf("### sc_thread ###\n");
	while(1) {
		scTask();
		usleep(1000);
	}
	return  (void *)0;	
}
