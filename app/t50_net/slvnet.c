#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "defs.h"
#include "defs_pkt.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
//#include "cr.h"
#include "dev.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "acu.h"
#include "evt.h"
#include "uv.h"
#include "linux/msp_driver.h"
#include "slvnet.h"

#define T_INACTIVITY		12		//	1.2 seconds
#define MAX_IBUF_SZ			824
#define MAX_IDATA_SZ		812
#define MAX_SDATA_SZ		524

static unsigned char  slvAddress, rxSn, txSn, retryCount, dataRetryCount, Req[4], Unconf[2], slvReset, ReqTx, RspTx, evtPending;
static int	 		  rxCount, txCount, txSize;
static int			  Trxdelay, Ttxdelay, Tinterval;
static unsigned long  inactivityTimer;
static unsigned char  masterIpAddress[4];
static unsigned short masterIpPort, masterDevId;
static volatile int	  slvDataSize, confReqTxSize, confReqRxSize, confRspTxSize, confRspRxSize;
static volatile int	  confReq2TxSize, confReq2RxSize;
static unsigned char  confReqBuf[MAX_IDATA_SZ], confRspBuf[MAX_IDATA_SZ], confReq2Buf[MAX_SDATA_SZ];
static unsigned char  txReqCmd, txRspCmd, txReq2Cmd;
static unsigned char  txBuf[MAX_IBUF_SZ], rxBuf[MAX_IBUF_SZ];
unsigned char	slvnetState, slvnetCmd;
unsigned long	slvnetTimer;
long	slvnetTimeout;

//#define _DEBUG		1


int SlvnetMasterDevId(void)
{
	return (int)masterDevId;
}

void SlvnetSetConfReqTxSize(int size, int seg)
{
	confReqTxSize = size;
	if(seg) txReqCmd = 3; else txReqCmd = 1;
	SlvnetSendRequest();
}

void SlvnetSetConfRspTxSize(int size, int seg)
{
	confRspTxSize = size;
	if(seg) txRspCmd = 4; else txRspCmd = 2;
	SlvnetSendRequest();
}

int SlvnetConfReqRxSize(void)
{
	return (int)confReqRxSize;
}

int SlvnetConfRspRxSize(void)
{
	return (int)confRspRxSize;
}

int SlvnetConfRspTxSize(void)
{
	return (int)confRspTxSize;
}

unsigned char *SlvnetConfReqBuf(void)
{
	return confReqBuf;
}

unsigned char *SlvnetConfRspBuf(void)
{
	return confRspBuf;
}

void SlvnetSetConfReq2TxSize(int size)
{
	confReq2TxSize = size;
	txReq2Cmd = 5;
	SlvnetSendRequest();
}

int SlvnetConfReq2RxSize(void)
{
	return (int)confReq2RxSize;
}

unsigned char *SlvnetConfReq2Buf(void)
{
	return confReq2Buf;
}

int SlvnetIsConnected(void)
{
	if(slvnetState > S_SLVNET_IDLE) return 1;
	else	return 0;
}

void SlvnetSetAddress(void)
{
	if(syscfgNetworkType(NULL)) SlvnetSioSetAddress();
	else	SlvnetUdpSetAddress();
}

void _PostSlvnetChanged(void)
{
    unsigned char   msg[12];

	msg[0] = GM_SLVNET_CHANGED; msg[1] = slvAddress;
	msg[2] = masterDevId << 8; msg[3] = masterDevId; memset(msg+4, 6, 0);
	appPostMessage(msg);
}

//#define _DEBUG		1

void _SlvnetConnected(unsigned char *buf, int size)
{
	unsigned char	ipAddr[4];
	unsigned short	port;

	BYTEtoSHORT(buf+4, &masterDevId);
	if(size >= 12) {
		memcpy(ipAddr, buf+6, 4);
		BYTEtoSHORT(buf+10, &port);
		if(memcmp(ipAddr, sys_cfg->serverIpAddress, 4) || port != sys_cfg->serverIpPort) {
			memcpy(sys_cfg->serverIpAddress, ipAddr, 4);
			sys_cfg->serverIpPort = port;
			syscfgWrite(sys_cfg);
		}
//#ifdef _DEBUG
printf("%u master connected: devId=%d\n", MS_TIMER, (int)masterDevId);
//#endif
	}
	txSn = 0; rxSn = 1;
	retryCount = dataRetryCount = 0;
	confReqTxSize = confReqRxSize = confRspTxSize = confRspRxSize = 0;
	ReqTx = 0;
	evtPending = 0;
	slvnetState = S_SLVNET_READY;
	_PostSlvnetChanged();
//	SlvnetSendRequest();
}

