#include "sin.h"

static unsigned char slvsin_buf[8+2048];
static SLAVE	*SlaveSin, SlaveSins[MAX_CR_SZ];
static volatile unsigned char SlaveSinSize;
static unsigned long	_SlaveSinBuf[216];
unsigned char  SlaveSinState;
unsigned short SlaveDevID;
unsigned long  SlaveSinTimer;
int			   SlaveSinTimeout;


void SlaveSinInit(void)
{
	SLAVE	*slv;
	int		i;

	for(i = 0, slv = SlaveSins;i < MAX_CR_SZ;i++, slv++) {
		slv->State = 0;
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
	}
	SlaveSinSize = 0;
	cbuf_init(slvsin_buf, 2047);
	SlaveSinState = 0;
}

int SlaveSinWriteBuffer(void *buf, int size)
{
	int		rval;

	rval = 0;
	if(cbuf_wrlock(slvsin_buf)) {
		rval = cbuf_put(slvsin_buf, buf, size);
		cbuf_wrunlock(slvsin_buf);
	}
	return rval;
}

void SlaveSinGetIPAddress(int ID, unsigned char *IPAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	Address = 0x60 + ID;
	for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
		if(slv->State > 1 && slv->Address == Address) break;
	if(i < SlaveSinSize) memcpy(IPAddr, slv->IPAddress, 4);
	else	memset(IPAddr, 0, 4);
} 
		
void _SlaveSinControl(void);
void _SlaveSinRx(SLAVE *slv, unsigned char *buf, int size);
int  _SlaveSinTx(SLAVE *slv, unsigned char *buf);
SLAVE *_SlaveSinValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port);
void _SlaveSinConnected(SLAVE *slv, unsigned char *buf);
void _SlaveSinDisconnected(SLAVE *slve);

void SlaveSinOpen(void)
{
	int		rval;
		
	rval = sinSocket(SOCK_SLAVE, SIN_UDP, sys_cfg->ServerPort);
	if(rval == 0) {
		SlaveDevID = sys_cfg->DeviceID; SlaveSinState = 1;
	} else {
		SlaveSinTimer = TICK_TIMER; SlaveSinTimeout = 320;	// 5 seconds
	}
}

void SlaveSinClose(void)
{
	SLAVE	*slv;
	int		i;

cprintf("%ld Close Slave...\n", DS_TIMER);
	for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
		if(slv->State) _SlaveSinDisconnected(slv);
	sinClose(SOCK_SLAVE);
	SlaveSinTimer = TICK_TIMER; SlaveSinTimeout = 320;		// 5 seconds
	SlaveSinState = 0;
}

void SlaveSin2LogRx(unsigned char *buf, int size, SLAVE *slv);

void SlaveSinTask(void)
{
	unsigned char	*buf, IPAddr[4];
	unsigned short	Port;
	int		rval, size;
	
	_SlaveSinControl();
	buf = (unsigned char *)_SlaveSinBuf;
	switch(SlaveSinState) {
	case 1:
		rval = sinRecvFrom(SOCK_SLAVE, buf, IPAddr, &Port, 0);
		if(rval < 0) {
			SlaveSinClose();
		} else if(rval > 0) {
//SlaveSinLogRx(buf, rval, IPAddr, Port);
			SlaveSin = _SlaveSinValidateData(buf, rval, IPAddr, Port);
			if(SlaveSin) {
//SlaveSin2LogRx(buf, rval, SlaveSin);
				_SlaveSinRx(SlaveSin, buf, rval);
				if(SlaveSin->State == 1) memcpy(SlaveSin->IPAddress, IPAddr, 4);
				size = _SlaveSinTx(SlaveSin, buf);
				SlaveSin->InactiveTimer = DS_TIMER;
				sinSendToNoWait(SOCK_SLAVE, buf, size, SlaveSin->IPAddress, sys_cfg->ServerPort);
//SlaveSinLogTx(buf, size, SlaveSin->IPAddress, sys_cfg->ServerPort);
				SlaveSinState = 2; SlaveSinTimer = TICK_TIMER;
			}
		}
		break;
	case 2:
		rval = sinSendToCompleted(SOCK_SLAVE);
		if(rval) {
			if(rval < 0) {
cprintf("%ld Tx Timeout: %s.%d\n", DS_TIMER, inet2addr(SlaveSin->IPAddress), (int)sys_cfg->ServerPort);
				_SlaveSinDisconnected(SlaveSin);
			}
			SlaveSinState = 1;
		} else if((TICK_TIMER-SlaveSinTimer) > 224L) {	// 3.5 seconds
			SlaveSinClose();
		}
		break;
	}
}

