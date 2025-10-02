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
#include "tftpc.h"

#define _DEBUG				1
#define MAX_BLOCK_SIZE		1428
#define DEFAULT_BLOCK_SIZE	512
#define TFTPC_RSP_TIMEOUT	55555 //12000

static unsigned char	txBuf[64], rxBuf[MAX_BLOCK_SIZE+4];
static uv_udp_t			*udpTftpc, _udpTftpc;
static uv_udp_send_t	udpsend;
static uv_timer_t		*timerTftpc, _timerTftpc;
static unsigned short	blockCount, tftpPort;
static int				blockSize, transferSize, dataSize;
static FILE				*fp;
static char				_local[80], _target[80], _errstr[64];;
static int				_status, _result;
static void (*_tftpcCallback)(int status);


void *MainLoop(void);
void _OnTftpcTimer(uv_timer_t *handle);

#define _TftpcTimer(timeout)	uv_timer_start(timerTftpc, _OnTftpcTimer, timeout, 0)

void _OnTftpcAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void _OnTftpcRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);
int _TftpcCodeRRQ(unsigned char *buf, char *fileName);
void _TftpcSend(unsigned char *buf, int len);
void _TftpcEnd(int result);

int _TftpcOpen(void)
{
	struct sockaddr_in	addr;
	int		fd, rval;
	int		fd_open = 0;
	int		udp_initialized = 0;

	udpTftpc = &_udpTftpc;
	timerTftpc = &_timerTftpc;

	fd = uv__socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
		snprintf(_errstr, sizeof(_errstr), "%s", strerror(errno));
printf("_TftpcOpen...socket error: %s\n", _errstr);
		return -errno;
	}
	fd_open = 1;

	uv_ip4_addr("127.0.0.1", 69, &addr);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	rval = uv_udp_init(MainLoop(), udpTftpc);
	if(rval) {
		snprintf(_errstr, sizeof(_errstr), "%s", uv_strerror(rval));
printf("_TftpcOpen...uv_udp_init error: %s\n", _errstr);
		goto _tftpc_open_fail;
	}
	udp_initialized = 1;

	rval = uv_udp_open(udpTftpc, fd);
	if(rval) {
		snprintf(_errstr, sizeof(_errstr), "%s", uv_strerror(rval));
printf("_TftpcOpen...uv_udp_open error: %s\n", _errstr);
		goto _tftpc_open_fail;
	}
	fd_open = 0;

	rval = uv_udp_bind(udpTftpc, (const struct sockaddr*)&addr, 0);
	if(rval) {
		snprintf(_errstr, sizeof(_errstr), "%s", uv_strerror(rval));
printf("_TftpcOpen...uv_udp_bind error: %s\n", _errstr);
		goto _tftpc_open_fail;
	}

	rval = uv_udp_recv_start(udpTftpc, _OnTftpcAlloc, _OnTftpcRecv);
	if(rval) {
		snprintf(_errstr, sizeof(_errstr), "%s", uv_strerror(rval));
printf("_TftpcOpen...uv_udp_recv_start error: %s\n", _errstr);
		goto _tftpc_open_fail;
	}

	rval = uv_timer_init((uv_loop_t *)MainLoop(), timerTftpc);
	if(rval) {
		snprintf(_errstr, sizeof(_errstr), "%s", uv_strerror(rval));
printf("_TftpcOpen...uv_timer_init error: %s\n", _errstr);
		goto _tftpc_open_fail;
	}

	return 0;

_tftpc_open_fail:
	if(udp_initialized) uv_close((uv_handle_t *)udpTftpc, NULL);
	if(fd_open) close(fd);
	return (rval) ? rval : -errno;
}

void _TftpcClose(void)
{
	uv_timer_stop(timerTftpc);
	uv_udp_recv_stop(udpTftpc);
	uv_close((uv_handle_t *)udpTftpc, NULL);
printf("_TftpcClose...\n");
}

