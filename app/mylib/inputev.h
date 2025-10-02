#ifndef _INPUT_EV_H_
#define _INPUT_EV_H_


int  pipeOpen(int *fds);
int  spievOpen(void);
void spievClose(int fd);
int  spievRead(int fd);
int  tsOpen(void);
void tsClose(int fd);
int  tsRead(int fd, unsigned char *buf);
int  inputevPoll(int pife_fd, int spi_fd, int ts_fd, unsigned char *buf);


#endif
