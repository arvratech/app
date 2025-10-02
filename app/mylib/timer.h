#ifndef _TIMER_H_
#define _TIMER_H_

#include <signal.h>


void timerInit(void (*timerHandler)(int timerId));
int  timerCreate(int id);
void timerDelete(int id);
int  timerStart(int id, int msec, int periodic);
int  timerStartEx(int id, int usec, int periodic, void (*timerHandler)(int, siginfo_t *, void *));
int  timerStop(int id);


#endif