static void _NetAddSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	Address = IDAddr;
	for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i >= SlaveSinSize) {
		for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
			if(!slv->State) break;
		if(i >= SlaveSinSize) {
			slv->State = 0; SlaveSinSize++; 
		}
	}
	if(!slv->State) {
		slv->Address		= Address;
		slv->State			= 1;
		slv->ChState[0] = slv->ChState[1] = 0;
		slv->Type			= 0;
		slv->TxSN			= 0;
		slv->DataRetryCount	= 0;
		slv->CommLoop		= 0;
		slv->InactiveTimer	= DS_TIMER;
		memset(slv->IPAddress, 0, 4); 
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
	}
cprintf("AddSlave...0x%02x %d\n", (int)Address, SlaveSinSize); 		
}

static void _NetRemoveSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	Address = IDAddr;
	for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i < SlaveSinSize) {
		slv->State = 0;
		if(i == SlaveSinSize - 1 && !slv->State) SlaveSinSize--;
	}
cprintf("RemoveSlave...0x%02x %d\n", (int)Address, SlaveSinSize); 		
}

static void _NetControlSlave(unsigned char *buf)
{
	if(buf[3])  _NetAddSlave(buf[1]);
	else	_NetRemoveSlave(buf[1]);
}

void _SlaveSinControl(void)
{
	SLAVE	*slv;
	unsigned char	buf[8];
	int		c, i, type;

	while(1) {
		c = cbuf_get(slvsin_buf, buf, 8);
		if(c < 8) break;
		c = buf[0]; i = buf[1] & 0x1f; type = buf[1] >> 5;
		if(c == DATA_REQ) {
			buf[4] = IouReq[i][2]; buf[5] = IouReq[i][3];
			IouReq[i][2] = buf[2]; IouReq[i][3] = buf[3];
			if(buf[4] != IouReq[i][2] || buf[5] != IouReq[i][3]) IouReq[i][1] = 0x01;
		} else if(c == CONTROL_REQ) {
			_NetControlSlave(buf);
		} else if(c == RESET_REQ) {
			SlaveSinSize = 0;
		}
	}
	for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
		if(slv->State > 1 && (DS_TIMER-slv->InactiveTimer) > 30) _SlaveSinDisconnected(slv);
}

SLAVE *_SlaveSinValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	SLAVE	*slv;
	unsigned char	c;
	int		i;

	c = buf[2];
	if(size >= 3 && buf[0] == 0x66 && (c < 2 || c > 3 && c < 8)) {
		for(i = 0, slv = SlaveSins;i < SlaveSinSize;i++, slv++)
			if(slv->State && slv->Address == buf[1]) break;
		//if(i < SlaveSinSize && slv->State == 1 && c > 3 || slv->State > 1 && c < 2) i = SlaveSinSize;
		if(i < SlaveSinSize && (slv->State == 1 && c > 3 || slv->State > 1 && c < 2)) i = SlaveSinSize;
	} else	 i = SlaveSinSize;
	if(i >= SlaveSinSize) slv = (SLAVE *)0;
	return slv;
}

void _SlaveSinRx(SLAVE *slv, unsigned char *buf, int size)
{
	unsigned char	cbuf[8];
	unsigned char	c, val;
	int		n, len;

	if(buf[2] < 2) {
		_SlaveSinConnected(slv, buf);
	} else {
		len = size - 3;
		c = buf[2];	// Frame Type
		// Data
		if(c < 6) val = 0; else val = 1;
		if(slv->RxSN == val) {
			if(slv->RxSN) slv->RxSN = 0; else slv->RxSN = 1;
		} else	len = 0;
		n = buf[1] & 0x1f;
		// Ack
		if(c & 1) val = 1; else val = 0;
		if(slv->TxSN == val) {
			if(slv->DataRetryCount) {
				slv->DataRetryCount = 0; IouReq[n][0] = 0;
			}		 
		} else if(slv->DataRetryCount > 2) {
			if(slv->TxSN) slv->TxSN = 0; else slv->TxSN = 1;			
			slv->DataRetryCount = 0;
		}
		if(len) {
			cbuf[2] = buf[3]; cbuf[3] = buf[4]; len -= 2;
			if(len < 1) cbuf[0] = DATA_IND;
			else {
				len--;
				if(buf[5] & 1) {
					if(buf[5] == 1) cbuf[0] = REQUEST_IND; else cbuf[0] = SEG_REQ_IND;
					memcpy(RxReqData[n], buf+6, len); RxReqDataSize[n] = len;
				} else {
					if(buf[5] == 2) cbuf[0] = RESPONSE_CNF; else cbuf[0] = SEG_RSP_CNF;
					memcpy(RxRspData[n], buf+6, len); RxRspDataSize[n] = len;
				}
			}
			cbuf[1] = slv->Address;
			cbuf_put(sp_icbuf[0], cbuf, 8);
		}
	}
}

