void cardPrint(unsigned char *buf)
{
	int		i;

	printf("[%02x", (int)buf[0]);
	for(i = 1;i < 9;i++) printf("-%02x", (int)buf[i]);
	printf("]\n");
}

void bin2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x10 + len;
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, 0x00, len);
}

// len: nibble length
void bcd2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;
	int		c, val;

	p = buf;
	*p++ = 0x20;
	val = len >> 1;
	memcpy(p, data, val); p += val;
	if(len & 1) {
		c = data[val]; c |= 0x0f;
		*p++ = c; val++;
	}
	val = buf + 9 - p;
	if(val > 0) memset(p, 0xff, val);
}

void digits2card(char *data, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x20;
	string2bcd(data, 16, p);
}

void digitslen2card(char *data, int len, unsigned char *buf)
{
	unsigned char	*p,  tmpbuf[20];
	
	memcpy(tmpbuf, data, len); tmpbuf[len]  = 0;
	p = buf;
	*p++ = 0x20;
	string2bcd(tmpbuf, 16, p);
}

void str2card(char *data, unsigned char *buf)
{
	unsigned char	*p;
	int		len;

	len = strlen(data);
	p = buf;
	*p++ = 0x30;	
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, ' ', len);
}

void strlen2card(char *data, int len, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x30;	
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, ' ', len);
}

int cardisdigits(unsigned char *data)
{
	unsigned char	*p;
	int		i, c, n, cnt;
	
	p = data + 1;
	for(i = cnt = 0;i < 8;i++) {
		c = p[i];
		n = c >> 4;
		if(n >= 0 && n <= 9) cnt++;
		else if(n == 0xf) break;
		else	return -1;
		n = c & 0x0f;
		if(n >= 0 && n <= 9) cnt++;
		else if(n == 0xf) break;
		else	return -1;
	} 
	return cnt;
}

/*

int _CardReadSerialRF900(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	int		rval;
		
//#ifdef CARD_DEBUG
printf("Serial RF900: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	return 0;
}

int _CardReadSerialBarCode(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	unsigned char	*p;
	int		rval, dataType, dataPos, dataLen;

#ifdef CARD_DEBUG
printf("Serial BarCode: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
#endif
	dataType = sys_cfg->extData[1];	dataPos = sys_cfg->extData[2]; dataLen = sys_cfg->extData[3]; 
	len = buf[0]; p = buf + 1;
	if(sys_cfg->extData[3] == 0 || len < dataPos+dataLen) rval = 0;
	else {
		p += dataPos;
		if(dataType == 3) {			// BCD code
			dataLen <<= 1;
			bcd2card(p, dataLen, cr->data);
		} else if(dataType == 2) {	// Binary
			bin2card(p, dataLen, cr->data);	
		} else {					// Digits
			p[dataLen] = 0;
			digits2card((char *)p, cr->data);
		}
		if(dataType != 2) {
			rval = cardisdigits(cr->data);
printf("%d %d [", dataLen, rval); for(dataPos = 0;dataPos < 9;dataPos++) printf("%02x", (int)cr->data[dataPos]); printf("]\n");
			if(rval == dataLen) rval = 1;
			else	rval = 0;
		} else	rval = 1;
	}
	return rval;
}

int _CardReadSerialEM(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	unsigned char	checksum;
	int		i, rval, dataType;

//#ifdef CARD_DEBUG
printf("Serial EM: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	rval = 0;
	if(len == 9 && buf[0] == ASCII_STX && buf[8] == ASCII_ETX && buf[1] == 9) {
		checksum = 0;
		for(i = 0;i < 5;i++) checksum += buf[i+2];
		if(checksum == buf[7]) rval = 1;	
	}
	if(rval) {
		dataType = (cr->SCDataType >> 4) & 0x0f;
		if(dataType == 3) {
			buf[4] &= 0x0f;
			bin2card(buf+4, 3, cr->data);
		} else if(dataType == 2) {
			bin2card(buf+4, 3, cr->data);
		} else {
			bin2card(buf+3, 4, cr->data);
		}
		rval = 9;
	}
	return rval;
}

int _CardReadSerialHID(CREDENTIAL_READER *cr, unsigned char *buf, int len)
{
	int		rval;

//#ifdef CARD_DEBUG
printf("Serial HID: %d [", len); for(rval = 0;rval < len;rval++) printf("%02x", (int)buf[rval]); printf("]\n");
//#endif
	if(len == 18) {
		buf[0] = (buf[14] << 6) | (buf[15] >> 2);
		buf[1] = (buf[15] << 6) | (buf[16] >> 2);
		buf[2] = (buf[16] << 6) | (buf[17] >> 2);
		buf[3] = buf[17] << 6;
		rval = wgCheck26Data(buf);
		if(rval == 26) {
			bin2card(buf, 3, cr->data);
			rval = 9;
		} else	rval = 0;
	} else	rval = 0;
	return rval;
}
*/
