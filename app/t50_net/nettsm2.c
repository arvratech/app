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
unsigned long	_TxNet[4], _RxNet[520];	// 2080
NET_TSM	ReqTsms[MAX_REQ_TSM_SZ], RspTsms[MAX_RSP_TSM_SZ];

#define REQUEST_TIMER		15000
#define OUT_TIMER			15000
#define SEGMENT_TIMER		15000

void _SendNet(NET *net)
{
	int		size;

//netLogTx(net, DevID);
	size = 8 + net->dataLength;
	if(size & 1) size++;
//	if(size <= cbuf_free(sintx)) { 
//		cbuf_put(sintx, (unsigned char *)net, 8);
//		if(size > 8) cbuf_put(sintx, net->Data, size-8);
//	}
}

static void _SendAbort(NET_TSM *tsm, int abortReason)
{
	NET		*net, _net;

	net = &_net;
	net->address	= tsm->address;
	if(tsm->type) net->type = T_ABORT_SRV_PDU;
	else		  net->type = T_ABORT_PDU;
	net->invokeId	= tsm->invokeId;
	net->head0		= (unsigned char)abortReason;
	net->head1		= 0;
	net->dataLength	= 0; 
	_SendNet(net);
	tsm->svcPending	= tsm->pduPending = 0;
	tsm->state		= N_IDLE;
}

void _IdleSendAbort(NET *net)
{
	if(net->type == T_ACK) net->type = T_ABORT_SRV_PDU;
	else	net->type = T_ABORT_PDU;
	net->head0		= (unsigned char)9;
	net->head1		= 0;
	net->dataLength	= 0; 
	_SendNet(net);
}
 
static void _AbortInd(NET_TSM *tsm, int abortReason)
{
	NET		*net;

	net = (NET *)tsm->pduNet;
	net->address	= tsm->address;
	if(tsm->type) net->type = T_ABORT;
	else		  net->type = T_ABORT_SRV;
	net->invokeId	= tsm->invokeId;
	net->head0		= (unsigned char)abortReason;
	net->head1		= 0;
	net->dataLength	= 0; 
tsm->svcPending = 0;
	tsm->pduPending	= 1;
	tsm->state		= N_IDLE;
}

void _ReqTsmProcessSvc(NET_TSM *tsm, NET *net);
void _RspTsmProcessSvc(NET_TSM *tsm, NET *net);
void _ReqTsmProcessPdu(NET_TSM *tsm, NET *net);
void _RspTsmProcessPdu(NET_TSM *tsm, NET *net);

void _TsmProcessSvcTimer(void)
{
	NET_TSM	*tsm;
	NET		*net, _net;
	int		i;

	net = &_net;
	for(i = 0, tsm = ReqTsms;i < MAX_REQ_TSM_SZ;i++, tsm++) {
		if(tsm->svcPending) _ReqTsmProcessSvc(tsm, (NET *)tsm->svcNet);
		else if(tsm->state && (MS_TIMER-tsm->timer) > tsm->timeout) {
			net->invokeId = tsm->invokeId;
			if(tsm->state == N_AWAIT_SEG_REQ || tsm->state == N_AWAIT_ACK) _SendAbort(tsm, 5);
			else	_AbortInd(tsm, 5);
		}
	}
	for(i = 0, tsm = RspTsms;i < MAX_RSP_TSM_SZ;i++, tsm++) {
		if(tsm->svcPending) _RspTsmProcessSvc(tsm, (NET *)tsm->svcNet);
		else if(tsm->state && (MS_TIMER-tsm->timer) > tsm->timeout) {
			net->invokeId = tsm->invokeId;
			if(tsm->state == N_SEG_REQ || tsm->state == N_SEG_RSP) _AbortInd(tsm, 5);
			else	_SendAbort(tsm, 5);
		}
	}
}

#define _ActionSvc(tsm, net, _state, _timeout)	_SendNet(net); tsm->timeout = _timeout; tsm->timer = MS_TIMER; tsm->state = _state
#define _ActionSvcNoTimer(tsm, net, _state)	_SendNet(net); tsm->state = _state

