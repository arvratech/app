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
#include "spimst.h"

#define SPI_READ_STATUS_ALL		0
#define SPI_WRITE_COMMAND		1
#define SPI_WRITE_REGISTER_S	2
#define SPI_WRITE_BUFFER_S		3
#define SPI_READ_STATUS			4
#define SPI_WRITE_REGISTER		5
#define SPI_READ_REGISTER		6
#define SPI_WRITE_BUFFER		7
#define SPI_READ_BUFFER			8

static int		spi_fd;

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static char	spidev[32] = "/dev/spidev0.0";
static unsigned char	spiMode, spiBits;
static unsigned long	spiSpeed;


int spiOpen(void)
{
#ifndef _HOST_EMU
	int		rval;

printf("spiOpen: Open...\n");
	spi_fd = open(spidev, O_RDWR);
	if(spi_fd < 0) {
		printf("can't open device(%s)\n", spidev);
		return -1;
	}
printf("spiOpen: [%s] opened\n", spidev);
/*
printf("spiOpen: Mode...\n");
	spiMode =  SPI_MODE_1;
	rval = ioctl(spi_fd, SPI_IOC_WR_MODE, &spiMode);
	if(rval < 0) {
		printf("can't set spi mode\n");
		close(fd);
		return -1;
	}
	spiBits = 8;
printf("spiOpen: BitsPerWord...\n");
	rval = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits);
	if(rval < 0) {
		printf("can't set bits per word\n");
		close(fd);
		return -1;
	}
*/
	// sclk = pclk / ((divider+1) * 2)
	//	spiSpeed   divider   sclk(MHz)
	//    > 15.0      0        30.0
	//	 <= 15.0      1        15.0
	//	 <= 10.0      2        10.0
	//	 <=  7.5	  3         7.5
	//	 <=  6.0      4         6.0
	//	 <=  5.0      5         5.0
	//	 <=  4.2857   6         4.2857
	//	 <=  3.75     7         3.75
	//	 <=  3.3333   8         3.3333
	//	 <=  3.0      9         3.0
	//	 <=  2.7272   10        2.7272
	//	 <=  2.5      11        2.5
	spiSpeed = 10000000;
spiSpeed = 5000000;
printf("spiOpen: Speed...\n");
	rval = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed);
	if(rval < 0) {
		printf("can't set max speed hz\n");
		close(spi_fd);
		return -1;
	}
printf("spiOpen: Speed=%d.%dMHz\n", spiSpeed/1000000, spiSpeed%1000000);
	return spi_fd;
#else
	return 1;
#endif
}

void spiClose(void)
{
#ifndef _HOST_EMU
	close(spi_fd);
#endif
}

int spiWriteCommand(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (1 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer; xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
printf("spiWriteCmd(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

int spiReadStatus(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (2 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 1;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
//printf("spiReadStatus(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

int spiWriteRegister(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (3 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
printf("spiWriteRegister(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

int spiReadRegister(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (4 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 1;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
printf("spiReadRegister(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

int spiWriteBuffer(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (5 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
printf("spiWriteBuffer(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

int spiReadBuffer(int block, int addr, unsigned char *buf, int size)
{
#ifndef _HOST_EMU
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (6 << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 1;
	rval = ioctl(spi_fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
printf("spiReadBuffer(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
#else
	return 0;
#endif
}

void spiWriteTest(void)
{
#ifndef _HOST_EMU
	struct spi_ioc_transfer	*xfer, _xfer;
	unsigned char	headbuf[4], buf[4];
	int		i, rval;

	headbuf[0] = 0xaa; headbuf[1] = (9 << 4) | 0; headbuf[2] = 0; headbuf[3] = 0;
	xfer = &_xfer;
	xfer->tx_buf = (unsigned long)headbuf;
	xfer->rx_buf = (unsigned long)buf;
	xfer->len = 4;
	xfer->delay_usecs = 0;
	xfer->speed_hz = spiSpeed;
	xfer->bits_per_word = spiBits;
	xfer->cs_change = 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 1) printf("can't send spi message");
	printf("spiWriteTest Rx: [%02x", (int)buf[0]);
	for(i = 1; i < 4;i++) printf("-%02x", (int)buf[i]);
	printf("]\n");
#endif
}
