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

NET_TSM		*Tsm;
NET_TSM		reqTsms[MAX_REQ_TSM_SZ], rspTsms[MAX_RSP_TSM_SZ];
int			Tseg, Tout, Treq;


void tsmsInit(NET_TSM *tsms, int size, int type)
{
	NET_TSM		*tsm;
	int		i;
	
	for(i = 0, tsm = tsms;i < size;i++, tsm++) {
		tsm->state = NS_NULL;
		if(type) tsm->type = 1; else tsm->type = 0;
	}
}

NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net)
{
	NET_TSM		*tsm;
	int		i;
	

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

void _TsmSendAbortSvc(NET_TSM *tsm);
void _TsmSendAbortPdu(NET_TSM *tsm);
 
void _ReqTsmProcessSvc(NET_TSM *tsm, unsigned char *buf);
void _RspTsmProcessSvc(NET_TSM *tsm, unsigned char *buf);
void _ReqTsmProcessPdu(NET_TSM *tsm, unsigned char *buf);
void _RspTsmProcessPdu(NET_TSM *tsm, unsigned char *buf);

void _TsmProcessTimer(void)
{
	NET_TSM	*tsm;
	NET		*net, _net;
	int		i;

	net = &_net;
	for(i = 0, tsm = reqTsms;i < MAX_REQ_TSM_SZ;i++, tsm++) {
		if(tsm->state && (MS_TIMER-tsm->timer) > tsm->timeout) {
			net->invokeId = tsm->invokeId;
			if(tsm->state == NS_AWAIT_SEG_REQ || tsm->state == NS_AWAIT_ACK) {
				tsm->abort = 5; tsm->state = NS_IDLE;
				_TsmSendAbortPdu(tsm);
			} else {
				tsm->abort = 5; tsm->state = NS_IDLE;
				_TsmSendAbortSvc(tsm);
			}
		}
	}
	for(i = 0, tsm = rspTsms;i < MAX_RSP_TSM_SZ;i++, tsm++) {
		if(tsm->state && (MS_TIMER-tsm->timer) > tsm->timeout) {
			net->invokeId = tsm->invokeId;
			if(tsm->state == NS_SEG_REQ || tsm->state == NS_SEG_RSP) {
				tsm->abort = 5; tsm->state = NS_IDLE;
				_TsmSendAbortSvc(tsm);
			} else {
				tsm->abort = 5; tsm->state = NS_IDLE;
				_TsmSendAbortPdu(tsm);
			}
		}
	}
}

void _TsmAbortSvc(NET_TSM *tsm, unsigned char *buf, int abortReason)
{
	if(tsm->type) buf[0] = T_ABORT;
	else		  buf[0] = T_ABORT_SRV;
	buf[0] = tsm->invokeId;
	buf[0] = (unsigned char)abortReason;
	buf[0] = 0;
	tsm->length	= 4;
}

void _TsmAbortPdu(NET_TSM *tsm, unsigned char *buf, int abortReason)
{
	if(tsm->type) buf[0] = T_ABORT_SRV_PDU;
	else		  buf[0] = T_ABORT_PDU;
	buf[1] = tsm->invokeId;
	buf[2] = (unsigned char)abortReason;
	buf[3] = 0;
	tsm->length	= 4;
}

void _TsmSendSvc(NET_TSM *tsm, unsigned char *buf)
{ 
	SQ_NODE		*node;

	node = txQueueNodeAlloc(tsm->length+1);
	node->type = 1;
	node->data[0] = tsm->address;
	memcpy(node->data+1, buf, len);
	QUEUE_INSERT_TAIL(&txQueue, &node->queue);
	devSetBufIrq();
}

void _TsmSendAbortSvc(NET_TSM *tsm)
{
	SQ_NODE		*node;
	unsigned char	buf[8];

	_TsmAbortSvc(tsm, buf, (int)tsm->abort);
	node = txQueueNodeAlloc(tsm->length+1);
	node->type = 1;
	node->data[0] = tsm->address;
	memcpy(node->data+1, buf, len);
	QUEUE_INSERT_TAIL(&txQueue, &node->queue);
	devSetBufIrq();
}

void _TsmSendAbortPdu(NET_TSM *tsm)
{
	SQ_NODE		*node;
	unsigned char	buf[8];

	_TsmAbortPdu(tsm, buf, (int)tsm->abort);
	node = rxQueueNodeAlloc(tsm->length+1);
	node->type = 1;
	node->data[0] = tsm->address;
	memcpy(node->data+1, buf, len);
	QUEUE_INSERT_TAIL(&rxQueue, &node->queue);
}

void _TsmActionSvc(NET_TSM *tsm, int state, int timeout)
{
	tsm->timeout = timeout;
	tsm->state = state
}

void _TsmActionPdu(NET_TSM *tsm, int state, int timeout)
{
	tsm->timeout = timeout;
	tsm->state = state
}

void _ReqTsmProcessSvc(NET_TSM *tsm, unsigned char *buf)
{
	int		state, timeout;

	state = -1; tsm->abort = 0;
	switch(tsm->state) {
	case NS_IDLE:
		switch(buf[0]) {
		case T_REQ:			tsm->serviceChoice = buf[2]; state = NS_AWAIT_RSP; timeout = Tout; break;
		case T_REQ_MOR:		tsm->serviceChoice = buf[2]; state = NS_SEG_REQ; timeout = Tseg; break;
		}
		break;
	case NS_AWAIT_SEG_REQ:
		switch(buf[0]) {
		case T_REQ:			state = NS_AWAIT_RSP; timeout = Tout; break;
		case T_REQ_MOR:		state = NS_SEG_REQ; timeout = Tseg; break;
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		}
		break;
	case NS_AWAIT_ACK:
		switch(buf[0]) {
		case T_ACK:			state = NS_SEG_RSP, T; timeout = seg << 1; break;
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		}
		break;
	case NS_SEG_REQ:
	case NS_AWAIT_RSP:
	case NS_SEG_RSP:
		switch(buf[0]) {
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		}
		break;
	}
	if(state >= 0) _TsmActionSvc(tsm, state, timeout);
	else	tsm->length = 0;
}

void _RspTsmProcessSvc(NET_TSM *tsm, unsigned char *buf)
{
	int		state, timeout;

	state = -1; tsm->abort = 0;
	switch(tsm->state) {
	case NS_IDLE:
		break;
	case NS_SEG_REQ:
	case NS_SEG_RSP:
		switch(buf[0]) {
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		}
		break;
	case NS_AWAIT_ACK:
		switch(buf[0]) {
		case T_ACK_SRV:		state = NS_SEG_REQ; timeout = Tseg << 1; break;
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		}
		break;
	case NS_AWAIT_RSP:
		switch(buf[0]) {
		case T_RSP_MOR:		state = NS_SEG_RSP; timeout = Tseg; break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		}
		break;
	case NS_AWAIT_SEG_RSP:
		switch(buf[0]) {
		case T_RSP_MOR:		state = NS_SEG_RSP; timeout = Tseg; break;
		case T_RSP:
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		}
		break;
	}
	if(state >= 0) _TsmActionSvc(tsm, state, timeout);
	else	tsm->length = 0;
}			

static void _UnexpectedPduReceived(NET_TSM *tsm, unsigned char *buf)
{
	_TsmAbortSvc(tsm, buf, 5);
	tsm->abort = 5;
	tsm->state = NS_IDLE;
}

void _ReqTsmProcessPdu(NET_TSM *tsm, unsigned char *buf)
{
	int		state, timeout;

	state = -1; tsm->abort = 0;
	switch(tsm->state) {
	case NS_IDLE:
		switch(buf[0]) {
		case T_RSP_MOR:
		case T_ACK_SRV:		tsm->abort = 7; break;
		}
		break;
	case NS_AWAIT_SEG_REQ:
		switch(buf[0]) {
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_AWAIT_ACK:
		switch(buf[0]) {
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_SEG_REQ:
		switch(buf[0]) {
		case T_ACK_SRV:		state = NS_AWAIT_SEG_REQ; timeout = Treq; break;
		case T_REJECT:
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_AWAIT_RSP:
		switch(buf[0]) {
		case T_RSP_MOR:		state = NS_AWAIT_ACK; timeout = Treq; break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_SEG_RSP:
		switch(buf[0]) {
		case T_RSP_MOR:		state = NS_AWAIT_ACK; timeout = Treq; break;
		case T_RSP:
		case T_ABORT_SRV:	state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	}
	if(state == 99) _UnexpectedPduReceived(tsm, buf);
	else if(state >= 0) _TsmActionPdu(tsm, state, timeout);
	else	tsm->length = 0;
}

void _RspTsmProcessPdu(NET_TSM *tsm, unsigned char *buf)
{
	int		state, timeout;

	state = -1; tsm->abort = 0;
	switch(tsm->state) {
	case NS_IDLE:
		switch(buf[0]) { 
		case T_REQ:			tsm->serviceChoice = buf[2]; state = NS_AWAIT_RSP; timeout = Tout; break;
		case T_REQ_MOR:		tsm->serviceChoice = buf[2]; state = NS_AWAIT_ACK; timeout = Treq; break;
		case T_ABORT:		break;
		default:			tsm->abort = 9;
		}
		break;
	case NS_SEG_REQ:
		switch(buf[0]) {
		case T_REQ:			state = NS_AWAIT_RSP; timeout = Tout; break;
		case T_REQ_MOR:		state = NS_AWAIT_ACK; timeout = Treq; break;
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_SEG_RSP:
		switch(buf[0]) {
		case T_ACK:			state = NS_AWAIT_SEG_RSP; timeout = Treq; break;
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	case NS_AWAIT_ACK:
	case NS_AWAIT_RSP:
	case NS_AWAIT_SEG_RSP:
		switch(buf[0]) {
		case T_ABORT:		state = NS_IDLE; timeout = 0; break;
		default:			state = 99;
		}
		break;
	}
	else if(state == 99) _UnexpectedPduReceived(tsm, buf);
	else if(state >= 0) _TsmActionPdu(tsm, state, timeout);
	else	tsm->length = 0;
}

