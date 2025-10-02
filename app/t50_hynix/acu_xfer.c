static FILE		*xferfp;


static void _XferServerSendAck(void)
{
	unsigned char	*p, *p0;

//printf("%lu Tx SegRsp-XferAck\n", MS_TIMER);
	p = p0 = SlvnetConfRspBuf();
	*p++ = P_XFER_ACK;
//	gettimeofday(slvnetTimer, NULL);
	SlvnetSetConfRspTxSize(p - p0, 1);
}

static void _XferServerSendResult(int Result)
{
	unsigned char	*p, *p0;

printf("%lu Tx Rsp: %d\n", MS_TIMER, Result);
	p = p0 = SlvnetConfRspBuf();
	*p++ = Result;
	xferBusy = 0;
	SlvnetSetConfRspTxSize(p - p0, 0);
	if(xferfp) {
		fclose(xferfp); xferfp = NULL;
	}
}

static unsigned long	xferSize, xferChksum, chksum; 

void acuXferServerWRQ(unsigned char *rxData, int dataSize)
{
//printf("%lu Rx Req-XferWRQ\n", MS_TIMER);
	xferfp = NULL;
	if(rxData[0]) {
		_XferServerSendResult(R_XFER_PROTOCOL_ERROR);
		return;
	}
	if(dataSize >= 9) {
		BYTEtoLONG(rxData+1, &xferSize);
		BYTEtoLONG(rxData+5, &xferChksum);	
printf("xferSize=%d chksum=%08x\n", xferSize, xferChksum);
	} else {
		xferSize = 0;
	}
	chksum = 0;
	xferfp = fopen("xfer", "w");
	if(xferfp) {
		xferBusy = 1; xferCount = 0;
		_XferServerSendAck();
	} else {
		_XferServerSendResult(R_XFER_SYSTEM_ERROR);
	}
}

void _AdmSysReinitializeDevice(void);

void _AcuXferEnd(vod)
{
	FILE	*fp;
	char	target[32];
	unsigned char	buf[2048];
	unsigned long	*lp, chksum, size;
	int		i, rval, len;

	sprintf(target, "%s.out", devFirmwareName(NULL));
	unlink(target);
	rval = rename("xfer", target);
	fileSize(target, &size);
	if(rval < 0) rval = R_R_SYSTEM_ERROR;
	else {
		fileSize(target, &size);
		if(xferCount != size) rval = R_R_SYSTEM_ERROR;
		else if(xferSize > 0) {
			if(xferCount != (int)xferSize) rval = R_R_SYSTEM_ERROR;
			else {
				fp = fopen(target, "r");
				if(!fp) rval = R_R_SYSTEM_ERROR;
				else {
					chksum = 0;
					while(1) {
						rval = fread(buf, 1, 2048, fp);
						if(rval > 0) {
							lp = (unsigned long *)buf; len = rval >> 2;
							for(i = 0;i < len;i++) chksum += *lp++;
						}
						if(rval < 2048) break;
					}
					fclose(fp);
					if(chksum != xferChksum) rval = R_R_SYSTEM_ERROR;
					else	rval = 0;
				}
			}
		} else {
			rval = 0;
		}
	}
	_XferServerSendResult(rval);
	if(rval) unlink(target);
	sync();
	if(rval == 0) _AdmSysReinitializeDevice();
}

void acuXferServerData(unsigned char *rxData, int dataSize)
{
	int		rval;

	if(!xferBusy) _XferServerSendResult(R_XFER_PROTOCOL_ERROR);
	else if(xferCount + dataSize > XFER_MAX_SIZE) _XferServerSendResult(R_XFER_SYSTEM_ERROR);
	else {
//printf("%lu Rx Req-XferData: %d+%d\n", MS_TIMER, xferCount, dataSize);
		rval = fwrite(rxData, 1, dataSize, xferfp);
		if(rval < dataSize) {
			_XferServerSendResult(R_XFER_SYSTEM_ERROR);
		} else {
			xferCount += dataSize;
			if(dataSize == XFER_DATA_SIZE) _XferServerSendAck();
			else {
printf("%lu File receive OK: size=%d %d\n", MS_TIMER, xferCount, dataSize);
				fclose(xferfp); xferfp = NULL;
				_AcuXferEnd();
			}
		}
	}
}

void acuXferServerError(unsigned char *rxData, int dataSize)
{
	if(!xferBusy) _XferServerSendResult(R_XFER_PROTOCOL_ERROR);
	else	_XferServerSendResult((int)rxData[0]);
}

