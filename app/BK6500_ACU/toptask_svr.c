
extern NET_TSM	ReqTSMs[], RspTSMs[];

int		ReqIndex, RspIndex;

void ctSvrInit(void)
{
	ReqIndex = RspIndex = -1;
}

void _TxnReqReceived(NET_TSM *tsm);
void _TxnRspReceived(NET_TSM *tsm);
void _TxnReqSendEvent(NET_TSM *tsm);
void TxnReqSendStatus(NET_TSM *tsm, int ObjType);

void ctSvr(void *arg)
{
	NET_TSM		*tsm;
	NET			*net, _net;

	RspIndex = tsmsGetNextPending(RspTSMs, MAX_RSP_TSM_SZ, RspIndex);
	if(RspIndex >= 0) {
		tsm = &RspTSMs[RspIndex];
		_TxnRspReceived(tsm);
	}
	ReqIndex = tsmsGetNextPending(ReqTSMs, MAX_REQ_TSM_SZ, ReqIndex);
	if(ReqIndex >= 0) {
		tsm = &ReqTSMs[ReqIndex];
		_TxnReqReceived(tsm);
	}
	if(ServerNetState >= 3) {
		if(!evtIsNetPending() && fsEventSize() > 0) {
			net = &_net;
			net->Address  = 0;
			net->InvokeID = netGetNextInvokeID(); 
			net->Head0	  = 0;
//cprintf("tsmsFree: %d %d\n", tsmsGetFree(ReqTSMs, MAX_REQ_TSM_SZ), tsmsGetFree(RspTSMs, MAX_RSP_TSM_SZ));
			tsm = tsmsAdd(ReqTSMs, MAX_REQ_TSM_SZ, net);
			if(tsm) {
				_TxnReqSendEvent(tsm);
			}
		}
	}
}

void _TxnReqReceived(NET_TSM *tsm)
{
	NET		*net;

	net = (NET *)tsm->PduNet;
//cprintf("%ld TxnReqReceived: Type=%02x ServiceChoice=%02x\n", DS_TIMER, (int)net->Type, (int)tsm->ServiceChoice);
	switch(net->Type) {
	case T_RSP:
		if(tsm->ServiceChoice == P_EVENT_NOTIFICATION) {
			evtClear();
cprintf("%lu svrnet: event OK: %d\n", DS_TIMER, (int)net->InvokeID); 
		} else if(tsm->ServiceChoice == P_STATUS_NOTIFICATION) {
cprintf("%lu svrnet: status OK: %d\n", DS_TIMER, (int)net->InvokeID);
			TxnReqSendStatus(tsm, (int)tsm->ObjectType);
		}
		break;
	case T_ERROR:
		if(tsm->ServiceChoice == P_EVENT_NOTIFICATION) {
			evtClearNetPending();
cprintf("%lu svrnet: event error: %d %d-%d\n", DS_TIMER, (int)net->InvokeID, (int)net->Head0, (int)net->Head1);
		}
		break;
	case T_REJECT:
		if(tsm->ServiceChoice == P_EVENT_NOTIFICATION) {
			evtClearNetPending();
cprintf("%lu svrnet: event reject: %d %d\n", DS_TIMER, (int)net->InvokeID, (int)net->Head0);
		}
		break;
	case T_ABORT_SRV:
		if(tsm->ServiceChoice == P_EVENT_NOTIFICATION) {
			evtClearNetPending();
cprintf("%lu svrnet: event abort: %d %d\n", DS_TIMER, (int)net->InvokeID, (int)net->Head0);
		}
		break;
	default:
cprintf("%lu svrnet: req-%d %d s=%02x unexpected:%02x\n", DS_TIMER, (int)tsm->InvokeID, (int)tsm->State, (int)tsm->ServiceChoice, (int)net->Type);
	}
	tsm->PduPending = 0;
}

