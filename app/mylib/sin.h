#ifndef _SIN_H_
#define _SIN_H_

#include <arpa/inet.h>
#include <sys/socket.h>


#define SIN_SYSERR		0xfff7	// GetLastError()
#define SIN_ERROR		0xfff8	// other error
#define SIN_TIMEOUT		0xfff9	// time_out

#define SIN_UDP_ONE	0x01		// connectionless UDP(one connection) 
#define SIN_UDP_ANY	0x02		// connectionless UDP(any connection) 
#define SIN_UDP_BROADCAST 0x04	// connectionless UDP(broadcasting) 

#define SIN_MAXBUF_SZ	8192	// SIN transmit/receive max buffer length


extern int sin_errno;



int  sinInitTCPClient(void);
int  sinInitUDPClient(void);
int  sinOpenClient(int s, unsigned char *ipAddr, unsigned short port);
void sinClose(int s);
int  sinWrite(int s, unsigned char *buf, int size);
int  sinRead(int s, unsigned char *buf, int size);
void sinSetAddr(struct sockaddr *saddr, unsigned char *ipAddr, unsigned short port);
void sinSetAddr(struct sockaddr *saddr, unsigned char *ipAddr, unsigned short port);
int  sinGetAddress(int s, unsigned char *ipAddr, unsigned short *port);
int  sinPollRead(int s, unsigned char *buf, int size, int timeout);
int  sinSendTo(int s, void *buf, int size, unsigned char *ipAddr, unsigned short port);
int  sinRecvFrom(int s, void *buf, int size, unsigned char *ipAddr, unsigned short *port);
int  sinPoll(int s, int timeout);
int  sinPipePoll(int s, int pipe_fd, int timeout);
void sinError(char *s);
char *sinStrError();
void _sinError(int errnum, ...);


#endif
