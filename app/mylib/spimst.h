#ifndef _SPIMST_H_
#define _SPIMST_H_


int  spiOpen(void);
void spiClose(void);
int  spiReadInterrupt(unsigned char *buf);
int  spiWriteCommand(int block, int addr, unsigned char *buf, int size);
int  spiReadStatus(int block, int addr, unsigned char *buf, int size);
int  spiWriteRegister(int block, int addr, unsigned char *buf, int size);
int  spiReadRegister(int block, int addr, unsigned char *buf, int size);
int  spiWriteBuffer(int block, int addr, unsigned char *buf, int size);
int  spiReadBuffer(int block, int addr, unsigned char *buf, int size);
void spiWriteTest(void);


#endif
