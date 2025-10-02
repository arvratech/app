#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/input.h>
#include <linux/watchdog.h>
#include <sys/epoll.h>
#include "spidev.h"
#include "rtc.h"
#include "inputlib.h"


static void pabort(const char *s)
{
	perror(s);
	abort();
}

int pipeOpen(int *fds)
{
	int		rval;

	rval = pipe(fds);
	if(rval < 0) {
		printf("pipe() error\n");
		return -1;
	}
	return 0;
}

static char spidev[] = "/dev/input/event0";
static char tsdev[]  = "/dev/input/event1";
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))
#ifdef _A20
#define EVIOCGKDK1		_IOR('E', 0xe1, int)
#define EVIOCGKDK2		_IOR('E', 0xe2, int)
#define EVIOCGKDK3		_IOR('E', 0xe3, int)
#else
#define EVIOCGKDK1		_IOR('E', 0x91, int)
#define EVIOCGKDK2		_IOR('E', 0x92, int)
#define EVIOCGKDK3		_IOR('E', 0x93, int)
#endif

int spievOpen(void)
{
	char	devname[256];
	struct input_id device_info;
	unsigned char evtype_bitmask[EV_MAX/8 + 1];
	int		i, fd, val;

printf("spievOpen: %s...\n", spidev);
	fd = open(spidev, O_RDONLY);
	if(fd < 0) {
		printf("can't open device(%s)\n", spidev);
		return -1;
	}
	memset(devname, 0x0, sizeof(devname));
    if(ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) < 0) {
		printf("%s evdev ioctl error!\n", spidev);
		close(fd);
		return -1;
	}
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("Supported event types:");
	for(i = 0;i < EV_MAX;i++) {
		if(test_bit(i, evtype_bitmask)) {
			// this means that the bit is set in the event types list */
			switch(i) {
			case EV_SYN: printf(" EV_SYN"); break;
			case EV_SW:  printf(" EV_SW"); break;
			default: printf(" Unknown: 0x%04hx", i);
			}
		}
	}
	printf("\n");
	val = fcntl(fd, F_GETFL, 0);
    val |= O_NONBLOCK;
    fcntl(fd, F_SETFL, val);
	return fd;
}

void spievClose(int fd)
{
	close(fd);
}

int spievRead(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK1, &val);
	return val;
}

int spievReadRdy(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK2, &val);
	return val;
}

void spievReadEvent(int fd)
{
	struct input_event *ev, _ev;
	int		rval;

	ev = &_ev;
	while(1) {
		rval = read(fd, ev, sizeof(struct input_event));
		if(rval <= 0) break;
		//switch(ev->type) {
		//case EV_SYN: printf("%lu EV_SYN\n", MS_TIMER); break;
		//case EV_SW:	 printf("%lu EV_SW: Code=%d Val=%d\n", MS_TIMER, ev->code, ev->value); break;
		//default:	 printf("%lu ev->type=0x%x\n", MS_TIMER, ev->type);
		//}
	}
}

#ifndef _OLDVER

static unsigned short	ts_x, ts_y;
static unsigned char	ts_state;

int tsOpen(void)
{
	char	devname[256];
	struct input_id device_info;
	unsigned char evtype_bitmask[EV_MAX/8 + 1];
	int		i, fd;

printf("tsOpen %s...\n", tsdev);
	fd = open(tsdev, O_RDONLY);
	if(fd < 0) {
		printf("can't open device(%s)\n", tsdev);
		return -1;
	}
	memset(devname, 0x0, sizeof(devname));
    if(ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) < 0) {
		printf("%s evdev ioctl error!\n", tsdev);
		close(fd);
		return -1;
	}
//printf("[%s]\n", devname);
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("Supported event types:");
	for(i = 0;i < EV_MAX;i++) {
		if(test_bit(i, evtype_bitmask)) {
			// this means that the bit is set in the event types list */
			switch(i) {
			case EV_SYN: printf(" EV_SYN"); break;
			case EV_ABS: printf(" EV_REL"); break;
			case EV_KEY: printf(" EV_KEY"); break;
			case EV_REP: printf(" EV_REP"); break;
			default: printf(" Unknwon: 0x%04hx\n", i);
			}
		}
	}
	printf("\n");
	return fd;
}

void tsClose(int fd)
{
	close(fd);
}

void tsReset(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK3, &val);
}

int tsRead(int fd, unsigned char *buf)
{
	struct input_event *ev, _ev;
	int		rval;

	ev = &_ev;
	rval = read(fd, ev, sizeof(struct input_event));
	rval = 0;
	switch(ev->type) {
	case EV_SYN:
		buf[0] = ts_state;
		buf[1] = ts_x >> 8; buf[2] = ts_x;
		buf[3] = ts_y >> 8; buf[4] = ts_y; 
//printf("-EV_SYN\n");
		rval = 1;
		break;
	case EV_ABS:
		if(ev->code == 0) ts_x = ev->value;
		else if(ev->code == 1) ts_y = ev->value;
//if(ev->code == 0) printf("-ABSx=%d", ts_x);
//else if(ev->code == 1) printf("-ABSy=%d", ts_y);
//else printf("-ABSxxx=%d", ev->value);
		break;
	case EV_KEY:
//printf("-EV_KEY=%d", ev->value);
		if(ev->code == BTN_TOUCH) ts_state = ev->value;
		break;
	default:	 printf("ev->type=0x%x\n", ev->type);
	}
	return rval;
}

#endif

static int	wdFd;

int wdOpen(void)
{
	unsigned long	val;
	int		fd;

	wdFd = -1;
	fd = open("/dev/watchdog", O_RDWR | O_NDELAY);
	if(fd < 0) {
		printf("can't open device watchdog\n");
		return -1;
	}
	val = 7;
    if(ioctl(fd, WDIOC_SETTIMEOUT, &val) < 0) {
		printf("ioctl WDIOC_SETTIMEOUT error\n");
		close(fd);
		return -1;
	}
printf("%u watchdog opened...\n", MS_TIMER);
	wdFd = fd;
	return fd;
}

void wdClose(void)
{
	// The 'V' value needs to be written into watchdog device file to indicate
	// that we intend to close/stop the watchdog. Otherwise, debug message
	// 'Watchdog timer closed unexpectedly' will be printed
	write(wdFd, "V", 1);
	// Closing the watchdog device will deactivate the watchdog. */
	close(wdFd);
	wdFd = -1;
printf("%u watchdog closed...\n", MS_TIMER);
}

void wdPing(void)
{
    if(ioctl(wdFd, WDIOC_KEEPALIVE, NULL) < 0) {
		printf("ioctl WDIOC_KEPALIVE error\n");
	}
//printf("%u watchdog...\n", MS_TIMER);
}

