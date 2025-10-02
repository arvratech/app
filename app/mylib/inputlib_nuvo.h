#ifndef _INPUT_LIB_H_
#define _INPUT_LIB_H_


int  pipeOpen(int *fds);

int  spievOpen(void);
void spievClose(int fd);
int  spievRead(int fd);
void spievReadEvent(int fd);

int  tsOpen(void);
void tsClose(int fd);
void tsReset(int fd);
int  tsRead(int fd, unsigned char *buf);

int  wdOpen(void);
void wdClose(void);
void wdPing(void);


#endif

