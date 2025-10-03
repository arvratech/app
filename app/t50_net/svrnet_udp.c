#define SVRNET_CONN_TIMEOUT		2800
#define SVRNET_PENDING_TIMEOUT	2200
#define SVRNET_IDLE_TIMEOUT		3500

static unsigned char	camIpAddress[4];
static unsigned short	camIpPort;
static unsigned short	netAddress;
static unsigned char	rxBuf[64];
static unsigned char	txBuf[2400];

static unsigned char	txBufAck[8];
static uv_udp_t			*udpSvrnet, _udpSvrnet;
static uv_udp_send_t	udpsend[4];
static uv_timer_t		*timerSvrnet, _timerSvrnet;
static volatile int		usendIndex;

static int		camSize, camCount;
unsigned char	svrnetInvokeId, camSn, svrnetServiceChoice, svrnetStatusChoice, retryCount;
static unsigned long	_SvrnetCamBuffer[16000];
static unsigned char	*camBuffer;


void *MainLoop(void);
void _OnSvrnetTimer(uv_timer_t *handle);

void SvrnetUdpInit(void )
{
	udpSvrnet = &_udpSvrnet;
	usendIndex = 0;
	timerSvrnet = &_timerSvrnet;
	uv_timer_init((uv_loop_t *)MainLoop(), timerSvrnet);
	svrnetState = S_SVRNET_NULL;
	uv_timer_start(timerSvrnet, _OnSvrnetTimer, 700, 0);
	camBuffer = (unsigned char *)_SvrnetCamBuffer;
}

#define _SvrnetTimer(timeout)	uv_timer_start(timerSvrnet, _OnSvrnetTimer, timeout, 0)

void _OnSvrnetAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
void _OnSvrnetRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags);

int _SvrnetUdpOpen(void)
{
	struct sockaddr_in	addr;
	unsigned short	ipPort;
	int		fd, rval, addrlen;
	int		fd_open = 0;
	int		udp_initialized = 0;

	fd = uv__socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
		rval = -errno;
printf("_SvrnetUdpOpen...socket error: %s\n", strerror(errno));
		goto _svrnet_udp_open_fail;
	}
	fd_open = 1;
	ipPort = syscfgServerIpPort(NULL)+1;
	uv_ip4_addr("127.0.0.1", ipPort, &addr);
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	rval = uv_udp_init(MainLoop(), udpSvrnet);
	if(rval) {
printf("_SvrnetUdpOpen...uv_udp_init error: %s\n", uv_strerror(rval));
		goto _svrnet_udp_open_fail;
	}
	udp_initialized = 1;

	rval = uv_udp_open(udpSvrnet, fd);
	if(rval) {
printf("_SvrnetUdpOpen...uv_udp_open error: %s\n", uv_strerror(rval));
		goto _svrnet_udp_open_fail;
	}
	fd_open = 0;

	rval = uv_udp_bind(udpSvrnet, (const struct sockaddr*)&addr, 0);
	if(rval) {
printf("_SvrnetUdpOpen...uv_udp_bind error: %s\n", uv_strerror(rval));
		goto _svrnet_udp_open_fail;
	}

	rval = uv_udp_recv_start(udpSvrnet, _OnSvrnetAlloc, _OnSvrnetRecv);
	if(rval) {
printf("_SvrnetUdpOpen...uv_udp_recv_start error: %s\n", uv_strerror(rval));
		goto _svrnet_udp_open_fail;
	}

	svrnetState = S_SVRNET_IDLE;
#ifdef _DEBUG
printf("_SvrnetUdpOpen...\n");
#endif
	_SvrnetTimer(7);
	return 0;

_svrnet_udp_open_fail:
	if(udp_initialized) uv_close((uv_handle_t *)udpSvrnet, NULL);
	if(fd_open) close(fd);
	_SvrnetTimer(5000);
	return (rval) ? rval : -errno;
}

void _SvrnetUdpClose(void)
{
	int		rval;

	if(svrnetState > S_SVRNET_IDLE) _SvrnetDisconnected();
	rval = uv_udp_recv_stop(udpSvrnet);
	uv_close((uv_handle_t *)udpSvrnet, NULL);
	svrnetState = S_SVRNET_NULL;
#ifdef _DEBUG
printf("_SvrnetUdpClose...\n");
#endif
	_SvrnetTimer(700);
}

void _SvrnetUdpError(void)
{
printf("_SvrnetUdpError...\n");
	_SvrnetDisconnected();
	_SvrnetUdpClose();
	//niFatalError();
}

void _SvrnetUdpConnected(NET_UDP *net)
{
	netAddress = net->destinationAddress;
	BYTEtoSHORT(net->data, &camIpPort);
//net->data[2] = 1;
	if(net->data[2]) camjpgOpenServer();
	else	 camjpgCloseServer();
	svrnetServiceChoice = 0;
	retryCount = 0;
	_SvrnetConnected();
}

