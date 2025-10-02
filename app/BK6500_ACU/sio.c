#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "cotask.h"
#include "rtc.h"
#include "cbuf.h"
#include "defs_ascii.h"
#include "sio.h"

extern unsigned long Pclk;

#define	RS485_CTRL0_H	(rGPHDAT |= 0x00000001)		// GPH0=1
#define	RS485_CTRL0_L	(rGPHDAT &= 0xfffffffe)		// GPH0=0

static unsigned char sp_icbuf[3][8+2048];
static unsigned char sp_ocbuf[3][8+2048];
static unsigned long overrun_error[3], parity_error[3], frame_error[3], break_detect[3];
static int	inter_tmout[3];
volatile int sio0_state;	// unsigned char 선언하면 Pabort 발생 


int sioPeek(int port)
{
	return cbuf_peek(sp_icbuf[port]);
}

int sioGetc(int port)
{
	int		c;

	if(cbuf_rdlock(sp_icbuf[port])) {
		c = cbuf_getch(sp_icbuf[port]);
		cbuf_rdunlock(sp_icbuf[port]);
	} else c = -1;
	return c;
}

int sioRead(int port, void *buf, int size)
{
	unsigned long 	timer;
	unsigned char	*p;
	int		rval, read_size, timedOut;

	p = (unsigned char *)buf;
	read_size = timedOut = 0;
	if(cbuf_rdlock(sp_icbuf[port])) {
		timer = DS_TIMER;
		while(read_size < size && !timedOut) {
			// once in, finish or timeout
			rval = cbuf_get(sp_icbuf[port], p + read_size, size - read_size);
			if(rval > 0) {
				timer = DS_TIMER;
				read_size += rval;
			} else {
				if(read_size && DS_TIMER >= timer+inter_tmout[port]) timedOut = 1;
				else	taskYield();
			}
		}
		cbuf_rdunlock(sp_icbuf[port]);
	}
	return read_size;
}

int sioReadNoWait(int port, void *buf, int size)
{
	int		rval;

	rval = 0;
	if(cbuf_rdlock(sp_icbuf[port])) {
		rval = cbuf_get(sp_icbuf[port], buf, size);
		cbuf_rdunlock(sp_icbuf[port]);
	}
	return rval;
}

// timeout	-1=Forever  0=No wait  >0=Wait in deci-seconds
int sioReadEx(int port, void *buf, int size, int timeout)
{
	unsigned long 	timer, f_timer;
	unsigned char	*p;
	int		rval, read_size, timedOut;

	p = (unsigned char *)buf;
	read_size = timedOut = 0;
	f_timer = timer = DS_TIMER;
	if(cbuf_rdlock(sp_icbuf[port])) {
		while(read_size < size && !timedOut) {
			rval = cbuf_get(sp_icbuf[port], p + read_size, size - read_size);
			if(rval > 0) {
				timer = DS_TIMER;
				read_size += rval;
			} else {
				if(!read_size && (!timeout || timeout > 0 && DS_TIMER >= f_timer+timeout) || read_size && DS_TIMER >= timer+inter_tmout[port]) timedOut = 1;
				else	taskYield();
			}
		}
		cbuf_rdunlock(sp_icbuf[port]);
	}
	return read_size;
}

#include "defs.h"
#include "prim.h"

int sioReadPacket(int port, void *buf)
{
	unsigned long 	timer;
	unsigned short	length;
	unsigned char	*p, *p1;
	int		rval, size, read_size, timedOut;

	p1 = p = (unsigned char *)buf;
	timedOut = 0;
	size = 9; read_size = 0;
	timer = DS_TIMER;
	if(cbuf_rdlock(sp_icbuf[port])) {
		while(read_size < size && !timedOut) {
			rval = cbuf_get(sp_icbuf[port], p + read_size, size - read_size);
			if(rval > 0) {
				timer = DS_TIMER;
				read_size += rval;
				if(p1 && read_size >= 9) {
					if((p1[0] == ASCII_ACK || p1[0] == ASCII_BS) && p1[1] == ASCII_STX) {
						BYTEtoSHORT(p1+2, &length);
						size = length;
						if(size > MAX_SVRBUF_SZ) size = MAX_SVRBUF_SZ; 
					}
					p1 = NULL;
				}
			} else {
				if(!read_size || read_size && DS_TIMER >= timer+inter_tmout[port]) timedOut = 1;
				else	taskYield();
			}
		}
		cbuf_rdunlock(sp_icbuf[port]);
	}
	return read_size;
}

