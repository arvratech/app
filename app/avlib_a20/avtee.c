#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "avtee.h"

void _AvTeePushChain(AV_PORT *port, AV_BUFFER *buffer);
void _AvTeePullChain(AV_PORT *port, AV_BUFFER *buffer);


int avTeeOpen(AV_TEE *tee)
{
	AV_PORT		*sink, *src;
	int		i;

	memset(tee, 0, sizeof(AV_TEE));
	tee->state		= AV_STATE_NULL;
	sink = &tee->_sink;
	sink->parent = tee;
	sink->chain  = _AvTeePullChain;
	for(i = 0;i < MAX_AV_TEE_SRC_SZ;i++) {
		src = &tee->_srcs[i];
		src->parent = tee;
	}
	tee->state = AV_STATE_IDLE;
	return 0;
}

void avTeeClose(AV_TEE *tee)
{
	int		i;

	if(tee->state >= AV_STATE_IDLE) {
		tee->state = AV_STATE_NULL;
	}
}

AV_PORT *avTeeSinkPort(AV_TEE *tee)
{
	return &tee->_sink;
}

AV_PORT *avTeeSrcPort(AV_TEE *tee, int index)
{
	AV_PORT	*port;

	if(index >= 0 && index < MAX_AV_TEE_SRC_SZ) port = &tee->_srcs[index];
	else	port = NULL;
	return port;
}


int avTeeConnect(AV_TEE *tee, int index, AV_PORT *sink)
{
	AV_PORT		*src;

	src = avTeeSrcPort(tee, index);
	if(src) {
		avPortLink(src, sink);
		src->chain = _AvTeePushChain;
	}
}

int avTeeDisconnect(AV_TEE *tee, int index, AV_PORT *sink)
{
	AV_PORT		*src;

	src = avTeeSrcPort(tee, index);
	if(src) avPortUnlink(src, sink);
}

int avTeeStart(AV_TEE *tee)
{
	tee->state = AV_STATE_BUSY;
	return 0;
}

void avTeeStop(AV_TEE *tee)
{
	if(tee->state >= AV_STATE_BUSY) {
		//_AvPlayerStopped(player);
		tee->state = AV_STATE_IDLE;
	}
}

int avTeeState(AV_TEE *tee)
{
	return (int)tee->state;
}

void _AvTeePutFrame(AV_TEE *tee, AV_BUFFER *buffer)
{
}

void _AvTeePushChain(AV_PORT *port, AV_BUFFER *buffer)
{
	AV_TEE		*tee;
	AV_PORT		*src;
	int		i;

	tee = (AV_TEE *)port->parent;
	_AvTeePutFrame(tee, buffer);
	for(i = 0;i < MAX_AV_TEE_SRC_SZ;i++) {
		src = avTeeSrcPort(tee, i);
		avPortPush(src, buffer);
	}
}

void _AvTeePullChain(AV_PORT *port, AV_BUFFER *buffer)
{
	AV_TEE		*tee;
	AV_PORT		*sink;

	tee = (AV_TEE *)port->parent;
	sink = avTeeSinkPort(tee);
	avPortPull(sink, buffer);
	_AvTeePutFrame(tee, buffer);
}