int _SlaveSinTx(SLAVE *slv, unsigned char *buf)
{
	unsigned char	*p;
	unsigned char	c;
	int		n, len;

	if(slv->State < 2) {
		slv->State = 2;
		p = buf;
		*p++ = 0x55;
		*p++ = slv->Address;
		*p++ = 0x00;	// Connect
		SHORTtoBYTE(sys_cfg->DeviceID, p); p += 2;
		memcpy(p, sys_cfg->ServerIPAddress, 4); p += 4;
		SHORTtoBYTE(sys_cfg->ServerPort, p); p += 2;		
	} else {
		n = slv->Address & 0x1f;
		if(slv->DataRetryCount) {
			slv->DataRetryCount++;
			len = slv->DataSize;
		} else {
			if(TxRspDataSize[n]) {
				len = TxRspDataSize[n]; p = TxRspData[n]; TxRspDataSize[n] = 0;
				c = TxRspCmd[n];
			} else if(TxReqDataSize[n]) {
				len = TxReqDataSize[n]; p = TxReqData[n]; TxReqDataSize[n] = 0;
				c = TxReqCmd[n];
			} else if(IouReq[n][0] || IouReq[n][1]) {
				len = 2; p = NULL;
				c = 0;
			} else	len = 0;
			if(len) {
				if(p) {
					slv->Data[2] = c; memcpy(slv->Data+3, p, len); len += 3;
				}
				p = IouReq[n];
				memcpy(slv->Data, p+2, 2);
				p[0] |= p[1]; p[1] = 0;
				slv->DataSize = len;
				if(slv->TxSN) slv->TxSN = 0; else slv->TxSN = 1;
				slv->DataRetryCount = 1;
			}
		}
		p = buf;
		*p++ = 0x55;
		*p++ = slv->Address;
		if(slv->TxSN) c = 6; else c = 4; if(!slv->RxSN) c++;
		*p++ = c;
		if(len) {
			memcpy(p, slv->Data, len); p += len;
		}
	}
	return p - buf;
}

void _SlaveSinConnected(SLAVE *slv, unsigned char *buf)
{
	unsigned char	cbuf[8];
	int		n;

	n = buf[1] & 0x1f;
	cbuf[0] = CONNECT_RSP; cbuf[1] = slv->Address; cbuf[2] = buf[2]; 
	cbuf_put(sp_icbuf[0], cbuf, 8);
	slv->TxSN = 0; slv->RxSN = 1; slv->DataRetryCount = 0;
	TxReqDataSize[n] = TxRspDataSize[n] = RxReqDataSize[n] = RxRspDataSize[n] = 0;
	IouReq[n][1] = 0x01;
	//if(RxBuf[2]) // CfgData[n][2] = 0xc0;
	slv->State = 1;
}

void _SlaveSinDisconnected(SLAVE *slv)
{
	unsigned char	buf[8];

	if(slv->State >= 2) {
		buf[0] = DISCONNECT_RSP; buf[1] = slv->Address; memset(buf+2, 0, 6);
		cbuf_put(sp_icbuf[0], buf, 8);
		slv->State = 1;
	}
	slv->InactiveTimer = DS_TIMER;
	slv->CommLoop = slv->DataRetryCount = slv->TxSN = 0;
}

void SlaveSinLogRx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

	cprintf("%ld Rx %d [%02x", TICK_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 5;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

void SlaveSinLogTx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

	cprintf("%ld Tx %d [%02x", TICK_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 5;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

void SlaveSin2LogRx(unsigned char *buf, int size, SLAVE *slv)
{
	int		i;

	cprintf("%ld Rx SN=%d,%d %d [%02x", TICK_TIMER, (int)slv->TxSN, (int)slv->RxSN, size, (int)buf[0]);
	for(i = 1;i < size && i < 5;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

/*
void SlaveSinProcess(void)
{
	unsigned char	*buf, IPAddr[4];
	unsigned short	Port;
	int		rval;

	switch(NetState) {
	case 0:
		if((TICK_TIMER-NetTimer) > 320L) {
			rval = sinSocket(SOCK_SLAVE, SIN_UDP, sys_cfg->ServerPort);
			if(rval == 0) {
				NetState = 1;
			}
			NetTimer = TICK_TIMER;
		}
		break;
	case 1:
		buf = (unsigned char *)_SlaveSinBuf;
		rval = sinRecvFrom(SOCK_SLAVE, buf, IPAddr, &Port, 0);
		if(rval > 0) {
			_SlaveSinLogRx(buf, rval, IPAddr, Port);
			sinSendToNoWait(SOCK_SLAVE, buf, rval, IPAddr, Port);		
			_SlaveSinLogTx(buf, rval, IPAddr, Port);
			NetState = 2; NetTimer = TICK_TIMER;
		
		}
	case 2:
		rval = sinSendToCompleted(SOCK_SLAVE);
		if(rval) {
			NetState = 1;
		} else if((DS_TIMER-NetTimer) > 30L) {
			NetState = 1;			
//			_CloseUDP();
		}
		break;
	}
}
*/