unsigned char	slvnetSinOpened, slvnetDisc, slvnetSendAck;


void SlvnetSinInit(void)
{
	slvnetState = 0; slvnetDisc = 0;
	slvnetSinOpened = 0;
}	

void SlvnetSinSetAddress(int address, unsigned char *IPAddress)
{
	slvAddress = 0x60 + address;
	memcpy(masterIpAddress, IPAddress, 4);
	slvReset = 1;
	slvnetCmd = 2;
	write(pipe_fds[1], buf, 1);
#ifdef _DEBUG
printf("slvAddress=%02x masterIpAddress=%s\n", (int)slvAddress, inet2addr(masterIpAddress));
#endif
}

int SlvnetSinOpen(void)
{
	struct sockaddr_in	addr;
	int		s, rval, addrlen;

	s = sinInitUDPClient();
	if(!s) return s;
	addrlen = sizeof(struct sockaddr_in);
	memset(&addr, 0, addrlen);
	addr.sin_family      = PF_INET; 
	addr.sin_port        = htons(sys_cfg->masterPort);  
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	rval = bind(s, (struct sockaddr *)&addr, addrlen);
	if(rval < 0) {
		printf("bind() error");
		close(s);
		return 0;
	}
	slvnetDisc  = 0;
	slvnetState = 0; 
	slvnetTimer = rtcMiliTimer(); slvnetTimeout = 4;
//printf("SlvnetSinOpen: s=%d bind to port=%d\n", s, (int)sys_cfg->masterPort);
	slvnetSinOpened = 1;
	return s;
}

void SlvnetSinClose(int s)
{
	if(slvnetState) _SlvnetDisconnected();
	close(s);
	slvnetSinOpened = 0;
}

void _SlvnetSinSendData(int s, unsigned char *buf, int size);
void _SlvnetSinSendAck(int s, unsigned char *buf, int size);
void _SlvnetSinRx(unsigned char *buf, int size);
int  _SlvnetSinConnect(unsigned char *buf);
int  _SlvnetSinDisconnect(unsigned char *buf);
int  _SlvnetSinData(unsigned char *buf);
int  _SlvnetSinNullData(unsigned char *buf);
int  _SlvnetSinAck(unsigned char *buf);
int  _SlvnetSinValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port);


void *SlvnetSinTask(void *arg)
{
	unsigned long	curTimer, buf[2];
	unsigned char	*p, IPAddr[8];
	unsigned short	Port;
	int		s, fd, rval, size, timeout;

	SlvnetSinInit();
	slvAddress = 0x60 + devId(NULL);
	memcpy(masterIpAddress, sys_cfg->masterIpAddress, 4);
	slvReset = 1;
	slvnetCmd = 0;
	rval = pipeOpen(pipe_fds);
	while(1) {
		while(!slvnetSinOpened) {
			sleep(5);
			if(niState() >= 2) {
				s = SlvnetSinOpen();
			}
		}
		if(retryCount) {
			if(slvnetState) timeout = 2200;	// M_PENDING 1600
			else	timeout = 2800;				// M_CONNECT 2500
		} else {
			if(slvnetState) timeout = 3500;	// M_READY
			else	timeout = slvnetTimeout;	// M_IDLE
		}
		curTimer = rtcMiliTimer();
		rval = curTimer - slvnetTimer;
		if(timeout > rval) timeout -= rval;
		else	timeout = 1;
//printf("slvNet sinPoill...timeout=%d\n", timeout);
		rval = sinPipePoll(s, pipe_fds[0], timeout);
//printf("slvNet sinPoill=%d state=%d\n", rval, (int)slvnetState);
		if(rval & 1) {
			p = (unsigned char *)rxBuf;	
			rval = sinRecvFrom(s, p, 1024, IPAddr, &Port);
			if(rval < 0) {			// error
				printf("%s\n", sinStrError());
				SlvnetSinClose(s);
				continue;
			}
#ifdef _DEBUG
if(rval > 0) {
printf("Rx from: %s.%d\n", inet2addr(IpAddr), (int)Port);
	SlvnetSinLogRx(p, rval, IPAddr, Port);
}
#endif
			if(rval > 0 && _SlvnetSinValidateData(p, rval, IPAddr, Port)) _SlvnetSinRx(p, rval);
		}
		if(retryCount) {
			if(slvnetState) timeout = 2200;	// M_PENDING   1600
			else	timeout = 2800;				// M_CONNECT   2500
			if((curTimer - slvnetTimer) >= timeout) {
				if(slvnetState) {
#ifdef _DEBUG
printf("%ld Rx Data Timeout\n", MS_TIMER);
#endif
					if(retryCount >= 3) _SlvnetDisconnected();
					else	_SlvnetSinSendData(s, (unsigned char *)txBuf, txSize);
				} else {
#ifdef _DEBUG
printf("%ld Rx Connect Timeout\n", MS_TIMER);
#endif
					_SlvnetDisconnected();
					slvnetTimer = curTimer;
					slvnetCmd = 0;
				}
			}
		} else if(slvnetState) {	// N_READY
			p = (unsigned char *)txBuf;
			size = 0;
			if(slvnetCmd == 2) {
//printf("slvReset...\n");
				_SlvnetDisconnected();
				size = _SlvnetSinDisconnect(p);
				slvnetCmd = 0; slvnetSendAck = 0;
			} else if(slvnetSendAck) {
				size = _SlvnetSinAck(p);
			} else {
				size = _SlvnetSinData(p);
				if(!size && (curTimer-slvnetTimer) >= 3500) size = _SlvnetSinNullData(p);
			}
			if(size) {
				if(slvnetSendAck) _SlvnetSinSendAck(s, p, size);
				else {
					txSize = size;
					_SlvnetSinSendData(s, p, size);
				}
			}
		} else {		// M_IDLE			
			if((curTimer - slvnetTimer) >= slvnetTimeout) {
				p = (unsigned char *)txBuf;
				size = _SlvnetSinConnect(p);
				txSize = size;
				_SlvnetSinSendData(s, p, size);
			}
		}
	}
	return arg;
}

