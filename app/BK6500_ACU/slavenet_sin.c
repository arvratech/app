#include "nwpcfg.h"
#include "objdefs.h"
#include "event.h"
#include "sin.h"

//#define  _DEBUG		1

static unsigned char slvsin_buf[8+2048];
static SLAVE	slvSins[MAX_CR_SZ];
static int		slvSinSize, slvSinIndex;
static unsigned long	_slvSinBuf[216];
static unsigned char	slvSinIpAddrs[MAX_CR_SZ][4];


void SlaveSinInit(void)
{
	SLAVE	*slv;
	int		i;

	for(i = 0, slv = slvSins;i < MAX_CR_SZ;i++, slv++) {
		slv->State = 0;
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
		memset(slvSinIpAddrs[i], 0, 4);
	}
	slvSinSize = 0; slvSinIndex = -1;
	cbuf_init(slvsin_buf, 2047);
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

void SlaveSinGetIPAddress(int id, unsigned char *ipAddr)
{
	SLAVE	*slv;
	unsigned char	address;
	int		i;

	address = 0x60 + id;
	for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
		if(slv->State > 1 && slv->Address == address) break;
	if(i < slvSinSize) memcpy(ipAddr, slv->IPAddress, 4);
	else	memset(ipAddr, 0, 4);
} 
		
SLAVE *_SlaveSinValidateData(unsigned char *buf, int size, unsigned char *ipAddr, unsigned short port);
void _SlaveSinConnected(SLAVE *slv, unsigned char *buf);
void _SlaveSinDisconnected(SLAVE *slv);

void SlaveSinOpen(SIN_PORT *sinport)
{
	sinportOpen(sinport);
}

void SlaveSinClose(SIN_PORT *sinport)
{
//	SLAVE	*slv;
//	int		i;
//	for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
//		if(slv->State > 1) _SlaveSinDisconnected(slv);
	sinportClose(sinport);
}

void _NetAddSlave(unsigned char IDAddr);
void _NetRemoveSlave(unsigned char IDAddr);

void SlaveSinComTask(void)
{
	unsigned char	buf[8];
	SLAVE	*slv;
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
			if(buf[3])  _NetAddSlave(buf[1]);
			else	_NetRemoveSlave(buf[1]);
		} else if(c == RESET_REQ) {
			slvSinSize = 0;
		}
	}
	for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
		if(slv->State > 1 && !slv->DataRetryCount && (TICK_TIMER-slv->Timer) > 448L) {	// 7.0s
			_SlaveSinDisconnected(slv);
		}
}

void _SlaveSinSendData(SIN_PORT *sinport, SLAVE *slv, unsigned char *buf, int size);
void _SlaveSinSendAck(SIN_PORT *sinport, SLAVE *slv, unsigned char *buf, int size);
void _SlaveSinRx(SLAVE *slv, unsigned char *buf, int size);
int  _SlaveSinData(SLAVE *slv, unsigned char *buf);
int  _SlaveSinRetryData(SLAVE *slv, unsigned char *buf);
int  _SlaveSinAck(SLAVE *slv, unsigned char *buf);

void SlaveSinRxTask(SIN_PORT *sinport)
{
	SLAVE	*slv;
	unsigned char	*p, *ip, ipAddr[4], ctm[8], data[8], evtData[12];
	unsigned short	port;
	int		rval, id;
	
	p = (unsigned char *)_slvSinBuf;
	rval = sinRecvFrom(sinport->s, p, ipAddr, &port, 0);
#ifdef  _DEBUG
if(rval > 0) {
	cprintf("s=%d From: %s.%d\n", sinport->s, inet2addr(ipAddr), (int)port);
	SlaveSinLogRx(p, rval);
}
#endif
	if(rval < 0) {
		SlaveSinClose(sinport);
	} else if(rval > 0) {
		slv = _SlaveSinValidateData(p, rval, ipAddr, port);
		if(slv) {
			id = slv->Address & 0x1f;
			ip = slvSinIpAddrs[id];
			if(slv->State < 2) {
				if(p[2] < 0x12) {
					_SlaveSinConnected(slv, p);
					memcpy(slv->IPAddress, ipAddr, 4);
					if(IPIsNull(ip)) memcpy(ip, ipAddr, 4);
					else if(n_memcmp(ip, ipAddr, 4)) {
cprintf("####### IP address: %s isn't equal to CONNECT_REQ:%s.%d #######\n", inet2addr(ip), inet2addr(ipAddr), (int)port);
						rtcGetDateTime(ctm);
						memcpy(data, ip, 4); memcpy(data+4, ipAddr, 4);
						bin2card(data, 8, evtData);
						EventAdd(OT_CREDENTIAL_READER, id, 167, ctm, evtData);		
					}
				}
			} else {
				if(n_memcmp(slv->IPAddress, ipAddr, 4)) {
cprintf("####### IP address: %s isn't equal to received %s.%d #######\n", inet2addr(slv->IPAddress), inet2addr(ipAddr), (int)port);
					rtcGetDateTime(ctm);
					memcpy(data, slv->IPAddress, 4);
					memcpy(data+4, ipAddr, 4);
					bin2card(data, 8, evtData);
					EventAdd(OT_CREDENTIAL_READER, id, 167, ctm, evtData);
				} else {
			 		_SlaveSinRx(slv, p, rval);
					if(n_memcmp(ip, ipAddr, 4)) memcpy(ip, ipAddr, 4);			 		
			 	}
			}	
		}
	}
}

