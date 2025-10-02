#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "pktdefs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "svrnet.h"

extern int	gRequest;

unsigned char	svrnetState;
unsigned long	svrnetTimer, svrnetTxTimer;
long	svrnetTimeout;
unsigned char	svrBuf[MAX_SVRBUF_SZ];
unsigned char	OldStatus[8];
int		gCommand, tcpDevID, EvtFileSize;
unsigned char svrevt_buf[EVENT_RECORD_SZ];
int		svrevt_count;
static int	spipe_fds[2];


void SvrnetSinInit(void)
{
	svrnetState = 0;
	gCommand = tcpDevID = EvtFileSize = 0;
	svrevt_count = 0;
}	

void _PostSvrnetChanged(void)
{
	unsigned char	msg[12];

	msg[0] = GM_SVRNET_CHANGED; msg[1] = svrnetState; memset(msg+1, 8, 0);
	appPostMessage(msg);
}

void _SvrnetConnected(void)
{
printf("SvrnetConnected...%s.%d\n", inet2addr(sys_cfg->serverIpAddress), (int)sys_cfg->serverPort);
	svrnetState = 2;
	svrnetTimer = rtcMiliTimer();
	_PostSvrnetChanged();
}

void _SvrnetDisconnected(void)
{
	if(svrnetState) {
		svrnetState = 0;
		svrnetTimer = rtcMiliTimer();
printf("SvrnetDisconnected...\n", MS_TIMER);
		_PostSvrnetChanged();
	}
	svrnetTimeout = 9000;
}

void _SvrnetLogoned(void)
{
	memset(OldStatus, 0, 8);
	gCommand = 0;
	svrnetState = 3;
	svrnetTimer = rtcMiliTimer();
	_PostSvrnetChanged();
printf("SvrnetLogoned...\n");
}

void SvrnetRequest(void)
{
	unsigned char	buf[4];

	buf[0] = 1;
	write(spipe_fds[1], buf, 1);
}

int SvrnetSinOpen(void)
{
	struct sockaddr_in	addr;
	int		s, rval, addrlen;

	svrnetState = 0; 
	s = sinInitTCPClient();
	if(!s) return s;
	svrnetTimer = rtcMiliTimer(); svrnetTimeout = 4;
	rval = sinOpenClient(s, sys_cfg->serverIpAddress, (int)sys_cfg->serverPort);
	if(rval < 0) {
		printf("%s\n", sinStrError());
		SvrnetSinClose(s);
		s = 0;
	} else {
		_SvrnetConnected();
	}
	return s;
}

void SvrnetSinClose(int s)
{
	if(svrnetState) _SvrnetDisconnected();
	sinClose(s);

}

static pthread_t	_thread;
static int			threadid;

unsigned char	svrBuf[MAX_SVRBUF_SZ];

void SvrnetSinThreadInit(void *(*ThreadMain)(void *))
{
	threadid = 1;
	pthread_create(&_thread, NULL, ThreadMain, (void *)&threadid);
}

void _SvrnetReceivedData(int s, unsigned char *buf, int size);
void _SvrnetReceivedError(int s, int result);
void _SvrnetSendCommand(int s);

