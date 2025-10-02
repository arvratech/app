#include "net.h"
#include "sinnet.h"

int		gCommand;
unsigned long	_SvrBuffer[MAX_SVRBUF_QTR_SZ+64];	// Byte alignment for Wiznet5300 16-bit access


int CheckMACAddress(void)
{
	unsigned char	*p, addr[8];
	int		rval;

	p = syscfgGetMACAddress();
	memset(addr, 0, 6); 
	sockGetMACAddress(addr);
	if(n_memcmp(addr, p, 6)) rval = 1;
	else	rval = 0;
#ifdef BK_DEBUG
cprintf("%ld CheckMacAddress=%d\n", SEC_TIMER, rval);
#endif
	return rval;
}

unsigned long	niTimer;
int		niState;
void (*niCallBack)(int State);

// Network Interface Initialize
void niInit(void (*CallBack)())
{
	int		seed;

	memcpy(&seed, syscfgGetMACAddress()+2, 4);
	seed += (sys_cfg->DeviceID << 16) + sys_cfg->DeviceID;
	dhcpOpen(syscfgGetMACAddress(), seed);
	niCallBack = CallBack;
	niTimer = DS_TIMER - 51;
	niState = 0;
}

// Network Interface Task
void niTask(void)
{
	NETWORK_PORT_CFG	*nwp_cfg;
	unsigned char	IPAddr[4];
	int		rval, link, reset;

	nwp_cfg = &sys_cfg->NetworkPort;
	link = sockGetLinkStatus();
	reset = 0;
	switch(niState) {
	case 0:		// Init
		if((DS_TIMER-niTimer) > 50L) {
			rval = sockInit();
			if(!rval) {
				sockSetMACAddress(syscfgGetMACAddress());
				IPSetNull(IPAddr);
				sockSetIfConfig(IPAddr, IPAddr, IPAddr);
				niState = 1;		
			} else {
				niTimer = DS_TIMER;
			}
		}
		break;
	case 1:		// Inititalized and Link Down
		if(link) {
			if(nwp_cfg->TCPIPOption & USE_DHCP) {
				rval = dhcpEnable(nwp_cfg->IPAddress);
				if(!rval) {
					niState = 2;
					if(niCallBack) (*niCallBack)(niState);
				} else {
					niTimer = DS_TIMER;
					niState = 0;
				}
			} else {
				sockSetIfConfig(nwp_cfg->IPAddress, nwp_cfg->SubnetMask, nwp_cfg->GatewayIPAddress);
				niState = 3;
				if(niCallBack) (*niCallBack)(niState);
			}
		}
		break;
	case 2:		// Link Up and DHCP Unbounded
		if(link) {
			rval = dhcpTask();
			if(rval) {
				niState = 3;
				if(niCallBack) (*niCallBack)(niState);
				dhcpGetIPAddress(IPAddr);	
				if(n_memcmp(nwp_cfg->IPAddress, IPAddr, 4)) {	
					memcpy(nwp_cfg->IPAddress, IPAddr, 4);
					syscfgWrite(sys_cfg);
				}
			}
		} else {
			reset = 1;	
		}
		break;
	case 3:		// Data Transfer
		if(link) {
			if(dhcpIsEnabled()) {
				rval = dhcpTask();
				if(rval) {
					dhcpGetIPAddress(IPAddr);	
					if(n_memcmp(nwp_cfg->IPAddress, IPAddr, 4)) {	
						memcpy(nwp_cfg->IPAddress, IPAddr, 4);
						syscfgWrite(sys_cfg);
					}
				} else if(!dhcpIsBound()) {
					niState = 2;
					if(niCallBack) (*niCallBack)(niState);
				}
			}
		} else {
			reset = 1;	
		}
		break;
	}
	if(gRequest & G_TCP_IP_CHANGED) {
		gRequest &= ~G_TCP_IP_CHANGED;
		if(niState > 1) reset = 1;
	}
	if(reset) {
		if(dhcpIsEnabled()) dhcpDisable();
		else	sockResetNetwork();
		niState = 1;
		if(niCallBack) (*niCallBack)(niState);
	}
}

int niGetState(void)
{
	return (int)niState;
}

#define MAX_REQ_TSM_SZ		8
#define MAX_RSP_TSM_SZ		8
#define T_HEART_BEAT		70