int tftpcGet(char *local, char *remote, char *target, int estimatedSize, void (*callback)(int))
{
	int		rval, len;

printf("tftpcGet...%s <= %s\n", local, remote);
	_status = 1; _result = 0; dataSize = 0;
	_errstr[0] = 0;
	strcpy(_local, local);
	if(target) strcpy(_target, target);
	else	_target[0] = 0;
	_tftpcCallback = callback;
	(*_tftpcCallback)(_status);

	rval = _TftpcOpen();
	if(rval) {
		_status = 3; _result = 3;
		(*_tftpcCallback)(_status);
		return rval;
	}
	fp = fopen(local, "w");
	if(!fp) {
		strcpy(_errstr, strerror(errno));
printf("tftpcGet: fopen error: %s\n", _errstr);
		_TftpcEnd(3);
		return -1;
	}
	len = _TftpcCodeRRQ(txBuf, remote);
	tftpPort = 69;
	_TftpcSend(txBuf, len);
	blockSize = DEFAULT_BLOCK_SIZE;
	transferSize = estimatedSize;
	blockCount = 1;
	_TftpcTimer(TFTPC_RSP_TIMEOUT);
	return 0;
}

int tftpcStatus(void)
{
	return _status;
}

int tftpcResult(void)
{
	return _result;
}

int tftpcDownloadSize(void)
{
	return dataSize;
}

int tftpcTransferSize(void)
{
	return transferSize;
}

char *tftpcErrorString(void)
{
	return _errstr;
}

void _TftpcError(void)
{
printf("_TftpcError...\n");
	strcpy(_errstr, strerror(errno));
	_TftpcEnd(5);
	_TftpcClose();
	niFatalError();
}

int  _TftpcCodeAck(unsigned char *buf, int blockNo);
int  _TftpcCodeError(unsigned char *buf, int errCode, char *errStr);
void _TftpcLogTx(unsigned char *buf, int len);
void _TftpcLogRx(unsigned char *buf, int len);

void _OnTftpcTimer(uv_timer_t *handle)
{
	printf("Tftpc timeout\n");
	_TftpcClose();	
}

void _OnTftpcAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
//printf("%x suggested size=%d\n", handle, suggested_size);
	buf->base = rxBuf;
	buf->len  = blockSize + 4;
}

void _TftpcRx(unsigned char *buf, int size);

void _OnTftpcRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
	struct sockaddr_in *iaddr;
	unsigned char	*p, ipAddr[4];
	unsigned short	opCode;
	int		rval;

	if(nread < 0) {
printf("======= OnTftpcRecv error: %d\n", nread);
		_TftpcError();
	} else if(nread > 0) {
		rval = nread;
		iaddr = (struct sockaddr_in *)addr;
		memcpy(ipAddr, &iaddr->sin_addr.s_addr, 4);
		tftpPort = ntohs(iaddr->sin_port);
		p = buf->base;	
#ifdef _DEBUG
//_TftpcLogRx(p, rval);
#endif
		if(rval >= 4) {
			_TftpcRx(p, rval);
		} else {
			strcpy(_errstr, "data receive error");
			_TftpcEnd(5);
		}
	} else {
//printf("Recv: size=0............\n");
	}
}

void _OnTftpcSend(uv_udp_send_t *req, int status)
{
	if(status) {
printf("======= TftpcSend error=%d\n", status);
		_TftpcError();
	}
}

void _TftpcSend(unsigned char *buf, int len)
{	
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	int		rval;	

#ifdef _DEBUG
//_TftpcLogTx(buf, len);
#endif
	sinSetAddr((struct sockaddr *)&_addr, syscfgServerIpAddress(NULL), tftpPort);
	uvbuf.base = buf; uvbuf.len = len;
	rval = uv_udp_send(&udpsend, udpTftpc, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnTftpcSend);
	if(rval) _TftpcError();
	else {
		_TftpcTimer(TFTPC_RSP_TIMEOUT);
	}
}

void _TftpcEnd(int result)
{
	char	temp[256];

	if(fp) {
		fclose(fp); fp = NULL;
	}
	_result = result;
	if(result) _status = 3; else _status = 2;
	if(result) {
		unlink(_local);
	} else {
		if(_target[0]) {
			unlink(_target);
			ExecPipeFinal(temp, "/bin/mv", "mv", _local, _target, NULL);
		}
	}
	sync();
	_TftpcClose();
	(*_tftpcCallback)(_status);
}

