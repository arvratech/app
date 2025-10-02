#define SLVNET_CONN_TIMEOUT		3800
#define SLVNET_PENDING_TIMEOUT	3500
#define SLVNET_IDLE_TIMEOUT		3500

static unsigned char	slvnetDisc, txBufAck[8];
static uv_udp_t			*udpSlvnet, _udpSlvnet;
static uv_udp_send_t	udpsend[4];
static uv_timer_t		*timerSlvnet, _timerSlvnet;
static volatile int		usendIndex;

//#define _DEBUG 1


void SlvnetUdpSetAddress(void)
{
//printf("slvnetState=%d\n", (int)slvnetState);
	if(slvnetState > S_SLVNET_IDLE) SlvnetUdpDisconnect();
	slvAddress = 0x60 + devId(NULL);
	memcpy(masterIpAddress, syscfgMasterIpAddress(NULL), 4);
	masterIpPort = syscfgMasterIpPort(NULL);
	slvReset = 1;
#ifdef _DEBUG
printf("slvAddress=%02x masterIpAddress=%s\n", (int)slvAddress, inet2addr(masterIpAddress));
#endif
}

void *MainLoop(void);
void _OnSlvnetTimer(uv_timer_t *handle);

void SlvnetUdpInit(void )
{
	udpSlvnet = &_udpSlvnet;
	usendIndex = 0;
	timerSlvnet = &_timerSlvnet;
	uv_timer_init((uv_loop_t *)MainLoop(), timerSlvnet);
	slvnetDisc = 0;
	slvnetState = S_SLVNET_NULL;	
	SlvnetUdpSetAddress();
	uv_timer_start(timerSlvnet, _OnSlvnetTimer, 700, 0);
}

void _SlvnetUdpSendData(unsigned char *buf, int size);
int  _SlvnetUdpCodeDisconnect(unsigned char *buf);

void SlvnetUdpDisconnect(void)
{
	int		size;

printf("SlvnetUdp disconnect...\n");
	txSize = _SlvnetUdpCodeDisconnect(txBuf);
	_SlvnetUdpSendData(txBuf, txSize);
	slvnetDisc = 1;
	_SlvnetDisconnected();
}

void SlvnetUdpSendRequest(void)
{
	if(SlvnetIsConnected() && !retryCount) {
		txSize = _SlvnetUdpCodeData(txBuf);
		if(txSize) _SlvnetUdpSendData(txBuf, txSize);
	} else {
	}
}

#define _SlvnetTimer(timeout)	uv_timer_start(timerSlvnet, _OnSlvnetTimer, timeout, 0)

void _OnSlvnetUdpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);

void _OnSlvnetUdpRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);

void _SlvnetUdpOpen(void)
{
	struct sockaddr_in	addr;
	int		fd, rval, addrlen;

	fd = uv__socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(!fd) {
printf("_SlvnetUdpOpen...socket error: %s\n", strerror(errno));
		_SlvnetTimer(5000);
		return;
	}
	addrlen = sizeof(addr);
	memset(&addr, 0, addrlen);
	addr.sin_family		= AF_INET;
	addr.sin_addr.s_addr	= htonl(INADDR_ANY);
	addr.sin_port			= htons(masterIpPort);
	//uv_ip4_addr("127.0.0.1", masterIpPort, &addr);
	rval = uv_udp_init(MainLoop(), udpSlvnet);
	rval = uv_udp_open(udpSlvnet, fd);
	rval = uv_udp_bind(udpSlvnet, (const struct sockaddr*)&addr, 0);
	rval = uv_udp_recv_start(udpSlvnet, _OnSlvnetUdpAlloc, _OnSlvnetUdpRecv);
	slvnetDisc  = 0;
	slvnetState = S_SLVNET_IDLE;
//#ifdef _DEBUG
printf("_SlvnetUdpOpen...%d\n", (int)masterIpPort);
//#endif
	_SlvnetTimer(7);
}

void _SlvnetUdpClose(void)
{
	int		rval;

	if(slvnetState > S_SLVNET_IDLE) _SlvnetDisconnected();
	rval = uv_udp_recv_stop(udpSlvnet);
	uv_close((uv_handle_t *)udpSlvnet, NULL);
	slvnetState = S_SLVNET_NULL;
//#ifdef _DEBUG
printf("_SlvnetUdpClose...\n");
//#endif
	_SlvnetTimer(700);
}

void _SlvnetUdpError(void)
{
printf("_SlvetUdpError...\n");
	_SlvnetDisconnected();
	_SlvnetUdpClose();
	//niFatalError();
}

void _SlvnetUdpDisconnected(void)
{
	_SlvnetDisconnected();
	_SlvnetTimer(9000);
}

void _SlvnetUdpSendAck(unsigned char *buf, int size);
int  _SlvnetUdpCodeConnect(unsigned char *buf);
int  _SlvnetUdpCodeData(unsigned char *buf);
int  _SlvnetUdpCodeNullData(unsigned char *buf);
int  _SlvnetUdpCodeAck(unsigned char *buf);
void _SlvnetUdpLogTx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port);
void _SlvnetUdpLogRx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port);