int IsValidMACAddress(unsigned char *addr)
{
	int		i, rval;
	
	rval = 0;
	for(i = 0;i	< 6;i++)
		if(addr[i]) break;
	if(i < 6) {
		for(i = 0;i	< 6;i++)
			if(addr[i] != 0xff) break;
		if(i < 6) rval = 1;
	}
	return rval;
}	

void SlaveSinTxTask(SIN_PORT *sinport)
{
	SLAVE	*slv;
	unsigned char	*p, addr[8];
	int		i, rval, size;

	if(sinport->Context) {
		slv = (SLAVE *)sinport->Context;
		rval = sinSendToCompleted(sinport->s);
		if(rval) {
			if(rval < 0) {
cprintf("%lu slvnets(%d): s=%d Tx timeout: %s.%d\n", DS_TIMER, (int)(slv->Address&0x1f), sinport->s, inet2addr(slv->IPAddress), (int)sys_cfg->ServerPort);
				_SlaveSinDisconnected(slv);
			} else if(slv->State == 2) {
				sockGetDestMACAddress(sinport->s, addr);
cprintf("%lu slvnets(%d) DestMACAddress=%02x-%02x-%02x-%02x-%02x-%02x\n", DS_TIMER, (int)(slv->Address&0x1f), (int)addr[0], (int)addr[1], (int)addr[2], (int)addr[3], (int)addr[4], (int)addr[5]);
				if(IsValidMACAddress(addr)) {
					memcpy(slv->MACAddress, addr, 6);
					slv->State = 3;
				}
			}	
			sinport->Context = NULL;
		} else if((TICK_TIMER-slv->Timer) > 224L) {	// 3.5s
cprintf("%lu slvnets(%d): s=%d Tx timeout(3.5s)...\n", DS_TIMER, (int)(slv->Address&0x1f), sinport->s);
			_SlaveSinDisconnected(slv);
			SlaveSinClose(sinport);
		}
	} else {
		p = (unsigned char *)_slvSinBuf;
		i = slvSinIndex + 1;
		if(i >= slvSinSize) i = 0;
		slv = &slvSins[i];
		rval = i; size = 0;
		do {
			if(slv->State > 1) {
				if(slv->AckSend) {
					size = _SlaveSinAck(slv, p);
					_SlaveSinSendAck(sinport, slv, p, size);
					break;
				} else {
					if(!slv->DataRetryCount) size = _SlaveSinData(slv, p);
					else if((TICK_TIMER-slv->Timer) > 320L) {		// 1.6s(102) ==> 2.5s(160)  2019.1.10
cprintf("%lu slvnets(%d): s=%d Rx timeout: %s.%d\n", DS_TIMER, (int)(slv->Address&0x1f), sinport->s, inet2addr(slv->IPAddress), (int)sys_cfg->ServerPort);
						if(slv->DataRetryCount >= 3) _SlaveSinDisconnected(slv);
						else	size = _SlaveSinRetryData(slv, p);
					}
					if(size) {
						_SlaveSinSendData(sinport, slv, p, size);
						break;
					}
				}
			}
			i++;
			if(i < slvSinSize) slv++;
			else {
				i = 0; slv = slvSins;
			}
		} while(i != rval) ;
		if(size <= 0) i = -1;
		slvSinIndex = i;
	}
}

void _NetAddSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	Address = IDAddr;
	for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i >= slvSinSize) {
		for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
			if(!slv->State) break;
		if(i >= slvSinSize) {
			slv->State = 0; slvSinSize++; 
		}
	}
	if(!slv->State) {
		slv->Address	= Address;
		slv->State		= 1;
		slv->ChState[0] = slv->ChState[1] = 0;
		slv->Type		= 0;
		memset(slv->IPAddress, 0, 4); 
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
	}
