#ifndef _SIO_H_
#define _SIO_H_


#define SIO_OVERRUN_ERROR	0x01
#define SIO_PARITY_ERROR	0x02
#define SIO_FRAME_ERROR		0x04
#define SIO_BREAK_DETECT	0x08


int  sioGetc(int port);
int  sioRead(int port, void *buf, int size);
int  sioReadEx(int port, void *buf, int size, int tmout);
int  sioReadNoWait(int port, void *buf, int size);
int  sioReadPacket(int port, void *buf);
int  sioPeek(int port);
int  sioPutc(int port, int c);
int  sioWrite(int port, void *buf, int size);
int  sioWriteNoWait(int port, void *buf, int size);
int  sioWrite2(int port, void *buf, int size);
int	 sioWriteBuffer(int port, void *buf, int size);
int  sioGetReadFree(int port);
int  sioGetWriteFree(int port);
int  sioGetReadUsed(int port);
int  sioGetWriteUsed(int port);
unsigned long sioGetOverrunError(int port);
unsigned long sioGetParityError(int port);
unsigned long sioGetFrameError(int port);
unsigned long sioGetBreakDetect(int port);
void sioClearError(int port);
void sioClearRead(int port);
void sioClearWrite(int port);
int  sioOpen(int port, long BaudRate, int Mode);
void sioClose(int port);
void sioSetMode(int port, int RS485);
int  sioSetBaudRate(int port, long BaudRate);
void sioPrintStatistics(int port);


#endif	/* _SIO_H_ */
