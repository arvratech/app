unsigned long	_YUVBuffer[115200];		// 320*240 + 320*240/4 + 320*240/4 = 115200
unsigned long	_JPEGBuffer[7200];		// 28800

int _CodeCamImage(unsigned char *buf, int command, int devId, int dataSize)
{
	unsigned char	*p, c;
	long	length;
	int		i, size;

	p = buf;
	*p++ = ASCII_ACK; *p++ = ASCII_STX;
	p += 4;
	term2addr(devId, p); p += 2; 
	*p++ = (unsigned char)command;
	p += dataSize;
	size = p - buf;
	length = size + 2;
	LONGtoBYTE(length, buf+2);
	for(c = i = 0;i < 24;i++) c += buf[i];
	*p++ = c;
	*p++ = ASCII_ETX;
	return (int)length;
}

unsigned char	camevtbuf[16];

void SetCamEvent(unsigned char *evt)
{
	memcpy(camevtbuf, evt, 16);
}

void ClearCamEvent(void)
{
	camevtbuf[0] = 0;
}

int		gCamNetState, gCamTimeout;
unsigned long	gCamTimer, gCamCommand, gCamPoll;

void ctCamInit(void)
{
	gCamNetState = 0;
	gCamCommand = 0;
}

void _DisconnectCam(void)
{
//cprintf("%ld Cam Disconnected...\n", DS_TIMER);
	evtcamFlush();
	sinClose(SOCK_CAM);
	gCamTimer = DS_TIMER; gCamTimeout = 30;
	gCamCommand = 0;
	gCamNetState = 1;
}

int CamCheckPacketTrail(unsigned char *buf, int size)
{
	unsigned char	checksum;
	unsigned long	length;
	int		i, len;

	BYTEtoLONG(buf+2, &length); len = length;
	if(size != len || buf[size-1] != ASCII_ETX) return R_PACKET_FORMAT_ERROR;
	checksum = 0;
	for(i = 0;i < size-2;i++) checksum += buf[i];
	if(buf[size-2] != checksum) return R_PACKET_CHECKSUM_ERROR;	// CHECKSUM
	return 0;
}

int CamSockReadPacket(int sock, unsigned char *buf)
{
	unsigned long	length;
	int		size, count, s_count;

	length = MAX_SVRBUF_SZ; count = 0;
	size = sinRead(sock, buf, 40, 0);
	while(size > 0) {
		s_count = count;
		count += size;
		if(s_count < 6 && count >= 6) {
			s_count = 0;
			if((buf[0] == ASCII_ACK || buf[0] == ASCII_BS) && buf[1] == ASCII_STX) {
				BYTEtoLONG(buf+2, &length);
				if(length >= 11 && length <= 32) s_count = 1;
			}
			if(s_count == 0) return -3;
		}
		if(count >= length) return count;
		size = sinRead(sock, buf+count, length-count, 10);
	}
	return size;
}

static int CamFileSize;