void _TxnRspReceived(NET_TSM *tsm)
{
	NET		*net;

	net = (NET *)tsm->PduNet;
	switch(net->Type) {
	case T_REQ:
		SvrProcessConfRequest(tsm, net);
		break;
	case T_REQ_MOR:
		SvrProcessConfRequestMor(tsm, net);
		break;
	case T_ACK:
		SvrProcessAck(tsm, net);
		break;
	case T_ABORT:
		break;
	}
	tsm->PduPending = 0;
}

void _TxnReqSendEvent(NET_TSM *tsm)
{
	NET		*net;
	int		len;

	net = (NET *)tsm->SvcNet;
	len = evtRead(net->Data);
	net->DataLength = len;
	netCodeConfRequest(net, P_EVENT_NOTIFICATION);
	net->InvokeID = tsm->InvokeID;
	tsm->SvcPending = 1;
}

int _GetNextObjectType(int ObjectType)
{
	int		ObjType;

	switch(ObjectType) {
#ifndef STAND_ALONE
	case OT_DEVICE:				ObjType = OT_AC_UNIT; break;
	case OT_AC_UNIT:			ObjType = OT_IO_UNIT; break;
	case OT_IO_UNIT:			ObjType = OT_CREDENTIAL_READER; break;
#else
	case OT_DEVICE:				ObjType = OT_CREDENTIAL_READER; break;
#endif
	case OT_CREDENTIAL_READER:	ObjType = OT_ACCESS_DOOR; break;
	case OT_ACCESS_DOOR:		ObjType = OT_BINARY_INPUT; break;
	case OT_BINARY_INPUT:		ObjType = OT_BINARY_OUTPUT; break;
	case OT_BINARY_OUTPUT:		ObjType = OT_FIRE_ZONE; break;
	case OT_FIRE_ZONE:			ObjType = 0; break;
	default:					ObjType = OT_DEVICE;
	}
	return ObjType;
}

extern int	gIOStatus;

int devEncodeStatus(unsigned char *Buffer)
{
	unsigned char	*p, c;
	
	p = Buffer;
	*p++ = OT_DEVICE;
	if(!(sys_cfg->Device.DeviceOption & TAMPER)) c = 0x00;
	else if(gIOStatus & G_TAMPER) c = 0x02;
	else	c = 0x01;
	*p++ = c;
	*p++ = 0;
	*p++ = 0;
	return p - Buffer;
}

void TxnReqSendStatus(NET_TSM *tsm, int ObjType)
{
	NET		*net, _net;
	unsigned char	*p;
	int		size;
	
	if(tsm) {
		net = (NET *)tsm->SvcNet;
		net->InvokeID = netGetNextInvokeID();
		tsm->InvokeID = net->InvokeID;
	} else {
		net = &_net;
		net->Address  = 0;
		net->InvokeID = netGetNextInvokeID(); 
		net->Head0	  = 0;
		tsm = tsmsAdd(ReqTSMs, MAX_REQ_TSM_SZ, net);
		net = (NET *)tsm->SvcNet;
		net->Address = 0;
		net->InvokeID = tsm->InvokeID;
	}
	p = net->Data;
	while(1) {
		ObjType = _GetNextObjectType(ObjType);
		if(!ObjType) break;
		switch(ObjType) {
		case OT_DEVICE:				size = devEncodeStatus(p); break;
#ifndef STAND_ALONE
		case OT_AC_UNIT:			size = acunitsEncodeStatus(p); break;
		case OT_IO_UNIT:			size = iounitsEncodeStatus(p); break;
#endif
		case OT_CREDENTIAL_READER:	size = crsEncodeStatus(p); break;
		case OT_ACCESS_DOOR:		size = adsEncodeStatus(p); break;
		case OT_BINARY_INPUT:		size = bisEncodeStatus(p); break;
		case OT_BINARY_OUTPUT:		size = bosEncodeStatus(p); break;
		case OT_FIRE_ZONE:			size = fzsEncodeStatus(p); break;
		}
		if(size > 1) {
			net->DataLength = size;
			netCodeConfRequest(net, P_STATUS_NOTIFICATION);
			tsm->ObjectType = ObjType;
			tsm->SvcPending = 1;
			break;
		}
	}
}