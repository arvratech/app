
SLAVE *SiomGetSlave(unsigned char address)
{
	SLAVE	*slv;
	int		i;

	for(i = 0, slv = slaves;i < slaveSize;i++, slv++)
		if(slv->address == address) break;
	if(i >= slaveSize) slv = (SLAVE *)0;
	return slv;
}

static void _SiomAddSlave(unsigned char address)
{
	SLAVE	*slv;
	int		i;

cprintf("_AddSlave...0x%02x\n", (int)address);
	for(i = 0, slv = slaves;i < slaveSize;i++, slv++)
		if(slv->address == address) break;
	if(i >= slaveSize) {
		slaveSize++; 
		slv->address		= address;
		slv->state			= 0;
		slv->cmd			= 0;
		slv->cmdLoop		= 0;
		slv->txSN			= 0;
		slv->rxSN			= 1;
		slv->dataRetryCount	= 0;
		slv->inactivityTimer = MS_TIMER - INACTIVITY_TIMEOUT;
		slv->commErrorCount = slv->crcErrorCount = slv->timeoutCount = 0L;
	}
}

static void _SiomRemoveSlave(unsigned char address)
{
	SLAVE	*slv;
	int		i;

	for(i = 0, slv = slaves;i < slaveSize;i++, slv++)
		if(slv->address == address) break;
	if(i < slaveSize) {
		slaveSize--;
		for( ;i < slaveSize;i++, slv++) memcpy(slv, slv+1, sizeof(SLAVE));
	}
}

static void _SiomControlSlaves(void)
{
	unsigned char	m, msk, val;
	int		i, j, id;

	unitRegs[0][2] &= 0x70;	// Unit0-CH1 is always NULL
	for(i = id = 0;i < MAX_UNIT_SZ;i++, id += 4) {
		m = unitModel(i);
		if(m != _unitModels[i]) {
			_SiomRemoveSlave((unsigned char)i);
			for(j = 0;j < 4;j++) _SiomRemoveSlave((unsigned char)(id + j + 0x20));
			if(m > 4) m = 0xff;
			else if(m) _SiomAddSlave((unsigned char)i);
			_unitModels[i] = m;
		}
		if(m < 3) {
			m = unitSubdevModels(i);
			val = _unitSubdevModels[i] ^ m;
			if(val) {
				for(j = 0, msk = 0x08;j < 4;j++, msk >>= 1) {
					if(val & msk) {
						if(m & msk) _SiomAddSlave((unsigned char)(id + j + 0x20));
						else		_SiomRemoveSlave((unsigned char)(id + j + 0x20));	
					}
				}
				_unitSubdevModels[i] = m;
			}
		}
	}
}

static void _SiomStop(void)
{
	memset(_unitModels, 0xf0, MAX_UNIT_SZ);
	slaveSize = 0;
}

void _SiomUnitConnected(SLAVE *slv)
{
	int		addr;

	addr = slv->address & 0x1f;
cprintf("%u Unit connected: %d\n", MS_TIMER, addr);
	slv->dataRetryCount = 0;
	slv->cmd = slv->invokeId = 0;
	cbufFlush(unitCmds(addr));		
	unitSetSystemStatus(addr, 0);
}

void _SiomUnitDisconnected(SLAVE *slv, int status)
{
	int		addr;

	addr = slv->address & 0x1f;
cprintf("%u Unit disconnected: %d status=%d\n", MS_TIMER, addr, status);
	if(unitSystemStatus(addr) != status) {
		unitSetSystemStatus(addr, status);
	}	
	slv->inactivityTimer = MS_TIMER;
	slv->state = 0;
	retryCount = 0;
	slv->txSN = 0; slv->rxSN = 1;
	slv->dataRetryCount = 0;
}

int _SiomUnitTxStart(SLAVE *slv, unsigned char *buf)
{
	unsigned char	*p, c, c_h, c_l;
	int		i, len, addr;

	addr = slv->address & 0x1f;
	if(!slv->state) len = 0;
	else if(slv->dataRetryCount) len = slv->dataSize; 
	else if(slv->cmd) len = 0;
	else {
		p = slv->data + 2;
		if(!unitSystemStatus(addr)) {
			len = cbufGet(unitCmds(addr), p, 36);
			if(len > 0) slv->cmd = 2;				// 2:WriteCommand
			else if(unitSyncs[addr]) slv->cmd = 4;	// 4:WriteRegister
		} else if(slv->state == 1) {
			slv->cmd = 5; len = 0;	// 5:ReadRegister
		} else {
			slv->cmd = 4;			// 4:WriteRegister
		}
		if(slv->cmd == 4) {
			len = unitRegSize(unitModel(addr)) - 2;
			memcpy(p, unitRegs(addr)+2, len);
			unitSyncs[addr] = 0;
		}
		if(slv->cmd) {
			slv->data[0] = slv->cmd;
			slv->invokeId++;
			slv->data[1] = slv->invokeId;
			len += 2;
			slv->dataSize = len;
			slv->txSN = !slv->txSN;
		} else	len = 0;
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
		slv->dataRetryCount++; slv->cmdLoop = 0;
		memcpy(p, slv->data, len);
		c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
		for(i = 0;i < len;i++) {
			c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
		}
		p += len; *p++ = c_h; *p++ = c_l;
	}
	*p++ = 0xff;		// null padding for RS-485
	return p - buf;
//if(len) { cprintf("tx[%02x", (int)txBuf[0]); for(i = 1;i < txSize-1;i++) cprintf("-%02x", (int)txBuf[i]); cprintf("]\n"); }
}

