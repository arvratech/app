#define MAX_IBUF_SZ		112
#define MAX_IDATA_SZ	100


void _SlvnetsConnected(SLAVE *slv)
{
	unitSubdevConnected(0, 0);
	slv->dataRetryCount = 0;
	sqRemoveSubdev(txQueue, (unsigned char)0);
cprintf("%u SlvnetsConnected...\n", MS_TIMER);

}

void _SlvnetsDisconnected(SLAVE *slv)
{
	if(unitSubdevConnection(0, 0)) {
		unitSubdevDisconnected(0, 0);
cprintf("%ld SlvenetsDisconnected...%d\n", MS_TIMER, (int)(slv->address & 0x1f));
	}
	slv->state = 0;
	retryCount = 0;
	slv->txSN = 0; slv->rxSN = 1;
	slv->dataRetryCount = 0;
	sqRemoveSubdev(txQueue, (unsigned char)0);
}

#define T_INACTIVITY	1200		//	1.2 seconds

int _SiosSubdevTxStart(SLAVE *slv, unsigned char *buf)
{
	SQ_NODE		*node;
	unsigned char	*p, c, c_h, c_l;
	int		i, len, addr;

	if(!slv->state) {
		slv->state = 1;
		_SlvnetsConnected(slv);
		 len = 0;
	} else if(slv->dataRetryCount) {
		len = slv->dataSize;
	} else {
		addr = 0;
		node = sqFindSubdev(rxQueue, (unsigned char)0);
		if(node) {
			len = node->length - 1;
			memcpy(slv->data, node->data+1, len); slv->dataSize = len;
			slv->txSN = !slv->txSN;
		} else	len = 0;
	}
	p = buf;
	*p++ = 0x66;
	*p++ = slv->address;
	// 1:Data0-Ack0  2:Data0-Ack1  3:Data1-Ack0  4:Data1-Ack1
	if(slv->txSN) c = 3; else c = 1; if(!slv->rxSN) c++;
	*p++ = c;		// FrameType	
	*p++ = len >> 8; *p++ = len;
	for(i = 1, c = 0xff;i < 5;i++) c = crc8tab[c ^ buf[i]];		
	*p++ = c;
	if(len) {
		slv->dataRetryCount++;
		memcpy(p, slv->data, len);
		c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
		for(i = 0;i < len;i++) {
			c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
		}
		p += len; *p++ = c_h; *p++ = c_l;
	}
	*p++ = 0xff;		// null padding for RS-485
	return p - buf;
//cprintf("Tx: %d [%02x", txSize-1, (int)txBuf[0]); for(i = 1;i < txSize-1;i++) cprintf("-%02x", (int)txBuf[i]); cprintf("]\n");
}

void _SiosSubdevRxCompleted(SLAVE *slv, unsigned char *buf, int size)
{
	SQ_NODE		*node;
	unsigned char	*p;
	unsigned char	c, val, c_h, c_l;
	int		i, n, len, addr;

cprintf("sios Rx: %d [%02x", size, (int)buf[0]); for(i = 1;i < size;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	rxFlag = 0; val = buf[2];		// Frame Type
	if(size > 5 && buf[0] == 0x55 && buf[1] == slv->address && val && val < 5) {
		for(i = 1, c = 0xff;i < 5;i++) c = crc8tab[c ^ buf[i]];
		if(c == buf[5]) rxFlag = 1;
		else { slv->crcErrorCount++; slv->rxError = 1; }
	} else if(!slv->rxError) {
		slv->crcErrorCount++; slv->rxError = 1;
	}
	if(rxFlag) {
		addr = 0;
		if(!slv->state) {
			len = 0;
		} else {
			n = 0; len = (buf[3] << 8) + buf[4];
			if(size >= len + 8) {
				p = buf + 6; c_h = 0xff; c_l = 0xff;	// CRC High - CRC Low
				for(i = 0;i < len;i++) {
					c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
				}
				if(c_h == p[i] && c_l == p[i+1]) n = 1;
			}
		}
		// Data
		if(val < 3) c = 0; else c = 1;
		if(c == slv->rxSN && (!len || n)) slv->rxSN = !slv->rxSN;
		else	len = 0;
		// Ack
		if(val & 1) c = 0; else c = 1;
		if(c == slv->txSN) {					// Normal
			 if(slv->dataRetryCount) slv->dataRetryCount = 0; // Data Ack
		} else if(slv->dataRetryCount > 2) {	// Duplicate - Data Failed
			slv->txSN = !slv->txSN; slv->dataRetryCount = 0;
		}
		if(len > 3) {
			node = txQueueNodeAlloc(len+1);
			node->type = 1;
			node->data[0] = addr;
			memcpy(node->data+1, p, len);
			sqEnqueue(txQueue, node);
			devSetBufIrq();
		}
	}
}

void _SiosSubdevCheckInactivity(SLAVE *slv)
{	
	if(unitSubdevConnection(0, 0) && (slv->address == 0x7f || (MS_TIMER-slv->inactivityTimer) >= T_INACTIVITY)) {
		_SlvnetsDisconnected(slv);
	}
}
