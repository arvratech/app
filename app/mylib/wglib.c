#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "wglib.h"

extern const unsigned char _OneCountTbl[];


int wgEncode(unsigned char *rawData, int bitLen, unsigned char *encData)
{
	unsigned char	*s;
	int		i, val, m, n, plen, mask, ep, op;

	s = rawData;
	plen = (bitLen + 1) >> 1;
	ep = 0; n = plen; i = val = 0;
	while(n > 3) {
		if(i & 1) m = val  & 0xf;
		else {			
			val = *s++; m = val >> 4;
		}
		ep += _OneCountTbl[m];
		n -= 4; i++;
	}
	if(n) {
		if(i & 1) m = val  & 0xf;
		else	m = (*s) >> 4;
		m &= 0xf0 >> n;
		ep += _OneCountTbl[m];
	}
	n = bitLen - plen;
	s = rawData + (n >> 3);
	mask = n & 0x3;
	op = 0;
	if((n & 0x7) < 4) i = 0;
	else {
		i = 1; val = *s++;
	}
	n = plen + mask;
	mask = 0xf >> mask;
	if(n > 3) {
		if(i & 1) m = val  & 0xf;
		else {			
			val = *s++; m = val >> 4;
		}
		m &= mask;
		op += _OneCountTbl[m];
		n -= 4; i++; mask = 0xf;
	}
	while(n > 3) {
		if(i & 1) m = val  & 0xf;
		else {			
			val = *s++; m = val >> 4;
		}
		op += _OneCountTbl[m];
		n -= 4; i++;
	}
	if(n) {
		if(i & 1) m = val  & 0xf;
		else	m = (*s) >> 4;
		m &= mask & (0xf0 >> n);
		op += _OneCountTbl[m];
	}
	if(ep & 1) encData[0] = 0x80; else encData[0] = 0;
	bitcpy(encData, 1, rawData, 0, bitLen);
	if(op & 1) bitsetzero(encData, 1+bitLen, 1);
	else	bitsetone(encData, 1+bitLen, 1);
	return bitLen+2;
}

void wgReverse(unsigned char *src, unsigned char *dst, int len)
{
	unsigned char	*s, *d;

	s = src + len - 1;
	d = dst;
	while(len--) *d++ = *s--;
}

int wgDecode(unsigned char *encData, int bitLen, unsigned char *rawData)
{
	unsigned char	*s, e_p, o_p;
	int		len;

	s = encData;
	if(s[0] & 0x80) e_p = 1; else e_p = 0;
	//if(s[(bitLen+3)>>3] & (0x80 >> (bitLen & 0x07))) o_p = 1; else o_p = 0;
	//len = (bitLen + 1) >> 1;
	// modified at 2022.10.6
	len = bitLen - 1;
	if(s[len>>3] & (0x80 >> (len & 0x07))) o_p = 1; else o_p = 0;
	len = (bitLen - 2) >> 1;
	e_p += bitOneCount(s, 1, len);
	o_p += bitOneCount(s, bitLen-1-len, len);
	if(!(e_p & 0x01) && (o_p & 0x01)) {
		len = bitLen - 2;
		bitcpy(rawData, 0, s, 1, len);
	} else	len = 0;
	return len;
}

int wgDecode35Bits(unsigned char *encData, unsigned char *rawData)
{
	unsigned char	*s, *d, e_p, o_p, o2_p, se, so;
	int		rval;

	s = encData; d = rawData;
	if(s[0] & 0x40) se = e_p = 1; else se = e_p = 0;
	if(s[4] & 0x20) so = o_p = 1; else so = o_p = 0;
	if(s[0] & 0x80) o2_p = 1; else o2_p = 0;
	d[0] = (s[0] << 2) | (s[1] >> 6);
	d[1] = (s[1] << 2) | (s[2] >> 6);
	d[2] = (s[2] << 2) | (s[3] >> 6);
	d[3] = (s[3] << 2) | (s[4] >> 6);
	e_p += _OneCountTbl[(d[0] >> 4) & 0x0d];
	e_p += _OneCountTbl[d[0] & 0x0b];
	e_p += _OneCountTbl[(d[1] >> 4) & 0x06];
	e_p += _OneCountTbl[d[1] & 0x0d];
	e_p += _OneCountTbl[(d[2] >> 4) & 0x0b];
	e_p += _OneCountTbl[d[2] & 0x06];
	e_p += _OneCountTbl[(d[3] >> 4) & 0x0d];
	e_p += _OneCountTbl[d[3] & 0x0b];

	o_p += _OneCountTbl[(d[0] >> 4) & 0x0b];
	o_p += _OneCountTbl[d[0] & 0x06];
	o_p += _OneCountTbl[(d[1] >> 4) & 0x0d];
	o_p += _OneCountTbl[d[1] & 0x0b];
	o_p += _OneCountTbl[(d[2] >> 4) & 0x06];
	o_p += _OneCountTbl[d[2] & 0x0d];
	o_p += _OneCountTbl[(d[3] >> 4) & 0x0b];
	o_p += _OneCountTbl[d[3] & 0x06];
	o_p += se;

	o2_p += bitOneCount(d, 0, 32);

	o2_p += se + so;

	if(!(e_p & 0x01) && (o_p & 0x01) && (o2_p & 0x01)) rval = 32;
	else	rval = 0;
	return rval;
}

