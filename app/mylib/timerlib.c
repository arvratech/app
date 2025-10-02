#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <linux/types.h>
#include <linux/input.h>
#include "rtc.h"
#include "timer.h"

#define MAX_TMR_SZ		8

static timer_t			tids[MAX_TMR_SZ];
static unsigned char	tused[MAX_TMR_SZ];
static void (*_TimerHandler)(int timerId);


void timerInit(void (*timerHandler)(int timerId))
{
	memset(tused, 0, 16);
	_TimerHandler = timerHandler;
}

//void TimerHandler(int signo, siginfo_t *siginfo, void *context)
void SigTimerHandler(int signo, siginfo_t *si, void *uc)
{
//	timer_t *tid;
	unsigned char	buf[4];
	int		rval, id;

//	tid = (void **)(si->si_value.sival_ptr);
	id = signo - SIGRTMIN;
	if(_TimerHandler) (*_TimerHandler)(id);
}

int timerCreate(int id)
{
	struct sigevent   *se, _se;
	struct sigaction  _sa;
	int		rval;

	if(id < 0) {
		for(id = 0;id < MAX_TMR_SZ;id++)
			if(!tused[id])  break;
		if(id >= MAX_TMR_SZ) {
			printf("timerCreate: timers are full!\n");
			return -1;
		}
	} else if(id >= MAX_TMR_SZ) {
		printf("timerCreate: Invalid timers id!\n");
		return -1;
	} else if(tused[id]) {
		timer_delete(tids[id]);
		tused[id] = 0;
	}
	se = &_se;
	se->sigev_notify = SIGEV_SIGNAL;
	se->sigev_signo = SIGRTMIN + id;
	se->sigev_value.sival_ptr = &tids[id];
	rval = timer_create(CLOCK_REALTIME, se, &tids[id]);
	if(rval < 0) {
		perror("timerCreate: timer_create");
		return -1;
	}
	//_sa.sa_handler = SigTimerHandler;
	_sa.sa_sigaction = SigTimerHandler;
	sigemptyset(&_sa.sa_mask);
	_sa.sa_flags = SA_SIGINFO;
	rval = sigaction(SIGRTMIN+id, &_sa, 0);
	if(rval < 0) {
		perror("timerCreate: sigaction");
		timer_delete(tids[id]);
		return -1;
	}	
	// unlock the timer signal, so that timer notification can be delivered
	tused[id] = 1;
	return id;
}

void timerDelete(int id)
{
	struct sigaction  _sa;
	int		rval;

	if(tused[id]) {
		_sa.sa_handler = SIG_IGN;
		sigemptyset(&_sa.sa_mask);
		_sa.sa_flags = 0;
		rval = sigaction(SIGRTMIN+id, &_sa, 0);
		if(rval < 0) {
			perror("timerDelete: sigaction");
		}
		timer_delete(tids[id]);
		tused[id] = 0;
	}
}

int timerStart(int id, int msec, int periodic)
{
	struct itimerspec *it, _it;
	sigset_t	mask;
	long	val;
	int		rval;

	if(!tused[id]) {
		printf("timerStart: timer(%d) isn't created!\n", id);
		return -1;
	}
	// block timer signal temporarily
//	sigemptyset(&mask);
//	sigaddset(&mask, SIGRTMIN+id);
//	sigprocmask(SIG_SETMASK, &mask, NULL);
	it = &_it;
	val = msec;
	if(val < 1000) it->it_value.tv_sec  = 0;
	else {
		it->it_value.tv_sec = val / 1000;
		val = val % 1000;
	}
	it->it_value.tv_nsec = val * 1000000;
	if(periodic) {
		it->it_interval.tv_sec = it->it_value.tv_sec; it->it_interval.tv_nsec = it->it_value.tv_nsec;
	} else {
		it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	}
	rval = timer_settime(tids[id], 0, it, NULL);
	if(rval < 0) {
		perror("timerStart: timer_settime");
		return -1;
	}		
//	sigprocmask(SIG_SETMASK, &mask, NULL);
	return 0;
}

int timerStartEx(int id, int usec, int periodic, void (*timerHandler)(int, siginfo_t *, void *))
{
	struct itimerspec *it, _it;
	long	val;
	int		rval;

	if(!tused[id]) {
		printf("timerStart: timer(%d) isn't created!\n", id);
		return -1;
	}
	it = &_it;
	val = usec;
	if(val < 1000000) it->it_value.tv_sec  = 0;
	else {
		it->it_value.tv_sec = val / 1000000;
		val = val % 1000000;
	}
	it->it_value.tv_nsec = val * 1000;
	if(periodic) {
		it->it_interval.tv_sec = it->it_value.tv_sec; it->it_interval.tv_nsec = it->it_value.tv_nsec;
	} else {
		it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	}
	rval = timer_settime(tids[id], 0, it, NULL);
	if(rval < 0) {
		perror("timerStartEx: timer_settime");
		return -1;
	}		
	return 0;
}

int timerStop(int id)
{
	struct itimerspec *it, _it;
	struct sigaction  _sa;
	long	val;
	int		rval;

	if(!tused[id]) {
		printf("timerStop: timer(%d) isn't created!\n", id);
		return -1;
	}
	it = &_it;
	it->it_value.tv_sec  = 0; it->it_value.tv_nsec = 0;
	it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	rval = timer_settime(tids[id], 0, it, NULL);
	if(rval < 0) {
		perror("timerStop: timer_settime");
		return -1;
	}		
	return 0;
}

