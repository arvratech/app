#ifndef _AV_PORT_H_
#define _AV_PORT_H_

#include "avstream.h"


typedef struct _AV_PORT {
	unsigned char	mode;
	unsigned char	reserve[3];
	void			*parent;
	struct _AV_PORT *peer;
	void			(*chain)(struct _AV_PORT *port, AV_BUFFER *buffer);
	void			(*onStopped)(struct _AV_PORT *port);
} AV_PORT;


void *avPortParent(AV_PORT *port);
AV_STREAM *avPortStream(AV_PORT *port);
AV_PORT *avPortPeer(AV_PORT *port);
void *avPortPeerParent(AV_PORT *port);
AV_STREAM *avPortPeerStream(AV_PORT *port);
void avPortLink(AV_PORT *port, AV_PORT *sink);
void avPortUnlink(AV_PORT *port, AV_PORT *sink);
int  avPortPush(AV_PORT *port, AV_BUFFER *buffer);
int  avPortPull(AV_PORT *port, AV_BUFFER *buffer);
int  avPortPullOnStopped(AV_PORT *port);
void avPortChain(AV_PORT *port, AV_BUFFER *buffer);
void avPortOnStopped(AV_PORT *port);


#endif

