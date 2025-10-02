#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "cr.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "svrnet.h"

extern AUTH_USER	*AuthUser;

long	SvrUserID;
int		SvrEvent;
unsigned char	CredData[12];
char	EventData[20];
unsigned char	svrnetState, SvrnetSinOpened, gCommand;
struct timeval	*SvrnetTimer, _SvrnetTimer;
long	SvrnetTimeout;
static int		spipe_fds[2];


void SvrnetSinInit(void)
{
	svrnetState = SvrnetSinOpened = 0;
	SvrnetTimer  = &_SvrnetTimer;
	gCommand = 0;
}	

static void _PostSvrnetChanged(void)
{
    unsigned char   msg[12];

    msg[0] = GM_SVRNET_CHANGED; msg[1] = svrnetState; memset(msg+1, 8, 0);
    appPostMessage(msg);
}

void _SvrnetConnected(void)
{
	unsigned char	*p;

p = sys_cfg->serverIpAddress;
printf("SvrnetConnected...%d.%d.%d.%d.%d\n", (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)sys_cfg->serverIpPort);
	svrnetState = 1;
	gettimeofday(SvrnetTimer, NULL);
	_PostSvrnetChanged();
}

void _SvrnetDisconnected(void)
{
	unsigned char	msg[12];

	if(svrnetState) {
		svrnetState = 0;
		gettimeofday(SvrnetTimer, NULL);
printf("SvrnetDisconnected...\n");
		_PostSvrnetChanged();
	}
	SvrnetTimeout = 9000;
}

void SvrnetRequestUser(AUTH_USER *au)
{
	unsigned char	buf[4];

	SvrUserID = au->id;
	SvrEvent = au->accessEvent;
	memcpy(CredData, au->credData, 9);
printf("UserID=%ld Event=%d\n", SvrUserID, SvrEvent);
	buf[0] = 1;
	write(spipe_fds[1], buf, 1);
}

void SvrnetRequest(void)
{
}

int SvrnetSinOpen(void)
{
	struct sockaddr_in	addr;
	int		s, rval, addrlen;
unsigned char	*p;

	if(niState() < 2) {
printf("SvrnetSinOpen: network isn't iactive yet\n");
		return 0;
	}
	s = sinInitTCPClient();
	if(!s) return s;
	svrnetState = 0; 
	gettimeofday(SvrnetTimer, NULL); SvrnetTimeout = 4;
p = sys_cfg->serverIpAddress;
printf("Connecting Server...%d.%d.%d.%d.%d\n", (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)sys_cfg->serverIpPort);
	rval = sinOpenClient(s, sys_cfg->serverIpAddress, (int)sys_cfg->serverIpPort);
	if(rval < 0) {
		printf("%s\n", sinStrError());
		SvrnetSinClose(s);
		s = 0;
	} else {
		SvrnetSinOpened = 1;
		_SvrnetConnected();
	}
	return s;
}

void SvrnetSinClose(int s)
{
	if(svrnetState) _SvrnetDisconnected();
	sinClose(s);
	SvrnetSinOpened = 0;
}

static pthread_t	_thread;
static int			threadid;

#define MAX_LARGE_SVRBUF_SZ		76800
unsigned char	SvrBuf[MAX_LARGE_SVRBUF_SZ];

void SvrnetSinThreadInit(void *(*ThreadMain)(void *))
{
	threadid = 1;
	pthread_create(&_thread, NULL, ThreadMain, (void *)&threadid);
}

int  SvrnetSinSend(int s);
void SvrnetSinReceived(unsigned char *buf, int size);

