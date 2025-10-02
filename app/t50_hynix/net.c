#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_ascii.h"
#include "defs_pkt.h"
#include "rtc.h"
#include "prim.h"
#include "net.h"

extern unsigned long sintx[];

static unsigned char	invokeId;


int netGetNextInvokeId(void)
{
	invokeId++;
	if(!invokeId) invokeId++;
	return (int)invokeId;
}

void netCodeLogin(NET *net, unsigned short devId)
{
	net->address	= 0;
	net->type		= T_REQ;
	net->invokeId	= 0;
	net->head0		= P_LOGIN;
	net->head1		= 0;
	SHORTtoBYTE(devId, net->data);
	net->dataLength = 2;
}

void netCodeUnconfRequest(NET *net, int serviceChoice)
{
	net->address	= 0;
	net->type		= T_UNCONF_REQ;
	net->invokeId	= 0;
	net->head0		= (unsigned char)serviceChoice;
	net->head1		= 0;
}

void netCodeConfRequest(NET *net, int serviceChoice)
{
	net->address	= 0;
	net->type		= T_REQ;
	net->head0		= (unsigned char)serviceChoice;
	net->head1		= 0;
}

void netCodeResponse(NET *net)
{
	net->type		= T_RSP;
	net->head0		= 0;
	net->head1		= 0;
}

void netCodeResponseMor(NET *net)
{
	net->type		= T_RSP_MOR;
	net->head0		= 0;
	net->head1		= 0;
}

void netCodeAck(NET *net)
{
	net->type		= T_ACK;
	net->head0		= 0;
	net->head1		= 0;
}

void netCodeAckSrv(NET *net)
{
	net->type		= T_ACK_SRV;
	net->head0		= 0;
	net->head1		= 0;
}

void netCodeError(NET *net, int errorClass, int errorCode)
{
	net->type		= T_ERROR;
	net->head0		= (unsigned char)errorClass;
	net->head1		= (unsigned char)errorCode;
}

void netCodeReject(NET *net, int rejectReason)
{
	net->type		= T_REJECT;
	net->head0		= (unsigned char)rejectReason;
	net->head1		= 0;
}

void netCodeAbort(NET *net, int abortReason)
{
	net->type		= T_ABORT;
	net->head0		= (unsigned char)abortReason;
	net->head1		= 0;
}

void netCodeAbortSrv(NET *net, int abortReason)
{
	net->type		= T_ABORT_SRV;
	net->head0		= (unsigned char)abortReason;
	net->head1		= 0;
}

void netLogTx(NET *net, unsigned short sourceAddress)
{
	int		i;

printf("%ld Tx %d-%d %02x-%02x-%02x-%02x", MS_TIMER, (int)sourceAddress, (int)net->address, (int)net->type, (int)net->invokeId, (int)net->head0 , (int)net->head1);
	if(net->dataLength) {
		printf(" %d=[%02x", (int)net->dataLength, (int)net->data[0]);
		for(i = 1;i < net->dataLength;i++) {
			printf(" %02x", net->data[i]);
			if(i > 64) {
				printf("...");
				break;
			}
		}
		printf("]");
	}
	printf("\n");
}

void netLogRx(NET *net, unsigned short destinationAddress)
{
	int		i;

printf("%ld Rx %d-%d %02x-%02x-%02x-%02x", MS_TIMER, (int)net->address, (int)destinationAddress, (int)net->type, (int)net->invokeId, (int)net->head0 , (int)net->head1);
	if(net->dataLength) {
		printf(" %d=[%02x", (int)net->dataLength, (int)net->data[0]);
		for(i = 1;i < net->dataLength;i++) {
			printf(" %02x", net->data[i]);
			if(i > 64) {
				printf("...");
				break;
			}
		}
		printf("]");
	}
	printf("\n");
}

void tsmsInit(NET_TSM *tsms, int size, int type)
{
	NET_TSM	*tsm;
	int		i;
	
	for(i = 0, tsm = tsms;i < size;i++, tsm++) {
		tsm->state	 = 0;		
		tsm->svcPending = tsm->pduPending =0;
		if(type) tsm->type = 1; else tsm->type = 0;
	}
}

NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net)
{
	NET_TSM	*tsm;
	int		i;
	

//if(tsm->type) printf("ReqTSMs : %d-%d\n", (int)net->address, (int)net->invokeId);
	for(i = 0, tsm = tsms;i < size;i++, tsm++) {
//if(tsm->state) printf("[%d] S=%d %d-%d\n", i, (int)tsm->state, (int)tsm->address, (int)tsm->invokeId);
//else	printf("[%d] S=0\n", i);
		if(tsm->state && tsm->address == net->address && tsm->invokeId == net->invokeId) break;
	}
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsAdd(NET_TSM *tsms, int size, NET *net)
{
	NET_TSM	*tsm;
	int		i;
	
	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(!tsm->state && !tsm->svcPending && !tsm->pduPending) break;
	if(i >= size) tsm = (NET_TSM *)0;
	else {
		tsm->address	= net->address;
		tsm->invokeId	= net->invokeId;
		tsm->wrState	= 0;
		tsm->objectType	= 0;
		tsm->propertyID	= 0;
	}
	return tsm;
}
 
int tsmsGetNextPending(NET_TSM *tsms, int size, int index)
{
	int		i, end;

	i = index + 1;
	if(i < 0 || i >= size) i = 0;
	end = i;
	do {
		if(tsms[i].pduPending) break;
		i++; if(i >= size) i = 0;
	} while(i != end) ;
	if(!tsms[i].pduPending) i = -1;
	return i;
}

NET_TSM *tsmsFindServiceChoice(NET_TSM *tsms, int size, int serviceChoice)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if((tsm->state || tsm->svcPending || tsm->pduPending) && tsm->serviceChoice == serviceChoice) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsWritingObject(NET_TSM *tsms, int size, NET_TSM *self_tsm, int objectType)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(tsm != self_tsm && (tsm->state || tsm->svcPending || tsm->pduPending) && (tsm->serviceChoice == P_WRITE_OBJECT_LIST || tsm->serviceChoice == P_WRITE_PROPERTY_LIST) && tsm->objectType == objectType) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsWritingFile(NET_TSM *tsms, int size, NET_TSM *self_tsm, int objectType)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(tsm != self_tsm && (tsm->state || tsm->svcPending || tsm->pduPending) && tsm->serviceChoice == P_WRITE_FILE && tsm->objectType == objectType) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

int tsmsGetFree(NET_TSM *tsms, int size)
{
	NET_TSM	*tsm;
	int		i, cnt;
	
	for(i = cnt = 0, tsm = tsms;i < size;i++, tsm++)
		if(!tsm->state && !tsm->svcPending && !tsm->pduPending) cnt++;
	return cnt;
}

