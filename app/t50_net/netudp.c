#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "dev.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "acu.h"
#include "evt.h"
#include "uv.h"
#include "netudp.h"


static unsigned char	invokeId;

int netudpGetNextInvokeId(void)
{
	invokeId++;
	if(!invokeId) invokeId++;
	return (int)invokeId;
}

void netudpCodeLogin(NET_UDP *net, int devId, int readerId)
{
	unsigned char	*p;

	net->sourceAddress	= 0;
	net->destinationAddress = 0;
	net->type		= T_REQ;
	net->invokeId	= 0;
	net->head0		= P_LOGIN;
	net->head1		= 0;
	p = net->data;
	SHORTtoBYTE((unsigned short)devId, p);
	p[2] = (unsigned char)readerId;
	net->dataLength = 3;
printf("%u netudpCodeLogin: %03d-%d\n", MS_TIMER, devId, readerId);
}

void netudpCodePing(NET_UDP *net, int devId, int readerId)
{
	unsigned char	*p;

	net->sourceAddress	= 0;
	net->destinationAddress = 0;
	net->type		= T_REQ;
	net->invokeId	= 0;
	net->head0		= P_PING;
	net->head1		= 0;
	p = net->data;
	SHORTtoBYTE((unsigned short)devId, p);
	p[2] = (unsigned char)readerId;
	net->dataLength = 3;
//printf("%u netudpCodePing: %03d-%d\n", MS_TIMER, devId, readerId);
}

void netudpCodeConfRequest(NET_UDP *net, unsigned short sourceAddress, int serviceChoice)
{
	net->sourceAddress		= sourceAddress;
	net->destinationAddress = 0;
	net->type		= T_REQ;
	net->head0		= (unsigned char)serviceChoice;
	net->head1		= 0;
}

void netudpCodeConfRequestMor(NET_UDP *net, unsigned short sourceAddress, int serviceChoice)
{
	net->sourceAddress		= sourceAddress;
	net->destinationAddress = 0;
	net->type		= T_REQ_MOR;
	net->head0		= (unsigned char)serviceChoice;
	net->head1		= 0;
}

void netudpCodeHeartBeat(NET_UDP *net, unsigned short destAddress)
{
	net->sourceAddress		= 0;
	net->destinationAddress = destAddress;
	net->invokeId			= 0;
	net->type				= T_UNCONF_REQ;
	net->head0				= 0xff;
	net->head1				= 0;
	net->dataLength			= 0;
}

void netudpCodeResponse(NET_UDP *net, NET_UDP *rdnet)
{
	net->sourceAddress		= rdnet->destinationAddress;
	net->destinationAddress = rdnet->sourceAddress;
	net->type				= T_RSP;
	net->invokeId			= rdnet->invokeId;
	net->head0				= 0;
	net->head1				= 0;
	net->dataLength			= 0;
}

void netudpCodeError(NET_UDP *net, NET_UDP *rdnet, int errorClass, int errorCode)
{
	net->sourceAddress		= rdnet->destinationAddress;
	net->destinationAddress = rdnet->sourceAddress;
	net->type				= T_ERROR;
	net->invokeId			= rdnet->invokeId;
	net->head0				= (unsigned char)errorClass;
	net->head1				= (unsigned char)errorCode;
	net->dataLength			= 0;
}

void netudpCodeReject(NET_UDP *net, NET_UDP *rdnet, int rejectReason)
{
	net->sourceAddress		= rdnet->destinationAddress;
	net->destinationAddress = rdnet->sourceAddress;
	net->type				= T_REJECT;
	net->invokeId			= rdnet->invokeId;
	net->head0				= (unsigned char)rejectReason;
	net->head1				= 0;
	net->dataLength			= 0;
}

void netudpCodeAbort(NET_UDP *net, NET_UDP *rdnet, int abortReason)
{
	net->sourceAddress		= rdnet->destinationAddress;
	net->destinationAddress = rdnet->sourceAddress;
	net->type				= T_ABORT_SRV;
	net->invokeId			= rdnet->invokeId;
	net->head0				= (unsigned char)abortReason;
	net->head1				= 0;
	net->dataLength			= 0;
}

void netudpLogTx(NET_UDP *net, unsigned char *ipAddr, unsigned short ipPort)
{
	int		i;

printf("%u Tx [%s.%d] %d-%d %02x-%02x-%02x-%02x", MS_TIMER, inet2addr(ipAddr), (int)ipPort, (int)net->sourceAddress, (int)net->destinationAddress, (int)net->type, (int)net->invokeId, (int)net->head0 , (int)net->head1);
	if(net->dataLength) {
		printf(" %d=[%02x", (int)net->dataLength, (int)net->data[0]);
		for(i = 1;i < net->dataLength;i++) {
			if(i > 7) {
				printf("...");
				break;
			}
			printf(" %02x", net->data[i]);
		}
		printf("]");
	}
	printf("\n");
}

void netudpLogRx(NET_UDP *net, unsigned char *ipAddr, unsigned short ipPort)
{
	int		i;

printf("%u Rx [%s.%d] %d-%d %02x-%02x-%02x-%02x", MS_TIMER, inet2addr(ipAddr), (int)ipPort, (int)net->sourceAddress, (int)net->destinationAddress, (int)net->type, (int)net->invokeId, (int)net->head0 , (int)net->head1);
	if(net->dataLength) {
		printf(" %d=[%02x", (int)net->dataLength, (int)net->data[0]);
		for(i = 1;i < net->dataLength;i++) {
			if(i > 7) {
				printf("...");
				break;
			}
			printf(" %02x", net->data[i]);
		}
		printf("]");
	}
	printf("\n");
}