void *SvrnetSinTask(void *arg)
{
	unsigned long	length;
	unsigned char	*p, IPAddr[8];
	unsigned short	Port;
	int		s, fd, rval, pval, state, count, timeout;

	SvrnetSinInit();
	s = SvrnetSinOpen();
	rval = pipeOpen(spipe_fds);
printf("SvrnetSinTask: s=%d pipe()=%d\n", s, rval);
	state = 0; count = 0;
	while(1) {
		while(!SvrnetSinOpened) {
			sleep(5);
			s = SvrnetSinOpen();
			if(SvrnetSinOpened) state = 0;
		}
		if(state == 1) timeout = 3500;
		else if(state)	timeout = 700;		// 15
		else	timeout = 7000;
//		timeout = GetRemainTime(SvrnetTimer, timeout);
		if(timeout <= 0) timeout = 1;
//printf("svrNet sinPoill...timeout=%d\n", timeout);
		pval = sinPipePoll(s, spipe_fds[0], timeout);
//printf("svrNet sinPoill=%d state=%d\n", pval, state);
		if(pval & 2) {
			if(state) printf("BUSY..... abort\n");
			else {
				rval = SvrnetSinSend(s);
				if(rval < 0) {
					SvrnetSinClose(s);
				} else {
					state = 1; count = 0;
					gettimeofday(SvrnetTimer, NULL);
				}
			}
		}
		if(pval & 1) {
			rval = sinRead(s, SvrBuf+count, MAX_SVRBUF_SZ-count);
			if(rval <= 0) {			// error
				printf("%s\n", sinStrError());
				SvrnetSinClose(s);
				continue;
			} else if(!state) {
				printf("Unexpected.....%d ignore\n", rval);
				continue;
			}
			count += rval;
//printf("state=%d count=%d %d\n", state, count, rval);
			if(state == 1 || state == 2) {
				if(count > 4) {
//printf("[%02x-%02x-%02x-%02x-%02x]\n", (int)SvrBuf[0], (int)SvrBuf[1], (int)SvrBuf[2], (int)SvrBuf[3], (int)SvrBuf[4]);
					if(SvrBuf[0] != 0x02) {
printf("SvrNET error: first byte isn't STX\n");
						state = 9;
					} else {
						BYTEtoLONG(SvrBuf+1, &length);
						length += 6;
						if(length > MAX_SVRBUF_SZ) {
printf("Svrnet error: length=%ld is MAXIMUM(%d)\n", length, MAX_SVRBUF_SZ);
							state = 9;
						} else 	state = 3;
					}
					if(state > 3) count = 0;
				} else {
					state = 2;
				}
			} else if(state == 3) {
				if(count >= length) {
//printf("1 length=%d count=%d LastByte=%02x\n", length, count, SvrBuf[count-1]);
					if(SvrBuf[length-1] != 0x03) {
printf("SvrNET error: last byte isn't ETX\n");
						state = 9;
					} else {
						SvrnetSinReceived(SvrBuf, count);
						state = 0;
					}
				}
			}
		}
		if(pval == 0) {
			if(state == 3) {
				if(count >= length && SvrBuf[length-1] == 0x03) {
					SvrnetSinReceived(SvrBuf, count);
				} else {
printf("Svrnet timeout: length=%ld count=%d last=%02x\n", length, count, SvrBuf[length-1]);
				} 
				state = 0;
			} else if(state > 1) {
printf("Svrnet timeout: state=%d length=%ld count=%d\n", state, length, count);
				state = 0;
			} else if(state) {
printf("%ld Svrnet timeout: state=1", MS_TIMER);
				state = 0;
			}
		}
	}
	return arg;
}

int SvrnetSinSend(int s)
{	
	unsigned char	*p, buf[48], temp[20];
	int		rval, len;

	p = buf;
	*p++ = 0x02;
	if(SvrEvent == E_ACCESS_DENIED_UNREGISTERED_CARD || SvrEvent == E_ACCESS_DENIED_UNREGISTERED_EMV_CARD) {
		string_co2db(CredData, 9, temp); temp[17] = 0;
		sprintf(p+5, "%03d,%s", SvrEvent, temp);
	} else {
		sprintf(p+5, "%03d,%ld", SvrEvent, SvrUserID);
	}
	len = strlen(p+5);
	LONGtoBYTE((unsigned long)(len+1), p); p += 4;
	*p++ = 'Q';
	p += len;
	*p++ = 0x03;
	len = p - buf;
	rval = sinWrite(s, buf, len);	
	if(rval < 0) {
		printf("%s\n", sinStrError());
		SvrnetSinClose(s);
	} else {
SvrnetSinLogTx(buf, len);
	}
	return rval;
}

void SvrnetSinReceived(unsigned char *buf, int size)
{
	FILE	*fp;
	long	ID;
	unsigned char	msg[12];
	char	*s, temp[80];
	int		isize;

	SvrnetSinLogRx(buf, size);
	s = (char *)(buf + 6);
	s = read_token_comma(s, temp);
	ID = n_atoi(temp);
	s = read_token_comma(s, AuthUser->employNo);
	s = read_token_comma(s, AuthUser->userName);
	s = read_token_comma(s, AuthUser->company);
	s = read_token_comma(s, AuthUser->department);
//printf("[%02x-%02x-%02x-%02x]\n", *(s-1), *s, *(s+1), *(s+2));
	s = read_token_comma(s, AuthUser->position);
//printf("[%02x-%02x-%02x-%02x]\n", *(s-1), *s, *(s+1), *(s+2));
	isize = size - ((unsigned char *)s - buf) - 1;
	if(isize > 0) memcpy(AuthUser->jpegBuffer, s, isize);
	AuthUser->jpegSize = isize;
printf("[%ld] [%s] [%s] [%s] [%s] [%s] jpeg=%d\n", ID, AuthUser->employNo, AuthUser->userName, AuthUser->company, AuthUser->department, AuthUser->position, isize);
	msg[0] = GM_SVRNET_RESULT_USER; msg[1] = 0;
	memset(msg+2, 8, 0);
	appPostMessage(msg);
}

void SvrnetSinLogRx(unsigned char *buf, int size)
{
	int		i;

printf("%ld Rx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 20;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}

void SvrnetSinLogTx(unsigned char *buf, int size)
{
	int		i, len;

printf("%ld Tx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 26;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}
