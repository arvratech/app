#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/epoll.h>
#include "epolllib.h"


int epollOpen(void)
{
	int		fd;

	fd = epoll_create(1);
	if(fd < 0) {
		perror("epollOpen: epoll_create1()");
	}
	return  fd;
}

void epollClose(int fd)
{
	close(fd);
}

int epollAdd(int fd, EPOLL_CONTEXT *context)
{
	struct epoll_event ev;
	int		rval;

	ev.events = EPOLLIN;
	ev.data.ptr = context;
	rval = epoll_ctl(fd, EPOLL_CTL_ADD, context->fd, &ev);
	if(rval < 0) {
		perror("epllAdd: epoll_ctl(add)");
	}
	return rval;
}

int epollModReadOnly(int fd, EPOLL_CONTEXT *context)
{
	struct epoll_event ev;
	int		rval;

	ev.events = EPOLLIN;
	ev.data.ptr = context;
	rval = epoll_ctl(fd, EPOLL_CTL_MOD, context->fd, &ev);
	if(rval < 0) {
		perror("epllModReadOnly: epoll_ctl(mod)");
	}
	return rval;
}

int epollModReadWrite(int fd, EPOLL_CONTEXT *context)
{
	struct epoll_event ev;
	int		rval;

	ev.events = EPOLLIN | EPOLLOUT;
	ev.data.ptr = context;
	rval = epoll_ctl(fd, EPOLL_CTL_MOD, context->fd, &ev);
	if(rval < 0) {
		perror("epllModReadWrite: epoll_ctl(mod)");
	}
	return rval;
}

int epollRead(int fd, void *events, int max_events, int timeout)
{
	int		rval;

	rval = epoll_wait(fd, (struct epoll_event *)events, max_events, timeout);
	if(rval < 0) {
		perror("epollread: epoll_wait()");
	}
	return rval;
}

