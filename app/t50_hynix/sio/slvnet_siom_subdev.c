void _SiomSubdevsCheckQueue(void)
{
	SQ_NODE		*node;
	QUEUE	*h, *q, *q2;
	int		addr;

	h = &rxQueue;
	for(q = QUEUE_NEXT(h);q != h; ) {
		node = QUEUE_DATA(q, SQ_NODE, queue);
		q2 = QUEUE_NEXT(q);
		if(node->type == 1) {
			addr = node->data[0];
			if(addr >= slaveSize || !slaves[addr].state) QUEUE_REMOVE(q);
		}
		q = q2;
	}
}

void _SiomSubdevRemoveQueue(SLAVE *slv)
{
	SQ_NODE		*node;
	QUEUE	*h, *q, *q2;

	h = &rxQueue;
	for(q = QUEUE_NEXT(h);q != h; ) {
		node = QUEUE_DATA(q, SQ_NODE, queue);
		q2 = QUEUE_NEXT(q);
		if(node->type == 1 && node->data[0] == slv->address & 0x1f) {
			QUEUE_REMOVE(q);
		}
		q = q2;
	}
}

void _SiomSubdevConnected(SLAVE *slv)
{
	int		i, addr, id, ch;

	addr = slv->address & 0x1f;
	id = addr >> 2; ch = addr & 3;	
	unitSubdevConnected(id, ch);
cprintf("%u Subdev connected: %d\n", MS_TIMER, addr);
	slv->dataRetryCount = 0;
	_SiomSubdevRemoveQueue(slv);
}

void _SiomSubdevDisconnected(SLAVE *slv)
{
	int		addr, id, ch;

	addr = slv->address & 0x1f;
	id = addr >> 2; ch = addr & 3;	
	unitSubdevDisconnected(id, ch);
cprintf("%u Subdev disconnected: %d\n", MS_TIMER, addr);
	slv->inactivityTimer = MS_TIMER;
	slv->state = 0;
	retryCount = 0;
	slv->txSN = 0; slv->rxSN = 1;
	slv->dataRetryCount = 0;
	_SiomSubdevRemoveQueue(slv);
}

void slvnetSiomSubdevDisconnect(void)
{
}

int _SiomSubdevTxStart(SLAVE *slv, unsigned char *buf)
{
	QUEUE		*q, *h;
	SQ_NODE		*node;
	unsigned char	*p, c, c_h, c_l;
	int		i, len, addr;

	addr = slv->address & 0x1f;
	if(!slv->state) len = 0;
	else if(slv->dataRetryCount) len = slv->dataSize; 
	else {
		len = 0;
		h = &rxQueue; 
		for(q = QUEUE_NEXT(h);q != h;q = QUEUE_NEXT(q)) {
			node = QUEUE_DATA(q, SQ_NODE, queue);
			if(node->type == 1 && node->data[0] == slv->address & 0x1f) {
				QUEUE_REMOVE(q);
				len = node->length - 1;
				memcpy(slv->data, node->data+1, len); slv->dataSize = len;
				slv->txSN = !slv->txSN;
				break;
			}
		}
	}
	p = buf;
	*p++ = 0x55;
	*p++ = slv->address;
	// 1:Data0-Ack0  2:Data0-Ack1  3:Data1-Ack0  4:Data1-Ack1
	if(slv->txSN) c = 3; else c = 1; if(!slv->rxSN) c++;
	*p++ = c;		// FrameType
	*p++ = len >> 8; *p++ = len;
	for(i = 1, c = 0xff;i < 5;i++) c = crc8tab[c ^ txBuf[i]];		
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

int _SiomSubdevRxCompleted(SLAVE *slv, unsigned char *buf, int size)
{
	SQ_NODE		*node;
	unsigned char	*p;
	unsigned char	c, val, c_h, c_l;
	int		i, n, len, addr, rxFlag;
	
cprintf("Rx: %d [%02x", size, (int)buf[0]); for(i = 1;i < size;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	rxFlag = 0; val = buf[2];	// Frame Type
	if(size > 5 && buf[0] == 0x66 && buf[1] == slv->address && val && val < 5) {
		for(i = 1, c = 0xff;i < 5;i++) c = crc8tab[c ^ buf[i]];
		if(c == buf[5]) rxFlag = 1;
		else { slv->crcErrorCount++; slv->rxError = 1; }
	} else if(!slv->rxError) {
		slv->crcErrorCount++; slv->rxError = 1;
	}
	if(rxFlag) {
//cprintf("r[%02x", (int)buf[0]); for(i = 1;i < size;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
		addr = buf[1] & 0x1f;
		if(!slv->state) {
			slv->dataRetryCount = 0;
			slv->state = 1;
			_SiomSubdevConnected(slv);
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
			QUEUE_INSERT_TAIL(&txQueue, &node->queue);		
			devSetBufIrq();
		}
	}
	return rxFlag;
}
