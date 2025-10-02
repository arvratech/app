#define T_HEART_BEAT		70

#ifndef STAND_ALONE

extern unsigned short SlaveDevID;

SIN_PORT	SinPorts[3];
unsigned short SlaveDevID;
#endif

NET_TSM		*Tsm;
unsigned long	_TxNet[4], _RxNet[520];	// 2080
NET_TSM	ReqTSMs[MAX_REQ_TSM_SZ], RspTSMs[MAX_RSP_TSM_SZ];
SINNET			*sinServer, _sinServer;
unsigned char	ServerNetState;
static unsigned long  ServerSinTimer;
static int		ServerSinTimeout;
static unsigned short DevID, SinRxAddress;
static unsigned char  niOldState, ServerSinTxState, ServerSinRxState, _ServerSinChanged; 
unsigned long	sintx[2+8192];	// 8+32768


void NetworkInterfaceChanged(int niState)
{
	if(DesktopIconsIsVisible()) DesktopDrawServerNetState();	
}

void ServerSinInit(void);

void ctSinInit(void)
{
#ifndef STAND_ALONE
	SIN_PORT	*sinport;
	int		i;
#endif
		
	niInit(NetworkInterfaceChanged);
	niOldState = niGetState();
	ServerSinInit();
#ifndef STAND_ALONE
	for(i = 0, sinport = SinPorts;i < 3;i++, sinport++) {
		sinport->s			= SOCK_SLAVE + i;
		sinport->State		= 0;
		if(i) sinport->Port = 0;	// Tx
		else  sinport->Port = sys_cfg->ServerPort;	// Rx
		sinport->Timer		= TICK_TIMER;
		sinport->Timeout	= 0L;
		sinport->Context	= NULL;
	}
	SlaveDevID = sys_cfg->DeviceID;
#endif
}

void ServerSinTask(SINNET *sinnet);
void ServerSinClose(void);
void _SendHeartBeat(SINNET *sinnet, NET *net);
void _IdleSendAbort(NET *net);

void ctSin(void *arg)
{
#ifndef STAND_ALONE
	SIN_PORT	*sinport;
#endif
	unsigned char	niState;
	int		i, rval;

	niTask();
	niState = niGetState();
	if(niState < 3) {
		if(ServerNetState) ServerSinClose();
#ifndef STAND_ALONE
		for(i = 0, sinport = SinPorts;i < 3;i++, sinport++) 
			if(sinport->State) SlaveSinClose(sinport);
#endif
	} else {
		if(niOldState < 3) {
			ServerSinClose();
#ifndef STAND_ALONE
			for(i = 0, sinport = SinPorts;i < 3;i++, sinport++) {
				sinport->Timer = TICK_TIMER; sinport->Timeout = 0;
			}
#endif
		}
		if(!ServerNetState && (DS_TIMER-ServerSinTimer) > ServerSinTimeout) {
			ServerNetState = 1; ServerSinTxState = 0;
		} else if(ServerNetState >= 2) {
			if(_ServerSinChanged || DevID != sys_cfg->DeviceID) {
				ServerSinClose();
			} else {
				rval = sinnetCheck(sinServer);
				if(rval < -1) {
//cprintf("%ld TCP Timeout\n", DS_TIMER); 
					ServerSinClose();
				} else if(rval == -1 && ServerNetState >= 3 && ServerSinTxState == 0) {
//cprintf("%ld Tx HeartBeat...\n", DS_TIMER);
					_SendHeartBeat(sinServer, (NET *)_TxNet); sinnetSetIdleTimer(sinServer);
					 ServerSinTxState = 2;
				}
			}
		}
#ifndef STAND_ALONE
		for(i = 0, sinport = SinPorts;i < 3;i++, sinport++) 
			if(!sinport->State && (TICK_TIMER-sinport->Timer) > sinport->Timeout) SlaveSinOpen(sinport);
			else if(sinport->State && SlaveDevID != sys_cfg->DeviceID) SlaveSinClose(sinport);
		if(SlaveDevID != sys_cfg->DeviceID) SlaveDevID = sys_cfg->DeviceID;
#endif
	}
	niOldState = niState;
	if(ServerNetState) ServerSinTask(sinServer);
#ifndef STAND_ALONE
	SlaveSinComTask();
	for(i = 0, sinport = SinPorts;i < 3;i++, sinport++) 
		if(sinport->State) {
			if(i) SlaveSinTxTask(sinport);
			else  SlaveSinRxTask(sinport);
		}
#endif
}