NET		*txnet, _txnet, *rxnet, _rxnet;
NET_TSM	ReqTSM[MAX_REQ_TSM_SZ], RspTSM[MAX_RSP_TSM_SZ];		
unsigned long	TcpTimer, TcpIdleTimer;
int		TcpTimeout;
unsigned short	TcpDevID;
unsigned char	TcpTxState, TcpRxState, TcpIdle; 
unsigned char	OldStatus[8];

unsigned char sin_req[8+512], sin_rsp[8+512];
unsigned long sin_reqbuf[16384], sin_rspbuf[16384];


void NetworkInterfaceChanged(int State)
{
//cprintf("NI Changed: %d.....\n", State);
	if(DesktopIconIsVisible()) DesktopDrawNetworkState();	
}

void ctTcpInit(void)
{
	niInit(NetworkInterfaceChanged);
	SetNetworkState(0);
	cbuf_init(sin_req, 511);
	cbuf_init(sin_rsp, 511);
	lbufInit(sin_reqbuf, 65528);	// 8+65528=655364 655364/4=16384
	lbufInit(sin_rspbuf, 65528);	// 8+65528=655364 655364/4=16384
	txnet = &_txnet;
	rxnet = &_rxnet;
	tsmsInit(ReqTSM, MAX_REQ_TSM_SZ);
	tsmsInit(RspTSM, MAX_RSP_TSM_SZ);
}

void _CloseTCP(void);
void _SendHeartBeat(NET *net);
void _ReqServiceTSM(NET *net);
void _RspServiceTSM(NET *net);
void _ReqPDUTSM(NET *net);
void _RspPDUTSM(NET *net);
void _TimerTSM(void);

