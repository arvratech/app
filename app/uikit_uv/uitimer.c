#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rtc.h"
#include "gactivity.h"
#include "gapp.h"
#include "uv.h"
#include "UITimer.h"


#define MAX_TIMER_SZ	8

static uv_timer_t	timers[MAX_TIMER_SZ];
static int			tmdata[MAX_TIMER_SZ];


static void _OnUiTimer(uv_timer_t *handle)
{
	GACTIVITY	*act;
	int		id;

	act = appCurrentActivity();
	id = *(int *)handle->data;
	if(act->onTimer) {
		(*act->onTimer)(act, id);
//		appDrawView();
	}
}

void initUITimers(void *loop)
{
	uv_timer_t	*timer;
	int		i;
	
	for(i = 0, timer = timers;i < MAX_TIMER_SZ;i++, timer++) {
		uv_timer_init((uv_loop_t *)loop, timer);
		tmdata[i] = i;
		timer->data = &tmdata[i];
	}
}

int UItimerStart(int id, int msec, int repeatflag)
{
	uint64_t	timeout, repeat;

	if(repeatflag) {
		timeout = 0; repeat = msec;
	} else {	
		timeout = msec; repeat = 0;
	}
	uv_timer_start(&timers[id], _OnUiTimer, timeout, repeat);
}

int UItimerStop(int id)
{
	uv_timer_stop(&timers[id]);
}

int UItimerActive(int id)
{
	if(uv_is_active((uv_handle_t *)&timers[id])) return 1;
	else	return 0;
}

