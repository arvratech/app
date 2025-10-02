#ifndef _SIO_CRED_H_
#define _SIO_CRED_H_


int  sioOpen(long baudRate);
void sioClose(int fd);
void sioSetSpeed(int fd, long baudRate);
void sioOnRead(int fd, unsigned char *buf, int length);
 

#endif