void sp_starttx(int port)
{
	switch(port) {
	case 0:
		RS485_CTRL0_H;
		rUFCON0 = 0x71;		// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
		SET_IRQFIQ(); rINTSUBMSK &= ~BIT_SUB_TXD0; CLR_IRQFIQ();
		break;
	case 1:
		SET_IRQFIQ(); rINTSUBMSK &= ~BIT_SUB_TXD1; CLR_IRQFIQ();
		break;
	case 2:
		SET_IRQFIQ(); rINTSUBMSK &= ~BIT_SUB_TXD2; CLR_IRQFIQ();
		break;
	}
}

void sp_waittx(int port)
{
	switch(port) {
	case 0:
	    while(rUFSTAT0 & 0x7f00) ; //taskYield();	// Wait until the TX FIFO is empty
	    while(!(rUTRSTAT0 & 0x4)) ; // taskYield();	// Wait until THR & shifter register is empty(for RS485)
		break;
	case 1:
	    while(rUFSTAT1 & 0x7f00) taskYield();	// Wait until the TX FIFO is empty
		break;
	case 2:
	    while(rUFSTAT2 & 0x7f00) taskYield();	// Wait until the TX FIFO is empty
		break;
	}
}

int sioPutc(int port, int c)
{
	if(cbuf_wrlock(sp_ocbuf[port])) {
		while(cbuf_putch(sp_ocbuf[port], c)) ;
		if(port == 0) RS485_CTRL0_H;
		sp_starttx(port);
		do {
			taskYield();
		} while(!cbuf_empty(sp_ocbuf[port]));
		sp_waittx(port);
		if(port == 0) RS485_CTRL0_L;
		cbuf_wrunlock(sp_ocbuf[port]);
	} else	c = 0; 
	return c;
}

int sioWrite(int port, void *buf, int size)
{
	unsigned char	*p;
	int		bytesleft;
	
	p = (unsigned char *)buf;
	bytesleft = size;
	if(cbuf_wrlock(sp_ocbuf[port])) {
		bytesleft -= cbuf_put(sp_ocbuf[port], p+(size-bytesleft), bytesleft);
		sp_starttx(port);
		while(bytesleft) {
			taskYield();
			bytesleft -= cbuf_put(sp_ocbuf[port], p+(size-bytesleft), bytesleft);
		}
		if(port) {	
			do {
				taskYield();
			} while(!cbuf_empty(sp_ocbuf[port]));
			sp_waittx(port);
		}
		cbuf_wrunlock(sp_ocbuf[port]);
	}
	return size;
}

int sioWrite2(int port, void *buf, int size)
{
	unsigned char	*p;
	int		i, c, k;
	
	p = (unsigned char *)buf;
	switch(port) {
	case 0:
		rUFCON0 = 0x71;		// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=(Enable)
		RS485_CTRL0_H;
		for(i = 0;i < size;i++) {
		    while(rUFSTAT0 & 0x4000) ; //taskYield();	//until TX FIFO is not full
			c = *p++;
			WrUTXH0(c);
		}
	    while(rUFSTAT0 & 0x7f00) ; //taskYield();	// Wait until the TX FIFO is empty
	    while(!(rUTRSTAT0 & 0x4)) ; //taskYield();	// Wait until THR & shifter register is empty(for RS485)
		for(k = 0;k < 300;k++) ;	// 300
		RS485_CTRL0_L;
		break;
	case 1:
		for(i = 0;i < size;i++) {
		    while(rUFSTAT1 & 0x4000) ; //taskYield();	//until TX FIFO is not full
			c = *p++;
			WrUTXH1(c);
		}
	    while(rUFSTAT1 & 0x7f00) ; //taskYield();	// Wait until the TX FIFO is empty
		for(k = 0;k < 300;k++) ;	// 300
		break;
	case 2:
		for(i = 0;i < size;i++) {
		    while(rUFSTAT2 & 0x4000) ; //taskYield();	//until TX FIFO is not full
			c = *p++;
			WrUTXH2(c);
		}
	    while(rUFSTAT2 & 0x7f00) ; //taskYield();	// Wait until the TX FIFO is empty
		for(k = 0;k < 300;k++) ;	// 300
		break;
	}
	return size;
}