void _TftpcRx(unsigned char *buf, int len)
{
	unsigned short	opCode, blockNo;
	unsigned char	*p, *pe;
	int		rval, txLen, dlen;

	BYTEtoSHORT(buf, &opCode);
	BYTEtoSHORT(buf+2, &blockNo);
	if(opCode == 3) {
		if(blockNo == blockCount) {
			len -= 4;
			rval = fwrite(buf+4, 1, len, fp);
			if(rval < 0) {
				strcpy(_errstr, strerror(errno));
				txLen = _TftpcCodeError(txBuf, 3, _errstr);
				_TftpcSend(txBuf, txLen);
				usleep(250);
printf("Tftpc: fwrite error\n");
				_TftpcEnd(1);
			} else {
				dataSize += len;
				if(!(blockCount & 0x1f)) (*_tftpcCallback)(_status);
				txLen = _TftpcCodeAck(txBuf, (int)blockNo);
				_TftpcSend(txBuf, txLen);
				blockCount++; if(!blockCount) blockCount = 1;
				if(len < blockSize) {
					usleep(250);
printf("Tftpc: end dataSize=%d\n", dataSize);
					_TftpcEnd(0);
				}
			}
		} else {
printf("Tftpc: block number inconsistent: %d %d\n", (int)blockCount, (int)blockNo);
			txLen = _TftpcCodeAck(txBuf, (int)blockNo);
			_TftpcSend(txBuf, txLen);
			_TftpcTimer(TFTPC_RSP_TIMEOUT);
		}
	} else if(opCode == 5) {
		strcpy(_errstr, buf+4);
printf("Tftpc: ERROR: %d %s\n", blockNo, _errstr);
		_TftpcEnd(1);
	} else if(opCode == 6) {
_TftpcLogRx(buf, len);
		p = buf + 2; pe = buf + len;
		while(p < pe) {
			if(!strcmp(p, "blksize")) {
				p += strlen(p) + 1;
				blockSize = n_atoi(p);
printf("Tftpc: blksize=%d\n", blockSize);
			} else if(!strcmp(p, "tsize")) {
				p += strlen(p) + 1;
				transferSize = n_atoi(p);
printf("Tftpc: tsize=%d\n", transferSize);
			} else {
				p += strlen(p) + 1;
			}
			p += strlen(p) + 1;
		}
		txLen = _TftpcCodeAck(txBuf, 0);
		_TftpcSend(txBuf, txLen);
	}
}

int _TftpcCodeRRQ(unsigned char *buf, char *fileName)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0;
	*p++ = 1;
	strcpy(p, fileName); p += strlen(p) + 1;
	strcpy(p, "octet"); p += strlen(p) + 1; 
	strcpy(p, "blksize"); p += strlen(p) + 1; sprintf(p, "%d", MAX_BLOCK_SIZE); p += strlen(p) + 1; 
	strcpy(p, "tsize"); p += strlen(p) + 1; strcpy(p, "0"); p += strlen(p) + 1;
	return p - buf;
}

int _TftpcCodeAck(unsigned char *buf, int blockNo)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0;
	*p++ = 4;
	SHORTtoBYTE((unsigned short)blockNo, p); p += 2;
	return p - buf;
}

int _TftpcCodeError(unsigned char *buf, int errCode, char *errStr)
{
	unsigned char	*p;
	unsigned short	sVal;
	p = buf;
	*p++ = 0;
	*p++ = 4;
	sVal = errCode;
	SHORTtoBYTE(sVal, p); p += 2;
	sprintf(p, "%s", errStr); p += strlen(p) + 1;
	return p - buf;
}

#ifdef _DEBUG

void _TftpcLogRx(unsigned char *buf, int len)
{
	int		i;

	printf("%lu Rx %d [%02x", MS_TIMER, len, (int)buf[0]);
	for(i = 1;i < len && i < 32;i++) printf("-%02x", (int)buf[i]);
	if(i < len)  printf("...]\n"); else printf("]\n");
}

void _TftpcLogTx(unsigned char *buf, int len)
{
	int		i;

	printf("%lu Tx %d [%02x", MS_TIMER, len, (int)buf[0]);
	for(i = 1;i < len && i < 20;i++) printf("-%02x", (int)buf[i]);
	if(i < len)  printf("...]\n"); else printf("]\n");
}

#endif

