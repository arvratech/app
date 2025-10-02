#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <tme.h>
#include <sys/stat.h>
#include "mq.h"


int mqCreate(MQ *mq)
{
    struct mq_attr	attr;
	
	attr.mq_maxmsg	= mq->queueSize;
	attr.mq_msgsize	= mq->msgSize;
	mq->fd = mq_open(mq->name, O_RDWR | O_CREAT,  0666, &attr);
	if(mq->fd < 0) {
		printf("mqCreate: mq_open() error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void mqDelete(MQ *mq)
{
    mq_close(mq->fd);
	mq_unlink(mq->name);
}

int mqOpen(MQ *mq)
{
    struct mq_attr	attr;
	mqd_t	mq;
	
	attr.mq_maxmsg	= mq->queueSize;
	attr.mq_msgsize	= mq->msgSize;
	mq->mq = mq_open(mq->name, O_RDWR, 0666, &attr);
	if(mq->fd < 0) {
		printf("mqOpen: mq_open() error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

void mqClose(MQ *mq)
{
    mq_close(mq->fd);
}

int mgSend(MQ *mq, void *buf)
{
	int		rval;

	rval = mq_send(mq->fd, buf, (size_t)mq->msgSize, (unsigned)1);
	if(rval < 0) {
		printf("mqSend: mq_send() error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int mqReceive(MS *mq, void *buf)
{
	ssize_t		size;

    size = mq_receive(mq->fd, buf, mq->msgSize, NULL);
	if(size < 0) {
		printf("mqReceive: mq_receive() error: %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