void _SvrnetUdpDisconnected(void)
{
	camjpgCloseServer();
	evtcamFlush();
	_SvrnetDisconnected();
	_SvrnetTimer(7000);
}

void _SvrnetUdpSendData(NET_UDP *net);

void _OnSvrnetTimer(uv_timer_t *handle)
{
	NET_UDP		*net;

	if(svrnetState == S_SVRNET_NULL) {
		if(niState() >= S_NI_READY && camCodecState()) {
			int openStatus = _SvrnetUdpOpen();
			if(openStatus) return;
		} else	_SvrnetTimer(700);
	} else if(svrnetState == S_SVRNET_IDLE) {
		if(SlvnetIsConnected()) {
			memcpy(camIpAddress, syscfgServerIpAddress(NULL), 4);
			net = (NET_UDP *)txBuf;
			netudpCodeLogin(net, SlvnetMasterDevId(), devId(NULL));
			svrnetState = S_SVRNET_CONN;
			svrnetInvokeId = 0;
			svrnetServiceChoice = P_LOGIN;
			_SvrnetUdpSendData(net);
		} else	_SvrnetTimer(700);
	} else if(svrnetState == S_SVRNET_CONN) {
printf("%u login timeout\n", MS_TIMER);
		if(camCodecState()) _SvrnetUdpDisconnected();
		else	_SvrnetUdpClose();
	} else {
		if(svrnetServiceChoice) {
if(svrnetServiceChoice==P_PING) printf("%u ping timeout\n", MS_TIMER);
else if(svrnetServiceChoice==P_EVENT_NOTIFICATION) printf("%u evt timeout\n", MS_TIMER);
else if(svrnetServiceChoice==P_STATUS_NOTIFICATION) printf("%u cam timeout\n", MS_TIMER);
else	printf("%u unknown timeout: serviceChoice=%d\n", (int)svrnetServiceChoice);
			if(retryCount >= 3) {
printf("%u retryout: serviceChoice=%d\n", MS_TIMER, (int)svrnetServiceChoice);
				_SvrnetUdpDisconnected();
			} else {
				svrnetServiceChoice = 0;
				_SvrnetTimer(1000);
			}
		} else if(camCodecState()) { 
			net = (NET_UDP *)txBuf;
			netudpCodePing(net, SlvnetMasterDevId(), devId(NULL));
			net->sourceAddress = netAddress;
			svrnetInvokeId = netudpGetNextInvokeId();
			net->invokeId = svrnetInvokeId;
			svrnetServiceChoice = P_PING;
			retryCount++;
			_SvrnetUdpSendData(net);
		} else {
			_SvrnetUdpClose();
		}
	}
}

void _OnSvrnetAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
//printf("%x suggested size=%d\n", handle, suggested_size);
	buf->base = rxBuf;
	buf->len  = 64;
}

void _SvrnetUdpReqConnRx(NET_UDP *net);
void _SvrnetUdpReqRx(NET_UDP *net);
void _SvrnetUdpRspRx(NET_UDP *net);
int  _SvrnetUdpValidateData(unsigned char *buf, int size, unsigned char *ipAddr, unsigned short ipPort);

void _OnSvrnetRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *addr, unsigned flags)
{
	struct sockaddr_in *iaddr;
	NET_UDP		*net;
	unsigned char	ipAddr[4];
	unsigned short	ipPort;
	int		rval;

	if(nread < 0) {
printf("======= Svrnet RecvCb error: %d\n", nread);
		_SlvnetUdpError();
	} else if(nread > 0) {
//printf("RecvCb size=%d\n", nread);
		iaddr = (struct sockaddr_in *)addr;
		memcpy(ipAddr, &iaddr->sin_addr.s_addr, 4);
		ipPort = ntohs(iaddr->sin_port);
		net = (NET_UDP *)buf->base;	
		rval = _SvrnetUdpValidateData((unsigned char *)net, nread, ipAddr, ipPort);
		if(rval > 0) {
#ifdef _DEBUG
netudpLogRx(net, ipAddr, ipPort);
#endif
			if(net->type == T_REQ) {
				if(svrnetState == S_SVRNET_READY) _SvrnetUdpRspRx(net);
			} else if(net->invokeId == svrnetInvokeId) {
				if(svrnetState == S_SVRNET_CONN) _SvrnetUdpReqConnRx(net);
				else if(svrnetState == S_SVRNET_READY) _SvrnetUdpReqRx(net);
			}
		}
	} else {
printf("RecvCb: size=0............\n");
	}
}

