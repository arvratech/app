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
#include <sys/poll.h>
#include "spidev.h"
#include "rtc.h"
#include "inputev.h"


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
#ifndef _HOST_EMU
#define EVIOCGKDK1		_IOR('E', 0x91, int)
#define EVIOCGKDK2		_IOR('E', 0x92, int)
#endif


int spievOpen(void)
{
	char	devname[256];
	struct input_id device_info;
	unsigned char evtype_bitmask[EV_MAX/8 + 1];
	int		i, fd;

printf("spievOpen %s...\n", spidev);
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
printf("[%s]\n", devname);
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("Supported event types:");
	for(i = 0;i < EV_MAX;i++) {
		if(test_bit(i, evtype_bitmask)) {
			// this means that the bit is set in the event types list */
			switch(i) {
			case EV_SYN: printf(" EV_SYN"); break;
			case EV_REL: printf(" EV_REL"); break;
			case EV_KEY: printf(" EV_KEY"); break;
			default: printf(" Unknown: 0x%04hx\n", i);
			}
		}
	}
	printf("\n");
	return fd;
}

void spievClose(int fd)
{
	close(fd);
}

int spievRead(int ev_fd)
{
	int		val;

	ioctl(ev_fd, EVIOCGKDK1, &val);
	return val;
}

int spievReadRdy(int ev_fd)
{
	int		val;

	ioctl(ev_fd, EVIOCGKDK2, &val);
	return val;
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
printf("[%s]\n", devname);
	ioctl(fd, EVIOCGBIT(0, sizeof(evtype_bitmask)), evtype_bitmask);
	printf("Supported event types:\n");
	for(i = 0;i < EV_MAX;i++) {
		if(test_bit(i, evtype_bitmask)) {
			// this means that the bit is set in the event types list */
			printf("Event type 0x%02x ", i);
			switch(i) {
			case EV_SYN: printf(" EV_SYN\n"); break;
			case EV_ABS: printf(" EV_REL\n"); break;
			case EV_KEY: printf(" EV_KEY\n"); break;
			default: printf(" Other event type: 0x%04hx\n", i);
			}
		}
	}
	return fd;
}

void tsClose(int fd)
{
	close(fd);
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
		rval = 1;
		break;
	case EV_ABS:
		if(ev->code == 0) ts_x = ev->value;
		else if(ev->code == 1) ts_y = ev->value;
//printf("[x=%d,y=%d]\n", ts_x, ts_y);
		break;
	case EV_KEY:
		if(ev->code == BTN_TOUCH) ts_state = ev->value;
		break;
	default:	 printf("ev->type=0x%x\n", ev->type);
	}
	return rval;
}

#endif

int inputevPoll(int pipe_fd, int spi_fd, int ts_fd, unsigned char *buf)
{
	struct input_event *ev, _ev;
	struct pollfd	fds[3];
	int		rval, count, val;

	fds[0].fd = pipe_fd;
	fds[0].events = POLLIN;
	fds[1].fd = spi_fd;
	fds[1].events = POLLIN;
	count = 2;
#ifndef _OLDVER
	fds[2].fd = ts_fd;
	fds[2].events = POLLIN;
	count++;
#endif
	rval = poll(fds, count, 3000);
	ev = &_ev;
//printf("inputevPoll=%d...\n", rval);
	if(rval > 0) {
		rval = 0;
		if(fds[0].revents & (POLLIN | POLLOUT)) rval |= 0x01;
		if(fds[1].revents & POLLIN) rval |= 0x02;
#ifndef _OLDVER
		if(fds[2].revents & POLLIN) rval |= 0x04;
#endif
		if(rval & 0x01) {
			//val = read(pipe_fd, buf, 1);
		}
		if(rval & 0x02) {
			val = read(spi_fd, ev, sizeof(struct input_event));
			//switch(ev->type) {
			//case EV_SYN: printf("%ld EV_SYN\n", MS_TIMER); break;
			//case EV_SW:	 printf("%ld EV_SW: Code=%d Val=%d\n", MS_TIMER, ev->code, ev->value); break;
			//default:	 printf("%ld ev->type=0x%x\n", MS_TIMER, ev->type);
			//}
		}
	} else if(rval < 0) {
		if(errno == EINTR) { printf("spievPipePoll EINTR...\n"); rval = 0; }
		else	perror("spievPipePoll");
	}
	return rval;
}