void ServerSinConnected(SINNET *sinnet);
void TxnReqSendStatus(NET_TSM *tsm, int ObjType);

void _ReqTSMProcessService(NET_TSM *tsm, NET *net);
void _RspTSMProcessService(NET_TSM *tsm, NET *net);
void _ReqTSMProcessPDU(NET_TSM *tsm, NET *net);
void _RspTSMProcessPDU(NET_TSM *tsm, NET *net);
void _TSMProcessServiceTimer(void);

void ServerSinTask(SINNET *sinnet)
{
	NET			*net;
	int		rval;
	
	switch(ServerNetState) {
	case 1:		// Idle,Connecting
//cprintf("LinkStatus=%d...\n", sockGetLinkStatus());
		switch(ServerSinTxState) {
		case 0:
			_ServerSinChanged = 0;
			rval = sinStartClient(SOCK_SERVER, sys_cfg->ServerIPAddress, sys_cfg->ServerPort);
			if(rval) ServerSinClose();
			else {
				ServerSinTimer = DS_TIMER; ServerSinTimeout = 350; ServerSinTxState = 1;
			}
			break;
		case 1:
			rval = sinPollClient(SOCK_SERVER);	// -1:Error 0:Continue 1:Closed 2:Connected
//if(rval < 0) cprintf("%ld Connect error...\n", DS_TIMER);
//else if(rval == 1) cprintf("%ld Connect closed...\n", DS_TIMER);
//else if(rval > 1) cprintf("%ld Connected\n", DS_TIMER);
			if(rval == 0 && (DS_TIMER-ServerSinTimer) > ServerSinTimeout) rval = 1;
			if(rval > 1) {
				DevID = sys_cfg->DeviceID;
				SetServerNetState(2);
				ServerSinTxState = 0;
				sinnet->FreeSize = 0;
			} else if(rval) ServerSinClose();
			break;
		}
		break;
	case 2:
		switch(ServerSinTxState) {
		case 0:
			net = (NET *)_RxNet;
			netCodeLogin(net, DevID);
			rval = sinnetWrite(sinnet, DevID, net, NULL);
			if(rval) ServerSinTxState = 1;
//if(rval) netLogTx(net, DevID);
			break;
		case 1:
			net = (NET *)_RxNet;
			rval = sinnetWriteCompleted(sinnet, net, NULL);
			if(rval > 0) { ServerSinTimer = DS_TIMER; ServerSinTxState = 2;}
			else if(rval < 0) ServerSinClose();
			break;
		case 2:
			net = (NET *)_RxNet;
			rval = sinnetRead(sinnet, &SinRxAddress, net);
			if(rval == 0 && (DS_TIMER-ServerSinTimer) > 90) rval = -1;
			if(rval > 0) {
//netLogRx(net, SinRxAddress);
				if(net->Type == T_RSP) {
cprintf("%lu svrnet: login OK...\n", DS_TIMER);
					ServerSinConnected(sinnet);
				} else {
cprintf("%lu svrnet: login fail...\n", DS_TIMER);
					ServerSinClose();
				}
			} else if(rval < 0) ServerSinClose();
			break;
		}
		break;
	case 3:
		_TSMProcessServiceTimer();
		switch(ServerSinTxState) {
		case 0:
			net = (NET *)_TxNet;
			rval = cbuf_get(sintx, (unsigned char *)net, 8);
			if(rval) ServerSinTxState = 1;
			break;
		case 1:
			net = (NET *)_TxNet;
			rval = sinnetWrite(sinnet, DevID, net, sintx);
//if(rval) netLogTx(net, DevID);
			if(rval) ServerSinTxState = 2;
			break;
		case 2:
			net = (NET *)_TxNet;
			rval = sinnetWriteCompleted(sinnet, net, sintx);
//if(rval) cprintf("WriteCompleted=%d\n", rval);
			if(rval > 0) ServerSinTxState = 0;
			else if(rval < 0) ServerSinClose();
			break;
		}
		switch(ServerSinRxState) {
		case 0:
			net = (NET *)_RxNet;
			rval = sinnetRead(sinnet, &SinRxAddress, net);
			if(rval > 0) {
				if(net->Type == T_UNCONF_REQ) {
					Tsm = (NET_TSM *)0;
				} else if(net->Type == T_REQ || net->Type == T_REQ_MOR || net->Type == T_ACK || net->Type == T_ABORT) {
					Tsm = tsmsFind(RspTSMs, MAX_RSP_TSM_SZ, net);
					if(!Tsm) Tsm = tsmsAdd(RspTSMs, MAX_RSP_TSM_SZ, net);
					if(Tsm) {
						memcpy(Tsm->PduNet, net, sizeof(NET) + rval - 16);
					}
				//} else if(net->Type == T_RSP || net->Type == T_RSP_MOR || net->Type == T_ERROR || net->Type == T_REJECT || net->Type == T_ACK_SRV || net->Type == T_ABORT_SRV) {
				} else {
					Tsm = tsmsFind(ReqTSMs, MAX_REQ_TSM_SZ, net);
					if(Tsm) memcpy(Tsm->PduNet, net, sizeof(NET)+ rval - 16);
				}
				ServerSinRxState = 1;
			} else if(rval < 0) ServerSinClose();
			break;
		case 1:
			if(Tsm) net = (NET *)Tsm->PduNet;
			else	net = (NET *)_RxNet;
			rval = sinnetReadCompleted(sinnet, net);
			if(rval > 0) {
//netLogRx(net, SinRxAddress);
				if(net->Type == T_UNCONF_REQ) {
//cprintf("%ld svrnets: Rx HeartBeat...\n", DS_TIMER);
				} else {
					if(Tsm) rval = Tsm->Type;
					else if(net->Type == T_REQ || net->Type == T_REQ_MOR || net->Type == T_ACK || net->Type == T_ABORT) rval = 1;
					else	rval = 0;
					if(rval) _RspTSMProcessPDU(Tsm, net);
					else	 _ReqTSMProcessPDU(Tsm, net);
				}
				ServerSinRxState = 0;
			} else if(rval < 0) ServerSinClose();
			break;
		}
		break;
	}
}

