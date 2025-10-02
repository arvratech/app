#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include "rtc.h"
#include "i2cmlib.h"


static void pabort(const char *s)
{
	perror(s);
	abort();
}

#define I2C_ADDR		(0x5a>>1)
//#define I2C_ADDR		0xc0

static char		i2cm_dev[32] = "/dev/i2c-3";
static unsigned long	i2cmSpeed;


int i2cmOpen(void)
{
	int		fd;

	fd = open(i2cm_dev, O_RDWR);
	if(fd < 0) {
		printf("i2cmOpen: can't open device(%s)\n", i2cm_dev);
		return -1;
	}
	return fd;
}

void i2cmClose(int fd)
{
	close(fd);
}

static int _I2cmRead(int fd, int addr, unsigned char *buf, int size)
{
	struct i2c_rdwr_ioctl_data	rdwr;
	struct i2c_msg	msgs[2];
	unsigned char	mybuf[4];
	int		rval;

	mybuf[0] = addr;
	msgs[0].addr	= I2C_ADDR;
	msgs[0].flags	= 0;
	msgs[0].len		= 1;
	msgs[0].buf		= mybuf;
	msgs[1].addr	= I2C_ADDR;
	msgs[1].flags	= I2C_M_RD;
	msgs[1].len		= size;
	msgs[1].buf		= buf;
	rdwr.msgs		= msgs;
	rdwr.nmsgs		= 2;
	rval = ioctl(fd, I2C_RDWR, &rdwr);
	if(rval > 0) rval = 0;
	return rval;
}

static int _I2cmWrite(int fd, int addr, unsigned char *buf, int size)
{
	struct i2c_rdwr_ioctl_data	rdwr;
	struct i2c_msg	msgs[1];
	unsigned char	mybuf[256];
	int		rval;

	mybuf[0] = addr;
	memcpy(mybuf+1, buf, size);
	msgs[0].addr	= I2C_ADDR;
	msgs[0].flags	= 0;
	msgs[0].len		= 1 + size;
	msgs[0].buf		= mybuf;
	rdwr.msgs		= msgs;
	rdwr.nmsgs		= 1;
	rval = ioctl(fd, I2C_RDWR, &rdwr);
	if(rval > 0) rval = 0;
	return rval;
}

int i2cmReadInterrupt(int fd, unsigned char *buf)
{
	struct i2c_rdwr_ioctl_data	rdwr;
	struct i2c_msg	msgs[1];
	int		rval;

	msgs[0].addr	= I2C_ADDR;
	msgs[0].flags	= I2C_M_RD;
	msgs[0].len		= 2;
	msgs[0].buf		= buf;
	rdwr.msgs		= msgs;
	rdwr.nmsgs		= 1;
	rval = ioctl(fd, I2C_RDWR, &rdwr);
	if(rval > 0) rval = 0;
	return rval;
}

int i2cmWriteRegister(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmWrite(fd, addr, buf, size);
	return rval;
}

int i2cmReadRegister(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmRead(fd, addr, buf, size);
	return rval;
}

int i2cmWriteCommand(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmWrite(fd, addr | 0x40, buf, size);
	return rval;
}

int i2cmReadStatus(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmRead(fd, addr | 0x40, buf, size);
	return rval;
}

int i2cmWriteBuffer(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmWrite(fd, addr | 0x80, buf, size);
	return rval;
}

int i2cmReadBuffer(int fd, int addr, unsigned char *buf, int size)
{
	int		rval;

	rval = _I2cmRead(fd, addr | 0x80, buf, size);
	return rval;
}