void _SlvnetSinSendData(int s, unsigned char *buf, int size)
{	
	unsigned char	*p;
	int		rval;	

//printf("master: %s.%d\n", inet2addr(masterIpAddress), sys_cfg->masterPort);
#ifdef _DEBUG
SlvnetSinLogTx(buf, size, masterIpAddress, sys_cfg->masterPort);
#endif
	rval = sinSendTo(s, buf, size, masterIpAddress, sys_cfg->masterPort);	
	if(rval == 0) _SlvnetDisconnected();
	else if(rval < 0) {
		printf("sinSendTo: %s\n", sinStrError());
		SlvnetSinClose(s);
	} else {
		retryCount++;
		slvnetTimer = rtcMiliTimer();
	}
}

void _SlvnetSinSendAck(int s, unsigned char *buf, int size)
{
	int		rval;

#ifdef _DEBUG
SlvnetSinLogTx(buf, size, masterIpAddress, sys_cfg->masterPort);
#endif
	rval = sinSendTo(s, buf, size, masterIpAddress, sys_cfg->masterPort);
	if(rval == 0) _SlvnetDisconnected();
	else if(rval < 0) {
		printf("sinSendTo: %s\n", sinStrError());
		SlvnetSinClose(s);
	}
	slvnetSendAck = 0;
}

static unsigned char Unconf[2];

void _SlvnetSinRx(unsigned char *buf, int size)
{
	unsigned char	c;
	int		len, msg;

//printf("_SlvnetSinRx: %d\n", size);
	c = buf[2];
	if(slvnetState) {
		if(c == 0x13 && rxSn != buf[3]) {	// Data
			rxSn = buf[3];
			len = size - 4;
			if(len >= 2) {
				if(slvnetState < 3) slvnetState++;
				if(slvnetState < 3 || Unconf[0] != buf[4] || Unconf[1] != buf[5]) {
					Unconf[0] = buf[4]; Unconf[1] = buf[5];
					acuProcessRxCommon(buf+4, (int)slvnetState);
				}
				len -= 2;
				if(len > 0) {
					len--;
					if(buf[6] & 1) {
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
				}
			}
			slvnetSendAck = 1;
			if(!retryCount) slvnetTimer = rtcMiliTimer();
		} else if(retryCount && c == 0x14 && txSn == buf[3]) {	// Ack
			retryCount = 0;
			slvnetTimer = rtcMiliTimer();
		}
	} else {
		if(!slvnetDisc && c < 0x12) {
			_SlvnetConnected(buf, size);
			txSn = 255; rxSn = 255; slvnetSendAck = 0; slvReset = 0; slvnetCmd = 0;
			acuSetChanged();
		} else if(slvnetDisc && c == 0x12) {
			_SlvnetDisconnected();
			slvnetDisc = 0;
			slvnetTimer = rtcMiliTimer(); slvnetTimeout = 2000;
		}
	}
}

int _SlvnetSinConnect(unsigned char *buf)
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

int _SlvnetSinDisconnect(unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	*p++ = 0x12;
	*p++ = 0;
	return p - buf;
}

int _SlvnetSinData(unsigned char *buf)
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

int _SlvnetSinNullData(unsigned char *buf)
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

int _SlvnetSinAck(unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x66;
	*p++ = slvAddress;
	*p++ = 0x14;
	*p++ = rxSn;
	return p - buf;
}

int _SlvnetSinValidateData(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
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

void SlvnetSinLogRx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

if(buf[2] == 0x14) printf("%ld Rx Ack [%02x", MS_TIMER, (int)buf[0]);
else	printf("%ld Rx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 8;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}

void SlvnetSinLogTx(unsigned char *buf, int size, unsigned char *IPAddr, unsigned short Port)
{
	int		i;

if(buf[2] == 0x14) printf("%ld Tx Ack [%02x", MS_TIMER, (int)buf[0]);
else	printf("%ld Tx %d [%02x", MS_TIMER, size, (int)buf[0]);
	for(i = 1;i < size && i < 8;i++) printf("-%02x", (int)buf[i]);
	if(i < size)  printf("...]\n"); else printf("]\n");
}

