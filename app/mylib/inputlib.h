#ifndef _INPUT_LIB_H_
#define _INPUT_LIB_H_


int  pipeOpen(int *fds);
void pipeClose(int *fds);
int  pipeWrite(int fd, unsigned char *buf, int length);
int  pipeRead(int fd, unsigned char *buf);

#ifdef _A20
int  iowmEventOpen(void);
void iowmEventClose(int fd);
void iowmEventReadEvent(int fd);
int  iowmEventRead(int fd);
#else
int  slvmEventOpen(void);
void slvmEventClose(int fd);
void slvmEventReadEvent(int fd);
int  slvmEventRead(int fd);
#endif

int  blemEventOpen(void);
void blemEventClose(int fd);
void blemEventReadEvent(int fd);
int  blemEventRead(int fd);
int  blemEventReadReady(int fd);

int  scEventOpen(void);
void scEventClose(int fd);
void scEventReadEvent(int fd);
int  scEventRead(int fd);

int  tsOpen(void);
void tsClose(int fd);
void tsReset(int fd);
int  tsRead(int fd, unsigned char *buf);

int  wdOpen(void);
void wdClose(void);
void wdPing(void);


#endif