void _OnSvrnetSendData(uv_udp_send_t *req, int status)
{
	NET_UDP		*net;
	int		size;

	if(status) {
printf("======= _OnSvrnetSendData error: %d\n", status);
		_SvrnetUdpError();
	} else {
		if(svrnetServiceChoice == P_STATUS_NOTIFICATION) {
			if(camCount < camSize) {
printf("+"); fflush(stdout);
				net = (NET_UDP *)txBuf;
				size = _SvrCamRead(net);
				_SvrnetUdpSendData(net);
			}
		} else if(svrnetServiceChoice == P_EVENT_NOTIFICATION) {
			if(camCount < camSize) {
				net = (NET_UDP *)txBuf;
				size = _EvtCamRead(net);
				_SvrnetUdpSendData(net);
			}
		}
	}
}

int _SvrnetUdpValidateData(unsigned char *buf, int size, unsigned char *ipAddr, unsigned short ipPort)
{
	NET_UDP		*net;
	unsigned short	sVal;
	int		type;

	if(size < 10 || size > NET_MAX_PKT_SZ) return -2;
	net = (NET_UDP *)buf;
	if(buf[0] != PROTOCOL_ID) size = -3;
	BYTEtoSHORT(buf+2, &sVal);		// Network: Source Address
	net->sourceAddress	   = sVal;
	BYTEtoSHORT(buf+4, &sVal);		// Network: Destination Address
	net->destinationAddress = sVal;
	if(size > 0) {
		type = net->type;
		if(type==T_UNCONF_REQ || type==T_REQ || type==T_REQ_MOR || type==T_RSP || type == T_RSP_MOR) {
			net->dataLength = size - 10;
		} else {
			if(size > 10) size = -4;
			else	net->dataLength = 0;;
		}
	}
	return size;
}

void _SvrnetUdpRspRx(NET_UDP *net)
{
	NET_UDP		*rspnet;

	rspnet = (NET_UDP *)txBuf;
	SvrProcessConfRequest(net, rspnet);
	_SvrnetUdpSendData(rspnet);
}

int  _SvrnetReadSendCamEvent(void);
int  _EvtCamRead(NET_UDP *net);
int  _SvrCamRead(NET_UDP *net);

void _SvrnetUdpReqConnRx(NET_UDP *net)
{
	if(net->type == T_RSP) {
		_SvrnetUdpConnected(net);
printf("%u login OK: netAddress=%d port=%d cam=%d\n", MS_TIMER, (int)netAddress, (int)camIpPort, (int)net->data[2]);
		_SvrnetReadSendCamEvent();
	} else {
printf("%u login error=%d\n", MS_TIMER, (int)net->head1);
		_SvrnetUdpDisconnected();
	}
	svrnetServiceChoice = 0;
}

void _SvrnetUdpReqRx(NET_UDP *net)
{
	unsigned char	*data;
	int		rval;

	data = net->data;
	switch(svrnetServiceChoice) {
	case P_PING:
		svrnetServiceChoice = 0;
		if(net->type == T_RSP) {
//printf("%u ping OK\n", MS_TIMER);
			if(data[0]) camjpgOpenServer();
			else	 camjpgCloseServer();
			rval = _SvrnetReadSendCamEvent();
			if(rval <= 0) _SvrnetTimer(3000);
		} else {
printf("%u ping error: type=%d %d\n", MS_TIMER, (int)net->type, (int)net->head1);
			_SvrnetUdpDisconnected();		
		}
		retryCount = 0;
		break;
	case P_STATUS_NOTIFICATION:
		svrnetServiceChoice = 0;
		if(camCount < camSize) {
printf("%u cam interrupted\n", MS_TIMER);
		} else if(net->type == T_RSP) {
printf("."); fflush(stdout);
		} else {
printf("%u cam error: type=%d error=%d\n", MS_TIMER, (int)net->type, (int)net->head1);
		}
		retryCount = 0;
		camSize = 0;
		rval = _SvrnetReadSendCamEvent();
		if(rval <= 0) _SvrnetTimer(3000);
		break;
	case P_EVENT_NOTIFICATION:
		svrnetServiceChoice = 0;
		if(camCount < camSize) {
printf("%u evt interrupted\n", MS_TIMER);
		} else if(net->type == T_RSP) {
printf("%u evt OK\n", MS_TIMER);
			evtcamClear();
		} else {
printf("%u evt error: type=%d error=%d\n", MS_TIMER, (int)net->type, (int)net->head1);
		}
		retryCount = 0;
		camSize = 0;
		rval = _SvrnetReadSendCamEvent();
		if(rval <= 0) _SvrnetTimer(3000);
		break;	
	default:
		retryCount = 0;
		svrnetServiceChoice = 0;
	}
}

