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
static int		pipe_fd;


void timerInit(int fd)
{
	memset(tused, 0, 16);
	pipe_fd = fd;
}

//void TimerHandler(int signo, siginfo_t *siginfo, void *context)
void TimerHandler(int signo, siginfo_t *si, void *uc)
{
//	timer_t *tid;
	unsigned char	buf[4];
	int		rval, ID;

//	tid = (void **)(si->si_value.sival_ptr);
	ID = signo - SIGRTMIN;
	buf[0] = ID;	
	rval = write(pipe_fd, buf, 1);
	if(rval < 0) perror("write");
}

int timerCreate(int ID)
{
	struct sigevent   *se, _se;
	int		rval;

	if(ID < 0) {
		for(ID = 0;ID < MAX_TMR_SZ;ID++)
			if(!tused[ID])  break;
		if(ID >= MAX_TMR_SZ) {
			printf("timerCreate: timers are full!\n");
			return -1;
		}
	} else if(ID >= MAX_TMR_SZ) {
		printf("timerCreate: Invalid timers ID!\n");
		return -1;
	} else if(tused[ID]) {
		timer_delete(tids[ID]);
		tused[ID] = 0;
	}
	se = &_se;
	se->sigev_notify = SIGEV_SIGNAL;
	se->sigev_signo = SIGRTMIN + ID;
	se->sigev_value.sival_ptr = &tids[ID];
	rval = timer_create(CLOCK_REALTIME, se, &tids[ID]);
	if(rval < 0) {
		perror("timerCreate: timer_create");
		return -1;
	}
	tused[ID] = 1;
	return ID;
}

void timerDelete(int ID)
{
	if(tused[ID]) {
		timer_delete(tids[ID]);
		tused[ID] = 0;
	}
}

int timerStart(int ID, int msec, int periodic)
{
	struct itimerspec *it, _it;
	struct sigaction  _sa;
	sigset_t	mask;
	long	val;
	int		rval;

	if(!tused[ID]) {
		printf("timerStart: timer(%d) isn't created!\n", ID);
		return -1;
	}
	// block timer signal temporarily
//	sigemptyset(&mask);
//	sigaddset(&mask, SIGRTMIN+ID);
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
	rval = timer_settime(tids[ID], 0, it, NULL);
	if(rval < 0) {
		perror("timerStart: timer_settime");
		return -1;
	}		
	//_sa.sa_handler = TimerHandler;
	_sa.sa_sigaction = TimerHandler;
	sigemptyset(&_sa.sa_mask);
	_sa.sa_flags = SA_SIGINFO;
	rval = sigaction(SIGRTMIN+ID, &_sa, 0);
	if(rval < 0) {
		perror("timerStart: sigaction");
		return -1;
	}	
	// unlock the timer signal, so that timer notification can be delivered
//	sigprocmask(SIG_SETMASK, &mask, NULL);

	return 0;
}

int timerStartEx(int ID, int usec, int periodic, void (*timerHandler)(int, siginfo_t *, void *))
{
	struct itimerspec *it, _it;
	struct sigaction  _sa;
	long	val;
	int		rval;

	if(!tused[ID]) {
		printf("timerStart: timer(%d) isn't created!\n", ID);
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
	rval = timer_settime(tids[ID], 0, it, NULL);
	if(rval < 0) {
		perror("timerStartEx: timer_settime");
		return -1;
	}		
	//_sa.sa_handler = TimerHandler;
	_sa.sa_sigaction = timerHandler;
	sigemptyset(&_sa.sa_mask);
	_sa.sa_flags = SA_SIGINFO;
	rval = sigaction(SIGRTMIN+ID, &_sa, 0);
	if(rval < 0) {
		perror("timerStartEx: sigaction");
		return -1;
	}
	return 0;
}

int timerStop(int ID)
{
	struct itimerspec *it, _it;
	struct sigaction  _sa;
	long	val;
	int		rval;

	if(!tused[ID]) {
		printf("timerStop: timer(%d) isn't created!\n", ID);
		return -1;
	}
	it = &_it;
	it->it_value.tv_sec  = 0;
	it->it_value.tv_nsec = 0;
	it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	rval = timer_settime(tids[ID], 0, it, NULL);
	if(rval < 0) {
		perror("timerStop: timer_settime");
		return -1;
	}		
	_sa.sa_handler = SIG_IGN;
	sigemptyset(&_sa.sa_mask);
	_sa.sa_flags = 0;
	rval = sigaction(SIGRTMIN+ID, &_sa, 0);
	if(rval < 0) {
		perror("timerStop: sigaction");
		return -1;
	}
	return 0;
}
