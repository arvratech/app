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
#ifdef _A20
#include <linux/spi/spidev.h>
#else
#include "spidev.h"
#endif
#include <sys/epoll.h>
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

void pipeClose(int *fds)
{
	close(fds[0]);
	close(fds[1]);
}

int pipeWrite(int fd, unsigned char *buf, int length)
{
	int		rval;

	buf[0] = 0xaa; buf[1] = 0;
	SHORTtoBYTE((unsigned short)length, buf+2);	
	rval = write(fd, buf, length+4);
	return rval;
}

int pipeRead(int fd, unsigned char *buf)
{
	unsigned short	sval;
	int		rval, len;

	rval = read(fd, buf, 4);
	if(rval < 4) {
		printf("pipeRead(): read() error\n");
		rval = -2;
	} else if(buf[0] != 0xaa) {
printf("%02x-%02x-%02x-%02x\n", (int)buf[0], (int)buf[2], (int)buf[2], (int)buf[3]); 
		printf("pipeRead(): read() data error, flushed\n");
		while(1) {
			rval = read(fd, buf, 128);
			if(rval <= 0) break;
		}
		rval = -1;
	} else {
		BYTEtoSHORT(buf+2, &sval);
		len = sval;
		rval = read(fd, buf, len);
		if(rval < len) {
			printf("pipeRead(): length error: %d %d\n", len, rval);
			rval = -1;
		}
	}
	return rval;
}
	
static char iowmdev[]	= "/dev/input/event0";
static char blemdev[]	= "/dev/input/event1";
static char scdev[]		= "/dev/input/event2";
#define test_bit(bit, array)    (array[bit/8] & (1<<(bit%8)))
#ifdef _A20
static char tsdev[]		= "/dev/input/event3";
#define EVIOCGKDK1		_IOR('E', 0xe1, int)
#define EVIOCGKDK2		_IOR('E', 0xe2, int)
#define EVIOC_INITLED	_IOR('E', 0xe3, int)
#define EVIOC_SETLED	_IOR('E', 0xe4, int)
#define EVIOC_CLEARLED	_IOR('E', 0xe5, int)
#else
static char tsdev[]		= "/dev/input/event1";
#define EVIOCGKDK1		_IOR('E', 0x91, int)
#define EVIOCGKDK2		_IOR('E', 0x92, int)
#define EVIOCGKDK3		_IOR('E', 0x93, int)
#endif

static int _EventOpen(char *dev)
{
	struct input_id		device_info;
	unsigned char		evtype_bitmask[EV_MAX/8 + 1];
	char	devname[128];
	int		i, fd, val;

	fd = open(dev, O_RDONLY);
	if(fd < 0) {
		printf("can't open device(%s)\n", dev);
		return -1;
	}
	memset(devname, 0x0, sizeof(devname));
    if(ioctl(fd, EVIOCGNAME(sizeof(devname)), devname) < 0) {
		printf("%s evdev ioctl error!\n", dev);
		close(fd);
		return -1;
	}
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("%s: %s: supported event types:", dev, devname);
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

static void _EventClose(int fd)
{
	close(fd);
}

static int _EventRead(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK1, &val);
	return val;
}

void _EventReadEvent(int fd)
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

#ifdef _A20

int iowmEventOpen(void)
{
	return _EventOpen(iowmdev);
}

void iowmEventClose(int fd)
{
	_EventClose(fd);
}

void iowmEventReadEvent(int fd)
{
	_EventReadEvent(fd);
}

int iowmEventRead(int fd)
{
	return _EventRead(fd);
}

#else

int slvmEventOpen(void)
{
	return _EventOpen(iowmdev);
}

void slvmEventClose(int fd)
{
	_EventClose(fd);
}

void slvmEventReadEvent(int fd)
{
	_EventReadEvent(fd);
}

int slvmEventRead(int fd)
{
	return _EventRead(fd);
}

#endif

int blemEventOpen(void)
{
	return _EventOpen(blemdev);
}

void blemEventClose(int fd)
{
	_EventClose(fd);
}

void blemEventReadEvent(int fd)
{
	_EventReadEvent(fd);
}

int blemEventRead(int fd)
{
	return _EventRead(fd);
}

int blemEventReadReady(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK2, &val);
	return val;
}

int scEventOpen(void)
{
	return _EventOpen(scdev);
}

void scEventClose(int fd)
{
	_EventClose(fd);
}

void scEventReadEvent(int fd)
{
	_EventReadEvent(fd);
}

int scEventRead(int fd)
{
	return _EventRead(fd);
}

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
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("%s: %s: supported event types:", tsdev, devname);
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

/*
void tsReset(int fd)
{
	int		val;

	ioctl(fd, EVIOCGKDK3, &val);
}
*/

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