void _SvrnetUdpSendData(NET_UDP *net)
{
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	unsigned char	*p;
	unsigned short	sval;
	int		rval, type, size;
unsigned char  ipAddr[4];

	type = net->type;
	if(type == T_REQ && (net->head0 == P_LOGIN || net->head0 == P_PING)) sval = syscfgServerIpPort(NULL)+1;
	else	sval = camIpPort;	
#ifdef _DEBUG
netudpLogTx(net, camIpAddress, sval);
#endif
	sinSetAddr((struct sockaddr *)&_addr, camIpAddress, sval);
	size = 10;
	if(type==T_UNCONF_REQ || type==T_REQ || type==T_REQ_MOR || type==T_RSP || type==T_RSP_MOR) size += net->dataLength;
	p = (unsigned char *)net;
	*p++ = PROTOCOL_ID;				// Link(OnTCP): ProtocolID
	*p++ = 0;						// Network: Control
	sval = net->sourceAddress;
	SHORTtoBYTE(sval, p); p += 2;	// Network: Source Address
	sval = net->destinationAddress;
	SHORTtoBYTE(sval, p); p += 2;	// Network: Destination Address
	*p++ = net->type;				// Application: PDUType + MORE/SRV
	*p++ = net->invokeId;			// Application: InvokeID
	*p++ = net->head0;				// Application: ServiceChoice, ErrorClass, RejectReason, AbortReason   
	*p++ = net->head1;				// Application: ErrorCode
	uvbuf.base = (unsigned char *)net; uvbuf.len = size;
	rval = uv_udp_send(&udpsend[usendIndex], udpSvrnet, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnSvrnetSendData);
	usendIndex++; if(usendIndex > 3) usendIndex = 0;
	if(rval) _SvrnetUdpError();
	else {
		if(svrnetState == S_SVRNET_CONN) rval = SVRNET_CONN_TIMEOUT;
		else	rval = SVRNET_PENDING_TIMEOUT;
		_SvrnetTimer(rval);
	}
}

int _EvtCamRead(NET_UDP *net)
{
	int		size;

	if(camCount == 0) {
		svrnetInvokeId = netudpGetNextInvokeId();
		camCount = 0;
		camSn = 0;
	} else	camSn++;
	size = camSize - camCount;
	if(size > 1462) size = 1462;	
	memcpy(net->data, camBuffer+camCount, size);
if(camCount == 0) {
	printf("[%02x-%02x]\n", (int)net->data[16], (int)net->data[17]);
}
	camCount += size;
	if(camCount < camSize) netudpCodeConfRequestMor(net, netAddress, P_EVENT_NOTIFICATION);
	else	netudpCodeConfRequest(net, netAddress, P_EVENT_NOTIFICATION);
	net->invokeId	= svrnetInvokeId;
	net->head1		= camSn;
	net->dataLength = size;
	size += 10;
	return size;
}

int _SvrCamRead(NET_UDP *net)
{
	int		size;

	if(camCount == 0) {
		svrnetInvokeId = netudpGetNextInvokeId();
		camSn = 0;
	} else	camSn++;
	size = camSize - camCount;
	if(size > 1462) size = 1462;	
	memcpy(net->data, camBuffer+camCount, size);
	camCount += size;
	if(camCount < camSize) netudpCodeConfRequestMor(net, netAddress, P_STATUS_NOTIFICATION);
	else	netudpCodeConfRequest(net, netAddress, P_STATUS_NOTIFICATION);
	net->invokeId	= svrnetInvokeId;
	net->head1		= camSn;
	net->dataLength = size;
	size += 10;
	return size;
}

int _SvrnetReadSendCamEvent(void)
{
	NET_UDP		*net;
	int		rval;

	rval = evtcamRead(camBuffer);
	if(rval > 0) {
		camSize = rval; camCount = 0;
		net = (NET_UDP *)txBuf;
		_EvtCamRead(net);
printf("%u evt size=%d\n", MS_TIMER, camSize-6);
		svrnetServiceChoice = P_EVENT_NOTIFICATION;
		retryCount++;
		_SvrnetUdpSendData(net);
	}
	return rval;
}

void SvrnetSendCamEvent(void)
{
	if(!svrnetServiceChoice) _SvrnetReadSendCamEvent();
}

void SvrnetSendCamServer(unsigned char *jpegBuf, int jpegBufLenth)
{
	NET_UDP		*net;
	int		rval;

	if(!svrnetServiceChoice) {
		memcpy(camBuffer, jpegBuf, jpegBufLenth);
		camSize = jpegBufLenth; camCount = 0;
		net = (NET_UDP *)txBuf;
		rval = _SvrCamRead(net);
//printf("%u cam size=%d\n", MS_TIMER, camSize);
		svrnetServiceChoice = P_STATUS_NOTIFICATION;
		retryCount++;
		_SvrnetUdpSendData(net);
	}
}