int SlaveSioWriteBuffer(void *buf, int size)
{
	int		rval;

	rval = 0;
	if(cbuf_wrlock(sp_ocbuf[0])) {
		rval = cbuf_put(sp_ocbuf[0], buf, size);
		cbuf_wrunlock(sp_ocbuf[0]);
	}
	return rval;
}


/*
int sioPutc(int port, int c)
{
	switch(port) {
	case 0:
		RS485_CTRL0_H;
		while(!(rUTRSTAT0 & 0x2));	// Wait until THR is empty.
		WrUTXH0(c);
		while(!(rUTRSTAT0 & 0x4));	// Wait until THR & shifter register is empty.
		RS485_CTRL0_L;
		break;
	case 1:
		while(!(rUTRSTAT1 & 0x2));	// Wait until THR is empty.
		WrUTXH1(c);
		while(!(rUTRSTAT1 & 0x4));	// Wait until THR & shifter register is empty.
		break;
	case 2:
		RS485_CTRL1_H;
		while(!(rUTRSTAT2 & 0x2));	// Wait until THR is empty.
		WrUTXH2(c);
		while(!(rUTRSTAT2 & 0x4));	// Wait until THR & shifter register is empty.
		RS485_CTRL1_L;
		break;
	}
	return c;
}

int sioWrite(int port, void *buf, int size)
{
	char	*p;
	int		i, c;
	
	p = (char *)buf;
	switch(port) {
	case 0:
		RS485_CTRL0_H;
		for(i = 0;i < size;i++) {
			while(!(rUTRSTAT0 & 0x2));	// Wait until THR is empty.
        	c = *p++;
        	WrUTXH0(c);
		}
		while(!(rUTRSTAT0 & 0x4));	// Wait until THR & shifter register is empty.
		RS485_CTRL0_L;
		break;
	case 1:
		for(i = 0;i < size;i++) {
			while(!(rUTRSTAT1 & 0x2));	// Wait until THR is empty.
        	c = *p++;
        	WrUTXH1(c);
		}
		while(!(rUTRSTAT1 & 0x4));	// Wait until THR & shifter register is empty.
		break;
	case 2:
		RS485_CTRL1_H;
		for(i = 0;i < size;i++) {
			while(!(rUTRSTAT2 & 0x2));	// Wait until THR is empty.
        	c = *p++;
        	WrUTXH2(c);
		}
		while(!(rUTRSTAT2 & 0x4));	// Wait until THR & shifter register is empty.
		RS485_CTRL1_L;
		break;
	}
	return size;
}
*/

int sioGetReadFree(int port)
{
	return cbuf_free(sp_icbuf[port]);
}

int sioGetWriteFree(int port)
{
	return (int)cbuf_free(sp_ocbuf[port]);
}

int sioGetReadUsed(int port)
{
	return (int)cbuf_used(sp_icbuf[port]);
}

int sioGetWriteUsed(int port)
{
	return (int)cbuf_used(sp_ocbuf[port]);
}

unsigned long sioGetOverrunError(int port)
{
	return overrun_error[port];
}

unsigned long sioGetParityError(int port)
{
	return parity_error[port];
}

unsigned long sioGetFrameError(int port)
{
	return frame_error[port];
}

unsigned long sioGetBreakDetect(int port)
{
	return break_detect[port];
}

void sioClearError(int port)
{
	overrun_error[port] = parity_error[port] = frame_error[port] = break_detect[port] = 0;
}

void sioClearRead(int port)
{
	cbuf_flush(sp_icbuf[port]);
}

void sioClearWrite(int port)
{
	cbuf_flush(sp_ocbuf[port]);
}

//unsigned long	rxcnt, txcnt;
//volatile int rxcnt, errcnt;