cprintf("AddSlave...0x%02x %d\n", (int)Address, slvSinSize); 		
}

void _NetRemoveSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	Address = IDAddr;
	for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i < slvSinSize) {
		slv->State = 0;
		if(i == slvSinSize - 1 && !slv->State) slvSinSize--;
	}
cprintf("RemoveSlave...0x%02x %d\n", (int)Address, slvSinSize); 		
}

SLAVE *_SlaveSinValidateData(unsigned char *buf, int size, unsigned char *ipAddr, unsigned short port)
{
	SLAVE	*slv;
	int		i, c;

//	if(port != sys_cfg->ServerPort) {
//cprintf("####### unknown port: %s.%d #######\n", inet2addr(ipAddr), (int)port);
//		slv = (SLAVE *)0;
	c = buf[2];
	if(size >= 4 && buf[0] == 0x66 && c >= 0x10 && c <= 0x14) {
		for(i = 0, slv = slvSins;i < slvSinSize;i++, slv++)
			if(slv->State && slv->Address == buf[1]) break;
	} else	 i = slvSinSize;
	if(i >= slvSinSize) slv = (SLAVE *)0;
	return slv;
}

void _SlaveSinSendData(SIN_PORT *sinport, SLAVE *slv, unsigned char *buf, int size)
{
	if(slv->State == 2) sinSendToNoWait(sinport->s, buf, size, slv->IPAddress, sys_cfg->ServerPort);
	else	sinSendToMACNoWait(sinport->s, buf, size, slv->IPAddress, sys_cfg->ServerPort, slv->MACAddress);
	slv->DataRetryCount++;
#ifdef  _DEBUG
cprintf("s=%d To: %s.%d\n", sinport->s, inet2addr(slv->IPAddress), (int)sys_cfg->ServerPort);
SlaveSinLogTxDat(slv, buf, size);
#endif
	sinport->Context = slv; sinport->Timer = slv->Timer = TICK_TIMER;
}

void _SlaveSinSendAck(SIN_PORT *sinport, SLAVE *slv, unsigned char *buf, int size)
{
	if(slv->State == 2) sinSendToNoWait(sinport->s, buf, size, slv->IPAddress, sys_cfg->ServerPort);
	else	sinSendToMACNoWait(sinport->s, buf, size, slv->IPAddress, sys_cfg->ServerPort, slv->MACAddress);
#ifdef  _DEBUG
cprintf("s=%d To: %s.%d\n", sinport->s, inet2addr(slv->IPAddress), (int)sys_cfg->ServerPort);
SlaveSinLogTxAck(slv, buf, size);
#endif
	sinport->Context = slv; sinport->Timer = TICK_TIMER;
	slv->AckSend = 0;
}

void _SlaveSinRx(SLAVE *slv, unsigned char *buf, int size)
{
	unsigned char	cbuf[8], c;
	int		n, len;

	c = buf[2];
	n = buf[1] & 0x1f;
	if(slv->State > 1) {
		if(c == 0x13 && slv->RxSN != buf[3]) {	// Data
			slv->RxSN = buf[3];
			len = size - 4;
			if(len >= 2) {
				cbuf[2] = buf[4]; cbuf[3] = buf[5]; len -= 2;
				if(len < 1) cbuf[0] = DATA_IND;
				else {
					len--;
					if(buf[6] == 5) {
						cbuf[0] = REQ2_IND;
						memcpy(RxReq2Data[n], buf+7, len); RxReq2DataSize[n] = len;
					} else if(buf[6] & 1) {
						if(buf[6] == 1) cbuf[0] = REQ_IND; else cbuf[0] = SEG_REQ_IND;
						memcpy(RxReqData[n], buf+7, len); RxReqDataSize[n] = len;
					} else {
						if(buf[6] == 2) cbuf[0] = RSP_CNF; else cbuf[0] = SEG_RSP_CNF;
						memcpy(RxRspData[n], buf+7, len); RxRspDataSize[n] = len;
					}
				}
				cbuf[1] = slv->Address;
				cbuf_put(sp_icbuf[0], cbuf, 8);
			}
			slv->AckSend = 0x14;
			if(!slv->DataRetryCount) slv->Timer = TICK_TIMER;
		} else if(c == 0x12) {		// Disconnect
			_SlaveSinDisconnected(slv);
		} else if(slv->DataRetryCount && c == 0x14 && slv->TxSN == buf[3]) {	// Ack
			slv->DataRetryCount = 0;
			slv->Timer = TICK_TIMER;
		}
	} else {
		if(c < 0x12) {
			_SlaveSinConnected(slv, buf);
		}
	}
}