void ctCam(void *arg)
{
	unsigned char	*Buffer, *buf, data[4];
	unsigned long	CamFrameCount, lbuf[10];
	int		rval, nSize, state;

	buf = (unsigned char *)lbuf;
	state = camGetCodecState();
	nSize = fsGetCamImageSize();
	if(sys_cfg->NetworkPort.NetworkType != NETWORK_TCPIP || !(sys_cfg->Camera.Mode & EVENT_WITH_CAM_IMAGE) && state != CAM_RUNNING && nSize <= 0) {
		if(gCamNetState > 1) _DisconnectCam();
		gCamNetState = 0;
	} else if(!gCamNetState && niGetState() > 2) {
		gCamNetState = 1;
		gCamTimer = DS_TIMER;
		simRand(data, 1);
		rval = data[0] & 0x1f; rval -= 16;
		gCamTimeout = 60 + rval;		// 44 .. 76		2015.3.30: modified from no-wait
	}
	Buffer = (unsigned char *)_JPEGBuffer;
	switch(gCamNetState) {
	case 1:
		if((DS_TIMER-gCamTimer) > gCamTimeout) {
			rval = sinStartClient(SOCK_CAM, sys_cfg->ServerIPAddress, sys_cfg->ServerPort+2);
//cprintf("%d Cam Connecting...%d\n", DS_TIMER, rval);
			if(rval) {
				gCamTimer = DS_TIMER; gCamTimeout = 50;
			} else {
				gCamTimer = DS_TIMER; gCamTimeout = 350;
				gCamNetState = 2;
			}
		}
		break;
	case 2:
		if((DS_TIMER-gCamTimer) > gCamTimeout) {
//cprintf("%ld Cam Connect timeout...\n", DS_TIMER);
			rval = 1;
		} else {
			rval = sinPollClient(SOCK_CAM);	// -1:Error 0:Continue 1:Closed 2:Connected
//if(rval < 0) cprintf("%ld Cam Connect error...\n", DS_TIMER);
//else if(rval == 1) cprintf("%ld Cam Connect closed...\n", DS_TIMER);
		}
		if(rval > 1) {
//cprintf("%ld Cam Connected...\n", DS_TIMER);
			ClearCamEvent();
			gCamNetState = 3;
			gCamPoll = 0;
		} else if(rval) {
			_DisconnectCam();
		}	 
		break;
	case 3:
		nSize = CamFileSize = evtcamCopy(Buffer + 9);
		if(nSize > 0) {
			CamFrameCount = 0L; LONGtoBYTE(CamFrameCount, Buffer+11);
			gCamCommand = P_EVENT_CAM_IMAGE_NEW;
		}
		if(!gCamCommand && gCamPoll) {
			nSize = CaptureSlowCamJpeg(Buffer+31, _YUVBuffer, &CamFrameCount);
			if(nSize > 0) {
//cprintf("Cam FrameCount=%d Size=%d\n", CamFrameCount, nSize);
				LONGtoBYTE(CamFrameCount, Buffer + 11);
				memcpy(Buffer+15, camevtbuf, 16); ClearCamEvent();
				nSize += 22; gCamCommand = P_CAM_IMAGE_NEW;
			}
		}
		if(!gCamCommand && (DS_TIMER-gCamTimer) >= 10) {
			nSize = 1; gCamCommand = P_CAM_IDLE;
		}
		if(gCamCommand) {
			Buffer[9] = state; Buffer[10] = 0;
			nSize = _CodeCamImage(Buffer, gCamCommand, (int)sys_cfg->DeviceID, nSize);
//cprintf("sinWrite CMD=%02x nSize=%d\n", gCamCommand, nSize);
			rval = sinWrite(SOCK_CAM, Buffer, nSize);
//cprintf("Cam FrameCount=%d RVAL=%d %02x-%02x\n", CamFrameCount, rval, (int)Buffer[9], (int)Buffer[10]);
			if(rval > 0) {
				gCamTimer = DS_TIMER; gCamTimeout = 150;
				gCamNetState = 4;
			} else {
				_DisconnectCam();
			}
		}
		break;
	case 4:
		nSize = CamSockReadPacket(SOCK_CAM, buf);
		if(nSize == 0) rval = 0;
		else if(nSize < 0) rval = -1;
		else {
			rval = CamCheckPacketTrail(buf, nSize);
			if(rval || buf[8]) rval = -1;
			else {
				if(nSize > 11 && buf[9]) gCamPoll = 1;
				else	gCamPoll = 0;
				rval = 1;
			}
		}
		if(rval > 0) {
			if(gCamCommand == P_EVENT_CAM_IMAGE_NEW) evtcamClear(CamFileSize);
			gCamNetState = 3;
			gCamTimer = DS_TIMER;
			if(gCamCommand) gCamCommand = 0;
//cprintf("Cam Result=%d...\n", Buffer[8]);
		} else if(rval < 0 || !rval && (DS_TIMER-gCamTimer) > gCamTimeout) {
//if(!rval && (DS_TIMER-gCamTimer) > gCamTimeout) cprintf("Cam Timeout...\n");
			_DisconnectCam();
		}
		break;
	}
}