void ServerSinInit(void)
{
	sinServer = &_sinServer;
	sinServer->Sock = SOCK_SERVER;
	SetServerNetState(0);
	_ServerSinChanged = 0;
	cbuf_init(sintx, 32767);
	tsmsInit(ReqTSMs, MAX_REQ_TSM_SZ, 0);
	tsmsInit(RspTSMs, MAX_RSP_TSM_SZ, 1);
	Tsm = (NET_TSM *)0;
}

void ServerSinClose(void)
{
	unsigned char	data[4];
	int		val;

cprintf("%ld ServerSinClose: sock=%d state=%d\n", DS_TIMER, SOCK_SERVER, (int)ServerNetState);
	if(ServerNetState) {
		sinClose(SOCK_SERVER);
		SetServerNetState(0);
	}
	ServerSinTimer = DS_TIMER;
	simRand(data, 1);
	val = data[0] & 0x1f; val -= 16;
	ServerSinTimeout = 60 + val;		// 44 .. 76
	cbuf_flush(sintx);
	evtDisableNet();
	evtEnableSync();
	tsmsInit(ReqTSMs, MAX_REQ_TSM_SZ, 0);
	tsmsInit(RspTSMs, MAX_RSP_TSM_SZ, 1);
	Tsm = (NET_TSM *)0;
	_ServerSinChanged = 0;
}

void ServerSinConnected(SINNET *sinnet)
{
	ServerSinTxState = ServerSinRxState = 0;
	sinnetSetIdleTimer(sinnet);
	SetServerNetState(3);
	evtEnableNet();
	evtDisableSync();
	TxnReqSendStatus(NULL, 0);
}

void ServerSinChanged(void)
{
	_ServerSinChanged = 1;
}

void _SendHeartBeat(SINNET *sinnet, NET *net)
{
	net->Address	= 0;
	net->Type		= T_UNCONF_REQ;
	net->InvokeID	= 0;
	net->Head0		= 0xff;
	net->Head1		= 0;
	net->DataLength	= 0;
	sinnetWrite(sinnet, DevID, net, NULL);
}

#define REQUEST_TIMER		150
#define OUT_TIMER			150
#define SEGMENT_TIMER		150

void _SendNet(NET *net)
{
	int		size;

//netLogTx(net, DevID);
	size = 8 + net->DataLength;
	if(size & 1) size++;
	if(size <= cbuf_free(sintx)) { 
		cbuf_put(sintx, (unsigned char *)net, 8);
		if(size > 8) cbuf_put(sintx, net->Data, size-8);
	}
}