extern int		spim_fd;

void _SlvnetConnected2(unsigned char *buf)
{
	if(slvnetState != S_SLVNET_READY) {
		BYTEtoSHORT(buf, &masterDevId);
//#ifdef _DEBUG
printf("%u master connected: devId=%d\n", MS_TIMER, (int)masterDevId);
//#endif
		txSn = 0; rxSn = 1;
		retryCount = dataRetryCount = 0;
		confReqTxSize = confReqRxSize = confRspTxSize = confRspRxSize = 0;
		ReqTx = 0;
		slvnetState = S_SLVNET_READY;
		_PostSlvnetChanged();
	}
}

void _SlvnetDisconnected(void)
{
	if(slvnetState > S_SLVNET_IDLE) {
//#ifdef _DEBUG
printf("%u master disconnected...\n", MS_TIMER);
//#endif
		slvnetState = S_SLVNET_IDLE;
		_PostSlvnetChanged();
	}
	retryCount = dataRetryCount = 0;
	evtPending = 0;
}

void _SlvnetSioResponse(unsigned char *buf, int size)
{
	int		c, msg, len, newRx;

	len = size;
printf("Rx %d [%02x", len, (int)buf[0]); for(c = 1;c < len;c++) printf("-%02x", (int)buf[c]); printf("]\n");

	if(slvnetState < S_SLVNET_READY) {
printf("Connected is undetected\n");
		_SlvnetConnected2(buf);
	}
	if(slvnetState < S_SLVNET_READY+1 || Unconf[0] != buf[0] || Unconf[1] != buf[0]) {
		Unconf[0] = buf[0]; Unconf[1] = buf[1];
		if(slvnetState < S_SLVNET_READY+1) newRx = 1; else newRx = 0;
		acuProcessRxCommon(buf, newRx);
	}
	if(slvnetState < S_SLVNET_READY+1) slvnetState++;
	len -= 2;
	if(len > 0) {
		len--; c = buf[2];
		if(c == 6) {
			msg = GM_CONF2_CNF;
//printf("=====Evt len=%d Result=%d\n", len, (int)buf[3]);
			memcpy(confReq2Buf, buf+3, len); confReq2RxSize = len;
		} else if(c & 1) {
			if(c == 1) msg = GM_CONF_IND; else msg = GM_SEG_IND;
			memcpy(confRspBuf, buf+3, len); confRspRxSize = len;
		} else {
			if(c == 2) msg = GM_CONF_CNF; else msg = GM_SEG_CNF;
			memcpy(confReqBuf, buf+3, len); confReqRxSize = len;
		}
		switch(msg) {
		case GM_CONF_IND:
			acuProcessRequest();
			break;
		case GM_SEG_IND:
			acuProcessSegRequest();
			break;
		case GM_CONF_CNF:
			acuProcessResponse();
			break;
		case GM_CONF2_CNF:
			if(evtPending) {
				if(confReq2Buf[0] == 0) evtClear();
				else	printf("### evt conf result: %d\n", (int)confReq2Buf[0]);
				evtPending = 0;
			} else {
printf("### unexpected evt conf recieved, abort\n");
			}
			break;
		}
		SlvnetSendRequest();
	}
}

void SlvnetSendRequest(void)
{
	unsigned char	*p, *s, c, tmpbuf[4];
	int		len, flg;

	if(!SlvnetIsConnected()) return;
	if(syscfgNetworkType(NULL)) {
		acuGetReq(tmpbuf);
		if(tmpbuf[2] || Req[0] != tmpbuf[0] || Req[1] != tmpbuf[1]) {
			flg = 1; Req[0] = tmpbuf[0]; Req[1] = tmpbuf[1];
		} else	flg = 0;
		if(confReqTxSize) {
			len = confReqTxSize; s = confReqBuf; confReqTxSize = 0;
			c = txReqCmd;
		} else if(confRspTxSize) {
			len = confRspTxSize; s = confRspBuf; confRspTxSize = 0;
			c = txRspCmd;
        } else if(!evtPending) {
			s = confReq2Buf;
			len = evtRead(s+1);
			if(len > 0) {
				*s = P_EVENT_NOTIFICATION;
				len++;
				c = 5;
				evtPending = 1;
			}
		} else {
			len = 0;
		}
		if(flg || len) {
			p = txBuf;
			memcpy(p, Req, 2); p += 2;
			if(len > 0) {
				*p++ = c; memcpy(p, s, len); p += len;
			}
			len = p - txBuf;
			SlvnetSioWriteData(txBuf, len);
		}
	} else {
		SlvnetUdpSendRequest();
	}
}

#include "slvnet_udp.c"
#include "slvnet_sio.c"
