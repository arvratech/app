#ifndef _RTCTIME_H
#define _RTCTIME_H


#include <sys/types.h>
#include <sys/time.h>


#define MS_TIMER	rtcMiliTimer()
#define SEC_TIMER	(rtcMiliTimer()/1000)	


int  rtcOpen(void);
void rtcSync(void);
unsigned long rtcMiliTimer(void);
void rtcGetMicroTimer(unsigned long long *tval);
void rtcGetDateTime(unsigned char *ctm);
void rtcGetDate(unsigned char *ctm);
void rtcGetTime(unsigned char *ctm);
void rtcSetInt64time(int64_t ltime);
void rtcSetDateTime(unsigned char *ctm);
void ms_sleep(int msec);
void us_sleep(int usec);


#endif