void __irq UART0_ISR(void)
{
	int		c, subsrcpnd, erstat;
	
	c = rSRCPND; c = rINTPND; subsrcpnd = rSUBSRCPND;
	if(subsrcpnd & BIT_SUB_RXD0) {
//WrUTXH2('R');
		while(rUFSTAT0 & 0x007f) {	// During the RX FIFO is not empty
			erstat = rUERSTAT0;
			if(erstat & 0x01) overrun_error[0]++; 
			if(erstat & 0x02) parity_error[0]++; 
			if(erstat & 0x04) frame_error[0]++; 
			if(erstat & 0x08) break_detect[0]++; 
			c = (int)RdURXH0();
			if(erstat < 2) {	// no error, overrun error
				cbuf_putch(sp_icbuf[0], c);
			}
		}
		rSUBSRCPND = BIT_SUB_RXD0;
	}
	if(subsrcpnd & BIT_SUB_TXD0) {
//if(sio0_state == 1) WrUTXH2('S'); else if(sio0_state == 2) WrUTXH2('T'); else WrUTXH2('t');
		if(sio0_state == 1) {
			while(!(rUFSTAT0 & 0x4000)) {	//until TX FIFO is full
				c = cbuf_getch(sp_ocbuf[0]);
				if(c < 0) {
					WrUTXH0(0);		// null padding for RS-485
					rUFCON0 = 0x31;	// Tx FIFO Trigger Level[7:6]=00(Empty) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
					sio0_state = 2;
					break;
				} else {
					WrUTXH0(c);
				}
			}
		} else if(sio0_state == 2) {
			RS485_CTRL0_L;
			rINTSUBMSK |= BIT_SUB_TXD0;
			sio0_state = 0;
		}
		rSUBSRCPND = BIT_SUB_TXD0;
	}
	rSRCPND = BIT_UART0; rINTPND = BIT_UART0;
}

void __irq UART1_ISR(void)
{
	int		c, subsrcpnd, erstat;
	
	c = rSRCPND; c = rINTPND; subsrcpnd = rSUBSRCPND;
	if(subsrcpnd & BIT_SUB_RXD1) {
		while(rUFSTAT1 & 0x007f)	{	// During the RX FIFO is not empty
			erstat = rUERSTAT1;
			if(erstat & 0x01) overrun_error[1]++; 
			if(erstat & 0x02) parity_error[1]++; 
			if(erstat & 0x04) frame_error[1]++; 
			if(erstat & 0x08) break_detect[1]++; 
			c = (int)RdURXH1();
			if(erstat < 2) cbuf_putch(sp_icbuf[1], c);	// no error, overrun error
		}
		rSUBSRCPND = BIT_SUB_RXD1;
	}
	if(subsrcpnd & BIT_SUB_TXD1) {
		if(!(rINTSUBMSK & BIT_SUB_TXD1)) {
			while(!(rUFSTAT1 & 0x4000)) {	//until TX FIFO is full
 	 			c = cbuf_getch(sp_ocbuf[1]);
				if(c < 0) {
					rINTSUBMSK |= BIT_SUB_TXD1;
					break;
				} else	WrUTXH1(c);
			}
		}
		rSUBSRCPND = BIT_SUB_TXD1;
	}
	rSRCPND = BIT_UART1; rINTPND = BIT_UART1;
}

void __irq UART2_ISR(void)
{
	int		c, subsrcpnd, erstat;
	
	c = rSRCPND; c = rINTPND; subsrcpnd = rSUBSRCPND;
	if(subsrcpnd & BIT_SUB_RXD2) {
		while(rUFSTAT2 & 0x007f)	{	// During the RX FIFO is not empty
			erstat = rUERSTAT2;
			if(erstat & 0x01) overrun_error[1]++; 
			if(erstat & 0x02) parity_error[1]++; 
			if(erstat & 0x04) frame_error[1]++; 
			if(erstat & 0x08) break_detect[1]++; 
			c = (int)RdURXH2();
			if(erstat < 2) cbuf_putch(sp_icbuf[2], c);	// no error, overrun error
		}
		rSUBSRCPND = BIT_SUB_RXD2;
	}
	if(subsrcpnd & BIT_SUB_TXD2) {
		if(!(rINTSUBMSK & BIT_SUB_TXD2)) {
			while(!(rUFSTAT2 & 0x4000)) {	//until TX FIFO is full
 	 			c = cbuf_getch(sp_ocbuf[2]);
				if(c < 0) {
					rINTSUBMSK |= BIT_SUB_TXD2;
					break;
				} else	WrUTXH2(c);
			}
		}
		rSUBSRCPND = BIT_SUB_TXD2;
	}
	rSRCPND = BIT_UART2; rINTPND = BIT_UART2;
}