void *SvrnetSinTask(void *arg)
{
	unsigned long	curTimer;
	int		s, rval, pval, size, result, timeout;

	SvrnetSinInit();
	rval = pipeOpen(spipe_fds);
printf("SvrnetSinTask: pipe()=%d\n", rval);
	while(1) {
		if(svrnetState >= 3 && (tcpDevID != devId(NULL) || (gRequest & G_TCP_IP_CHANGED))) {
			gRequest &= ~G_TCP_IP_CHANGED;
printf("config changed..  disconnect\n");
			_SvrnetDisconnected();
		}
		curTimer = rtcMiliTimer();
		switch(svrnetState) {
		case 0:
			sleep(5);
			if(niState() >= S_NI_READY) {
				svrnetState = 1;
			}
			break;
		case 1:
			s = SvrnetSinOpen();
			break;
		case 2:
tcpDevID = devId(NULL);
			size = CodeCommand(svrBuf, tcpDevID, P_LOGIN, 0);
			rval = sinWrite(s, svrBuf, size);
			if(rval <= 0) {
				_SvrnetDisconnected();
				break;
			}
//svrLog(svrBuf, size, 1);
			size = sinPollRead(s, svrBuf, 9, 9000); // 50
if(size > 0) svrLog(svrBuf, size, 0);
else	sinError("Logon");
            if(size > 0) result = CheckPacket(svrBuf, size);
            else   result = -1;
if(result >= 0) printf("CheckPacket=%d\n", result);
            if(result == 0) result = GetResult(svrBuf);
			else  result = -1;
			if(result) {
printf("Logon Failed: 0x%x(%d)\n", result, result);
				if(result > 0) result = 3; else result = 0;
				_SvrnetDisconnected();
			} else {
				_SvrnetLogoned();
				_SvrnetSendCommand(s);
			}
			break;
		case 3:
			rval = curTimer - svrnetTimer;
			timeout = 7000 - rval;
			if(gCommand) {
				rval = 15000 - rval;
				if(timeout > rval) timeout = rval;
			}
			if(timeout <= 0) timeout = 1;
//printf("sinPoill...timeout=%d\n", timeout);
			pval = sinPipePoll(s, spipe_fds[0], timeout);
			result = -1;
			if(pval & 1) {
				size = SockReadPacket(s, svrBuf);
				if(size < 0) {			// error
					if(size == -3) result = R_PACKET_FORMAT_ERROR;
					else {
#ifdef SVR_MON
						if(size == -1) printf("cleared in reading\n");
						else	printf("fatal error in reading\n");
#endif
						result = -2;
					}
				} else {
					result = CheckPacketTrail(svrBuf, size);
					if(result == 0) {
						rval = GetDeviceID(svrBuf);
						if(tcpDevID != rval) result = R_PACKET_ADDRESS_ERROR;
					}
				}
			}
			if(result == -1) {
				if(!gCommand) _SvrnetSendCommand(s);
				else if((curTimer-svrnetTxTimer) >= 15000L) {
#ifdef SVR_MON
					printf("Result time out: Command=%02x\n", gCommand);
#endif
					_SvrnetDisconnected();
				}
			} else if(result == 0) {
svrLog(svrBuf, size, 0);
				_SvrnetReceivedData(s, svrBuf, size);
			} else {	// result=-2 or result>0
				 _SvrnetReceivedError(s, result);
			}
			break;
		}
	}
	return arg;
}

void _SvrnetReceivedData(int s, unsigned char *buf, int size)
{
	int		rval, command, result;

	svrnetTimer = rtcMiliTimer();
	command = GetCommand(buf);
/*
	if(gCommand == P_XFER_RRQ || gCommand == P_XFER_WRQ) {
		switch(command) {
		case P_XFER_DATA:	rval = XferServerData(s, svrBuf, size); break;
		case P_XFER_ACK:	rval = XferServerAck(s, svrBuf, size); break;
		case P_XFER_ERROR:	rval = XferServerError(s, svrBuf, size); break;
		default:			rval = XferServerSendResult(s, svrBuf, R_XFER_PROTOCOL_ERROR);
		}
		if(rval < 0) {
		}
	} else if(command >= 0) {
		svrProcessCommand(s, buf, size);
	} else {
		if(gCommand == P_EVENT || gCommand == P_EVENT_EX) {
			result = GetResult(buf);
			if(result == 0) {
				if(svrevt_count) svrevt_count = 0;
				else	evtClear(EvtFileSize);
			}
		}
		if(gCommand) gCommand = 0;
		else {
#ifdef SVR_MON
			printf("received unexpected result\n");
#endif
		}
	}
*/
}

void _SvrnetReceivedError(int s, int result)
{
	int		rval;

	if(result < 0) rval = -1;
	else {
#ifdef SVR_MON
		printf("received error packet: Result=%02x\n", result);
#endif
		if(result >= R_PACKET_CHECKSUM_ERROR && IsCommand(svrBuf)) rval = svrSendResult(s, svrBuf, result, 0);
		else	rval = 0;
	}
	if(rval <= 0) _SvrnetDisconnected();
	if(gCommand) gCommand = 0;
}

void _SvrnetSendCommand(int s)
{
	unsigned long	curTimer;
	unsigned char	*p;
	int		rval, size, statSize;

	p = svrBuf + 7;
	statSize = EncodeStatus(p);
	p += statSize;
	if(svrevt_count) {
		memcpy(p, svrevt_buf, EVENT_RECORD_SZ);
		size = EVENT_RECORD_SZ; EvtFileSize = 0;

	} else	size = evtCopy(p, &EvtFileSize);
	curTimer = rtcMiliTimer();
	if(size > 0) gCommand = P_EVENT;	
	else if(memcmp(OldStatus, svrBuf+7, statSize) || (curTimer-svrnetTimer) >= 7000L) gCommand = P_PING;
	if(gCommand) {
		size = CodeCommand(svrBuf, tcpDevID, gCommand, size+statSize);
svrLog(svrBuf, size, 1);
		rval = sinWrite(s, svrBuf, size);
		if(rval <= 0) _SvrnetDisconnected();
		else {
			memcpy(OldStatus, svrBuf+7, statSize);
			svrnetTxTimer = rtcMiliTimer();
		}
	}
}

