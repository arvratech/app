#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "hw.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "siocred.h"


int sioOpen(long baudRate)
{
	struct termios	tio;
	char	name[32];
	long	baud;
	int		fd;

	strcpy(name, "/dev/ttyS1");
	fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(fd < 0) {
		printf("sioOpen: can't open device(%s)\n", name);
		return -1;
    }
//	tcgetattr(fd, &tio);		// save current serial port settings
	bzero(&tio, sizeof(tio));	// clear struct for new port settings
	tio.c_cflag = B19200 | CS8 | CLOCAL | CREAD;
	tcsetattr(fd, TCSANOW, &tio);
printf("sio:%s opened\n", name);
	return fd;
}

void sioClose(int fd)
{
	close(fd);
printf("sio closed\n");
}

void sioSetSpeed(int fd, long baudRate)
{
	struct termios	tio;
	speed_t		speed;

	switch(baudRate) {
	case 9600L:		speed = B9600; break;
	case 19200L:	speed = B19200; break;
	case 38400L:	speed = B38400; break;
	case 57600L:	speed = B57600; break;
	case 115200L:	speed = B115200; break;
	}
	tcgetattr(fd, &tio);		// save current serial port settings
	cfsetospeed(&tio, speed);
	cfsetispeed(&tio, speed);
}

void _SioWrite(int fd)
{
	unsigned char	buf[16], ctm[8], bcc;
	signed char		rssi;
	char	temp[16];
	int		i, year;

	buf[0] = 0x02;
	buf[1] = 0x0a;
	buf[2] = 0x01;
	rtcGetDateTime(ctm);
	year = ctm[0] + 2000;
	sprintf(temp, "%04d", year);
	string2bcd(temp, 4, buf+3);
	char2bcd(ctm[1], buf+5);	// mm
	char2bcd(ctm[2], buf+6);	// dd
	char2bcd(ctm[3], buf+7);	// hh
	char2bcd(ctm[4], buf+8);	// mi
	char2bcd(ctm[5], buf+9);	// ss
	buf[10] = 0x18;
	rssi = -60;
	buf[11] = rssi;
	buf[11] = 0;
	bcc = 0;
	for(i = 1;i < 12;i++) bcc ^= buf[i];
	buf[12] = bcc;
printf("sioWrite: 13 [%02x", (int)buf[0]); for(i = 1;i < 13;i++) printf("-%02x", (int)buf[i]); printf("]\n");
	write(fd, buf, 13);
}

void sioOnRead(int fd, unsigned char *buf, int length)
{
	CREDENTIAL_READER	*cr;
	unsigned char	bcc, msg[12];
	int		rval, i;

printf("sioOnRead: %d [%02x", length, (int)buf[0]); for(i = 1;i < length;i++) printf("-%02x", (int)buf[i]); printf("]\n");
	if(length >= 4 && buf[0] == 0x02 && buf[1] == 0x01 && buf[2] == 0x00) {
		bcc = 0;
		for(i = 1;i < 3;i++) bcc ^= buf[i];
		if(bcc != buf[3]) {
			printf("sioOnRead() bcc error: bcc=%02x buf[3]=%02x\n", (int)bcc, (int)buf[3]);
		} else {
			_SioWrite(fd);
		}
	} else if(length >= 13 && buf[0] == 0x02 && buf[1] == 0x0a && buf[2] == 0x00) {
		bcc = 0;
		for(i = 1;i < 12;i++) bcc ^= buf[i];
		if(bcc != buf[12]) {
			printf("sioOnRead() bcc error: bcc=%02x buf[12]=%02x\n", (int)bcc, (int)buf[12]);
			return;
		}
		_SioWrite(fd);
		cr = crsGet(0);
		cr->readerFormat = 0x30;
		cr->cardType = 2;
		cr->credType = CREDENTIAL_CARD;
		memcpy(cr->data, buf+3, 9);
		cr->dataLength = 9;
		msg[0] = GM_CRED_CAPTURED; msg[1] = 0; memset(msg+2, 0, 8);
//		appSendMessage(msg);
	} else {
		printf("sioOnRead() error frame\n");
	}
}