void ctTcp(void *arg)
{
	unsigned char	buf[8];
	unsigned short	Address;
	int		rval, NewTcpIdle;

	niTask();
	if(niGetState() < 3) {
		if(gNetworkState) {
			gNetworkState = 0;
//cprintf("%d NetworkState=0 NI=%d\n", DS_TIMER, niGetState());
		}
	} else if(gNetworkState == 0) {
		gNetworkState = 1;
		TcpTimer = DS_TIMER; TcpTimeout = 20; TcpTxState = 0;
		gNetworkState = 1;
	}
	switch(gNetworkState) {
	case 1:		// Idle,Connecting
//cprintf("LinkStatus=%d...\n", sockGetLinkStatus());
		switch(TcpTxState) {
		case 0:
			if((DS_TIMER-TcpTimer) > TcpTimeout) {
				rval = sinStartClient(SOCK_SERVER, sys_cfg->ServerIPAddress, sys_cfg->ServerPort);
				if(rval) TcpTimeout = 50;
				else {
					TcpTimeout = 350; TcpTxState = 1;
				}
				TcpTimer = DS_TIMER;
			}
			break;
		case 1:
			rval = sinPollClient(SOCK_SERVER);	// -1:Error 0:Continue 1:Closed 2:Connected
//if(rval < 0) cprintf("%ld Connect error...\n", DS_TIMER);
//else if(rval == 1) cprintf("%ld Connect closed...\n", DS_TIMER);
			if(rval == 0 && (DS_TIMER-TcpTimer) > TcpTimeout) rval = 1;
			if(rval > 1) {
				TcpDevID = sys_cfg->DeviceID;
				SHORTtoBYTE(TcpDevID, buf);
				netCodeLogin(txnet, buf, 2);
				rval = sinWriteNet(SOCK_SERVER, TcpDevID, txnet);
if(rval > 0) netLogTx(txnet, TcpDevID);
				if(rval > 0) TcpTxState = 2;
				else	_CloseTCP();
			} else if(rval) _CloseTCP();
			break;
		case 2:
			rval = sinWriteNetCompleted(SOCK_SERVER);
			if(rval > 0) { TcpTimer = DS_TIMER; TcpTxState = 3;}
			else if(rval < 0) _CloseTCP();
			break;
		case 3:
			rval = sinReadNet(SOCK_SERVER, &Address, rxnet);
			if(rval == 0 && (DS_TIMER-TcpTimer) > 90) rval = -1;
if(rval > 0) netLogRx(rxnet, Address);
			if(rval > 0) {
				if(rxnet->Type == (RESPONSE_PDU << 4) && rxnet->InvokeID == 0) {
cprintf("Login OK\n");
					TcpTxState = TcpRxState = 0; TcpIdle = 0;
					SetNetworkState(3);
				} else {
cprintf("Login Fail\n");
					_CloseTCP();
				}
			} else if(rval < 0) _CloseTCP();
			break;
		}
		break;
	case 3:
		_TimerTSM();
		NewTcpIdle = 0;
		switch(TcpTxState) {
		case 0:
			rval = netGetRequest(txnet);
			if(rval) {
				if(txnet->Type == T_UNCONF_REQUEST) {
					TcpTxState = 1;
				} else if(txnet->Type == T_ABORT_PDU || txnet->Type == T_ABORT_SRV_PDU) {
					txnet->Type &= 0xfd; TcpTxState = 1;
				} else if(txnet->Type == T_CONF_REQUEST || txnet->Type == T_CONF_REQUEST_MOR || txnet->Type == T_ACK || txnet->Type == T_ABORT) {
					_ReqServiceTSM(txnet);
				} else {
					_RspServiceTSM(txnet);
				}
			} else if(sinTxBufferFull(SOCK_SERVER)) NewTcpIdle = 1;;
			break;
		case 1:
			rval = sinWriteNet(SOCK_SERVER, TcpDevID, txnet);
if(rval > 0) netLogTx(txnet, TcpDevID);				
			if(rval > 0) { netFreeRequest(txnet); TcpTxState = 2;}
			else if(rval < 0) _CloseTCP();
			break;
		case 2:
			rval = sinWriteNetCompleted(SOCK_SERVER);
			if(rval > 0) TcpTxState = 0;
			else if(rval < 0) _CloseTCP();
			break;
		}
		switch(TcpRxState) {
		case 0:
			rval = sinReadNet(SOCK_SERVER, &Address, rxnet);
if(rval > 0) netLogRx(rxnet, Address);
			if(rval > 0) {
				if(rxnet->BufferLength) {
					rval = netAllocResponse(rxnet);
					if(rval >= 0) TcpRxState = 1; 
					else	_CloseTCP();
				} else {
					if(rxnet->Type == T_UNCONF_REQUEST) {
cprintf("%lu Rx HeartBeat\n", DS_TIMER);
					} else if(rxnet->Type == T_CONF_REQUEST || rxnet->Type == T_CONF_REQUEST_MOR || rxnet->Type == T_ACK || rxnet->Type == T_ABORT) {
						_RspPDUTSM(rxnet);
					} else {
						_ReqPDUTSM(rxnet);
					}
				}
			} else if(rval < 0) _CloseTCP();
			else if(NewTcpIdle) NewTcpIdle = 2;
			break;
		case 1:
			rval = sinReadNetCompleted(SOCK_SERVER, rxnet);
			if(rval > 0) {
				if(rxnet->Type == T_UNCONF_REQUEST) {
					netPutResponse(rxnet);
				} else if(rxnet->Type == T_CONF_REQUEST || rxnet->Type == T_CONF_REQUEST_MOR || rxnet->Type == T_ACK || rxnet->Type == T_ABORT) {
					_RspPDUTSM(rxnet);
				} else {
					_ReqPDUTSM(rxnet);
				}
				TcpRxState = 0;
			} else if(rval < 0) _CloseTCP();
			else if(NewTcpIdle) NewTcpIdle = 2;
			break;
		}
		if(NewTcpIdle > 1) {
			if(!TcpIdle) TcpIdleTimer = DS_TIMER;
			else if((DS_TIMER-TcpIdleTimer) > T_HEART_BEAT) _SendHeartBeat(txnet);
		}
		if(NewTcpIdle > 1) TcpIdle = 1; else TcpIdle = 0;
		break;
	}
}

void _CloseTCP(void)
{
cprintf("%ld CloseTCP...\n", DS_TIMER);
	sinClose(SOCK_SERVER);
	SetNetworkState(1);
	//if(CheckMACAddress()) ctTcpInit();
	//else	TcpTimer = DS_TIMER + TimeDelay;
//cprintf("%ld CloseTCP...2\n", DS_TIMER);
	TcpTimer = DS_TIMER; TcpTimeout = 70; TcpTxState = 0;
	cbuf_flush(sin_req);
	cbuf_flush(sin_rsp);
	lbufFlush(sin_reqbuf);
	lbufFlush(sin_rspbuf);
}

void _SendHeartBeat(NET *net)
{
	int		rval;

	netCodeHeartBeat(net);
	rval = sinWriteNet(SOCK_SERVER, TcpDevID, net);
if(rval > 0) netLogTx(txnet, TcpDevID);
	if(rval > 0) TcpTxState = 2;
	else	_CloseTCP();
}

