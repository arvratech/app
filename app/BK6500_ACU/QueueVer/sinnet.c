unsigned long sinTxTimer, sinRxTimer;

// Return  -2:ProtocolError -1:Closed 0:None 12:Readed
int sinReadNet(int s, unsigned short *pDestinationAddress, NET *net)
{
	unsigned short	sVal, buf[8];
	unsigned char	*p;
	int		rval, val, size;

	val = _ReadShort(Sn_SSR(s)) & 0xff;
	if(val != SOCK_ESTABLISHED) {
#ifdef SIN_DEBUG
		cprintf("%ld sinReadNet(%d): Sn_SSR=%02x\n", DS_TIMER, s, val);
#endif
		return -1;
	}
	rval = _ReadLong(Sn_RX_RSR(s));
	if(rval < 12) rval = 0;
	else {
		 _ReadFifoData2(s, buf, 12);
		p = (unsigned char *)buf;
//cprintf("FIFO=%d [%02x", rval, (int)p[0]); for(val = 1;val < 12;val++) cprintf(" %02x", (int)p[val]); cprintf("]\n");
		val = *p++;						// Link(OnTCP): ProtocolID
		if(val != PROTOCOL_ID) rval = -2;
		BYTEtoSHORT(p, &sVal); p += 2;	// Link(OnTCP): Length
		size = sVal;
		if(size < 9 || size > NET_NAX_PKT_SZ) rval = -2;
		val = *p++;						// Network: Control
		BYTEtoSHORT(p, &net->Address); p += 2;		 // Network: Source Address
		BYTEtoSHORT(p, pDestinationAddress); p += 2; // Network: Destination Address
		net->Type     = *p++;			// Application: PDUType + MORE/SRV
		net->InvokeID = *p++;			// Application: InvokeID
		net->Head0    = *p++;			// Application: ServiceChoice, ErrorClass, RejectReason, AbortReason   
		net->Head1    = *p++;			// Application: ErrorCode
		if(rval > 0) {	
			size -= 9;
			val = net->Type >> 4;
			if(val == CONFIRMED_REQUEST_PDU || val == UNCONFIRMED_REQUEST_PDU || val == RESPONSE_PDU) {
				net->BufferLength = size; net->Count = 0;
				sinRxTimer = DS_TIMER;
			} else {
				if(size) rval = -2;
				else {
					net->BufferLength = 0; net->Count = 0;
				}
			}
		}
	}
#ifdef SIN_DEBUG
	if(rval < 0) cprintf("%ld sinReadNet(%d): ProtocolError\n", DS_TIMER, s);
	else if(rval > 0) cprintf("%ld sinReadNet(%d)=%d\n", DS_TIMER, s, rval);
#endif
	return rval;
}

// Return  -2:Timeout -1:Closed 0:Continue 12:Completed
int sinReadNetCompleted(int s, NET *net)
{
	int		size, length, remain;

	length = net->BufferLength;
	remain = length - net->Count;
	if(remain > 0) {
		size = _ReadShort(Sn_SSR(s)) & 0xff;
		if(size != SOCK_ESTABLISHED) size = -1;
		else {
			size = _ReadLong(Sn_RX_RSR(s));
			if(size) {
				if(size > remain) size = remain;
				 _ReadFifoData2(s, net->Buffer+net->Count, size);
				net->Count += size;
			} else if((DS_TIMER-sinRxTimer) > 90) size = -2;
		}
	} else size = 0;
	if(size >= 0) {
		if(net->Count < length) size = 0;
		else	size = 1;
	}
#ifdef SIN_DEBUG
	if(size < -1) cprintf("%ld sinReadNetCompleted(%d): Timeout\n", DS_TIMER, s);
	else if(size < 0) cprintf("%ld sinReadNetCompleted(%d): Sn_SSR=%02x\n", DS_TIMER, s, _ReadShort(Sn_SSR(s)));
	else if(size) cprintf("%ld sinReadNetCompleted(%d): OK\n", DS_TIMER, s);
#endif
	return size;
}

int sinTxBufferFull(int s)
{
	int		val;

	val = _ReadLong(Sn_TX_FSR(s));
	if(val >= 8192) val = 1;
	else	val = 0;
	return val;
}

// Return  -1:Closed 0:None >0:Writed
int sinWriteNet(int s, unsigned short SourceAddress, NET *net)
{
	unsigned short	sVal, buf[8];
	unsigned char	*p;
	int		val, type, size;

	val = _ReadShort(Sn_SSR(s)) & 0xff;
	if(val != SOCK_ESTABLISHED) {
#ifdef SIN_DEBUG
		cprintf("%ld sinWriteNet(%d): Sn_SSR=%02x\n", DS_TIMER, s, val);
#endif
		return -1;
	}
	type = net->Type >> 4;
	size = 12;
	if(type == CONFIRMED_REQUEST_PDU || type == UNCONFIRMED_REQUEST_PDU || type == RESPONSE_PDU) size += net->BufferLength;
	val = _ReadLong(Sn_TX_FSR(s));
	if(size > val) size = 0;
	else {
		p = (unsigned char *)buf;
		*p++ = PROTOCOL_ID;				// Link(OnTCP): ProtocolID
		sVal = size - 3;
		SHORTtoBYTE(sVal, p); p += 2;	// Link(OnTCP): Length
		*p++ = 0;						// Network: Control
		SHORTtoBYTE(SourceAddress, p); p += 2;	// Network: Source Address
		SHORTtoBYTE(net->Address, p); p += 2;	// Network: Destination Address
		*p++ = net->Type;				// Application: PDUType + MORE/SRV
		*p++ = net->InvokeID;			// Application: InvokeID
		*p++ = net->Head0;				// Application: ServiceChoice, ErrorClass, RejectReason, AbortReason   
		*p++ = net->Head1;				// Application: ErrorCode
		_WriteFifoData(s, buf, 12);
		val = size - 12;
		if(val) _WriteFifoData(s, net->Buffer, val);
		_WriteLong(Sn_TX_WRSR(s), size);
		_WriteShort(Sn_CR(s), Sn_CR_SEND);
		sinTxTimer = DS_TIMER;
	}
#ifdef SIN_DEBUG
	if(size) cprintf("%ld sinWriteNet(%d)=%d\n", DS_TIMER, s, size);
#endif
	return size;
}

// Return  -2:Error -1:Closed 0:None 1:Completed
int sinWriteNetCompleted(int s)
{
	int		rval, val;

	val = _ReadShort(Sn_SSR(s)) & 0xff;
	if(val != SOCK_ESTABLISHED) {
#ifdef SIN_DEBUG
		cprintf("%ld sinWriteNetCompleted(%d): Sn_SSR=%02x\n", DS_TIMER, s, val);
#endif
		return -1;
	}
	val = _ReadShort(Sn_IR(s)) & Sn_IR_SEND_FLAG;	// SENDOK, TIMEOUT, DISCON 
	if(val) {
		_WriteShort(Sn_IR(s), val);
		if(val == Sn_IR_SENDOK) rval = 1;
		else	rval = -2;
	} else if((DS_TIMER-sinTxTimer) > 90) rval = -1;
#ifdef SIN_DEBUG
	if(rval < -1) cprintf("%ld sinWriteNetCompleted(%d): Sn_IR=%02x\n", DS_TIMER, s, val);
	else if(rval < 0) cprintf("%ld sinWriteNetCompleted(%d): Timeout\n", DS_TIMER, s);
	else if(rval > 0) cprintf("%ld sinWriteNetCompleted(%d): OK\n", DS_TIMER, s);
#endif
	return rval;
}