static void _SendAbort(NET_TSM *tsm, int AbortReason)
{
	NET		*net, _net;

	net = &_net;
	net->Address	= tsm->Address;
	if(tsm->Type) net->Type = T_ABORT_SRV_PDU;
	else		  net->Type = T_ABORT_PDU;
	net->InvokeID	= tsm->InvokeID;
	net->Head0		= (unsigned char)AbortReason;
	net->Head1		= 0;
	net->DataLength	= 0; 
	_SendNet(net);
	tsm->SvcPending	= tsm->PduPending = 0;
	tsm->State		= N_IDLE;
}

void _IdleSendAbort(NET *net)
{
	if(net->Type == T_ACK) net->Type = T_ABORT_SRV_PDU;
	else	net->Type = T_ABORT_PDU;
	net->Head0		= (unsigned char)9;
	net->Head1		= 0;
	net->DataLength	= 0; 
	_SendNet(net);
}
 
static void _AbortInd(NET_TSM *tsm, int AbortReason)
{
	NET		*net;

	net = (NET *)tsm->PduNet;
	net->Address	= tsm->Address;
	if(tsm->Type) net->Type = T_ABORT;
	else		  net->Type = T_ABORT_SRV;
	net->InvokeID	= tsm->InvokeID;
	net->Head0		= (unsigned char)AbortReason;
	net->Head1		= 0;
	net->DataLength	= 0; 
tsm->SvcPending = 0;
	tsm->PduPending	= 1;
	tsm->State		= N_IDLE;
}

void _TSMProcessServiceTimer(void)
{
	NET_TSM	*tsm;
	NET		*net, _net;
	int		i;

	net = &_net;
	for(i = 0, tsm = ReqTSMs;i < MAX_REQ_TSM_SZ;i++, tsm++) {
		if(tsm->SvcPending) _ReqTSMProcessService(tsm, (NET *)tsm->SvcNet);
		else if(tsm->State && (DS_TIMER-tsm->Timer) > tsm->Timeout) {
			net->InvokeID = tsm->InvokeID;
			if(tsm->State == N_AWAIT_SEG_REQ || tsm->State == N_AWAIT_ACK) _SendAbort(tsm, 5);
			else	_AbortInd(tsm, 5);
		}
	}
	for(i = 0, tsm = RspTSMs;i < MAX_RSP_TSM_SZ;i++, tsm++) {
		if(tsm->SvcPending) _RspTSMProcessService(tsm, (NET *)tsm->SvcNet);
		else if(tsm->State && (DS_TIMER-tsm->Timer) > tsm->Timeout) {
			net->InvokeID = tsm->InvokeID;
			if(tsm->State == N_SEG_REQ || tsm->State == N_SEG_RSP) _AbortInd(tsm, 5);
			else	_SendAbort(tsm, 5);
		}
	}
}

#define _ActionSVC(net, state, timeout)	_SendNet(net); if(timeout) { tsm->Timeout = timeout; tsm->Timer = DS_TIMER; } tsm->State = state