void _OnSlvnetTimer(uv_timer_t *handle)
{
	if(slvnetState == S_SLVNET_NULL) {
		if(niState() >= S_NI_READY) _SlvnetUdpOpen();
		else	_SlvnetTimer(700);
	} else if(slvnetState == S_SLVNET_IDLE) {
		if(retryCount) {
#ifdef _DEBUG
printf("%lu connect timeout\n", MS_TIMER);
#endif
			_SlvnetUdpDisconnected();
		} else {
			txSize = _SlvnetUdpCodeConnect(txBuf);
			_SlvnetUdpSendData(txBuf, txSize);
		}
	} else {
		if(retryCount) {
			if(retryCount >= 3) {
				_SlvnetUdpDisconnected();
			} else {
				_SlvnetUdpSendData(txBuf, txSize);
			}
		} else {
			txSize = _SlvnetUdpCodeNullData(txBuf);
			_SlvnetUdpSendData(txBuf, txSize);
		}
	}
}

void _OnSlvnetUdpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
//printf("%x suggested size=%d\n", handle, suggested_size);
	buf->base = rxBuf;
	buf->len  = 824;
}

void _SlvnetUdpRx(unsigned char *buf, int size);
int  _SlvnetUdpValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port);

void _OnSlvnetUdpRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
	struct sockaddr_in *iaddr;
	unsigned char	*p, ipAddr[4];
	unsigned short	port;
	int		rval;

	if(nread < 0) {
printf("======= OnSlvnetUdpRecv error: %d\n", nread);
		_SlvnetUdpError();
	} else if(nread > 0) {
//printf("recv size=%d\n", nread);
		rval = nread;
		iaddr = (struct sockaddr_in *)addr;
		memcpy(ipAddr, &iaddr->sin_addr.s_addr, 4);
		port = ntohs(iaddr->sin_port);
		p = buf->base;	
#ifdef _DEBUG
if(rval > 0) _SlvnetUdpLogRx(p, rval, ipAddr, port);
#endif
		if(rval > 0 && _SlvnetUdpValidateData(p, rval, ipAddr, port)) {
			_SlvnetUdpRx(p, rval);
		}
	} else {
//printf("_OnSlvnetRecv: size=0............\n");
	}
}

void _OnSlvnetUdpSendAck(uv_udp_send_t *req, int status)
{
//printf("OnSlvnetUdpSendAck=%d\n", status);
	if(status) {
printf("======= SlvnetUdpSendAck error=%d\n", status);
		_SlvnetUdpError();
	}
}

void _OnSlvnetUdpSendData(uv_udp_send_t *req, int status)
{
//printf("OnSendUdpSendData=%d\n", status);
	if(status) {
printf("======= OnSlvnetUdpSendData error=%d\n", status);
		_SlvnetUdpError();
	}
}

void _SlvnetUdpSendData(unsigned char *buf, int size)
{	
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	unsigned char	*p;
	int		rval;	
unsigned char  ipAddr[4];

#ifdef _DEBUG
if(slvnetState < S_SLVNET_READY) printf("master: %s.%d\n", inet2addr(masterIpAddress), (int)masterIpPort);
_SlvnetUdpLogTx(buf, size, masterIpAddress, masterIpPort);
#endif
	sinSetAddr((struct sockaddr *)&_addr, masterIpAddress, masterIpPort);
	uvbuf.base = buf; uvbuf.len = size;
	rval = uv_udp_send(&udpsend[usendIndex], udpSlvnet, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnSlvnetUdpSendData);
	usendIndex++; if(usendIndex > 3) usendIndex = 0;
	if(rval) _SlvnetUdpError();
	else {
		retryCount++;
		if(slvnetState == S_SLVNET_IDLE) rval = SLVNET_CONN_TIMEOUT;
		else	rval = SLVNET_PENDING_TIMEOUT;
		_SlvnetTimer(rval);
	}
}

void _SlvnetUdpSendAck(unsigned char *buf, int size)
{
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	int		rval;

#ifdef _DEBUG
_SlvnetUdpLogTx(buf, size, masterIpAddress, masterIpPort);
#endif
	sinSetAddr((struct sockaddr *)&_addr, masterIpAddress, masterIpPort);
	uvbuf.base = buf; uvbuf.len = size;
	rval = uv_udp_send(&udpsend[usendIndex], udpSlvnet, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnSlvnetUdpSendAck);
	usendIndex++; if(usendIndex > 3) usendIndex = 0;
	if(rval) _SlvnetUdpError();
}

