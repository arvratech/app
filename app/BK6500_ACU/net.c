#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_pkt.h"
#include "rtc.h"
#include "prim.h"
#include "cbuf.h"
#include "net.h"

extern unsigned long sintx[];

unsigned char	InvokeID;


int netGetNextInvokeID(void)
{
	InvokeID++;
	if(!InvokeID) InvokeID++;
	return (int)InvokeID;
}

void netCodeLogin(NET *net, unsigned short DevID)
{
	net->Address	= 0;
	net->Type		= T_REQ;
	net->InvokeID	= 0;
	net->Head0		= P_LOGIN;
	net->Head1		= 0;
	SHORTtoBYTE(DevID, net->Data);
	net->DataLength = 2;
}

void netCodeUnconfRequest(NET *net, int ServiceChoice)
{
	net->Address	= 0;
	net->Type		= T_UNCONF_REQ;
	net->InvokeID	= 0;
	net->Head0		= (unsigned char)ServiceChoice;
	net->Head1		= 0;
}

void netCodeConfRequest(NET *net, int ServiceChoice)
{
	net->Address	= 0;
	net->Type		= T_REQ;
	net->Head0		= (unsigned char)ServiceChoice;
	net->Head1		= 0;
}

void netCodeResponse(NET *net)
{
	net->Type		= T_RSP;
	net->Head0		= 0;
	net->Head1		= 0;
}

void netCodeResponseMor(NET *net)
{
	net->Type		= T_RSP_MOR;
	net->Head0		= 0;
	net->Head1		= 0;
}

void netCodeAck(NET *net)
{
	net->Type		= T_ACK;
	net->Head0		= 0;
	net->Head1		= 0;
}

void netCodeAckSrv(NET *net)
{
	net->Type		= T_ACK_SRV;
	net->Head0		= 0;
	net->Head1		= 0;
}

void netCodeError(NET *net, int ErrorClass, int ErrorCode)
{
	net->Type		= T_ERROR;
	net->Head0		= (unsigned char)ErrorClass;
	net->Head1		= (unsigned char)ErrorCode;
}

void netCodeReject(NET *net, int RejectReason)
{
	net->Type		= T_REJECT;
	net->Head0		= (unsigned char)RejectReason;
	net->Head1		= 0;
}

void netCodeAbort(NET *net, int AbortReason)
{
	net->Type		= T_ABORT;
	net->Head0		= (unsigned char)AbortReason;
	net->Head1		= 0;
}

void netCodeAbortSrv(NET *net, int AbortReason)
{
	net->Type		= T_ABORT_SRV;
	net->Head0		= (unsigned char)AbortReason;
	net->Head1		= 0;
}

void netLogTx(NET *net, unsigned short SourceAddress)
{
	int		i;

cprintf("%ld Tx %d-%d %02x-%02x-%02x-%02x", DS_TIMER, (int)SourceAddress, (int)net->Address, (int)net->Type, (int)net->InvokeID, (int)net->Head0 , (int)net->Head1);
	if(net->DataLength) {
		cprintf(" %d=[%02x", (int)net->DataLength, (int)net->Data[0]);
		for(i = 1;i < net->DataLength;i++) {
			cprintf(" %02x", net->Data[i]);
			if(i > 64) {
				cprintf("...");
				break;
			}
		}
		cprintf("]");
	}
	cprintf("\n");
}

void netLogRx(NET *net, unsigned short DestinationAddress)
{
	int		i;

cprintf("%ld Rx %d-%d %02x-%02x-%02x-%02x", DS_TIMER, (int)net->Address, (int)DestinationAddress, (int)net->Type, (int)net->InvokeID, (int)net->Head0 , (int)net->Head1);
	if(net->DataLength) {
		cprintf(" %d=[%02x", (int)net->DataLength, (int)net->Data[0]);
		for(i = 1;i < net->DataLength;i++) {
			cprintf(" %02x", net->Data[i]);
			if(i > 64) {
				cprintf("...");
				break;
			}
		}
		cprintf("]");
	}
	cprintf("\n");
}

void tsmsInit(NET_TSM *tsms, int size, int Type)
{
	NET_TSM	*tsm;
	int		i;
	
	for(i = 0, tsm = tsms;i < size;i++, tsm++) {
		tsm->State	 = 0;		
		tsm->SvcPending = tsm->PduPending =0;
		if(Type) tsm->Type = 1; else tsm->Type = 0;
	}
}

NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net)
{
	NET_TSM	*tsm;
	int		i;
	

//if(tsm->Type) cprintf("ReqTSMs : %d-%d\n", (int)net->Address, (int)net->InvokeID);
	for(i = 0, tsm = tsms;i < size;i++, tsm++) {
//if(tsm->State) cprintf("[%d] S=%d %d-%d\n", i, (int)tsm->State, (int)tsm->Address, (int)tsm->InvokeID);
//else	cprintf("[%d] S=0\n", i);
		if(tsm->State && tsm->Address == net->Address && tsm->InvokeID == net->InvokeID) break;
	}
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsAdd(NET_TSM *tsms, int size, NET *net)
{
	NET_TSM	*tsm;
	int		i;
	
	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(!tsm->State && !tsm->SvcPending && !tsm->PduPending) break;
	if(i >= size) tsm = (NET_TSM *)0;
	else {
		tsm->Address	= net->Address;
		tsm->InvokeID	= net->InvokeID;
		tsm->WrState	= 0;
		tsm->ObjectType	= 0;
		tsm->PropertyID	= 0;
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
		if(tsms[i].PduPending) break;
		i++; if(i >= size) i = 0;
	} while(i != end) ;
	if(!tsms[i].PduPending) i = -1;
	return i;
}

NET_TSM *tsmsFindServiceChoice(NET_TSM *tsms, int size, int ServiceChoice)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if((tsm->State || tsm->SvcPending || tsm->PduPending) && tsm->ServiceChoice == ServiceChoice) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsWritingObject(NET_TSM *tsms, int size, NET_TSM *self_tsm, int ObjectType)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(tsm != self_tsm && (tsm->State || tsm->SvcPending || tsm->PduPending) && (tsm->ServiceChoice == P_WRITE_OBJECT_LIST || tsm->ServiceChoice == P_WRITE_PROPERTY_LIST) && tsm->ObjectType == ObjectType) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

NET_TSM *tsmsWritingFile(NET_TSM *tsms, int size, NET_TSM *self_tsm, int ObjectType)
{
	NET_TSM	*tsm;
	int		i;

	for(i = 0, tsm = tsms;i < size;i++, tsm++)
		if(tsm != self_tsm && (tsm->State || tsm->SvcPending || tsm->PduPending) && tsm->ServiceChoice == P_WRITE_FILE && tsm->ObjectType == ObjectType) break;
	if(i >= size) tsm = (NET_TSM *)0;
	return tsm;
}

int tsmsGetFree(NET_TSM *tsms, int size)
{
	NET_TSM	*tsm;
	int		i, cnt;
	
	for(i = cnt = 0, tsm = tsms;i < size;i++, tsm++)
		if(!tsm->State && !tsm->SvcPending && !tsm->PduPending) cnt++;
	return cnt;
}
