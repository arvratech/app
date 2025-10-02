int CliGetXferState(void)
{
	return XferState;
}

void CliSetXferState(int State)
{
	XferState = State;
}

unsigned char *CliGetXferBuffer(void)
{
	return (unsigned char *)XferBuf;
}

int CliGetXferBufferSize(void)
{
	return XFER_BUFFER_SIZE;
}

int CliGetXferSize(void)
{
	return XferSize;
}

void CliSetXferSize(int nSize)
{
	XferSize = nSize;
}

int CliGetXferCount(void)
{
	return XferCount;
}

int CliGetXferID(void)
{
	return CliID;
}

void CliGetXferLastTime(unsigned char *ctm)
{
	memcpy(ctm, XferLastTime, 6);
}

void CliXferWRQ(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	unsigned long	*lp, chksum;
	int		i, id, len;

	id = cr->ID;
	p = p0 = SlaveNetGetTxReqData(id);
	*p++ = P_XFER_WRQ;
	*p++ = 0;
	LONGtoBYTE((unsigned long)XferSize, p); p += 4;
	lp = (unsigned long *)XferBuf;
	len = XferSize >> 2;
	chksum = 0;
	for(i = 0;i < len;i++) chksum += *lp++;
	LONGtoBYTE(chksum, p); p += 4;
cprintf("%lu slvnets(%d): Tx Req-XferWRQ\n", DS_TIMER, id);
	XferCount = 0;
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(id, p - p0, 0);
}

void CliXferData(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0, *s;
	int		id, dataSize;

	id = cr->ID;
	p = p0 = SlaveNetGetTxReqData(id);
	*p++ = P_XFER_DATA;
	if(XferSize < XferCount + XFER_DATA_SIZE) dataSize = XferSize - XferCount;
	else	dataSize = XFER_DATA_SIZE;
	s = (unsigned char *)XferBuf; s += XferCount;
	memcpy(p, s, dataSize);
	p += dataSize;
//cprintf("slvnets(%d): Tx Req-XferData: %d+%d/%d\n", id, XferCount, dataSize, XferSize);
	XferCount += dataSize;
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(id, p - p0, 1);
}

void CliXferError(CREDENTIAL_READER *cr, int result)
{
	unsigned char	*p, *p0;
	int		id;

	id = cr->ID;
	p = p0 = SlaveNetGetTxReqData(id);
	*p++ = P_XFER_ERROR;
	*p++ = result;
cprintf("%lu slvnets(%d): Tx Req-XferError: result=%d\n", DS_TIMER, id, result);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(id, p - p0, 1);
}
