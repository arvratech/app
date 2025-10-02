#include "slvnet_sio.h"


void _SiosSubdevRemoveQueue(void)
{
	SQ_NODE		*node;
	QUEUE	*h, *q, *q2;

	h = &rxQueue;
	for(q = QUEUE_NEXT(h);q != h; ) {
		node = QUEUE_DATA(q, SQ_NODE, queue);
		q2 = QUEUE_NEXT(q);
		if(node->type == 1) {
			QUEUE_REMOVE(q);
		}
		q = q2;
	}
}

void _SiosSubdevConnected(SLAVE *slv, unsigned char *buf)
{
	unsigned short	devId;
	int		len;

	BYTEtoSHORT(buf+4, &devId);
	unitRegs[0][3] = devId >> 8;  unitRegs[0][4] = devId;
	unitSubdevConnected(0, 0);
	slv->state = 1;
	slv->dataRetryCount = 0;
	_SiosSubdevRemoveQueue();
cprintf("%lu SlvnetConnected...%d dev=%d\n", MS_TIMER, (int)(slv->address & 0x1f), (int)devId);
}

void _SiosSubdevDisconnected(SLAVE *slv)
{
	if(unitSubdevConnection(0, 0)) {
		unitSubdevDisconnected(0, 0);
cprintf("%lu SlvnetDisconnected...%d\n", MS_TIMER, (int)(slv->address & 0x1f));
	}
	slv->state = 0;
	retryCount = 0;
	slv->txSN = 0; slv->rxSN = 1;
	slv->dataRetryCount = 0;
	_SiosSubdevRemoveQueue();
}

void _SiosSubdevCheckAddress(SLAVE *slv)
{
	unsigned char	addr;

	addr = devSubdeviceId() + 0x60;
	if(addr != slv->address) {
		_SiosSubdevDisconnected(slv);
		slv->address = addr;
		slvReset = 1;
	}	
}

void slvnetSiosSubdevDisconnect(SLAVE *slv)
{
	_SiosSubdevDisconnected(slv);
}

#define T_INACTIVITY	1200		//	1.2 seconds

int _SiosSubdevTxStart(SLAVE *slv, unsigned char *buf)
{
	QUEUE		*q, *h;
	SQ_NODE		*node;
	unsigned char	*p, c, c_h, c_l;
	int		i, len, addr;

	if(slv->state < 2) {
		p = buf;
		*p++ = 0x66;
		*p++ = slv->address;
		if(slvReset) c = 1; else c = 0;
		*p++ = c;
		*p++ = 0; *p++ = 0; *p++ = 0; *p++ = 0;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ buf[i]];		
		*p++ = c;
	} else {
		if(slv->dataRetryCount) {
			slv->dataRetryCount++;
			len = slv->dataSize;
		} else {
			len = 0;
			h = &rxQueue; 
			for(q = QUEUE_NEXT(h);q != h;q = QUEUE_NEXT(q)) {
				node = QUEUE_DATA(q, SQ_NODE, queue);
				if(node->type == 1) {
					QUEUE_REMOVE(q);
					len = node->length - 1;
					memcpy(slv->data, node->data+1, len); slv->dataSize = len;
					slv->txSN = !slv->txSN;
					break;
				}
			}
		}
		p = buf;
		*p++ = 0x66;
		*p++ = slv->address;
		if(slv->txSN) c = 6; else c = 4; if(!slv->rxSN) c++;
		*p++ = c;
		*p++ = 0; *p++ = 0;
		*p++ = len >> 8; *p++ = len;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ buf[i]];		
		*p++ = c;
		if(len) {
			memcpy(p, slv->data, len);
			c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
			for(i = 0;i < len;i++) {
				c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
			}
			p += len; *p++ = c_h; *p++ = c_l;
		}
	}
	*p++ = 0xff;		// null padding for RS-485
//len = p - buf - 1;
//cprintf("%lu tx: %d [%02x", MS_TIMER, len, (int)buf[0]); for(i = 1;i < len;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	return p - buf;
}

int _SiosSubdevRxCompleted(SLAVE *slv, unsigned char *buf, int size)
{
	SQ_NODE		*node;
	QUEUE		queue;
	unsigned char	*p;
	unsigned char	c, val, c_h, c_l;
	int		i, n, len, rxFlag;

//cprintf("%lu rx: %d [%02x", MS_TIMER, size, (int)buf[0]); for(i = 1;i < size;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	rxFlag = 0; val = buf[2];		// Frame Type
	if(size >= 8 && buf[0] == 0x55 && buf[1] == slv->address && (val < 2 || val > 3 && val < 8)) {
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ buf[i]];
		if(c == buf[7]) rxFlag = 1;
		else { slv->crcErrorCount++; slv->rxError = 1; }
	} else if(!slv->rxError) {
		slv->crcErrorCount++; slv->rxError = 1;
	}
	if(rxFlag) {
		c = buf[2];
		if(c < 2) {
			_SiosSubdevConnected(slv, buf);
		} else {
			if(slv->state < 3) {
				slvReset = 0; slv->state++;
			}
			n = 0; len = (buf[5] << 8) + buf[6];
			if(size >= len + 10) {
				p = buf + 8; c_h = 0xff; c_l = 0xff;	// CRC High - CRC Low
				for(i = 0;i < len;i++) {
					c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
				}
				if(c_h == p[i] && c_l == p[i+1]) n = 1;
			}
			val = buf[2];		// Frame Type
			// Data
			if(val < 6) c = 0; else c = 1;
			if(c == slv->rxSN && (!len || n)) slv->rxSN = !slv->rxSN;
			else	len = 0;
			// Ack
			if(val & 1) c = 1; else c = 0;
			if(c == slv->txSN) {					// Normal
				if(slv->dataRetryCount) {
					slv->dataRetryCount = 0;		// Data Ack
				}	
			} else if(slv->dataRetryCount > 2) {	// Duplicate - Data Failed
				slv->txSN = !slv->txSN; slv->dataRetryCount = 0;
			}	
			if(len) {
				node = txQueueNodeAlloc(len+1);
				node->type = 1;
				node->data[0] = 0;
				memcpy(node->data+1, buf+8, len);
				QUEUE_INSERT_TAIL(&txQueue, &node->queue);
//cprintf("ins txQueue=%x len=%d\n", node, len+1);
				devSetBufIrq();
 			}
		}
		slv->inactivityTimer = DS_TIMER;
	}
	return rxFlag;
}

void _SiosSubdevCheckInactivity(SLAVE *slv)
{
	if(!unitSubdevConnection(0, 0)) _SiosSubdevRemoveQueue();
	else if(slv->address == 0x7f || (MS_TIMER-slv->inactivityTimer) >= T_INACTIVITY) {
		_SiosSubdevDisconnected(slv);
	}
}
