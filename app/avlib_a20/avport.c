#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <limits.h>
#include "avstream.h"
#include "avport.h"


void *avPortParent(AV_PORT *port)
{
	return port->parent;
}

AV_STREAM *avPortStream(AV_PORT *port)
{
	return (AV_STREAM *)port->parent;
}

AV_PORT *avPortPeer(AV_PORT *port)
{
	return port->peer;
}

void *avPortPeerParent(AV_PORT *port)
{
	if(!port->peer) return (void *)0;
	else	return port->peer->parent;
}

AV_STREAM *avPortPeerStream(AV_PORT *port)
{
	if(!port->peer) return (AV_STREAM *)0;
	else	return (AV_STREAM *)port->peer->parent;
}

void avPortLink(AV_PORT *port, AV_PORT *sink)
{
	port->peer = sink;
	sink->peer = port;
}

void avPortUnlink(AV_PORT *port, AV_PORT *sink)
{
	port->peer = NULL;
	sink->peer = NULL;
}

int avPortPush(AV_PORT *port, AV_BUFFER *buffer)
{
	AV_PORT		*sink;

	sink = port->peer;
	if(sink && sink->chain) {
		(*sink->chain)(sink, buffer);
	}
	return 0;
}

int avPortPull(AV_PORT *port, AV_BUFFER *buffer)
{
	AV_PORT		*src;

	src = port->peer;
	if(src && src->chain) {
		(*src->chain)(src, buffer);
	}
	return 0;
}

int avPortPullOnStopped(AV_PORT *port)
{
	AV_PORT		*src;

	src = port->peer;
	if(src->onStopped) {
		(*src->onStopped)(src);
	}
	return 0;
}

void avPortChain(AV_PORT *port, AV_BUFFER *buffer)
{
	if(port->chain) {
		(*port->chain)(port, buffer);
	}
}

void avPortOnStopped(AV_PORT *port)
{
	if(port->onStopped) {
		(*port->onStopped)(port);
	}
}

