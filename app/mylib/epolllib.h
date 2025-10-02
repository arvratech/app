#ifndef _EPOLL_LIB_H_
#define _EPOLL_LIB_H_


// epoll context
typedef struct _EPOLL_CONTEXT {
	unsigned char	type;
	unsigned char	reserve[3];
	int				fd;
} EPOLL_CONTEXT;


int  epollOpen(void);
void epllClose(int fd);
int  epollAdd(int fd, EPOLL_CONTEXT *context);
int  epollModReadOnly(int fd, EPOLL_CONTEXT *context);
int  epollModReadWrite(int fd, EPOLL_CONTEXT *context);
int  epollRead(int fd, void *events, int max_events, int timeout);


#endif