#include "sio_crc.c"
#include "slavenet_sio.c"
#include "slavenet_sin.c"

int sioOpen(int port, long BaudRate, int Mode)
{
	int		i;
	
	// UCONn : Control Register
	//    [10]      [9]     [8]       [7]        [6]      [5]        [4]           [3:2]          [1:0]
	// Clock Sel, Tx Int, Rx Int, Rx Time Out, Rx err, Loop-back, Send break,  Transmit Mode, Receive Mode
	//      0       1       1         1           1        0           0             01            01
	//    PCLK    Level   Pulse     Disable    Generate  Normal      Normal        Interrupt or Polling
	switch(port) {
	case 0:		// UART0
		rGPHCON = (rGPHCON & 0xffffff0c) | 0x000000a1;	// GPH3[7:6]=10(RXD0) GPH2[5:4]=10(TXD0) GPH0[1:0]=01(Output):RS485_CTRL0
		RS485_CTRL0_L;		// RS485 transmit disable
rGPHUP |= 0x0000000c;
		rULCON0 = 0x03;		// Line Control Register: Normal,No parity,1 stop bit,8-bits
		rUCON0  = 0x0385;	// Control Register   * 이전 0x345 로 Rx Timeout interrupt 발생 않음, fix 2011.10.25
		rUFCON0 = 0x71;		// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
		rUMCON0 = 0x00;		// MODEM Control Register: AFC disable
		rUBRDIV0 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1
		overrun_error[0] = parity_error[0] = frame_error[0] = break_detect[0] = 0;
		cbuf_init(sp_icbuf[0], 2047);
		cbuf_init(sp_ocbuf[0], 2047);
		if(Mode) {
			pISR_UART0 = (unsigned)UART0_Slaves_ISR;
			pISR_TIMER1 = (int)Timer1Done;
			rSRCPND = BIT_TIMER1; rINTPND = BIT_TIMER1;
			rSUBSRCPND = BIT_SUB_TXD0 | BIT_SUB_RXD0 | BIT_SUB_ERR0; rSRCPND = BIT_UART0; rINTPND = BIT_UART0;
			SET_IRQFIQ(); rINTMSK &= ~BIT_TIMER1; rINTSUBMSK |= BIT_SUB_RXD0 | BIT_SUB_TXD0; rINTMSK &= ~BIT_UART0; CLR_IRQFIQ();
		} else {
			pISR_UART0 = (unsigned)UART0_ISR;
			rSUBSRCPND = BIT_SUB_TXD0 | BIT_SUB_RXD0 | BIT_SUB_ERR0; rSRCPND = BIT_UART0; rINTPND = BIT_UART0;
			SET_IRQFIQ(); rINTSUBMSK |= BIT_SUB_RXD0 | BIT_SUB_TXD0; rINTMSK &= ~BIT_UART0; CLR_IRQFIQ();
		}
		sio0_state = 0;
		inter_tmout[0] = 3;		// 1
		break;
	case 1:		// UART1
		rGPHCON = (rGPHCON & 0xfffff0ff) | 0x00000a00;	// GPH5[11:10]=10(RXD1) GPH4[9:8]=10(TXD1)
		rULCON1 = 0x03;		// Line Control Register: Normal,No parity,1 stop bit,8-bits
		rUCON1  = 0x03c5;	// Control Register
		rUFCON1 = 0x71;		// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
		rUMCON1 = 0x00;		// MODEM Control Register: AFC disable
		rUBRDIV1 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1
		overrun_error[1] = parity_error[1] = frame_error[1] = break_detect[1] = 0;
		cbuf_init(sp_icbuf[1], 2047);
		cbuf_init(sp_ocbuf[1], 2047);
		pISR_UART1 =(unsigned)UART1_ISR;
		rSUBSRCPND = BIT_SUB_TXD1 | BIT_SUB_RXD1 | BIT_SUB_ERR1; rSRCPND = BIT_UART1; rINTPND = BIT_UART1;
		SET_IRQFIQ(); rINTSUBMSK &= ~BIT_SUB_RXD1; rINTSUBMSK |= BIT_SUB_TXD1; rINTMSK &= ~BIT_UART1; CLR_IRQFIQ();
		inter_tmout[1] = 3;		// 1	
		break;
	case 2:		// UART2
		rGPHCON = (rGPHCON & 0xffff0fff) | 0x0000a000;	// GPH7[15:14]=10(RXD2) GPH6[13:12]=10(TXD2) 
		rULCON2 = 0x03;		// Line Control Register: Normal,No parity,1 stop bit,8-bits
		rUCON2  = 0x03c5;	// Control Register
		rUFCON2 = 0x71;		// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
		rUMCON2 = 0x00;		// MODEM Control Register: AFC disable
		rUBRDIV2 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1	
		overrun_error[2] = parity_error[2] = frame_error[2] = break_detect[2] = 0;
		cbuf_init(sp_icbuf[2], 2047);
		cbuf_init(sp_ocbuf[2], 2047);
		pISR_UART2 =(unsigned)UART2_ISR;
		rSUBSRCPND = BIT_SUB_TXD2 | BIT_SUB_RXD2 | BIT_SUB_ERR2; rSRCPND = BIT_UART2; rINTPND = BIT_UART2;
		SET_IRQFIQ(); rINTSUBMSK &= ~BIT_SUB_RXD2; rINTSUBMSK |= BIT_SUB_TXD2; rINTMSK &= ~BIT_UART2; CLR_IRQFIQ();
		inter_tmout[2] = 3;		// 1	
		break;
	default:
		return -1;
	}		
	for(i = 0;i < 36000;i++) ;
//cprintf("sioOpen(%d,%d) Pclk=%d UBRDIV2=%d\n", port, BaudRate, Pclk, rUBRDIV2);
	return 0; 
}

