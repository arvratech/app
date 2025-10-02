#ifndef _UI_TIMER_H_
#define _UI_TIMER_H_


void initUITimers(void *loop);
int  UItimerStart(int id, int msec, int periodic);
int  UItimerStop(int id);


#endif