#define REQUEST_TIMER		150
#define OUT_TIMER			150
#define SEGMENT_TIMER		150


static void _SendAbort(NET *net, NET_TSM *tsm, int AbortReason)
{
	NET		*tnet, _tnet;

	tnet = &_tnet;
	if(tsm->State > 10) tnet->Type = T_ABORT_SRV_PDU;
	else	tnet->Type = T_ABORT_PDU;
	tnet->InvokeID		= net->InvokeID;
	tnet->Head0			= (unsigned char)AbortReason;
	tnet->Head1			= 0;
	tnet->BufferOffset	= 0;
	tnet->BufferLength	= 0; 
	netPutRequest(tnet);
	tsm->State = 0;
}

static void _IdleSendAbort(NET *net, int SRV, int AbortReason)
{
	NET_TSM	*tsm, _tsm;

	tsm = &_tsm;
	if(SRV) tsm->State = RSP_SEGMENTED_CONF;
	else	tsm->State = REQ_AWAIT_MORE_REQUEST;
	_SendAbort(net, tsm, AbortReason);
}

static void _AbortInd(NET *net, NET_TSM *tsm, int AbortReason)
{
	NET		*tnet, _tnet;

	tnet = &_tnet;
	if(tsm->State > 10) tnet->Type = T_ABORT;
	else	tnet->Type = T_ABORT_SRV;
	tnet->InvokeID		= net->InvokeID;
	tnet->Head0			= (unsigned char)AbortReason;
	tnet->Head1			= 0;
	tnet->BufferOffset	= 0;
	tnet->BufferLength	= 0; 
	netPutResponse(tnet);
	tsm->State = 0;
}

static void _IdleAbortInd(NET *net, int SRV, int AbortReason)
{
	NET_TSM	*tsm, _tsm;

	tsm = &_tsm;
	if(SRV) tsm->State = RSP_SEGMENTED_CONF;
	else	tsm->State = REQ_AWAIT_MORE_REQUEST;
	_AbortInd(net, tsm, AbortReason);
}

static void _UnexpectedPDUReceived(NET *net, NET_TSM *tsm)
{
	if(net->BufferLength) {
		net->Type = 0xff;
		netPutResponse(net);
	}
	_SendAbort(net, tsm, 5);		
	_AbortInd(net, tsm, 5);
}

static void _IdleUnexpectedPDUReceived(NET *net, int SRV, int SendAbort)
{
	if(net->BufferLength) {
		net->Type = 0xff;
		netPutResponse(net);
	}
	if(SendAbort) _IdleSendAbort(net, SRV, 5);
}

void _TimerTSM(void)
{
	NET_TSM	*tsm;
	NET		*net, _net;
	int		i;

	net = &_net;
	for(i = 0, tsm = ReqTSM;i < 8;i++, tsm++)
		if(tsm->State && (DS_TIMER-tsm->Timer) > tsm->Timeout) {
			net->InvokeID = tsm->InvokeID;
			if(tsm->State == REQ_AWAIT_MORE_REQUEST || tsm->State == REQ_AWAIT_CONF) _SendAbort(net, tsm, 5);
			else	_AbortInd(net, tsm, 5);
		}
	for(i = 0, tsm = RspTSM;i < 8;i++, tsm++)
		if(tsm->State && (DS_TIMER-tsm->Timer) > tsm->Timeout) {
			net->InvokeID = tsm->InvokeID;
			if(tsm->State == RSP_SEGMENTED_CONF || tsm->State == RSP_SEGMENTED_RESPONSE) _AbortInd(net, tsm, 5);
			else	_SendAbort(net, tsm, 5);
		}
}

