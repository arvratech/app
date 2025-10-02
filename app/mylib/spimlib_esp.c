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
#ifdef _A20
#include <linux/spi/spidev.h>
#else
#include "spidev.h"
#endif
#include "rtc.h"
#include "mycrc.h"
#include "spimlib.h"

#define SPI_READ_INTERRUPT		1
#define SPI_WRITE_COMMAND		2
#define SPI_READ_STATUS			3
#define SPI_WRITE_REGISTER		4
#define SPI_READ_REGISTER		5
#define SPI_WRITE_BUFFER		6
#define SPI_READ_BUFFER			7


static void pabort(const char *s)
{
	perror(s);
	abort();
}

static char	spidev[32] = "/dev/spidev0.0";
static unsigned char	spiMode, spiBits;
static unsigned long	spiSpeed;
#include "uv.h"
void * MainLoop(void);
static uv_timer_t	*timerSpi, _timerSpi;
static unsigned long timer;


int spimOpen(void)
{
	int		fd, rval;

printf("spimOpen: Open...\n");
	fd = open(spidev, O_RDWR);
	if(fd < 0) {
		printf("can't open device(%s)\n", spidev);
		return -1;
	}
printf("spimOpen: [%s] opened\n", spidev);
/*
printf("spimOpen: Mode...\n");
	spiMode =  SPI_MODE_1;
	rval = ioctl(fd, SPI_IOC_WR_MODE, &spiMode);
	if(rval < 0) {
		printf("can't set spi mode\n");
		close(fd);
		return -1;
	}
	spiBits = 8;
printf("spimOpen: BitsPerWord...\n");
	rval = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits);
	if(rval < 0) {
		printf("can't set bits per word\n");
		close(fd);
		return -1;
	}
*/
	// sclk = pclk / ((divider+1) * 2)
	//	spiSpeed   divider   sclk(MHz)
	// Nuvoton: pclk=60MHz
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
	// A20: pclk=96MHz
	//   <= 24.0      1         24.0	
	//   <= 16.0      2         16.0	
	//   <= 12.0      3         12.0	
	//   <=  9.6      4          9.6
	//   <=  8.0      5          8.0
	//   <=  6.8571   6          6.8571
	//   <=  6.0      7          6.0
	//   <=  5.3333   8          5.3333
	//   <=  4.8      9          4.8
	//   <=  4.3636   10         4.3636
	//   <=  4.0      11         4.0
	//   <=  3.6923   12         3.6923
	//   <=  3.4285   13         3.4285
	//   <=  3.2      14         3.2
#ifdef _A20
	spiSpeed = 2000000;
#else
	spiSpeed = 1800000;
#endif
//spiSpeed = 2000000;
	rval = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed);
	if(rval < 0) {
		printf("can't set max speed hz\n");
		close(fd);
		return -1;
	}
printf("spimOpen: Speed=%d.%dMHz\n", spiSpeed/1000000, spiSpeed%1000000);
	void	*loop;
	loop = MainLoop();
	timerSpi = &_timerSpi;
	uv_timer_init(loop, timerSpi);

	return fd;
}

void spimClose(int fd)
{
	close(fd);
}

// change read:size=5 => read:size=4
int spimReadInterrupt(int fd, unsigned char *buf)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = SPI_READ_INTERRUPT << 4; headbuf[2] = 0;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = 3; xfer->read = 1;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) {
		rval = xfer->rval;
		if(!rval && crc8Check(buf, 3) < 0) {
printf("%d ##### slvmReadInterrupt invalid [%02x-%02x-%02x]\n", MS_TIMER, (int)buf[0], (int)buf[1], (int)buf[2]);
			rval = 99;
		}
	}
//printf("%lu spimReadInterrupt()=%d\n", MS_TIMER, rval);
	return rval;
}

int spimWriteCommand(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_WRITE_COMMAND << 4) | block; headbuf[2] = addr;
	xfer = &_xfer; xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
//printf("spimWriteCmd(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

int spimReadStatus(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_READ_STATUS << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size+1; xfer->read = 1;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) {
		rval = xfer->rval;
		if(!rval && crc8Check(buf, size+1) < 0) {
printf("%d ##### slvmReadStatus: invalid [%02x", MS_TIMER, (int)buf[0]);
			for(rval = 1;rval < size+1;rval++) printf("-%02x", (int)buf[rval]);
			printf("]\n");
			rval = 99;
		}
	}
printf("spimReadStatus(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

int spimWriteRegister(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_WRITE_REGISTER << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
//printf("spimWriteRegister(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

int spimReadRegister(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_READ_REGISTER << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size+1; xfer->read = 1;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) {
		rval = xfer->rval;
		if(!rval && crc8Check(buf, size+1) < 0) {
printf("%d ##### slvmReadRegister: invalid [%02x", MS_TIMER, (int)buf[0]);
			for(rval = 1;rval < size+1;rval++) printf("-%02x", (int)buf[rval]);
			printf("]\n");
			rval = 99;
		}
	}
//printf("spimReadRegister(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

int spimWriteBuffer(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_WRITE_BUFFER << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 0;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) rval = xfer->rval;
//printf("spimWriteBuffer(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

int spimReadBuffer(int fd, int block, int addr, unsigned char *buf, int size)
{
	struct spi_ioc_xfer	*xfer, _xfer;
	unsigned char	headbuf[4];
	int		rval;

	headbuf[0] = 0xaa; headbuf[1] = (SPI_READ_BUFFER << 4) | block; headbuf[2] = addr;
	xfer = &_xfer;
	xfer->headbuf = (unsigned long)headbuf; xfer->bodybuf = (unsigned long)buf; xfer->size = size; xfer->read = 1;
	if(addr == 0) xfer->size++;
	rval = ioctl(fd, SPI_IOC_WR_XFER, xfer);
	if(rval >= 0) {
		rval = xfer->rval;
		if(addr == 0 && !rval && crc8Check(buf, size+1) < 0) {
printf("%d ##### slvmReadBuffer(0): invalid [%02x", MS_TIMER, (int)buf[0]);
			for(rval = 1;rval < size+1;rval++) printf("-%02x", (int)buf[rval]);
			printf("]\n");
			rval = 99;
		}
	}
//printf("spimReadBuffer(%d,%d,%d)=%d\n", block, addr, size, rval);
	return rval;
}

/*
void spimWriteTest(int fd)
{
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
	rval = ioctl(fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 1) printf("can't send spi message");
	printf("spiWriteTest Rx: [%02x", (int)buf[0]);
	for(i = 1; i < 4;i++) printf("-%02x", (int)buf[i]);
	printf("]\n");
}
*/

