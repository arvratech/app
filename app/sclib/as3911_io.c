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
#include <poll.h>
#ifdef _A20
#include <linux/spi/spidev.h>
#else
#include "spidev.h"
#endif
#include "inputlib.h"
#include "as3911_io.h"

static int		spi_fd;
static int		evt_fd;
static unsigned long	_txbuf[64], _rxbuf[64];


static void pabort(const char *s)
{
	perror(s);
	abort();
}

#ifdef _A20
static char	spidev[32] = "/dev/spidev1.1";
#else
static char	spidev[32] = "/dev/spidev0.0";
#endif
static unsigned char	spiMode, spiBits;
static unsigned long	spiSpeed;

int as3911OpenCom(void)
{
	int		rval;

printf("as3911OpenCom: ...\n");
	spi_fd = open(spidev, O_RDWR);
	if(spi_fd < 0) {
		printf("can't open device(%s)\n", spidev);
		return -1;
	}
printf("as3911OpenCom: [%s] opened\n", spidev);
	spiMode =  SPI_MODE_1;
	rval = ioctl(spi_fd, SPI_IOC_WR_MODE, &spiMode);
	if(rval < 0) {
		printf("can't set spi mode\n");
		close(spi_fd);
		return -1;
	}
	spiBits = 8;
printf("as3911OpenCom: BitsPerWord...\n");
	rval = ioctl(spi_fd, SPI_IOC_WR_BITS_PER_WORD, &spiBits);
	if(rval < 0) {
		printf("can't set bits per word\n");
		close(spi_fd);
		return -1;
	}
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
#ifdef _A20
	spiSpeed = 5000000;
#else
	spiSpeed = 5000000;
#endif
printf("as3911OpenCom: spiSpeed...\n");
	rval = ioctl(spi_fd, SPI_IOC_WR_MAX_SPEED_HZ, &spiSpeed);
	if(rval < 0) {
		printf("can't set max speed hz\n");
		close(spi_fd);
		return -1;
	}
#ifdef _A20
	evt_fd = scEventOpen();
#endif
printf("as3911OpenCom: spiSpeed=%d.%dMHz\n", spiSpeed/1000000, spiSpeed%1000000);
	return spi_fd;
}

void as3911CloseCom(void)
{
//	CloseINT0();
#ifdef _A20
	close(evt_fd);
#endif
	close(spi_fd);
printf("as3911CloseCom\n");
}

#define AS3911_WRITE_MODE		0x00
#define AS3911_READ_MODE		0x40
#define AS3911_FIFO_LOAD		0x80
#define AS3911_FIFO_READ		0xbf
#define AS3911_CMD_MODE			0xc0

int as3911GetReg(int reg, unsigned char *data)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf, *rxbuf;
	int		rval;
 
	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
    rxbuf = (unsigned char *)_rxbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = reg | AS3911_READ_MODE;
	txbuf[1] = 0xff;
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->rx_buf		= (unsigned long)rxbuf;
    xfer->len			= 2;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911GetReg: ioctl() error: %s\n", strerror(errno));
	} else {
		data[0] = rxbuf[1];
	}
	return rval;

}

int as3911GetRegs(int reg, unsigned char *buf, int length)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf, *rxbuf;
	int		i, rval;
 
	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
    rxbuf = (unsigned char *)_rxbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = reg | AS3911_READ_MODE;
	memset(txbuf+1, 0xff, length);
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->rx_buf		= (unsigned long)rxbuf;
    xfer->len			= 1 + length;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911GetRegs: ioctl() error: %s\n", strerror(errno));
	} else {
		memcpy(buf, rxbuf+1, length);
	}
	return rval;

return 0;
}

int as3911SetReg(int reg, int data)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf;
	int		rval;

	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = reg | AS3911_WRITE_MODE;
	txbuf[1] = data;
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->len			= 2;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911SetReg: ioctl() error: %s\n", strerror(errno));
	}
	return rval;
}

int as3911SetRegs(int reg, unsigned char *buf, int length)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf;
	int		rval;

	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = reg | AS3911_WRITE_MODE;
	memcpy(txbuf+1, buf, length);
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->len			= 1 + length;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911SetRegs: ioctl() error: %s\n", strerror(errno));
	}
	return rval;
}

int as3911ModReg(int reg, int clr_mask, int set_mask)
{
	unsigned char	tmp;
	int		val;

	as3911GetReg(reg, &tmp);
	val = tmp;
	val &= ~clr_mask;
 	val |= set_mask;
	return as3911SetReg(reg, val);
}

int as3911ReadFifo(unsigned char *buf, int length)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf, *rxbuf;
	int		i, rval;
 
	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
    rxbuf = (unsigned char *)_rxbuf;
	//memset(xfer, 0, sizeof(struct spi_ioc_transfer)*2);
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));	// bug fix 2024-09-24
	txbuf[0] = AS3911_FIFO_READ;
	memset(txbuf+1, 0xff, length);
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->rx_buf		= (unsigned long)rxbuf;
    xfer->len			= 1 + length;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911ReadFifo: ioctl() error: %s\n", strerror(errno));
	} else {
		memcpy(buf, rxbuf+1, length);
	}
	return rval;
}

int as3911WriteFifo(unsigned char *buf, int length)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf;
	int		rval;

	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = AS3911_FIFO_LOAD;
	memcpy(txbuf+1, buf, length);
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->len			= 1 + length;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911WriteFifo: ioctl() error: %s\n", strerror(errno));
	}
	return rval;
}

int as3911ExecuteCommand(int command)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf;
	int		rval;

	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	txbuf[0] = command | AS3911_CMD_MODE;
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->len			= 1;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911ExecuteCommand: ioctl() error: %s\n", strerror(errno));
	}
	return rval;
}

int as3911ExecuteCommands(unsigned char *commands, int length)
{
	struct spi_ioc_transfer		*xfer, _xfer;
	unsigned char	*txbuf;
	int		i, rval;

	xfer = &_xfer;
    txbuf = (unsigned char *)_txbuf;
	memset(xfer, 0, sizeof(struct spi_ioc_transfer));
	for(i = 0;i < length;i++) {
		txbuf[i] = commands[i] | AS3911_CMD_MODE;
	}
    xfer->tx_buf		= (unsigned long)txbuf;
    xfer->len			= length;
	xfer->speed_hz		= spiSpeed;	
	xfer->bits_per_word	= 8;
	xfer->delay_usecs	= 0;
	xfer->cs_change		= 0;
	rval = ioctl(spi_fd, SPI_IOC_MESSAGE(1), xfer);
	if(rval < 0) {
        printf("as3911ExecuteCommands: ioctl() error: %s\n", strerror(errno));
	}
	return rval;
}

int as3911WaitScEvent(int timeout)
{
	struct pollfd	fds[1];
	int		rval;

	memset(fds, 0, sizeof(struct pollfd));
	fds[0].fd = evt_fd; fds[0].events = POLLIN | POLLPRI;
	rval = poll(fds, 1, timeout);
	if(rval < 0) {
		printf("poll() error: %s\n", strerror(errno));
	} else if(rval > 0) {
		scEventReadEvent(evt_fd);
	} else	printf("as3911WaitScEvent: timeout=%d\n", timeout);
	return rval;
}

