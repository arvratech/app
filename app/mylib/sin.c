#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/poll.h>
//#include <sys/eventfd.h>
#include "prim.h"
#include "sin.h"

int			sin_errno;


int sinInitTCPClient(void)
{
	int		s;

	s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	return s;
}

int sinInitUDPClient(void)
{
	int		s;

	s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	return s;
}

int sinOpenClient(int s, unsigned char *ipAddr, unsigned short port)
{
	struct sockaddr_in *addr, _addr;
	unsigned long	*lp;
	int		rval, addrlen;

	addr = &_addr;
	addrlen = sizeof(struct sockaddr_in);
	memset(addr, 0, addrlen);
	addr->sin_family	= AF_INET;
	//addr->sin_addr.s_addr = inet_addr(host);
	memcpy(&addr->sin_addr.s_addr, ipAddr, 4);
	addr->sin_port		= htons(port);
	rval = connect(s, (struct sockaddr *)addr, addrlen);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinOpenClient() connect() error");
		return -1;
	}
	return rval;
}

void sinClose(int s)
{
	close(s);
}

int sinWrite(int s, unsigned char *buf, int size)
{
	int		rval;

	rval = send(s, buf, size, 0);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinWrite() send() error");
		return -1;
	}
	return rval;
}

int sinRead(int s, unsigned char *buf, int size)
{
	int		rval;

	rval = recv(s, buf, size, 0);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinRead() recv() error");
		return -1;
	}
	return rval;
}

void sinSetAddr(struct sockaddr *saddr, unsigned char *ipAddr, unsigned short port)
{
	struct sockaddr_in	*addr;

	addr = (struct sockaddr_in *)saddr;
	memset(addr, 0, sizeof(*addr));
	addr->sin_family	= AF_INET;
	if(ipAddr) memcpy(&addr->sin_addr.s_addr, ipAddr, 4);
	addr->sin_port		= htons(port);
}

void sinGetAddr(struct sockaddr *saddr, unsigned char *ipAddr, unsigned short *port)
{
	struct sockaddr_in	*addr;

	addr = (struct sockaddr_in *)saddr;
	if(ipAddr) memcpy(ipAddr, &addr->sin_addr.s_addr, 4);
	if(port) *port = ntohs(addr->sin_port);
}

int sinGetAddress(int s, unsigned char *ipAddr, unsigned short *port)
{
	struct sockaddr_in *addr, _addr;
	unsigned long	*lp;
	int		rval, addrlen;

	addr = &_addr;
	addrlen = sizeof(struct sockaddr_in);
	rval = getsockname(s, (struct sockaddr *)addr, &addrlen);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinGetAddress() getsockname() error");
		return -1;
	}
	if(ipAddr) memcpy(ipAddr, &addr->sin_addr.s_addr, 4);
//printf("ADDR: %s\n", inet_ntoa(addr->sin_addr));
	if(port) *port = ntohs(addr->sin_port);
	return 0;
}

int sinPollRead(int s, unsigned char *buf, int size, int timeout)
{
	struct pollfd	fds[1];
	int		rval;

	fds[0].fd	  = s;
	fds[0].events = POLLIN;
	rval = poll(fds, 1, timeout);
	if(rval > 0) {
		if(fds[0].revents & POLLIN) {
			rval = recv(s, buf, size, 0);
			if(rval < 0) {
				_sinError(SIN_SYSERR, "sinPollRead() recv() error");
			}
		}
	} else if(rval < 0) {
		if(errno == EINTR) {
printf("sinPollRead EINTR...\n");
			rval = 0;
		} else {
			_sinError(SIN_SYSERR, "sinPollRead() poll() error");
		}
	}
	return rval;
}

int sinSendTo(int s, void *buf, int size, unsigned char *ipAddr, unsigned short port)
{
	struct sockaddr_in	_addr;;
	int		rval, addrlen;

	sinSetAddr((struct sockaddr *)&_addr, ipAddr, port);
	addrlen = sizeof(struct sockaddr_in);
	rval = sendto(s, buf, size, 0, &_addr, addrlen);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinSendTo() sendto() error");
		return -1;
	}
	return rval;
}

int sinRecvFrom(int s, void *buf, int size, unsigned char *ipAddr, unsigned short *port)
{
	struct sockaddr_in _addr;
	unsigned long	*lp;
	int		rval, addrlen;

	addrlen = sizeof(struct sockaddr_in);
	rval = recvfrom(s, buf, 1024, 0, (struct sockaddr *)&_addr, &addrlen);
	if(rval < 0) {
		_sinError(SIN_SYSERR, "sinRecvFrom() recvfrom() error");
		return -1;
	}
	sinGetAddr((struct sockaddr *)&_addr, ipAddr, port);
	return rval;
}

int sinPoll(int s, int timeout)
{
	struct pollfd	fds[1];
	int		rval;

	fds[0].fd	  = s;
	fds[0].events = POLLIN;
	rval = poll(fds, 1, timeout);
	if(rval > 0) {
		rval = 0;
		if(fds[0].revents & POLLIN) rval |= 0x01;
	} else if(rval < 0) {
		if(errno == EINTR) {
printf("sinPoll EINTR...\n");
			rval = 0;
		} else {
			_sinError(SIN_SYSERR, "sinPoll() poll() error");
		}
	}
	return rval;
}

int sinPipePoll(int s, int pipe_fd, int timeout)
{
	struct pollfd	fds[2];
	unsigned char	buf[8];
	int		rval;

	fds[0].fd	  = s;
	fds[0].events = POLLIN;
	fds[1].fd	  = pipe_fd;
	fds[1].events = POLLIN;
	rval = poll(fds, 2, timeout);
	if(rval > 0) {
		rval = 0;
		if(fds[0].revents & POLLIN) rval |= 0x01;
		if(fds[1].revents & (POLLIN | POLLOUT)) rval |= 0x02;
		if(rval & 0x02) read(pipe_fd, buf, 1);
	} else if(rval < 0) {
		if(errno == EINTR) {
printf("sinPipePoll EINTR...\n");
			rval = 0;
		} else {
			_sinError(SIN_SYSERR, "sinPipePoll() poll() error");
		}
	}
	return rval;
}

static char	_err_msg[256];

void sinError(char *s)
{
	if(s && s[0]) {
		if(_err_msg[0]) printf("%s: %s\n", s, _err_msg);
		else	printf("%s\n", s);
	} else if(_err_msg[0]) printf("%s\n", _err_msg);
}

char *sinStrError()
{
	return _err_msg;
}

#include	<stdarg.h>

void _sinError(int errnum, ...)
{
	va_list	args;
	char	*p, *fmt;
	int		status, len;

	va_start(args, errnum);
	fmt = va_arg(args, char *);
	vsprintf(_err_msg, fmt, args);
	va_end(args);
	p = _err_msg + strlen(_err_msg);
	sin_errno = errnum;
	if(errnum == SIN_SYSERR) {
		sprintf(p, " %d: ", errno); p += strlen(p);
		strcpy(p, strerror(errno));
    }
}

