#ifndef _I2CM_LIB_H_
#define _I2CM_LIB_H_


int  i2cmOpen(void);
void i2cmClose(int fd);
int  i2cmReadInterrupt(int fd, unsigned char *buf);
int  i2cmWriteRegister(int fd, int addr, unsigned char *buf, int size);
int  i2cmReadRegister(int fd, int addr, unsigned char *buf, int size);
int  i2cmWriteCommand(int fd, int addr, unsigned char *buf, int size);
int  i2cmReadStatus(int fd, int addr, unsigned char *buf, int size);
int  i2cmWriteBuffer(int fd, int addr, unsigned char *buf, int size);
int  i2cmReadBuffer(int fd, int addr, unsigned char *buf, int size);


#endif