void _ReqTSMProcessService(NET_TSM *tsm, NET *net)
{
	net->Address = tsm->Address;
//cprintf("ReqTSM-Svc: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)tsm->State, (int)net->Address, (int)net->Type, (int)net->InvokeID, (int)net->Head0, (int)net->Head1, (int)net->DataLength); 
	switch(tsm->State) {
	case N_IDLE:
		switch(net->Type) {
		case T_REQ:			tsm->ServiceChoice = net->Head0; _ActionSVC(net, N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		tsm->ServiceChoice = net->Head0; _ActionSVC(net, N_SEG_REQ, SEGMENT_TIMER); break;
		}
		break;
	case N_AWAIT_SEG_REQ:
		switch(net->Type) {
		case T_REQ:			_ActionSVC(net, N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		_ActionSVC(net, N_SEG_REQ, SEGMENT_TIMER); break;
		case T_ABORT:		_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	case N_AWAIT_ACK:
		switch(net->Type) {
		case T_ACK:			_ActionSVC(net, N_SEG_RSP, SEGMENT_TIMER << 1); break;
		case T_ABORT:		_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	case N_SEG_REQ:
	case N_AWAIT_RSP:
	case N_SEG_RSP:
		switch(net->Type) {
		case T_ABORT:		_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	}
	tsm->SvcPending = 0;
}

void _RspTSMProcessService(NET_TSM *tsm, NET *net)
{
	net->Address = tsm->Address;
//cprintf("RspTSM-Svc: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)tsm->State, (int)net->Address, (int)net->Type, (int)net->InvokeID, (int)net->Head0, (int)net->Head1, (int)net->DataLength); 
	switch(tsm->State) {
	case N_IDLE:
		break;
	case N_SEG_REQ:
	case N_SEG_RSP:
		switch(net->Type) {
		case T_ABORT_SRV:	_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	case N_AWAIT_ACK:
		switch(net->Type) {
		case T_ACK_SRV:		_ActionSVC(net, N_SEG_REQ, SEGMENT_TIMER << 1); break;
		case T_ABORT_SRV:	_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	case N_AWAIT_RSP:
		switch(net->Type) {
		case T_RSP_MOR:		_ActionSVC(net, N_SEG_RSP, SEGMENT_TIMER); break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	case N_AWAIT_SEG_RSP:
		switch(net->Type) {
		case T_RSP_MOR:		_ActionSVC(net, N_SEG_RSP, SEGMENT_TIMER); break;
		case T_RSP:
		case T_ABORT_SRV:	_ActionSVC(net, N_IDLE, 0); break;
		}
		break;
	}
	tsm->SvcPending = 0;
}			

#define _ActionPDU(state, timeout)	if(timeout) { tsm->Timeout = timeout; tsm->Timer = DS_TIMER; } tsm->State = state; tsm->PduPending = 1

static void _UnexpectedPDUReceived(NET_TSM *tsm)
{
	_SendAbort(tsm, 5);		
	_AbortInd(tsm, 5);
}

void _ReqTSMProcessPDU(NET_TSM *tsm, NET *net)
{
	int		state;

	if(tsm) state = tsm->State;
	else	state = N_IDLE;
//cprintf("ReqTSM-PDU: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)state, (int)net->Address, (int)net->Type, (int)net->InvokeID, (int)net->Head0, (int)net->Head1, (int)net->DataLength); 
	switch(state) {
	case N_IDLE:
		switch(net->Type) {
		case T_RSP_MOR:
		case T_ACK_SRV:		_IdleSendAbort(net); break;
		}
		break;
	case N_AWAIT_SEG_REQ:
		switch(net->Type) {
		case T_ABORT_SRV:	_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_AWAIT_ACK:
		switch(net->Type) {
		case T_ABORT_SRV:	_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_SEG_REQ:
		switch(net->Type) {
		case T_ACK_SRV:		_ActionPDU(N_AWAIT_SEG_REQ, REQUEST_TIMER); break;
		case T_REJECT:
		case T_ABORT_SRV:	_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_AWAIT_RSP:
		switch(net->Type) {
		case T_RSP_MOR:		_ActionPDU(N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_RSP:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_SEG_RSP:
		switch(net->Type) {
		case T_RSP_MOR:		_ActionPDU(N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_RSP:
		case T_ABORT_SRV:	_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	}
}

void _RspTSMProcessPDU(NET_TSM *tsm, NET *net)
{
	int		state;

	if(tsm) state = tsm->State;
	else	state = N_IDLE;
//cprintf("RspTSM-PDU: S=%d A=%d %02x-%02x-%02x-%02x %d\n", (int)state, (int)net->Address, (int)net->Type, (int)net->InvokeID, (int)net->Head0, (int)net->Head1, (int)net->DataLength); 
	switch(state) {
	case N_IDLE:
		if(tsm) {
			switch(net->Type) { 
			case T_REQ:		tsm->ServiceChoice = net->Head0; _ActionPDU(N_AWAIT_RSP, OUT_TIMER); break;
			case T_REQ_MOR:	tsm->ServiceChoice = net->Head0; _ActionPDU(N_AWAIT_ACK, REQUEST_TIMER); break;
			case T_ABORT:	break;
			default:		_SendAbort(tsm, 9);
			}
		} else {
			switch(net->Type) { 
			case T_ABORT:	break;
			default:		_IdleSendAbort(net);
			}			
		}
		break;
	case N_SEG_REQ:
		switch(net->Type) {
		case T_REQ:			_ActionPDU(N_AWAIT_RSP, OUT_TIMER); break;
		case T_REQ_MOR:		_ActionPDU(N_AWAIT_ACK, REQUEST_TIMER); break;
		case T_ABORT:		_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_SEG_RSP:
		switch(net->Type) {
		case T_ACK:			_ActionPDU(N_AWAIT_SEG_RSP, REQUEST_TIMER); break;
		case T_ABORT:		_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	case N_AWAIT_ACK:
	case N_AWAIT_RSP:
	case N_AWAIT_SEG_RSP:
		switch(net->Type) {
		case T_ABORT:		_ActionPDU(N_IDLE, 0); break;
		default:			_UnexpectedPDUReceived(tsm);
		}
		break;
	}
}	
