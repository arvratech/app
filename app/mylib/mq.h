#ifndef _MQ_H_
#define _MQ_H_


#include <mqueue.h>


typedef struct _MQ {
	char	name[64];
	mqd_t	fd;
	int		queueSize;
	int		msgSize;
} MQ;


int  mqCreate(MQ *mq);
void mqDelete(MQ *mq);
int  mqOpen(MQ *mq);
void mqClose(MQ *mq);
int  mgSend(MQ *mq, void *buf);
int  mgReceive(MQ *mq, void *buf);


#endif