void sioClose(int port)
{
	switch(port) {
	case 0:
		SET_IRQFIQ(); rINTSUBMSK |= BIT_SUB_TXD0 | BIT_SUB_RXD0; rINTMSK |= BIT_UART0; if(pISR_TIMER1) rINTMSK |= BIT_TIMER1; CLR_IRQFIQ();
		rGPHCON = rGPHCON & 0xffffff0c;	// GPH3[7:6}==90(Input) GPH2[5:4}=00(Input) GPH0[1:0]=00(Input):RS485_CTRL0
		rUFCON0 |= 0x6;		// reset Tx FIFO/Rx FIFO and UERSTAT0
		rSUBSRCPND = BIT_SUB_TXD0 | BIT_SUB_RXD0 | BIT_SUB_ERR0; rSRCPND = BIT_UART0; rINTPND = BIT_UART0;
		break;
	case 1:
		SET_IRQFIQ(); rINTSUBMSK |= BIT_SUB_TXD1 | BIT_SUB_RXD1; rINTMSK |= BIT_UART1; CLR_IRQFIQ();
		rGPHCON = rGPHCON & 0xfffff0ff;	// GPH5[11:10]=00(Input GPH4[9:8]=00(Input)
		break;
	case 2:
		SET_IRQFIQ(); rINTSUBMSK |= BIT_SUB_TXD2 | BIT_SUB_RXD2; rINTMSK |= BIT_UART2; CLR_IRQFIQ();
		rGPHCON = rGPHCON & 0xffff0fff;	// GPH7[15:14]=00(Input) GPH6[13:12]=00(Input)
		break;
	}
}

int sioSetBaudRate(int port, long BaudRate)
{
	switch(port) {
	case 0:		// UART0
		rUBRDIV0 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1	
		break;
	case 1:		// UART1
		rUBRDIV1 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1	
		break;
	case 2:		// UART2
		rUBRDIV2 = (Pclk + (BaudRate << 3)) / (BaudRate << 4) - 1;	// PBCLK / (BaudRate * 16) - 1	
		break;
	default:
		return -1;
	}		
	return 0; 
}

void sioPrintStatistics(int port)
{
#ifdef TCP_MON
//	c1printf("Rx=%u Tx=%u Overrun=%u Parity=%u Frame=%u Break=%u\n", rxcnt, txcnt, overrun_error[0], parity_error[0], frame_error[0], break_detect[0]);
	c1printf("Overrun=%u Parity=%u Frame=%u Break=%u\n", overrun_error[port], parity_error[port], frame_error[port], break_detect[port]);
#endif
cprintf("\nOverrun=%u Parity=%u Frame=%u Break=%u\n", overrun_error[port], parity_error[port], frame_error[port], break_detect[port]);
}
