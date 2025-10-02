#define MAX_REQ_TXN_SZ		1
#define MAX_RSP_TXN_SZ		8

SVR_TXN	ReqTxns[];
SVR_TXN	RspTxns[8];

int	EvtInvokeID, EvtFileSize;

void ctSvrInit(void)
{
	txnsInit(ReqTxns, MAX_REQ_TXN_SZ);
	txnsInit(RspTxns, MAX_RSP_TXN_SZ);
}

void _ReqTxnReceived(NET *net);
void _RspTxnReceived(NET *net);
void _TxnSendEvent(SVR_TXN *txn);

void ctSvr(void *arg)
{
	NET		*net, _net;
	SVR_TXN	*txn;
	int		rval;

	net = &_net;
	rval = netGetResponse(net);
	if(rval > 0) {
		if(net->Type == 0xff) {
			netFreeResponse(net);
		} else if(T_UNCONF_REQUEST) {
			netFreeResponse(net);
		} else if(net->Type == T_CONF_REQUEST || net->Type == T_CONF_REQUEST_MOR || net->Type == T_ACK || net->Type == T_ABORT) {
			_RspTxnReceived(net);
		} else {
			_ReqTxnReceived(net);
		}
	}
	if(gNetworkState == 3) {
		txn = &ReqTxns[0];
		if(txn->State == 0 && evtGetSize()) _TxnSendEvent(txn);
	}
}

void _ReqTxnReceived(NET *net)
{
	SVR_TXN		*txn;
	int		state;

	txn = txnsFind(ReqTxns, MAX_REQ_TXN_SZ, net);
	if(txn) state = txn->State; else state = REQ_IDLE;
	switch(state) {
	case REQ_AWAIT_CONFIRMATION:
		switch(net->Type) {
		case T_RESPONSE:
			evtClear(EvtFileSize);
cprintf("%ld Req-%d %d S=%02x RSP\n", DS_TIMER, (int)txn->InvokeID, (int)txn->State, (int)txn->ServiceChoice);
			break;
		case T_ERROR:
cprintf("%ld Req-%d %d S=%02x ERROR:%d-%d\n", DS_TIMER, (int)txn->InvokeID, (int)txn->State, (int)txn->ServiceChoice, (int)net->Head0, (int)net->Head1);
			break;
		case T_REJECT:
cprintf("%ld Req-%d %d S=%02x REJECT:%d\n", DS_TIMER, (int)txn->InvokeID, (int)txn->State, (int)txn->ServiceChoice, (int)net->Head0);
			break;
		case T_ABORT_SRV:
cprintf("%ld Req-%d %d S=%02x ABORT:%d\n", DS_TIMER, (int)txn->InvokeID, (int)txn->State, (int)txn->ServiceChoice, (int)net->Head0);
			break;
		default:
cprintf("%ld Req-%d %d S=%02x Unexpected:%02x\n", DS_TIMER, (int)txn->InvokeID, (int)txn->State, (int)txn->ServiceChoice, (int)net->Type);
		}
		txn->State = 0;
		break;
	default:
cprintf("%ld Req Unexpected:%02x\n", DS_TIMER, (int)net->Type);
	}
	netFreeResponse(net);
}

void _RspTxnReceived(NET *net)
{
	SVR_TXN		*txn;
	int		state;

	txn = txnsFind(ReqTxns, MAX_REQ_TXN_SZ, net);
	if(txn) state = txn->State; else state = REQ_IDLE;
	switch(state) {
	case RSP_IDLE:
		switch(net->Type) {
		case T_CONF_REQUEST:
			txn = txnsAdd(RspTxns, MAX_RSP_TXN_SZ, net);
			svrProcessRequest(txn, net);
			break;
		case T_ABORT:
			break;
		}
	}
	netFreeResponse(net);	
}

void _TxnSendEvent(SVR_TXN *txn)
{
	NET		*net, _net;
	int		rval, size;

	net = &_net;
	net->BufferLength = 2048;
	rval = netAllocRequest(net);
	if(rval >= 0) { 
		size = evtCopy(net->Buffer, &EvtFileSize);
		netReduceAllocRequest(net, size);
		txn->Address = net->Address = 0;
		netCodeConfRequest(net, P_EVENT);
		netPutRequest(net);
		txn->InvokeID		= net->InvokeID;
		txn->ServiceChoice	= P_EVENT;
		txn->Timer			= DS_TIMER;
		txn->State			= REQ_AWAIT_CONFIRMATION;
cprintf("%ld Req-%d S=%d REQ\n", DS_TIMER, (int)txn->InvokeID, (int)txn->ServiceChoice);
	}
}
