#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/rtc.h>
#include "w55fa92_rtc.h"
#include "rtc.h"

int		rtc_fd;


int rtcOpen(void)
{
	char	rtcdev[16];
	int		fd;

	strcpy(rtcdev, "/dev/rtc");
	fd = open(rtcdev, O_RDONLY);
	if(fd < 0) {
		printf("rtcOpen: %s faild!\n", rtcdev);
		strcpy(rtcdev, "/dev/rtc0");
		fd = open(rtcdev, O_RDONLY);
		if(fd < 0) {
			printf("rtcOpen: %s faild!\n", rtcdev);
			strcpy(rtcdev, "/dev/misc/rtc");
			fd = open(rtcdev, O_RDONLY);
			if(fd < 0) {
				printf("rtcOpen: %s faild!\n", rtcdev);
			}
		}
	}
	if(fd >= 0) printf("%s is opened: %d\n", rtcdev, rtc_fd);
	rtc_fd = fd;
	return fd;
}

void rtcSync(void)
{
	time_t		ltime;
	struct tm	*tm, _tm;
	struct rtc_time rtc_tm;
	int		rval;
	
	rval = ioctl(rtc_fd, RTC_RD_TIME, (void *)&rtc_tm);
	if(rval < 0) {
		printf("rtcSync: RTC_RD_TIME  faild!\n");
		return ;
	}
	tm = &_tm;
    tm->tm_year  = rtc_tm.tm_year;
    tm->tm_mon   = rtc_tm.tm_mon;
    tm->tm_mday  = rtc_tm.tm_mday;
    tm->tm_hour  = rtc_tm.tm_hour;
	tm->tm_min   = rtc_tm.tm_min;
	tm->tm_sec   = rtc_tm.tm_sec;
	tm->tm_isdst = 0;
    ltime = mktime(tm);
    stime(&ltime);
printf("### rtcSync: %04d-%02d-%02d %02d:%02d:%02d\n", (int)tm->tm_year+1900, (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
}

unsigned long rtcMiliTimer(void)
{
	struct timespec		tspec;
	unsigned long	val;

	clock_gettime(CLOCK_MONOTONIC, &tspec);
	val = tspec.tv_sec * 1000;
	val += tspec.tv_nsec / 1000000;
	return val;
}

void rtcGetMicroTimer(unsigned long long *tval)
{
	struct timespec		tspec;
	unsigned long long	val;

	clock_gettime(CLOCK_MONOTONIC, &tspec);
	val = tspec.tv_sec;
	val = val * 1000000 + tspec.tv_nsec / 1000;
	*tval = val;
}

//char  buf[32];
//struct tm   tm_now;
//clock_gettime(CLOCK_REALTIME, &tspec);
//localtime_r((time_t *)&tspec.tv_sec, &tm_now); 
//strftime(buf, sizeof(buf), "%y-%m-%d/%H:%M:%S", &tm_now);

void rtcGetDateTime(unsigned char *ctm)
{
	struct tm	*tm, _tm;
	struct rtc_time rtc_tm;
	time_t	ltime;
	int		rval;
	
/*
if(xxx & 1) {
printf("rtcGetDateTime: RTC_RD_TIME...\n");
usleep(1000);
rval = ioctl(rtc_fd, RTC_RD_TIME, (void *)&rtc_tm);
if(rval < 0) {
	printf("rtcRead: RTC_RD_TIME  faild!\n");
	return ;
}
tm = &_tm;
tm->tm_year  = rtc_tm.tm_year;
tm->tm_mon   = rtc_tm.tm_mon;
tm->tm_mday  = rtc_tm.tm_mday;
tm->tm_hour  = rtc_tm.tm_hour;
tm->tm_min   = rtc_tm.tm_min;
tm->tm_sec   = rtc_tm.tm_sec;
tm->tm_isdst = 0;
printf("rtcGetDateTime: %04d-%02d-%02d %02d:%02d:%02d\n", (int)tm->tm_year+1900, (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
} else {
}
xxx++;
*/
	time(&ltime);
	tm = &_tm;
	gmtime_r(&ltime, tm);	// localtime(&ltime);
	ctm[0] = tm->tm_year - 100;
	ctm[1] = tm->tm_mon + 1;
	ctm[2] = tm->tm_mday;
	ctm[3] = tm->tm_hour;
	ctm[4] = tm->tm_min;
	ctm[5] = tm->tm_sec;
//printf("%04d-%02d-%02d %02d:%02d:%02d\n", (int)tm->tm_year+1900, (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
//printf("GetTime: %02d-%02d-%02d %02d:%02d:%02d\n", (int)ctm[0], (int)ctm[1], (int)ctm[2], (int)ctm[3], (int)ctm[4], (int)ctm[5]);
}

void rtcGetDate(unsigned char *ctm)
{
	struct tm	*tm, _tm;
	time_t	ltime;

	time(&ltime);
	tm = &_tm;
	gmtime_r(&ltime, tm);	// localtime(&ltime);
	ctm[0] = tm->tm_year - 100;
	ctm[1] = tm->tm_mon + 1;
	ctm[2] = tm->tm_mday;
}

void rtcGetTime(unsigned char *ctm)
{
	struct tm	*tm, _tm;
	time_t	ltime;

	time(&ltime);
	tm = &_tm;
	gmtime_r(&ltime, tm);	// localtime(&ltime);
	ctm[0] = tm->tm_hour;
	ctm[1] = tm->tm_min;
	ctm[2] = tm->tm_sec;
}

void rtcSetInt64time(int64_t i64time)
{
	struct tm	*tm, _tm;
	struct rtc_time rtc_tm;
	time_t	ltime;
	int		rval;

	ltime = i64time / 1000;
	stime(&ltime);
	tm = &_tm;
	gmtime_r(&ltime, tm);	// localtime(&ltime);
	rtc_tm.tm_year	= tm->tm_year;
	rtc_tm.tm_mon	= tm->tm_mon;
	rtc_tm.tm_mday	= tm->tm_mday;
	rtc_tm.tm_hour	= tm->tm_hour;
	rtc_tm.tm_min	= tm->tm_min;
	rtc_tm.tm_sec	= tm->tm_sec;
	rtc_tm.tm_wday	= tm->tm_wday;
	rtc_tm.tm_isdst	= 0;
	rval = ioctl(rtc_fd, RTC_SET_TIME, (void *)&rtc_tm);
	if(rval < 0) {
		printf("rtcWrite: RTC_SET_TIME  faild!\n");
	}
printf("### rtcSetDataTime: %04d-%02d-%02d %02d:%02d:%02d\n", (int)tm->tm_year+1900, (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
}

void rtcSetDateTime(unsigned char *ctm)
{
	time_t		ltime;
	struct tm	*tm, _tm;
	struct rtc_time rtc_tm;
	int		rval;

	tm = &_tm;
	tm->tm_year  = ctm[0] + 100;
	tm->tm_mon   = ctm[1] - 1;
	tm->tm_mday  = ctm[2];
	tm->tm_hour  = ctm[3];
	tm->tm_min   = ctm[4];
	tm->tm_sec   = ctm[5];
	tm->tm_isdst = 0;
	ltime = mktime(tm);
	stime(&ltime);
/*
	rval = ioctl(rtc_fd, RTC_ACCESS_ENABLE, (void *)&rtc_tm);
	if(rval < 0) {
		printf("rtcWrite: RTC_SET_TIME  faild!\n");
	}
*/
	rtc_tm.tm_year	= tm->tm_year;
	rtc_tm.tm_mon	= tm->tm_mon;
	rtc_tm.tm_mday	= tm->tm_mday;
	rtc_tm.tm_hour	= tm->tm_hour;
	rtc_tm.tm_min	= tm->tm_min;
	rtc_tm.tm_sec	= tm->tm_sec;
	rtc_tm.tm_wday	= tm->tm_wday;
	rtc_tm.tm_isdst	= 0;
	rval = ioctl(rtc_fd, RTC_SET_TIME, (void *)&rtc_tm);
	if(rval < 0) {
		printf("rtcWrite: RTC_SET_TIME  faild!\n");
	}
printf("### rtcSetDataTime: %04d-%02d-%02d %02d:%02d:%02d\n", (int)tm->tm_year+1900, (int)tm->tm_mon+1, (int)tm->tm_mday, (int)tm->tm_hour, (int)tm->tm_min, (int)tm->tm_sec);
}

void ms_sleep(int msec)
{
	struct timespec	_tspec, _tspec2;
	int		rval;

	_tspec.tv_sec  = msec / 1000;
	_tspec.tv_nsec = (msec % 1000) * 1000000;
	while(1) {
		rval = nanosleep(&_tspec, &_tspec2);
		if(rval < 0 && errno == EINTR) {
			_tspec.tv_sec = _tspec2.tv_sec;
			_tspec.tv_nsec = _tspec2.tv_nsec; 
		} else {
			break;
		}
	}
}

void us_sleep(int usec)
{
	struct timespec	_tspec, _tspec2;
	int		rval;

	_tspec.tv_sec  = usec / 1000000;
	_tspec.tv_nsec = (usec % 1000000) * 1000;
	while(1) {
		rval = nanosleep(&_tspec, &_tspec2);
		if(rval < 0 && errno == EINTR) {
			_tspec.tv_sec = _tspec2.tv_sec;
			_tspec.tv_nsec = _tspec2.tv_nsec; 
		} else {
			break;
		}
	}
}