void _ReqServiceTSM(NET *net)
{
	NET_TSM		*tsm;
	int		state;

	tsm = tsmsFind(ReqTSM, MAX_REQ_TSM_SZ, net);
	if(tsm) state = tsm->State; else state = REQ_IDLE;
cprintf("_ReqServiceTSM: S=%d T=%02x\n", state, (int)net->Type); 
	switch(state) {
	case REQ_IDLE:
		switch(net->Type) {
		case T_CONF_REQUEST:
		case T_CONF_REQUEST_MOR:
			tsm = tsmsAdd(ReqTSM, MAX_REQ_TSM_SZ, net);
			if(tsm) {
				TcpTxState = 1;
				if(net->Type == T_CONF_REQUEST) {
					tsm->Timeout = OUT_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_AWAIT_CONFIRMATION;
				} else {
					tsm->Timeout = SEGMENT_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_SEGMENTED_REQUEST;
				}
			} else {
				netFreeRequest(net);
				_IdleAbortInd(net, 0, 5);
			}
			break;
		default:	netFreeRequest(net);
		}
		break;
	case REQ_AWAIT_MORE_REQUEST:
		switch(net->Type) {
		case T_CONF_REQUEST:
		case T_CONF_REQUEST_MOR:
			TcpTxState = 1;
			if(net->Type == T_CONF_REQUEST) {
				tsm->Timeout = OUT_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_AWAIT_CONFIRMATION;
			} else {
				tsm->Timeout = SEGMENT_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_SEGMENTED_REQUEST;
			}
			break;
		case T_ABORT:	TcpTxState = 1; tsm->State = REQ_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	case REQ_AWAIT_CONF:
		switch(net->Type) {
		case T_ACK:
			TcpTxState = 1;
			tsm->Timeout = SEGMENT_TIMER << 1; tsm->Timer = DS_TIMER; tsm->State = REQ_SEGMENTED_CONF;
			break;
		case T_ABORT:	TcpTxState = 1; tsm->State = REQ_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	case REQ_SEGMENTED_REQUEST:
	case REQ_AWAIT_CONFIRMATION:
	case REQ_SEGMENTED_CONF:
		switch(net->Type) {
		case T_ABORT:	TcpTxState = 1; tsm->State = REQ_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	}
}

void _ReqPDUTSM(NET *net)
{
	NET_TSM		*tsm;
	int		state;
	
	tsm = tsmsFind(ReqTSM, MAX_REQ_TSM_SZ, net);
	if(tsm) state = tsm->State; else state = REQ_IDLE;
cprintf("_ReqPDUTSM: S=%d T=%02x\n", state, (int)net->Type); 
	switch(state) {
	case REQ_IDLE:
		switch(net->Type) {
		case T_RESPONSE_MOR:
		case T_ACK_SRV:	  _IdleUnexpectedPDUReceived(net, 0, 1); break;
		default:		  _IdleUnexpectedPDUReceived(net, 0, 0);
		}
		break;
	case REQ_AWAIT_MORE_REQUEST:
		switch(net->Type) {
		case T_REJECT:
		case T_ABORT_SRV: netPutResponse(net); tsm->State = REQ_IDLE; break;
		default:		  _UnexpectedPDUReceived(net, tsm);
		}
		break;
	case REQ_AWAIT_CONF:
		switch(net->Type) {
		case T_ABORT_SRV: netPutResponse(net); tsm->State = REQ_IDLE; break;
		default:		  _UnexpectedPDUReceived(net, tsm);
		}
		break;
	case REQ_SEGMENTED_REQUEST:
		switch(net->Type) {
		case T_ACK_SRV:
			netPutResponse(net);
			tsm->Timer = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_AWAIT_MORE_REQUEST;
			break;
		case T_REJECT:
		case T_ABORT_SRV: netPutResponse(net); tsm->State = REQ_IDLE; break;
		default:		  _UnexpectedPDUReceived(net, tsm);
		}
		break;
	case REQ_AWAIT_CONFIRMATION:
		switch(net->Type) {
		case T_RESPONSE_MOR:
			netPutResponse(net);
			tsm->Timer = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_AWAIT_CONF;
			break;
		case T_RESPONSE:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV: netPutResponse(net); tsm->State = REQ_IDLE; break;
		default:		  _UnexpectedPDUReceived(net, tsm);
		}
		break;
	case REQ_SEGMENTED_CONF:
		switch(net->Type) {
		case T_RESPONSE_MOR:
			netPutResponse(net);
			tsm->Timer = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = REQ_AWAIT_CONF;
			break;
		case T_RESPONSE:
		case T_ABORT_SRV: netPutResponse(net); tsm->State = REQ_IDLE; break;
		default:		  _UnexpectedPDUReceived(net, tsm);
		}
		break;
	}
}

void _RspServiceTSM(NET *net)
{
	NET_TSM		*tsm;
	int		state;

	tsm = tsmsFind(RspTSM, MAX_RSP_TSM_SZ, net);
	if(tsm) state = tsm->State; else state = RSP_IDLE;
cprintf("_RspServiceTSM: S=%d T=%02x\n", state, (int)net->Type); 
	switch(state) {
	case RSP_IDLE:
		netFreeRequest(net);
		break;
	case RSP_SEGMENTED_CONF:
	case RSP_SEGMENTED_RESPONSE:
		switch(net->Type) {
		case T_ABORT_SRV:	TcpTxState = 1; tsm->State = RSP_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	case RSP_AWAIT_CONF:
		switch(net->Type) {
		case T_ACK_SRV:
			TcpTxState = 1;
			tsm->Timeout = SEGMENT_TIMER << 1; tsm->Timer = DS_TIMER; tsm->State = RSP_SEGMENTED_CONF;
			break;
		case T_ABORT_SRV:	TcpTxState = 1; tsm->State = RSP_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	case RSP_AWAIT_RESPONSE:
		switch(net->Type) {
		case T_RESPONSE_MOR:
			TcpTxState = 1;
			tsm->Timeout = SEGMENT_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_SEGMENTED_RESPONSE;
			break;
		case T_RESPONSE:
		case T_ERROR:
		case T_REJECT:
		case T_ABORT_SRV:	TcpTxState = 1; tsm->State = RSP_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	case RSP_AWAIT_MORE_RESPONSE:
		switch(net->Type) {
		case T_RESPONSE_MOR:
			TcpTxState = 1;
			tsm->Timeout = SEGMENT_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_SEGMENTED_RESPONSE;
			break;
		case T_RESPONSE:
		case T_ABORT_SRV: TcpTxState = 1; tsm->State = RSP_IDLE; break;
		default:	netFreeRequest(net);
		}
		break;
	}
}			

void _RspPDUTSM(NET *net)
{
	NET_TSM		*tsm;
	int		state;
	
	tsm = tsmsFind(RspTSM, MAX_RSP_TSM_SZ, net);
	if(tsm) state = tsm->State; else state = RSP_IDLE;
cprintf("_RspODUTSM: S=%d T=%02x\n", state, (int)net->Type); 
	switch(state) {
	case RSP_IDLE:
		switch(net->Type) {
		case T_REQUEST:
			tsm = tsmsAdd(RspTSM, MAX_REQ_TSM_SZ, net);
			if(tsm) {
				netPutResponse(net);
				tsm->Timeout = OUT_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_AWAIT_RESPONSE;
			} else {
				_IdleUnexpectedPDUReceived(net, 1, 0);
			}
			break;
		case T_REQUEST_MOR:
			tsm = tsmsAdd(RspTSM, MAX_RSP_TSM_SZ, net);
			if(tsm) {
				netPutResponse(net);
				tsm->Timeout = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_AWAIT_CONF;
			} else {
				_IdleUnexpectedPDUReceived(net, 1, 1);
			}
			break;
		case T_ABORT:	break;
		default:		_IdleUnexpectedPDUReceived(net, 1, 1);
		}
		break;
	case RSP_SEGMENTED_CONF:
		switch(net->Type) {
		case T_RESPONSE:
			netPutResponse(net);
			tsm->Timeout = OUT_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_AWAIT_RESPONSE;
			break;
		case T_RESPONSE_MOR:
			netPutResponse(net);
			tsm->Timeout = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_AWAIT_CONF;
			break;
		case T_ABORT:	netPutResponse(net); tsm->State = RSP_IDLE; break;
		default:		_UnexpectedPDUReceived(net, tsm);
		}
		break;
	case RSP_SEGMENTED_RESPONSE:
		switch(net->Type) {
		case T_ACK:
			netPutResponse(net);
			tsm->Timeout = REQUEST_TIMER; tsm->Timer = DS_TIMER; tsm->State = RSP_AWAIT_MORE_RESPONSE;
			break;
		case T_ABORT:	netPutResponse(net); tsm->State = RSP_IDLE; break;
		default:		_UnexpectedPDUReceived(net, tsm);
		}
		break;
	case RSP_AWAIT_CONF:
	case RSP_AWAIT_RESPONSE:
	case RSP_AWAIT_MORE_RESPONSE:
		switch(net->Type) {
		case T_ABORT:	netPutResponse(net); tsm->State = RSP_IDLE; break;
		default:		_UnexpectedPDUReceived(net, tsm);
		}
		break;
	}
}	

// gNetworkState 3 => 0  evtFlush();
