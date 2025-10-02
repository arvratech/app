#ifndef _SPIM_LIB_H_
#define _SPIM_LIB_H_


int  spimOpen(void);
void spimClose(int fd);
int  spimReadInterrupt(int fd, unsigned char *buf);
int  spimReadInterrupt2(int fd, unsigned char *buf);
int  spimWriteCommand(int fd, int block, int addr, unsigned char *buf, int size);
int  spimReadStatus(int fd, int block, int addr, unsigned char *buf, int size);
int  spimWriteRegister(int fd, int block, int addr, unsigned char *buf, int size);
int  spimReadRegister(int fd, int block, int addr, unsigned char *buf, int size);
int  spimWriteBuffer(int fd, int block, int addr, unsigned char *buf, int size);
int  spimReadBuffer(int fd, int block, int addr, unsigned char *buf, int size);
void spimWriteTest(int fd);


#endif