int _SlaveSinData(SLAVE *slv, unsigned char *buf)
{
	unsigned char	*p, *s, c;
	int		n, len;

	n = slv->Address & 0x1f;
	if(TxReqDataSize[n]) {
		len = TxReqDataSize[n]; s = TxReqData[n]; TxReqDataSize[n] = 0;
		c = TxReqCmd[n];
	} else if(TxRspDataSize[n]) {
		len = TxRspDataSize[n]; s = TxRspData[n]; TxRspDataSize[n] = 0;
		c = TxRspCmd[n];
	} else if(TxRsp2DataSize[n]) {
		len = TxRsp2DataSize[n]; s = TxRsp2Data[n]; TxRsp2DataSize[n] = 0;
		c = TxRsp2Cmd[n];
	} else {
		len = 0;
	}
	p = buf;
	if(IouReq[n][1] || len) {
		IouReq[n][1] = 0;
		*p++ = 0x55;
		*p++ = slv->Address;
		*p++ = 0x13;
		slv->TxSN++;
		*p++ = slv->TxSN;
		memcpy(p, &IouReq[n][2], 2); p += 2;
		if(len) {
			*p++ = c; memcpy(p, s, len); p += len;
		}
		len = p - buf - 4;
		memcpy(slv->Data, p-len, len); slv->DataSize = len;
	}
	return p - buf;
}

int _SlaveSinRetryData(SLAVE *slv, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x55;
	*p++ = slv->Address;
	*p++ = 0x13;
	*p++ = slv->TxSN;
	memcpy(p, slv->Data, slv->DataSize); p += slv->DataSize;
	return p - buf;
}

int _SlaveSinAck(SLAVE *slv, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x55;
	*p++ = slv->Address;
	*p++ = slv->AckSend;
	*p++ = slv->RxSN;
	if(slv->AckSend < 0x12) {
		SHORTtoBYTE(sys_cfg->DeviceID, p); p += 2;
		memcpy(p, sys_cfg->ServerIPAddress, 4); p += 4;
		SHORTtoBYTE(sys_cfg->ServerPort, p); p += 2;
	}
	return p - buf;
}

void _SlaveSinConnected(SLAVE *slv, unsigned char *buf)
{
	unsigned char	cbuf[8];
	int		n;

	n = buf[1] & 0x1f;
	cbuf[0] = CONNECT_RSP; cbuf[1] = slv->Address; 
	if(buf[2] == 0x11) cbuf[2] = 1; else cbuf[2] = 0;
	cbuf_put(sp_icbuf[0], cbuf, 8);
	slv->TxSN = 255; slv->RxSN = 255; slv->DataRetryCount = 0;
	TxReqDataSize[n] = TxRspDataSize[n] = RxReqDataSize[n] = RxRspDataSize[n] = 0;
	IouReq[n][1] = 0x01;
	//if(RxBuf[2]) // CfgData[n][2] = 0xc0;
	slv->AckSend = 0x11;
	slv->Timer = TICK_TIMER;	
	slv->State = 2;
}

void _SlaveSinDisconnected(SLAVE *slv)
{
	unsigned char	buf[8];

	if(slv->State >= 2) {
		buf[0] = DISCONNECT_RSP; buf[1] = slv->Address; memset(buf+2, 0, 6);
		cbuf_put(sp_icbuf[0], buf, 8);
		slv->State = 1;
	}
	slv->DataRetryCount = 0;
}

void SlaveSinLogRx(unsigned char *buf, int size)
{
	int		i;

	cprintf("%lu slvnets: Rx %d [%02x", DS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 16;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

void SlaveSinLogTxAck(SLAVE *slv, unsigned char *buf, int size)
{
	int		i;

	cprintf("%lu slvnets(%d): Tx ACK %d [%02x", DS_TIMER, (int)(slv->Address & 0x1f), size, (int)buf[0]);
	for(i = 1;i < size && i < 16;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

void SlaveSinLogTxDat(SLAVE *slv, unsigned char *buf, int size)
{
	int		i;

	cprintf("%lu slvnets(%d): Tx DAT retry=%d %d [%02x", DS_TIMER, (int)(slv->Address & 0x1f), (int)slv->DataRetryCount, size, (int)buf[0]);
	for(i = 1;i < size && i < 16;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}

void SlaveSin2LogRx(unsigned char *buf, int size, SLAVE *slv)
{
	int		i;

	cprintf("%lu Rx sn=%d,%d %d [%02x", DS_TIMER, (int)slv->TxSN, (int)slv->RxSN, size, (int)buf[0]);
	for(i = 1;i < size && i < 8;i++) cprintf("-%02x", (int)buf[i]);
	if(i < size)  cprintf("...]\n"); else cprintf("]\n");
}