int _SiomUnitRxCompleted(SLAVE *slv, unsigned char *buf, int size)
{
	SQ_NODE		*node;
	unsigned char	*p, c, val, c_h, c_l;
	int		i, n, len, addr, stat, rxFlag;

//cprintf("Rx: %d [%02x", rxCount, (int)rxBuf[0]); for(i = 1;i < rxCount;i++) cprintf("-%02x", (int)rxBuf[i]); cprintf("]\n");
	rxFlag = 0; val = buf[2];	// Frame Type
	if(size > 5 && buf[0] == 0x66 && buf[1] == slv->address && val && val < 5) {
		for(i = 1, c = 0xff;i < 5;i++) c = crc8tab[c ^ buf[i]];
		if(c == buf[5]) rxFlag = 1;
		else { slv->crcErrorCount++; slv->rxError = 1; }
	} else if(!slv->rxError) {
		slv->crcErrorCount++; slv->rxError = 1;
	}
	if(rxFlag) {
		addr = buf[1] & 0x1f;	
		if(!slv->state) {
			slv->cmd = 0; slv->dataRetryCount = 0;
			slv->state = 1;
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
			if(slv->cmd == 4) unitSyncs[addr] = 1;
			if(unitSystemStatus(addr)) _SiomUnitDisconnected(slv, 3);
			slv->cmd = 0;
		}
		if(len > 0) {
			stat = unitSystemStatus(addr);	
			c = *p++; len--;
			if(c == 0) {			// 0:Result 
				val = *p++; n = *p++; len -= 2;
				if(slv->cmd && !slv->dataRetryCount && val == slv->invokeId) { 
					if(n) {					
						_SiomUnitDisconnected(slv, 3);
					} else if(stat) {
						if(slv->cmd == 5) {			// 5:ReadRegister
							n = *p; unitSetModel(addr, n);
//cprintf("5: Model=%d %d\n", (int)unitModel(addr), (int)n);
							if(n == unitModel(addr)) slv->state = 2;
							else	_SiomUnitDisconnected(slv, 2);
						} else if(slv->cmd == 4) {	// 4:WriteRegister
							n = unitStatSize(unitModel(addr));
							if(n > len) n = len;
							memcpy(unitStats(addr)+2, p, n);
//cprintf("4: SN %d [%02x", len, (int)p[0]); for(i = 1;i < n;i++) cprintf("-%02x", (int)p[i]); cprintf("]\n");
							_SiomUnitConnected(slv);
						}
					}
					slv->cmd = 0;
				}
			} else if(c == 11) {	// 11:StatusNotification
				if(!stat) {
					n = unitStatSize(unitModel(addr));
					if(n > len) n = len;
					memcpy(unitStats(addr)+2, p, n);
					unitSetStatusIrq(addr);
//cprintf("%u SN %d [%02x", MS_TIMER, len, (int)p[0]); for(i = 1;i < n;i++) cprintf("-%02x", (int)p[i]); cprintf("]\n");
				}
			} else if(c == 12) {	// 12:CredentialNotification)
//cprintf("%u CN %d [%02x", MS_TIMER, len, (int)p[0]); for(i = 1;i < len;i++) cprintf("-%02x", (int)p[i]); cprintf("]\n");
				if(!stat) {
					while(len > 3) {
						// |   CH   |  type  | length |  data  |  data  | ...
						n = (addr << 2) + p[0];
						i = (p[2] + 7) << 3;
						if(len < i + 3) break;
						node = (SQ_NODE *)txQueueNodeAlloc(i+2);
						node->data[0] = 0; node->data[1] = n;
						memcpy(node->data+2, p+1, i);
						i += 3;
						p += i; len -= i;
					}
					devSetBufIrq();
				}
			}
		}
		if(slv->cmd) {
			slv->cmdLoop++;
			if(slv->cmdLoop > 10) {
				if(slv->cmd == 4) unitSyncs[addr] = 1;
				slv->cmd = 0;
				_SiomUnitDisconnected(slv, 3);
			}
		}
	}
	return rxFlag;
}
