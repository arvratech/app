#ifndef _SVR_NET_H_
#define _SVR_NET_H_


extern unsigned char	SvrNetState;


void SvrSinThreadInit(void *(*ThreadMain)(void *));
void SvrSinInit(void);
int  SvrSinOpen(void);
void SvrSinClose(int s);
void *SvrSinTask(void *arg);
void SvrSinLogRx(unsigned char *buf, int size);
void SvrSinLogTx(unsigned char *buf, int size);


#endif