void _ReqTsmProcessSvc(NET_TSM *tsm, NET *net)
{
	net->address = tsm->address;
//cprintf("ReqTSM-svc: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)tsm->state, (int)net->address, (int)net->type, (int)net->invokeId, (int)net->head0, (int)net->head1, (int)net->dataLength); 
	switch(tsm->state) {
	case N_IDLE:
		switch(net->type) {
		case T_REQ:			tsm->serviceChoice = net->head0; _ActionSvc(tsm, net, N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		tsm->serviceChoice = net->head0; _ActionSvc(tsm, net, N_SEG_REQ, SEGMENT_TIMER); break;
		}
		break;
	case N_AWAIT_SEG_REQ:
		switch(net->type) {
		case T_REQ:			_ActionSvc(tsm, net, N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		_ActionSvc(tsm, net, N_SEG_REQ, SEGMENT_TIMER); break;
		case T_ABORT:		_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	case N_AWAIT_ACK:
		switch(net->type) {
		case T_ACK:			_ActionSvc(tsm, net, N_SEG_RSP, SEGMENT_TIMER << 1); break;
		case T_ABORT:		_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	case N_SEG_REQ:
	case N_AWAIT_RSP:
	case N_SEG_RSP:
		switch(net->type) {
		case T_ABORT:		_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	}
	tsm->svcPending = 0;
}

void _RspTsmProcessSvc(NET_TSM *tsm, NET *net)
{
	net->address = tsm->address;
//cprintf("RspTSM-svc: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)tsm->state, (int)net->address, (int)net->type, (int)net->invokeId, (int)net->head0, (int)net->head1, (int)net->dataLength); 
	switch(tsm->state) {
	case N_IDLE:
		break;
	case N_SEG_REQ:
	case N_SEG_RSP:
		switch(net->type) {
		case T_ABORT_SRV:	_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	case N_AWAIT_ACK:
		switch(net->type) {
		case T_ACK_SRV:		_ActionSvc(tsm, net, N_SEG_REQ, SEGMENT_TIMER << 1); break;
		case T_ABORT_SRV:	_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	case N_AWAIT_RSP:
		switch(net->type) {
		case T_RSP_MOR:		_ActionSvc(tsm, net, N_SEG_RSP, SEGMENT_TIMER); break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	case N_AWAIT_SEG_RSP:
		switch(net->type) {
		case T_RSP_MOR:		_ActionSvc(tsm, net, N_SEG_RSP, SEGMENT_TIMER); break;
		case T_RSP:
		case T_ABORT_SRV:	_ActionSvcNoTimer(tsm, net, N_IDLE); break;
		}
		break;
	}
	tsm->svcPending = 0;
}			

#define _ActionPdu(tsm, _state, _timeout)	tsm->timeout = _timeout; tsm->timer = MS_TIMER; tsm->state = _state; tsm->pduPending = 1
#define _ActionPduNoTimer(tsm, _state)	tsm->state = _state; tsm->pduPending = 1

static void _UnexpectedPduReceived(NET_TSM *tsm)
{
	_SendAbort(tsm, 5);		
	_AbortInd(tsm, 5);
}

void _ReqTsmProcessPdu(NET_TSM *tsm, NET *net)
{
	int		state;

	if(tsm) state = tsm->state;
	else	state = N_IDLE;
//cprintf("ReqTSM-Pdu: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)state, (int)net->address, (int)net->type, (int)net->invokeId, (int)net->head0, (int)net->head1, (int)net->dataLength); 
	switch(state) {
	case N_IDLE:
		switch(net->type) {
		case T_RSP_MOR:
		case T_ACK_SRV:		_IdleSendAbort(net); break;
		}
		break;
	case N_AWAIT_SEG_REQ:
		switch(net->type) {
		case T_ABORT_SRV:	_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_AWAIT_ACK:
		switch(net->type) {
		case T_ABORT_SRV:	_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_SEG_REQ:
		switch(net->type) {
		case T_ACK_SRV:		_ActionPdu(tsm, N_AWAIT_SEG_REQ, REQUEST_TIMER); break;
		case T_REJECT:
		case T_ABORT_SRV:	_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_AWAIT_RSP:
		switch(net->type) {
		case T_RSP_MOR:		_ActionPdu(tsm, N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_SEG_RSP:
		switch(net->type) {
		case T_RSP_MOR:		_ActionPdu(tsm, N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_RSP:
		case T_ABORT_SRV:	_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	}
}

void _RspTsmProcessPdu(NET_TSM *tsm, NET *net)
{
	int		state;

	if(tsm) state = tsm->state;
	else	state = N_IDLE;
//cprintf("RspTSM-Pdu: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)state, (int)net->address, (int)net->type, (int)net->invokeId, (int)net->head0, (int)net->head1, (int)net->dataLength); 
	switch(state) {
	case N_IDLE:
		if(tsm) {
			switch(net->type) { 
			case T_REQ:		tsm->serviceChoice = net->head0; _ActionPdu(tsm, N_AWAIT_RSP, OUT_TIMER); break;
			case T_REQ_MOR:	tsm->serviceChoice = net->head0; _ActionPdu(tsm, N_AWAIT_ACK, REQUEST_TIMER); break;
			case T_ABORT:	break;
			default:		_SendAbort(tsm, 9);
			}
		} else {
			switch(net->type) { 
			case T_ABORT:	break;
			default:		_IdleSendAbort(net);
			}			
		}
		break;
	case N_SEG_REQ:
		switch(net->type) {
		case T_REQ:			_ActionPdu(tsm, N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		_ActionPdu(tsm, N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_ABORT:		_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_SEG_RSP:
		switch(net->type) {
		case T_ACK:			_ActionPdu(tsm, N_AWAIT_SEG_RSP, REQUEST_TIMER); break;
		case T_ABORT:		_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	case N_AWAIT_ACK:
	case N_AWAIT_RSP:
	case N_AWAIT_SEG_RSP:
		switch(net->type) {
		case T_ABORT:		_ActionPduNoTimer(tsm, N_IDLE); break;
		default:			_UnexpectedPduReceived(tsm);
		}
		break;
	}
}