void _SlvnetUdpRx(unsigned char *buf, int size)
{
	unsigned char	c;
	int		len, msg, newRx;

//printf("_SlvnetUdpRx: %d\n", size);
	c = buf[2];
	if(slvnetState > S_SLVNET_IDLE) {
		if(c == 0x13) {	// Data
			if(rxSn == buf[3]) newRx = 0;
			else {
				newRx = 1; rxSn = buf[3];
			}
			len = _SlvnetUdpCodeAck(txBufAck);
			_SlvnetUdpSendAck(txBufAck, len);
			if(slvnetState > S_SLVNET_IDLE && !retryCount) _SlvnetTimer(SLVNET_IDLE_TIMEOUT);
			if(newRx) {
				len = size - 4;
				if(len >= 2) {
					if(slvnetState < S_SLVNET_READY+1 || Unconf[0] != buf[4] || Unconf[1] != buf[5]) {
						Unconf[0] = buf[4]; Unconf[1] = buf[5];
						if(slvnetState < S_SLVNET_READY+1) newRx = 1; else newRx = 0;
						acuProcessRxCommon(buf+4, newRx);
					}
					if(slvnetState < S_SLVNET_READY+1) slvnetState++;
					len -= 2;
					if(len > 0) {
						len--;
						if(buf[6] == 6) {
							msg = GM_CONF2_CNF;
//printf("=====Evt len=%d Result=%d\n", len, (int)buf[7]);
							memcpy(confReq2Buf, buf+7, len); confReq2RxSize = len;
						} else if(buf[6] & 1) {
							if(buf[6] == 1) msg = GM_CONF_IND; else msg = GM_SEG_IND;
							memcpy(confRspBuf, buf+7, len); confRspRxSize = len;
						} else {
							if(buf[6] == 2) msg = GM_CONF_CNF; else msg = GM_SEG_CNF;
							memcpy(confReqBuf, buf+7, len); confReqRxSize = len;
						}
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
							else    printf("### evt conf result: %d\n", (int)confReq2Buf[0]);
							evtPending = 0;
						} else {
printf("### unexpected evt conf recieved, abort\n");
						}
						if(!retryCount) {
							txSize = _SlvnetUdpCodeData(txBuf);
							if(txSize) _SlvnetUdpSendData(txBuf, txSize);
						}
						break;
					}
				}
			}
		} else if(c == 0x14 && txSn == buf[3]) {	// Ack
			if(retryCount) {
				retryCount = 0;
				txSize = _SlvnetUdpCodeData(txBuf);
				if(txSize) _SlvnetUdpSendData(txBuf, txSize);
				else	_SlvnetTimer(SLVNET_IDLE_TIMEOUT);
			} else {
printf("### unexpected ack recieved, abort\n");
			}
		}
	} else if(slvnetState == S_SLVNET_IDLE) {
//printf("c=%02x retryCount=%d Disc=%d\n", c, (int)retryCount, (int)slvnetDisc);
		if(retryCount) {
			if(c == 0x10 || c == 0x11) {
				_SlvnetConnected(buf, size);
				_SlvnetTimer(SLVNET_IDLE_TIMEOUT);
				txSn = 255; rxSn = 255; slvReset = 0;
				acuSetChanged();
			} else if(slvnetDisc && c == 0x12) {
				_SlvnetDisconnected();
				slvnetDisc = 0;
				_SlvnetTimer(3000);
			}
		}
	}
}

int _SlvnetUdpCodeConnect(unsigned char *buf)
{
	unsigned char	*p, c;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	if(slvReset) c = 0x11;
	else	c = 0x10;
	*p++ = c;
	*p++ = 0;
	return p - buf;
}

int _SlvnetUdpCodeDisconnect(unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	*p++ = 0x12;
	*p++ = 0;
	return p - buf;
}

int _SlvnetUdpCodeData(unsigned char *buf)
{
	unsigned char	*p, *s, c, tmpbuf[4];
	int		len, flg;

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
printf("=====Tx Evt=%d\n", len);
			*s = P_EVENT_NOTIFICATION;
			len++;
			c = 5;
			evtPending = 1;
		}
	} else {
		len = 0;
	}
	p = buf;
	if(flg || len) {
		*p++ = 0x66;
		*p++ = slvAddress;
		*p++ = 0x13;
		txSn++;
		*p++ = txSn;
		memcpy(p, Req, 2); p += 2;
		if(len > 0) {
			*p++ = c; memcpy(p, s, len); p += len;
		}
	}
	return p - buf;
}

int _SlvnetUdpCodeNullData(unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	*p++ = 0x13;
	txSn++;
	*p++ = txSn;
	return p - buf;
}

int _SlvnetUdpCodeAck(unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	*p++ = 0x14;
	*p++ = rxSn;
	return p - buf;
}

int _SlvnetUdpValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	unsigned char	c;
	int		rval;

	rval = 0;
	c = buf[2];
	if(size >= 4 && buf[0] == 0x55 && c >= 0x10 && c <= 0x14) {
		rval = 1;
	}
	return rval;
}

#ifdef _DEBUG

void _SlvnetUdpLogRx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

if(buf[2] == 0x14) printf("%lu Rx Ack [%02x", MS_TIMER, (int)buf[0]);
else	printf("%lu Rx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 8;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}

void _SlvnetUdpLogTx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

if(buf[2] == 0x14) printf("%lu Tx Ack [%02x", MS_TIMER, (int)buf[0]);
else	printf("%lu Tx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 8;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}

#endif

