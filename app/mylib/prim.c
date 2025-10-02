#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include "rtc.h"
#include "prim.h"


void initRand(void)
{
	unsigned long seed;
	int		fd, rval;

	fd = open("/dev/urandom", 0);
	if(fd < 0) {
		printf("open(/dev/urandom) error: %s", strerror(errno));
		seed = time(0);
	}
	rval = read(fd, &seed, sizeof(seed));
	if(rval < 0) {
		printf("read(/dev/urandom) error: %s", strerror(errno));
		seed = time(0);
	}
	if(fd >= 0) close(fd);
	srand(seed);
}

void memrand(unsigned char *buf, int length)
{
	unsigned long	*lp, val;
	int		i, n;


	lp = (unsigned long *)buf;
	n = length >> 2;
	for(i = 0;i < n;i++) *lp++ = rand();
	val = rand();
	memcpy(lp, &val, length & 3);
}

void BYTEtoSHORT(unsigned char *pbVal, unsigned short *psVal)
{
	unsigned short lVal1, lVal2;
	lVal1 = (unsigned short)(*pbVal) << 8;
	lVal2 = (unsigned short)(*(pbVal+1));
	*psVal = lVal1+lVal2;
}

void SHORTtoBYTE(unsigned short sVal, unsigned char *pbVal)
{
	*pbVal = (unsigned char)(sVal >> 8);
	*(pbVal+1) = (unsigned char)sVal;
}

void BYTEtoLONG(unsigned char *pbVal, unsigned long *plVal)
{
	unsigned long lVal1, lVal2, lVal3, lVal4;
	lVal1 = (unsigned long)(*pbVal) << 24;
	lVal2 = (unsigned long)(*(pbVal+1)) << 16;
	lVal3 = (unsigned long)(*(pbVal+2)) << 8;
	lVal4 = (unsigned long)(*(pbVal+3));
	*plVal = lVal1+lVal2+lVal3+lVal4;
}

void LONGtoBYTE(unsigned long lVal, unsigned char *pbVal)
{
	*pbVal = (unsigned char)(lVal >> 24);
	*(pbVal+1) = (unsigned char)(lVal >> 16);
	*(pbVal+2) = (unsigned char)(lVal >> 8);
	*(pbVal+3) = (unsigned char)lVal;
}

void PACK3toID(unsigned char *pbVal, long *plVal)
{
	long	lVal1, lVal2, lVal3;
	lVal1 = (long)(*(pbVal+0)) << 16;
	lVal2 = (long)(*(pbVal+1)) << 8;
	lVal3 = (long)(*(pbVal+2));
	*plVal = lVal1+lVal2+lVal3;
	if((*plVal) >= 0xffffff) *plVal = -1;
}

void IDtoPACK3(long lVal, unsigned char *pbVal)
{
	*(pbVal+0) = (unsigned char)(lVal >> 16);
	*(pbVal+1) = (unsigned char)(lVal >> 8);
	*(pbVal+2) = (unsigned char)lVal;
}

void PACK2toID(unsigned char *pbVal, long *plVal)
{
	long	lVal1, lVal2;
	lVal1 = (long)(*(pbVal+1)) << 8;
	lVal2 = (long)(*(pbVal+2));
	*plVal = lVal1+lVal2;
	if((*plVal) >= 0xffff) *plVal = -1;
}

void IDtoPACK2(long lVal, unsigned char *pbVal)
{
	*(pbVal+0) = (unsigned char)(lVal >> 8);
	*(pbVal+1) = (unsigned char)lVal;
}

void PACK1toID(unsigned char *pbVal, long *plVal)
{
	*plVal = *pbVal;
	if((*plVal) >= 0xff) *plVal = -1;
}

void IDtoPACK1(long lVal, unsigned char *pbVal)
{
	*pbVal = (unsigned char)lVal;
}

unsigned short n_htons(unsigned short hVal)
{
	unsigned char	*p;
	unsigned short	nVal;	

	p = (unsigned char *)&nVal;
	*p = (unsigned char)(hVal >> 8);
	*(p+1) = (unsigned char)hVal;
	return nVal;
}

unsigned short n_ntohs(unsigned short nVal)
{
	unsigned char	*p;
	unsigned short	hVal;

	p = (unsigned char *)&nVal;
	hVal = (p[0] << 8) + p[1];
	return hVal;
}

void s_htons(unsigned short *s, unsigned char *d, int len)
{
	unsigned short	val;
	int		i;

	for(i = 0;i < len;i++) {
		val = *s++;
		*d++ = (unsigned char)(val >> 8);
		*d++ = (unsigned char)val;
	}
}

void s_ntohs(unsigned char *s, unsigned short *d, int len)
{
	unsigned short	val, tmp;
	int		i;

	for(i = 0;i < len;i++) {
		tmp = *s++;
		val = *s++;
		val += tmp << 8;
		*d++ = val;
	}
}


int n_atoi(char *digits)
{
	char	*p;
	int		i, sign, n;

	p = digits;
	for(i = 0;p[i] == ' ' || p[i] == '\n' || p[i] == '\t';i++) ;
	sign = 1;
	if(p[i] == '+' || p[i] == '-') sign = (p[i++] == '+') ? 1 : -1;
	for(n = 0L;p[i] >= '0' && p[i] <= '9';i++) n = 10 * n + p[i] - '0';
	return sign * n;
}

long n_atol(char *digits)
{
	char	*p;
	long	n;
	int		i, sign;

	p = digits;
	for(i = 0;p[i] == ' ' || p[i] == '\n' || p[i] == '\t';i++) ;
	sign = 1;
	if(p[i] == '+' || p[i] == '-') sign = (p[i++] == '+') ? 1 : -1;
	for(n = 0L;p[i] >= '0' && p[i] <= '9';i++) n = 10 * n + p[i] - '0';
	return sign * n;
}

unsigned long n_atou(char *digits)
{
	char	*p;
	unsigned long	n;
	int		i;

	p = digits;
	for(i = 0;p[i] == ' ' || p[i] == '\n' || p[i] == '\t';i++) ;
	for(n = 0L;p[i] >= '0' && p[i] <= '9';i++) n = 10 * n + p[i] - '0';
	return n;
}

long n_check_atou(char *digits)
{
	char	*p;
	unsigned long	n;
	int		i, c;

	p = digits;
	for(i = 0;p[i] == ' ' || p[i] == '\n' || p[i] == '\t';i++) ;
	for(n = 0L;(c = p[i]);i++) {
		if(c >= '0' && c <= '9') n = 10 * n + c - '0';
		else {
			n = -1;
			break;
		}
	}
	return n;
}

char *n_utoa(unsigned long val, char *digits)
{
    char	*p;

	if(!digits) return NULL;
    if(val < 10L) {
        digits[0] = (char)(val + '0'); digits[1] = 0;
    } else {
        for(p = n_utoa(val / 10L, digits);*p;p++) ;
        n_utoa(val % 10L, p);
    }
    return digits;
}

void itoacomma(int value, char *digits)
{
	char	*p, temp[32];
	int		val;
	
	val = value;
	p = digits;
	p[0] = 0;
	while(val >= 1000) {
		strcpy(temp, p);
		sprintf(p, ",%03d", val % 1000);
		strcpy(p+4, temp);
		val /= 1000;
	}
	strcpy(temp, p);
	sprintf(p, "%d", val);
	if(temp[0]) {
		p += strlen(p); strcpy(p, temp);
	}
}

static int _GetIPSeg(unsigned char ipseg, char *buf)
{
	int		n;

	if(ipseg < 10) {
		buf[0] = ipseg + '0';
		n = 1;
	} else if(ipseg < 100) {
		buf[1] = ipseg % 10 + '0'; ipseg /= 10;
		buf[0] = ipseg + '0'; 
		n = 2;
	} else {
		buf[2] = ipseg % 10 + '0'; ipseg /= 10;
		buf[1] = ipseg % 10 + '0'; ipseg /= 10;
		buf[0] = ipseg + '0'; 
		n = 3;
	}
	return n;
}

static char _strMacAddr[20];	

char *mac2addr(unsigned char *macAddr)
{
	char	*p;
	int		i;

	p = _strMacAddr;
	for(i = 0;i < 6;i++) {
		c2hex((int)macAddr[i], p); p += 2; *p++ = ':';
	}
	*(p-1) = 0;
	return _strMacAddr;
}

int addr2mac(char *strMacAddr, unsigned char *macAddr)
{
	unsigned char	*d;
	char	*s;
	int		i, c, n;
	
	s = strMacAddr; d = macAddr;
	for(i = 0;i < 6;i++) {
		c = *s++;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n = (c - '0') << 4;
		else if(c >= 'A' && c <= 'F') n = (c-'A'+10) << 4;
		else if(c >= 'a' && c <= 'f') n = (c-'a'+10) << 4;
		else	break;	
		c = *s++;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n |= c - '0';
		else if(c >= 'A' && c <= 'F') n |= (c-'A'+10);
		else if(c >= 'a' && c <= 'f') n |= (c-'a'+10);
		else	break;
		c = *s++;
		if(i < 5 && c != ':' || i == 5 && c) break; 
		*d++ = n;
	}
	if(i < 6) i = -1;
	return i;
}

static char _strIpAddr[20];	

char *inet2addr(unsigned char *ipAddr)
{
	char	*p;

	p = _strIpAddr;
	p += _GetIPSeg(ipAddr[0], p);
	*p++ = '.';
	p += _GetIPSeg(ipAddr[1], p);
	*p++ = '.';
	p += _GetIPSeg(ipAddr[2], p);
	*p++ = '.';
	p += _GetIPSeg(ipAddr[3], p);
	*p = 0;
	return _strIpAddr;
}

int addr2inet(char *strIpAddr, unsigned char *ipAddr)
{
	char	*s, temp[4];
	int		i, j, c, err;

	s = strIpAddr;
	err = 0;
	for(i = 0;i < 4;i++) {	
		for(j = 0;j < 4;j++) {
			c = *s++;
			if(!c || c < '0' || c > '9') break;
			temp[j] = c;
		}
		if(j < 1 || j > 3 || c && c != '.') {
//printf("J=%d c=%d\n", j, c);
			err = 1;
			break;
		}
		temp[j] = 0;
		ipAddr[i] = (unsigned char)n_atol(temp);
	}
	if(err) return -1;
	else	return 0;
}

int ipIsNull(unsigned char *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3]);
}

int ipIsBroadcast(unsigned char *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3]) == 0xff;
}

void ipSetNull(unsigned char *addr)
{
	addr[0] = addr[1] = addr[2] = addr[3] = 0;	
}

void ipSetBroadcast(unsigned char *addr)
{
	addr[0] = addr[1] = addr[2] = addr[3] = 0xff;	
}

int macAddrIsNull(unsigned char *addr)
{
	return !(addr[0] | addr[1] | addr[2] | addr[3] | addr[4] | addr[5]);
}

int macAddrIsBroadcast(unsigned char *addr)
{
	return (addr[0] & addr[1] & addr[2] & addr[3] & addr[4] & addr[5]) == 0xff;
}

int macAddrIsMulticast(unsigned char *addr)
{
    return 0x01 & addr[0];
}
 
int macAddrIsValid(unsigned char *addr)
{
	return !macAddrIsMulticast(addr) && !macAddrIsNull(addr);
}

int checkIpAddr(unsigned char *ip, int subnetmask)
{
	if(!ip[0]) return -1;
	else	return 0;
}

int validateIpAddress(char *str)
{
	char	*s;
    int		c, segs, chcnt, accum;

	segs = chcnt = accum = 0;
	s = str;
    while(c = *s++) {
        if(c == '.') { // must have some digits in segment
			if(chcnt == 0) return 0;
			segs++;
			if(segs > 3) return 0;
			chcnt = accum = 0;
        } else if(c >= '0' && c <= '9') {
			accum = accum * 10 + c - '0';
			if(accum  > 255) return 0;
			chcnt++;
		} else break;
    }
    if(segs != 3 || chcnt == 0) return 0;
    return 1;
}

/*
 *  전송 포멧 ==> 데이타베이스 포멧
 *	size은 byte length로 앞 헤더를 포함한 길이
 *
 *	0x14 0x37 0xa8 0x5d 0x6e 0x00 0x00 0x00 0x00  ==> b37a85d6e 
 *	0x20 0x36 0x78 0x4f 0xff 0xff 0xff 0xff 0xff  ==> d36784 
 *	0x30 '5'  '6'  'k'  'i'  'm'  ' '  ' '  ' '   ==> c56kim 
 */
void string_co2db(unsigned char *s, int size, char *d)
{
	int		type, len;

	type = ((*s >> 4) & 0xf); len = *s & 0xf;
	s++; size--;
	switch(type) {
	case 1:
		*d++ = 'b';
		bin2hexstr(s, len, d);
		break;
	case 2:
		*d++ = 'd';
		bcd2string(s, size<<1, d);	// size<<2 => size<<1 : Bug fix 2009.4.18
		break;
	case 3:
		*d++ = 'c';
		memcpy_chop(d, (char *)s, size);
		break;
	default:
		*d = 0;
	}
}

/*
 *  데이타베이스 포멧 ==> 전송 포멧
 *	size은 byte length로 앞 헤더를 포함한 길이
 *
 *	b37a85d6e  ==>  0x14 0x37 0xa8 0x5d 0x6e 0x00 0x00 0x00 0x00
 *	d36784     ==>  0x20 0x36 0x78 0x4f 0xff 0xff 0xff 0xff 0xff
 *	c56kimsu   ==>  0x30 '5'  '6'  'k'  'i'  'm'  ' '  ' '  ' '
 */
void string_db2co(char *s, int size, unsigned char *d)
{
	int		type, len;

	type = *s++;
	size--;
	switch(type) {
	case 'b':
		len = hexstr2bin(s, d+1);
		*d++ = 0x10 + len;
		d += len;
		if(len < size) memset(d, 0, size-len);
		break;
	case 'd':
		*d++ = 0x20;
		string2bcd(s, size<<1, d);		// size<<2 => size<<1 : Bug fix 2009.4.18
		break;
	case 'c':
		*d++ = 0x30;
		memcpy_pad(d, s, size);
		break;
	default:
		memset(d, 0, size+1);
	}
}

void c2hex(int c, char *data)
{
	int		c1;

	c1 = c >> 4;
	if(c1 <= 9) c1 += '0';
	else	c1 += 'a' - 10;
	data[0] = c1;
	c &= 0x0f;
	if(c <= 9) c += '0';
	else	c += 'a' - 10;
	data[1] = c;
}

int hexstr2bin(char *hex_data, unsigned char *bin_data)
{
	unsigned char	*d;
	int		c, n;

	d = bin_data;
	while(1) {
		c = *hex_data++;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n = (c - '0') << 4;
		else if(c >= 'A' && c <= 'F') n = (c-'A'+10) << 4;
		else if(c >= 'a' && c <= 'f') n = (c-'a'+10) << 4;
		else	return -1;
		c = *hex_data++;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n |= c - '0';
		else if(c >= 'A' && c <= 'F') n |= (c-'A'+10);
		else if(c >= 'a' && c <= 'f') n |= (c-'a'+10);
		else	return -1;
		*d++ = n;
	}
	return d - bin_data;
}

void bin2hexstr(unsigned char *bin_data, int size, char *hex_data)
{
	char	*d;
	int		c, n;

	d = hex_data;
	while(size--) {
		n = *bin_data++;
		if(n >= 16) {
			c = n / 16;
			if(c >= 10) c += 'a' - 10;
			else	    c += '0';
		} else	c = '0';
		*d++ = c;
		c = n % 16;
		if(c >= 10) c += 'a' - 10;
		else	    c += '0';
		*d++ = c;
	}
	*d = 0;
}

unsigned long hexstr2long(char *hex_data)
{
	char	*p;
	unsigned long	data;
	int		c, n, m;

	c = strlen(hex_data);
	if(c <= 1) return 0L;
	data = 0L;
	m = 0;
	p = hex_data + strlen(hex_data) - 1;
	while(1) {
		c = *p--;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n = c - '0';
		else if(c >= 'A' && c <= 'F') n = (c-'A'+10);
		else if(c >= 'a' && c <= 'f') n = (c-'a'+10);
		else	n = 0;
		c = *p--;
		if(!c) break;
		if(c >= '0' && c <= '9')	  n |= (c - '0') << 4;
		else if(c >= 'A' && c <= 'F') n |= (c-'A'+10) << 4;
		else if(c >= 'a' && c <= 'f') n |= (c-'a'+10) << 4;
		data |= (n << m);
		m += 8;
		if(m >= 32) break;
	}
	return data;
}

void long2hexstr(unsigned long data, int size, char *hex_data)
{
	char	*p;
	int		c, n, m;

	m = 0;
	p = hex_data + (size << 1);
	*p-- = 0;
	while(size--) {
		n = (data >> m) & 0xff;
		c = n % 16;
		if(c >= 10) c += 'a' - 10;
		else	    c += '0';
		*p-- = c;
		if(n >= 16) {
			c = n / 16;
			if(c >= 10) c += 'a' - 10;
			else	    c += '0';
		} else	c = '0';
		*p-- = c;
		m += 8;
	}
}

// return Nibble length
int bcdlen(unsigned char *s)
{
	int		i, c, c2;

	for(i = 0; ;i++) {
		if(i & 1) {
			s++;
			c = c2 & 0xf;
		} else {
			c2 = *s;
			c = (c2 >> 4) & 0xf;
		}
		if(c > 9) break;
	}
	return i;
}

// len is Nibble length
void bcd2string(unsigned char *s, int len, char *d)
{
	int		i, c, c2;

	for(i = 0;i < len;i++) {
		if(i & 1) {
			s++;
			c = c2 & 0xf;
		} else {
			c2 = *s;
			c = (c2 >> 4) & 0xf;
		}
		if(c == 0xf) break;
		*d++ = c + '0';
	}
	*d = 0;
}	

// len is Nibble length
void string2bcd(char *s, int len, unsigned char *d)
{
	int		i, c, end;

	for(i = end = 0;i < len;i++) {
		if(!end) {
			c = *s++;
			if(c == 0) {
				end = 1;
				c = 0xf + '0';
			}
		}
		if(i & 1) {
			*d += (c - '0') & 0xf;  
			d++;
		} else {
			*d = (c - '0') << 4;
		}
	}
}

// len is Nibble length
void string2bcd_nullpad(char *s, int len, unsigned char *d)
{
	int		i, c, end;

	for(i = end = 0;i < len;i++) {
		if(!end) {
			c = *s++;
			if(c == 0) {
				end = 1;
				c = '0';
			}
		}
		if(i & 1) {
			*d += (c - '0') & 0xf;  
			d++;
		} else {
			*d = (c - '0') << 4;
		}
	}
}

// len is Byte length
void bin2bcd(unsigned char *s, int len, unsigned char *d)
{
	int		c, value;

	while(len--) {	
		value = *s++;
		if(value >= 10) c = (value / 10) << 4;
		else	c = 0;
		if(value > 0) c |= value % 10;
		*d++ = c;
	}
}

// len is Byte length
void bcd2bin(unsigned char *s, int len, unsigned char *d)
{
	int		c, c2, value;

	while(len--) {	
		c = *s++;
		c2 = (c >> 4) & 0xf;
		if(c2 == 0xf) value = 0;
		else	value = c2 * 10;
		c2 = c & 0xf;
		if(c2 != 0xf) value += c2;
		*d++ = value;
	}
}

void char2bcd(unsigned char value, unsigned char *d)
{ 
	int		c;

	if(value >= 10) c = (value / 10) << 4;
	else	c = 0;
	if(value > 0) c |= value % 10;
	*d = c;
}

/*
void char2bcd(unsigned char value, unsigned char *d)
{ 
	int		c;
	
	if(value == 0) *d = 0xff;
	else {
		if(value >= 10) c = (value / 10) << 4;
		else	c = 0;
		c |= value % 10;
		*d = c;
	}
}  
*/

void bcd2char(unsigned char *s, unsigned char *value)
{ 
	int		c, c2;

	c = *s;
	c2 = (c >> 4) & 0xf;
	if(c2 == 0xf) *value = 0;
	else	*value = c2 * 10;
	c2 = c & 0xf;
	if(c2 != 0xf) *value += c2;
	/* *value = ((c >> 4) & 0xf) * 10 + (c & 0xf);  (c & 0xf) 에 가로가 반드시 있어야 */
}

/* len은 nibble length */
int bcd2int(unsigned char *s, int len)
{
	char	temp[32];

	bcd2string(s, len, temp);
	return (int)n_atol(temp);
}	

/* len은 nibble length */
void int2bcd(int val, int len, unsigned char *d)
{
	char	temp[16];

	n_utoa(val, temp);
	string2bcd(temp, len, d);
}

long bcd2long(unsigned char *s)
{
	char	temp[16];

	bcd2string(s, 8, temp);
	return n_atol(temp);
}	

void long2bcd(long n, unsigned char *d)
{
	char	temp[16];

	n_utoa(n, temp);
	string2bcd(temp, 8, d);
}

// offset, len Nibble length
void bcdcopy(unsigned char *src, int offset, int len, unsigned char *dst)
{
	unsigned char	*s, *d;
	int		i, c;

	s = src + (offset>>1);
	d = dst;
	if(offset & 1) {
		c = *s++;
		for(i = 0;i < len;i++) {
			if(i & 1) {
				c = *s++;
				*d++ |= c >> 4;	
			} else {
				*d = c << 4;
			}
		}
		if(len & 1) *d |= 0xf; 
	} else {
		memcpy(d, s, (len+1)>>1);
	}
}

void term2addr(int DeviceID, unsigned char *d)
{
	char	str[8];

	sprintf(str, "4%03d", DeviceID);
	string2bcd(str, 4, d);
}

void addr2term(unsigned char *s, int *DeviceID)
{
	int		c, c2;

	c2 = *s++;
	c = (c2 >> 4) & 0xf;
	if(c != 4) {
		*DeviceID = -1;
		return;
	} else {
		*DeviceID = 0;
		c = c2 & 0xf;
		if(c > 9) {
			*DeviceID = -1;
			return;
		}
		(*DeviceID) += c * 100;
		c2 = *s++;
		c = (c2 >> 4) & 0xf;
		if(c > 9) {
			*DeviceID = -1;
			return;
		}
		(*DeviceID) += c * 10;
		c = c2 & 0xf;
		if(c > 9) {
			*DeviceID = -1;
			return;
		}
		(*DeviceID) += c;
	}
}

void bubble_sort(unsigned long *data, int size)
{
	unsigned long	tmp;
	int		i, j;

	for(i = 0;i < size - 1;i++) {
		for(j = 0;j < size - 1;j++) {
			if(data[j] > data[j+1]) {
				tmp = data[j];
				data[j] = data[j+1];
				data[j+1] = tmp;
			}
		}
	}
}

void memcpy_backward(void *dst, void *src, int length)
{
	unsigned char	*s, *d;
	int		len;

	len = length;
	s = src;
	s += len - 1;
	d = dst;
	d += len - 1;
	while(len--) *d-- = *s--;
}

void rpad(char *buf, int c, int length)
{
	int		len, diff;

	len = strlen(buf);
	diff = length - len;
	if(diff > 0) {
		memset(buf+len, c, diff);
		buf[len+diff] = 0;
	}
}

void memcpy_pad(void *dst, void *src, int size)
{
	unsigned char	*d, *s;

	d = (unsigned char *)dst;
	s = (unsigned char *)src;
	while((*s) && size--) *d++ = *s++;
	if(size > 0) memset(d, ' ', size);
}

void memcpy_padnull(void *dst, void *src, int size)
{
	unsigned char	*d, *s;

	d = (unsigned char *)dst;
	s = (unsigned char *)src;
	while((*s) && size--) *d++ = *s++;
	if(size > 0) memset(d, 0, size);
}

void memcpy_chop(void *dst, void *src, int size)
{
	unsigned char	*p;

	memcpy(dst, src, size);
	p = (unsigned char *)dst;
	p[size] = 0;
	while(size-- && (p[size] == ' ' || p[size] == '\t' || p[size] == '\n')) p[size] = 0;
}

int memtoi(void *buf, int size)
{
	char	temp[32];

	memcpy(temp, buf, size);
	temp[size] = 0;
	return (int)n_atol(temp);
}

long memtol(void *buf, int size)
{
	char	temp[32];

	memcpy(temp, buf, size);
	temp[size] = 0;
	return n_atol(temp);
}

void str_chop(char *s)
{
	int		len;

	len = strlen(s);
	while(len-- && (s[len] == ' ' || s[len] == '\t' || s[len] == '\n')) s[len] = 0;
}

void strins(char *buf, int index, int c)
{
	char	*p;
	int		len;

	len = strlen(buf + index);
	p = buf + index + len;
	while(len >= 0) {
		*(p+1) = *p;
		if(len > 0) p--;
		len--;
	} 
	*p = c;
}

void strdel(char *buf, int index)
{
	char	*p;

	p = buf + index;
	while(1) {
		*p = *(p+1);
		if(!(*p)) break;
		p++;
	}
}

void strdel_space(void *dst, void *src)
{
	char	*d, *s;
	int		c;
	
	d = (char *)dst; s = (char *)src;
	while(c = *s++)
		if(c != ' ' && c != '\t' && c != '\n') *d++ = c;
	*d = 0;
}

char *strcpy_chr(char *d, char *s, int chr)
{
	char	*p, c;

    p = d;
    while(1) {
		c = *s;
		if(!c) break;
		s++;
		if(c == chr) break;
		*p++ = c;
	}
	*p = 0;
    if(!c && !d[0])  return (char *)0;
    else    return s;
}

char *strcpy_position(char *d, char *s, int n)
{
	char	*p;

	p = s;
	while(n >= 0) {
		p = strcpy_chr(d, p, '\n');
		n--;
	}
	return p;
}

char *strlist_position(char *d, char *s, int n)
{
	char	*p;
	int		len;

	p = s;
	len = 0;
	while(n >= 0) {
		p += len;
		len = strlen(p) + 1;
		n--;
	}
	memcpy(d, p, len);
	return p + len;
}

void strswap(char *st, int chOld, int chNew)
{
	char	*s;
	int		c;

	s = st;
	while(1) {
		c = *s;
		if(!c) break;
		else if(c == chOld) c = chNew;
		*s++ = c;
	}
}

void strcpy_swap(char *dst, char *src, int chOld, int chNew)
{
	char	*s, *d;
	int		c;

	s = src; d = dst;
	while(1) {
		c = *s++;
		if(!c) break;
		else if(c == chOld) c = chNew;
		*d++ = c;
	}
	*d = 0;
}

int str_linenum(char *s)
{
	char	*p;
	int		c, ncnt, ccnt;
	
	ncnt = ccnt = 0; p = s;
	while(c = *p++)
		if(c == '\n') {
			ncnt++; ccnt = 0;
		} else	ccnt++;
	if(ccnt) ncnt++;	
	return ncnt;
}

int isdigits(char *s)
{
	unsigned char	*p;
	int		c, cnt;

	cnt = 0;
	p = (unsigned char *)s;
	while(1) {
		c = *p++;
		if(!c || c < '0' || c > '9') break;
		cnt++;
	}
	if(c)  return -1;
	else	return cnt;
}

int iswilddigits(char *s)
{
	unsigned char	*p;
	int			c, cnt;

	cnt = 0;
	p = (unsigned char *)s;
	while(1) {
		c = *p++;
		if(!c) break;
		if(c != '*' && (c < '0' || c > '9')) break;
		cnt++;
	}
	if(c)  return -1;
	else	return cnt;
}

int iswilds(char *s)
{
	unsigned char	*p;
	int			c, cnt;

	cnt = 0;
	p = (unsigned char *)s;
	while(1) {
		c = *p++;
		if(!c) break;
		if(c == '*') cnt++;
	}
	return cnt;
}

void stolower(char *s)
{
	unsigned char	*p;
    int		c;

	p = (unsigned char *)s;
    while(c = *p) {
        if(c >= 'A' && c <= 'Z')  *p = c - 'A' + 'a';
        p++;
    }
}

void stoupper(char *s)
{
	unsigned char	*p;
    int		c;

	p = (unsigned char *)s;
    while(c = *p) {
        if(c >= 'a' && c <= 'z')  *p = c - 'a' + 'A';
        p++;
    }
}

void plainstr2password(char *plain, char *pwd)
{
	int		len;

	len = strlen(plain);
	memset(pwd, '*', len); pwd[len] = 0;
}

void plainbcd2password(unsigned char *plain, int len, char *pwd)
{
	char	temp[80];

	bcd2string(plain, len, temp);
	len = strlen(temp);
	memset(pwd, '*', len); pwd[len] = 0;
}

void split_wildtoken(char *buf, char *buf1, char *buf2)
{
	char	*s;
	int		c, i;

	s = buf;
	i = 0;
    while(1) {
		c = *s;
		if(!c) break;
		s++;
		if(c == '*') break;
		buf1[i++] = c;
	}
	buf1[i] = 0;
	i = 0;
    while(1) {
		c = *s++;
		if(!c || c == '*') break;
		buf2[i++] = c;
	}
	buf2[i] = 0;
}

char *make_token(char *buf)
{
	char	*s;
	int		c;

	s = buf;
	while((c = *s++) && (c != ' ' && c != '\t' && c != '\n' && c != '\r')) ;
	if(c) *(s-1) = 0;
	else	s = (char *)0;
	return s;
}

char *skip_space(char *buf)
{
	char	*s;
	int		c;

	s = buf;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) ;
	if(c) s--;
	else s = (char *)0;
	return s;
}

char *read_linestr(char *buf, char *text)
{
	char	*s, *d;
	int		c, ncnt;
	
	ncnt = 0;
	s = buf; d = text;
	while(c = *s++) {
		if(c == '\n' || c == '\r') ncnt++;
		else if(ncnt) break;
		else *d++ = c;	
	}
	*d = 0;
	if(c) s--;
	else  s = NULL;
	return s;
}

// Return	NULL=End of text
char *read_token(char *buf, char *text)
{
	char	*s, *d;
	int		c;

	s = buf; d = text;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) ;
	if(!c) return (char *)0;
	else {
		do {
			*d++ = c;
		} while((c = *s++) && (c != ' ' && c != '\t' && c != '\n' && c != '\r')) ;
		*d = 0;
		if(c) c = *s;
		if(!c) s = NULL;
		return s;
	}
}

char *read_token_ch(char *buf, char *text, int ch)
{
	char	*s, *d;
	int		c;

	s = buf; d = text;
	while((c = *s++) && c != ch && c != '\n') *d++ = c;
	*d = 0;
	if(c) c = *s;
	if(!c) s = NULL;
	return s;
}

char *read_token_comma(char *buf, char *text)
{
    char    *s, *d;
    int     c;

    s = buf; d = text;
    while((c = *s++) && c != ',') *d++ = c;
    if(!c) s--;
    *d = 0;
    return s;
}

//   "xxx = yyy"  =>  name="xxx" value=="yyy"	
char *get_profile(char *buf, char *name, char *value)
{
	char	*s, *d;
	int     c, n;

	name[0] = value[0] = 0;
	s = buf;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) ;
	if(!c) return (char *)0;
	d = s - 1;
	while((c = *s++) && (c != '=' && c != ' ' && c != '\t' && c != '\r' && c != '\n')) ;
	n = s - 1 - d;
	if(n > 255) return (char *)0;
	memcpy(name, d, n); name[n] = 0;
	if(c == ' ' || c == '\t') {
		while((c = *s++) && (c == ' ' || c == '\t')) ;
	}
	if(!c) return s-1;
	else if(c == '\r' || c == '\n') return s;
	if(c != '=') {
		while((c = *s++) && (c != '\r' && c != '\n')) ;
		if(!c) s--;
		return s;
	}
	value[0] = '='; value[1] = 0;
	while((c = *s++) && (c == ' ' || c == '\t')) ;
	if(!c) return s-1;
	else if(c == '\r' || c == '\n') return s;
	if(c == '"') {
		d = s;
		while((c = *s++) && (c != '"' && c != '\r' && c != '\n')) ;
		if(!c) s--;
		if(c != '"') {
			value[0] = 0;
			return s;
		}
	} else {
		d = s - 1;
		while((c = *s++) && (c != ' ' && c != '\t' && c != '\r' && c != '\n')) ;
	}
	n = s - 1 - d;
	if(n > 255) return (char *)0;
	memcpy(value+1, d, n); value[n+1] = 0;
	if(c == ' ' || c == '\t') {
		while((c = *s++) && (c != '\r' && c != '\n')) ;
	}
	if(!c) s--;
	return s;
}


//  kkk=vvv
char *get_keyvalue(char *buf, char *key, char *value)
{
	char	*s, *d;
	int     c, n;

	s = buf;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) ;
	if(!c) return (char *)0;
	s--;
	key[0] = value[0] = 0;
	d = s;
	while((c = *s++) && c != '=' && c != '\r' && c != '\n') ;
	n = s - 1 - d;
	if(c == '\r' && (*s) == '\n') s++;
	if(n > 255) n = 255;
	memcpy(key, d, n); key[n] = 0;
	if(!c) return (char *)0; 
	else if(c == '\r' || c == '\n') return s;
	d = s;
	while((c = *s++) && c != '\r' && c != '\n') ;
	n = s - 1 - d;
	if(c == '\r' && (*s) == '\n') s++;
	if(c) c = *s;
	if(n > 255) n = 255;
	memcpy(value, d, n); value[n] = 0;
	if(!c) return (char *)0; 
	else	return s;
}

//  kkk vvv
char *get_keyvalue_space(char *buf, char *key, char *value)
{
	char	*s, *d;
	int     c, n;

	key[0] = value[0] = 0;
	s = buf;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) ;
	if(!c) return (char *)0;
	s--;
	d = s;
	while((c = *s++) && c != ' ' && c != '\r' && c != '\n') ;
	n = s - 1 - d;
	if(c == '\r' && (*s) == '\n') s++;
	if(n > 255) n = 255;
	memcpy(key, d, n); key[n] = 0;
	if(!c) return (char *)0; 
	else if(c == '\r' || c == '\n') return s;
	d = s;
	while((c = *s++) && c != '\r' && c != '\n') ;
	n = s - 1 - d;
	if(c == '\r' && (*s) == '\n') s++;
	if(c) c = *s;
	if(n > 255) n = 255;
	memcpy(value, d, n); value[n] = 0;
	if(!c) return (char *)0; 
	else	return s;
}

//  kkk=vvv
int file_get_keyvalue(FILE *fp, char *key, char *value)
{
	char	*s, *d;
	int     c, n;

	do {
		c = getc(fp);
	} while(c >= 0 && (c == ' ' || c == '\t' || c == '\n' || c == '\r')) ;
	if(c < 0) return 0;
	key[0] = value[0] = 0;
	d = key;
	do {
		*d++ = c;
		c = getc(fp);
	} while(c >= 0 && c != '=' && c != '\r' && c != '\n') ;
	if(c == '\r') {
		c = getc(fp);
		if(c != '\n') ungetc(c, fp);
	}
	*d = 0;
	d = value;
	c = getc(fp);
	while(c >= 0 && c != '\r' && c != '\n') {
		*d++ = c;
		c = getc(fp);
	}
	if(c == '\r') {
		c = getc(fp);
		if(c != '\n') ungetc(c, fp);
	}
	*d = 0;
	return 1;
}

void file_strcpy_chr(FILE *fp, char *str, int chr)
{
	char	*p;
	int		c;

    p = str;
    while(1) {
		c = getc(fp);
		if(c < 0 || c == chr) break;
		*p++ = c;
	}
	*p = 0;
}

//   "xxx = yyy"  =>  name="xxx" value="=yyy"	
char *get_keyvalue2(char *buf, char *key, char *value)
{
	char	*s, *d;
	int     c, n;

	key[0] = value[0] = 0;
	s = buf;
	while((c = *s++) && (c == ' ' || c == '\t' || c == '\r' || c == '\n')) ;
	if(!c) return (char *)0;
	d = s - 1;
	while((c = *s++) && (c != '=' && c != ' ' && c != '\t' && c != '\r' && c != '\n')) ;
	n = s - 1 - d;
	if(n > 255) return (char *)0;
	memcpy(key, d, n); key[n] = 0;
	if(c == ' ' || c == '\t') {
		while((c = *s++) && (c == ' ' || c == '\t')) ;
	}
	if(!c) return s-1;
	else if(c == '\r' || c == '\n') return s;
	if(c != '=') {
		while((c = *s++) && (c != '\r' && c != '\n')) ;
		if(!c) s--;
		return s;
	}
	value[0] = '='; value[1] = 0;
	while((c = *s++) && (c == ' ' || c == '\t')) ;
	if(!c) return s-1;
	else if(c == '\r' || c == '\n') return s;
	if(c == '"') {
		d = s;
		while((c = *s++) && (c != '"' && c != '\r' && c != '\n')) ;
		if(!c) s--;
		if(c != '"') {
			value[0] = 0;
			return s;
		}
	} else {
		d = s - 1;
		while((c = *s++) && (c != ' ' && c != '\t' && c != '\r' && c != '\n')) ;
	}
	n = s - 1 - d;
	if(n > 255) return (char *)0;
	memcpy(value+1, d, n); value[n+1] = 0;
	if(c == ' ' || c == '\t') {
		while((c = *s++) && (c != '\r' && c != '\n')) ;
	}
	if(!c) s--;
	return s;
}

int atoi2(char *s)
{
	int		c, n;

	n = -1;
	c = s[0];
	if(c == ' ') {
		c = s[1];
		if(c == ' ') n = 0;
		else if(c >= '0' && c <= '9') n = c - '0';
	} else if(c >= '0' && c <= '9') {
		n = c - '0';
		c = s[1];
		if(c >= '0' && c <= '9') n = n * 10 + c - '0';
		else if(c != ' ') n = -1;
	}
	return n;
}

void utoa2(unsigned int value, char *digits)
{
	int		n;

	if(value >= 10) {
		digits[0] = '0' + value / 10;
		n = 1;
	} else n = 0; 
	digits[n++] = '0' + value % 10;
	digits[n] = 0;	  
}

void utoab2(unsigned int value, char *digits)
{
	if(value >= 10) digits[0] = '0' + value / 10;
	else	digits[0] = ' ';
	digits[1] = '0' + value % 10;
	digits[2] = 0;	  
}

void utoa02(unsigned int value, char *digits)
{
	if(value >= 10) digits[0] = '0' + value / 10;
	else	digits[0] = '0';
	digits[1] = '0' + value % 10;
	digits[2] = 0;	  
}

int is_hangul(char *buf, int index)
{
	unsigned char	*p;
	int				cnt;

	p = (unsigned char *)buf;
	cnt = 0;
	while(index >= 0) {
		if(p[index--] & 128) cnt++;
		else	break;
	}
	if(cnt > 0) cnt = (cnt & 0x01) + 1;
	return cnt;
}

void plain2hidden(char *plain, char *hidden)
{
	int		len;

	len = strlen(plain);
	if(len) memset(hidden, '.', len);
	hidden[len] = 0;
}

unsigned short k_1 = 0x1e95;
unsigned short k_2 = 0xa6c9;
unsigned long k1_1 = 0x7d53a72b;
unsigned long k1_2 = 0x93e64d18;
unsigned long k2_1 = 0x2e4a3824;
unsigned long k2_2 = 0xb239ac3b;

unsigned long	_seed;

void simSeed(unsigned long seed)
{
	unsigned long	val;

	seed = (seed * k_1) + k1_1;
	seed ^= k1_2;
	val = time(NULL);
	val = val + (val << 16);
	seed += val;
	//lp = &val;
	//seed += lp[0]; seed += lp[1]; seed |= lp[2]; seed |= lp[3];
	srand(seed);
	_seed = seed;
}

unsigned long _Rand(void)
{
	unsigned long	val;
	
	val = time(NULL);
	val = val + (val << 16);
	val += _seed;
	_seed ^= val;	// appended 2013.3.25
	return val;			
}

void simRand(void *data, int size)
{
	unsigned char	*p, *pe, *vp;
	unsigned long	val;
	int		i;

/*
	p = (unsigned char *)data;
	for(i = 0;i < size;i++) {
cprintf("111 %d rand()...\n", size);	
		*p++ = rand() & 0xff;
cprintf("222 %d rand()...\n", size);	
	}
*/
	p = (unsigned char *)data;
	pe = p + size;
	vp = (unsigned char *)&val;
	i = 0;
	while(1) {
		val = _Rand();
		*p++ = vp[0];
		if(p >= pe) break;
		*p++ = vp[1];
		if(p >= pe) break;
		*p++ = vp[2];
		if(p >= pe) break;
		*p++ = vp[3];
		if(p >= pe) break;
	}
}
unsigned short sim16Hashing(unsigned short data1, unsigned short data2)
{
	unsigned short	data;

	data = data1;
	data = ~data + (data << 7);
	data ^= data >> 5;
	data += data << 2;
	data ^= data >> 5;
	data *= 2057;
	data ^= data >> 8;
	data += data2;
	data = ~data + (data << 7);
	data ^= data >> 5;
	data += data << 2;
	data ^= data >> 5;
	data *= 2057;
	data ^= data >> 8;
	return data;
}

unsigned short sim16KeyedHashing(unsigned short data1, unsigned short data2, unsigned short key)
{
	unsigned short	data;

	data = data1 ^ key;
	data = ~data + (data << 7);
	data ^= data >> 1;
	data += data << 3;
	data ^= data << 5;
	data += data2 ^ key;
	data = ~data + (data << 7);
	data ^= data >> 1;
	data += data << 3;
	data ^= data << 5;
	return data;
}

void sim16Encrypt(unsigned short *src, int len, unsigned short *dst, unsigned short key)
{
	unsigned short	*s, *d, k, c;
	int		i;

	s = src; d = dst; k = key; c = 0;
	for(i = 0;i < len;i++) {
		k = (c + k) * k_1 + k_2;
		c = *s++;
		c ^= k;
		*d++ = c;
	}
}

void sim16Decrypt(unsigned short *src, int len, unsigned short *dst, unsigned short key)
{
	unsigned short	*s, *d, k, c;
	int		i;

	s = src; d = dst; k = key; c = 0;
	for(i = 0;i < len;i++) {
		k = (c + k) * k_1 + k_2;
		c = *s++;
		*d++ = c ^ k;
	}
}

unsigned long sim32Hashing(unsigned long data1, unsigned long data2)
{
	unsigned long	data;
	
	data = data1;
	data = ~data + (data << 15);	// key = (key << 15) - key - 1;
	data ^= data >> 12;
	data += data << 2;
	data ^= data >> 4;
	data *= 2057;		// key = (key + (key << 3)) + (key << 11);
	data ^= data >> 16;
	data += data2;
	data += ~data + (data << 15);	// key = (key << 15) - key - 1;
	data ^= data >> 12;
	data += data << 2;
	data ^= data >> 4;
	data *= 2057;		// key = (key + (key << 3)) + (key << 11);
	data ^= data >> 16;
	return data;
}

unsigned long sim32KeyedHashing(unsigned long data1, unsigned long data2, unsigned long key)
{
	unsigned long	a, b, c;
	
	a = data1; b = data2; c = key;
	a -= b;  a -= c;  a ^= c >> 13;
	b -= c;  b -= a;  b ^= a << 8; 
	c -= a;  c -= b;  c ^= b >> 13;
	a -= b;  a -= c;  a ^= c >> 12;
	b -= c;  b -= a;  b ^= a << 16;
	c -= a;  c -= b;  c ^= b >> 5;
	a -= b;  a -= c;  a ^= c >> 3;
	b -= c;  b -= a;  b ^= a << 10;
	c -= a;  c -= b;  c ^= b >> 15;
	return c;
}

void sim32Encrypt(unsigned char *src, int len, unsigned char *dst, unsigned long key1, unsigned long key2)
{
	unsigned char	*s, *d;
	int		i, c, k, val;

	s = src; d = dst;
	for(i = 0;i < len;i++) {
		c = *s++;
		k = key1 & 0xff; c ^= k;
		k = key2 & 0xff; c ^= k;
		k = (key1 >> 8) & 0xff;  c ^= k;
		k = (key2 >> 8) & 0xff;  c ^= k;
		k = (key1 >> 16) & 0xff; c ^= k;
		k = (key2 >> 16) & 0xff; c ^= k;
		k = (key1 >> 24) & 0xff; c ^= k;
		k = (key2 >> 24) & 0xff; c ^= k;
		*d++ =  (unsigned char)c; val = c;
		key1 = (val + key1) * k1_1 + k1_2;
		key2 = (val + key2) * k2_1 + k2_2;
	}
}

void sim32Decrypt(unsigned char *src, int len, unsigned char *dst, unsigned long key1, unsigned long key2)
{
	unsigned char	*s, *d;
	int		i, c, k, val;

	s = src; d = dst;
	for(i = 0;i < len;i++) {
		c = *s++; val = c;		
		k = key1 & 0xff; c ^= k;
		k = key2 & 0xff; c ^= k;
		k = (key1 >> 8) & 0xff;  c ^= k;
		k = (key2 >> 8) & 0xff;  c ^= k;
		k = (key1 >> 16) & 0xff; c ^= k;
		k = (key2 >> 16) & 0xff; c ^= k;
		k = (key1 >> 24) & 0xff; c ^= k;
		k = (key2 >> 24) & 0xff; c ^= k;
		*d++ = (unsigned char)c;
		key1 = (val + key1) * k1_1 + k1_2;
		key2 = (val + key2) * k2_1 + k2_2;
	}
}

static char _month_days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
static unsigned char _bit_mask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };

int get_month_days(int year, int mm)
{
	int		mday;

	if(mm <= 0 || mm > 12) return -1;
	mday = _month_days[mm - 1];
	if(mm == 2 && (!(year % 4) && ((year % 100) || !(year % 400)))) mday++;	// leap year
	return mday;
}

int get_year_days(int year, int mm, int dd)
{
	int		i, yday;

	if(mm <= 0 || mm > 12) return -1;
	for(i = yday = 0;i < mm-1;i++) yday += _month_days[i];
	if(mm > 2 && (!(year % 4) && ((year % 100) || !(year % 400)))) yday++;	// leap year
	yday += dd;
	return yday;
}

void get_mmdd_from_ydays(int year, int yday, int *mm, int *dd)
{
	int		i, mday;

	for(i = 0;i < 12;i++) {
		mday = _month_days[i];
		if(i == 1 && (!(year % 4) && ((year % 100) || !(year % 400)))) mday++;	// leap year
		if(yday > mday) yday -= mday;
		else	break;
	}
	*mm = i + 1;
	*dd = yday;
}

// return 0..6 : Sunday..Saturday
int get_week_day(int year, int mm, int dd)
{ 
	int		i, tday;

	for(i = tday = 0;i < mm-1;i++) tday += _month_days[i];
	tday += year * 365 + dd;
	tday += (year / 4) - (year / 100) + (year / 400);
	if(mm < 3 && (!(year % 4) && ((year % 100) || !(year % 400)))) tday--;	// leap year
	return (int)((tday + 6) % 7);
}

// return 0..6 : Monday..Sunday)
int get_week_day2(int year, int mm, int dd)
{ 
	int		i, tday;

	for(i = tday = 0;i < mm-1;i++) tday += _month_days[i];
	tday += year * 365 + dd;
	tday += (year / 4) - (year / 100) + (year / 400);
	if(mm < 3 && (!(year % 4) && ((year % 100) || !(year % 400)))) tday--;	// leap year
	return (int)((tday + 5) % 7);
}

/* return 1-5 */
/*
int get_week_cycle(int dd, int wday)
{
	int		weekday;
	
	weekday = wday - (dd % 7 - 1);	// week day of month first day
	if(weekday < 0) weekday += 7;
	return (dd + 6 + weekday) / 7;
}
*/

int is_holiday(unsigned char *holidays, int yy, int mm, int dd)
{
	int		yday;

	yday = get_year_days(yy, mm, dd);
	return holidays[(yday - 1) >> 3] & _bit_mask[(yday - 1) & 0x7];
}

void set_holiday(unsigned char *holidays, int yy, int mm, int dd)
{
	int		yday;

	yday = get_year_days(yy, mm, dd);
	holidays[(yday - 1) >> 3] |= _bit_mask[(yday - 1) & 0x7];
}

void clear_holiday(unsigned char *holidays, int yy, int mm, int dd)
{
	int		yday;

	yday = get_year_days(yy, mm, dd);
	holidays[(yday - 1) >> 3] &= ~(_bit_mask[(yday - 1) & 0x7]);
}

void date_add_day(unsigned char *cdate, int day)
{
	int		yyyy, mm, dd, mday;

	yyyy = 2000 + cdate[0];
	mm = cdate[1]; 
	dd = cdate[2] + day;
	if(day > 0) {
		while(1) {
			mday = get_month_days(yyyy, mm);
			if(dd <= mday) break;
			dd -= mday;
			mm++;
			if(mm > 12) {
				yyyy++;
				mm = 1;
			}
		}
	} else {
		while(1) {
			if(dd > 0) break;
			mm--;
			if(mm <= 0) {
				yyyy--;
				mm = 12;
			}
			mday = get_month_days(yyyy, mm);
			dd += mday;
		}
	}
	cdate[0] = yyyy - 2000;
	cdate[1] = mm;
	cdate[2] = dd;
}

// epoch: 2000/01/01 00:00:00
void datetime2longtime(unsigned char *ctm, unsigned long *ltime)
{
	unsigned long	seconds;
	int		days, i, leap, year, mm;

	year = ctm[0];
	days = year * 365;
	for(i = leap = 0;i < year;i++) {
		if(!(i % 4)) leap++;
    }
	days += leap;
	mm = ctm[1];
	for(i = 0;i < mm-1;i++) days += _month_days[i];
	if(mm > 2 && !(year % 4)) days++;	// leap year
	days += ctm[2] - 1;
	seconds = days * 86400 + ctm[3] * 3600 + ctm[4] * 60 + ctm[5];
	*ltime = seconds;
}

void longtime2datetime(unsigned long ltime, unsigned char *ctm)
{ 
	int		val, i, days, leap;

	// Buf fox 2012.12.3: ltime >= 0x10000000 인경우 val=ltime 음수로 인한 오류
	//val = ltime; ctm[5] = val % 60; val /= 60;
	ctm[5] = (unsigned char)(ltime % 60); val = ltime / 60;
	ctm[4] = val % 60; val /= 60;
	ctm[3] = val % 24; val /= 24;
	for(i = 0; ;i++) {
		if(i % 4) days = 365; else days = 366;
		if(val < days) break;
		val -= days;
	}
	ctm[0] = i;
	if((i % 4)) leap = 0; else leap = 1;
	for(i = 0; ;i++) {
		days = _month_days[i];
		if(i == 1 && leap) days++;
		if(val < days) break;
		val -= days;
	}
	ctm[1] = i + 1;
	ctm[2] = val + 1;
}

void datetime2str(unsigned char *ctm, char *str)
{
	sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", ctm[0]+2000, (int)ctm[1],  (int)ctm[2], (int)ctm[3], (int)ctm[4], (int)ctm[5]);
}

//  yyyy/mm/dd hh:mi:ss ==> ctm[5]
void str2datetime(char *strtime, unsigned char *ctm)
{
	char	*p, tmpbuf[32];

	p = strtime;
	memcpy(tmpbuf, p, 4); p += 5; tmpbuf[4] = 0; ctm[0] = atoi(tmpbuf) - 2000;	// yy
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[1] = atoi(tmpbuf);			// mm
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[2] = atoi(tmpbuf);			// dd
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[3] = atoi(tmpbuf);			// hh
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[4] = atoi(tmpbuf);			// mi
	memcpy(tmpbuf, p, 2); p += 2; tmpbuf[2] = 0; ctm[5] = atoi(tmpbuf);			// ss
}

//  yyyy/mm/dd ==> ctm[3]
void str2date(char *strtime, unsigned char *ctm)
{
	char	*p, tmpbuf[32];

	p = strtime;
	memcpy(tmpbuf, p, 4); p += 5; tmpbuf[4] = 0; ctm[0] = atoi(tmpbuf) - 2000;	// yy
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[1] = atoi(tmpbuf);			// mm
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[2] = atoi(tmpbuf);			// dd
}

//  hh:mi:ss ==> ctm[3]
void str2time(char *strtime, unsigned char *ctm)
{
	char	*p, tmpbuf[32];

	p = strtime;
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[0] = atoi(tmpbuf);			// hh
	memcpy(tmpbuf, p, 2); p += 3; tmpbuf[2] = 0; ctm[1] = atoi(tmpbuf);			// mi
	memcpy(tmpbuf, p, 2); p += 2; tmpbuf[2] = 0; ctm[2] = atoi(tmpbuf);			// ss
}

int date_validate(unsigned char *cdate)
{
	int		yyyy, mm, dd, mday;

	yyyy = 2000 + cdate[0];
	mm = cdate[1]; 
	dd = cdate[2];
	if(mm <= 0 || mm > 12) return 0;
	mday = get_month_days(yyyy, mm);
	if(dd <= 0 || dd > mday) return 0;
	return 1;
}

int time_validate(unsigned char *ctime)
{
	int		hh, mi, ss;

	hh = ctime[0]; 
	mi = ctime[1]; 
	ss = ctime[2]; 
	if(hh < 0 || hh > 23 || mi < 0 || mi > 59 || ss < 0 || ss > 59) return 0; 
	return 1;
}

int datetime_validate(unsigned char *cdatetime)
{
	int		rval;

	rval = date_validate(cdatetime);
	if(rval) rval = time_validate(cdatetime+3);
	return rval;
}

// => yyyymmddhhmi
void datetime2bcd(unsigned char *ctm, unsigned char *bcd)
{
	unsigned char	buf[4];
	int		val;
	
	val = ctm[0] + 2000;
	buf[0] = val / 100; buf[1] = val % 100;
	bin2bcd(buf, 2, bcd);
	bin2bcd(ctm+1, 4, bcd+2);
}

// yyyymmddhhmi =>
void bcd2datetime(unsigned char *bcd, unsigned char *ctm)
{
	unsigned char	buf[4];
	int		val;
	
	bcd2bin(bcd, 2, buf);
	val = buf[0] * 100 + buf[1];
	ctm[0] = val - 2000;
	bcd2bin(bcd+2, 4, ctm+1);
}

/* tm1, tm2 compare
 *		Return	1	tm1 > tm2
 *			    0	tm1 = tm2
 *			   -1	tm1 < tm2
*/

int datetime_comp(unsigned char *tm1, unsigned char *tm2)
{
	if(tm1[0] > tm2[0]) return 1;
	else if(tm1[0] < tm2[0]) return -1;
	if(tm1[1] > tm2[1]) return 1;
	else if(tm1[1] < tm2[1]) return -1;
	if(tm1[2] > tm2[2]) return 1;
	else if(tm1[2] < tm2[2]) return -1;
	if(tm1[3] > tm2[3]) return 1;
	else if(tm1[3] < tm2[3]) return -1;
	if(tm1[4] > tm2[4]) return 1;
	else if(tm1[4] < tm2[4]) return -1;
	if(tm1[5] > tm2[5]) return 1;
	else if(tm1[5] < tm2[5]) return -1;
	else return 0;
}

/* tm1(yymmdd), tm2(yymmdd) compare with time
 *		Return  1 tm1 > tm2
 *              0 tm1 = tm2
 *             -1 tm1 < tm2
 */
int date_comp(unsigned char *tm1, unsigned char *tm2)
{
	if(tm1[0] > tm2[0]) return 1;
	else if(tm1[0] < tm2[0]) return -1;
	if(tm1[1] > tm2[1]) return 1;
	else if(tm1[1] < tm2[1]) return -1;
	if(tm1[2] > tm2[2]) return 1;
	else if(tm1[2] < tm2[2]) return -1;
	else return 0;
}

/* tm1(hhmmss), tm2(hhmmss) compare with time
 *		Return  1 tm1 > tm2
 *              0 tm1 = tm2
 *             -1 tm1 < tm2
 */
int time_comp(unsigned char *tm1, unsigned char *tm2)
{
	if(tm1[0] > tm2[0]) return 1;
	else if(tm1[0] < tm2[0]) return -1;
	if(tm1[1] > tm2[1]) return 1;
	else if(tm1[1] < tm2[1]) return -1;
	if(tm1[2] > tm2[2]) return 1;
	else if(tm1[2] < tm2[2]) return -1;
	else return 0;
}

/* tm2 - tm1 in unit minutes */
int datetime_diff(unsigned char *tm1, unsigned char *tm2)
{
	int		diff, carry, val, yyyy, mm;

	diff = carry = 0;
	val = tm2[5];		// Second;
	if(val > tm1[5]) diff += val - tm1[5];
	else if(val < tm1[5]) {
		diff += val + 60 - tm1[5];
		carry = 1;
	}
	val = tm2[4];		// Minute
	if(carry == 1) {
		if(val > 0) {
			val--;
			carry = 0;
		} else	val = 59;
	}
	if(val > tm1[4]) diff += (val - tm1[4]) * 60;
	else if(val < tm1[4]) {
		diff += (val + 60 - tm1[4]) * 60;
		carry = 1;
	}
	val = tm2[3];	// Hour;
	if(carry == 1) {
		if(val > 0) {
			val--;
			carry = 0;
		} else	val = 23;
	}
	if(val > tm1[3]) diff += (val - tm1[2]) * 3600;
	else if(val < tm1[3]) {
		diff += (val + 24 - tm1[3]) * 3600;
		carry = 1;
	}
	yyyy = tm2[0];		// Year
	mm = tm2[1] - 1;	// Month-1
	if(mm <= 0) {
		yyyy--; mm = 12;
	}
	val = tm2[2];		// Day
	if(carry == 1) {
		if(val > 1) {
			val--;
			carry = 0;
		} else	val = get_month_days(yyyy, mm);
	}
	if(val > tm1[2]) diff += (val - tm1[2]) * 86400;
	else if(val < tm1[2]) {
		diff += (val + get_month_days(yyyy, mm) - tm1[2]) * 86400;
		carry = 1;
	}
	return diff;
}

int BitMaskGet(unsigned char *BitMask, int Index)
{
	return (int)BitMask[Index >> 3] & _bit_mask[Index & 0x7];
}

void BitMaskSet(unsigned char *BitMask, int Index)
{
	BitMask[Index >> 3] |= _bit_mask[Index & 0x7];
}

void BitMaskClear(unsigned char *BitMask, int Index)
{
	BitMask[Index >> 3] &= ~(_bit_mask[Index & 0x7]);
}

int bytegetflag(unsigned char byte, int flag)
{
	int		val;

	if(byte & flag) val = 1;
	else	val = 0;
	return val; 
}

void bytesetflag(unsigned char *byte, int flag, int val)
{
	if(val) (*byte) |= flag;
	else	(*byte) &= ~flag;
}

const unsigned char _OneCountTbl[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };

int bitOneCount(unsigned char *bits, int bitp, int len)
{
	unsigned char	*d;
	int		val, n, mask, count;

	d = bits + (bitp >> 3);
	mask = bitp & 0x07;
	n = len + mask;
	mask = 0xff >> mask;
	count = 0;
	if(n > 7) {
		val = *d++;
		val &= mask;
		count += _OneCountTbl[val >> 4];
		count += _OneCountTbl[val & 0xf];
		n -= 8; mask = 0xff;
	}
	while(n > 7) {
		val = *d++;
		count += _OneCountTbl[val >> 4];
		count += _OneCountTbl[val & 0xf];
		n -= 8;
	}
	if(n) {
		mask &= 0xff << (8-n);
		val = *d;
		val &= mask;
		count += _OneCountTbl[val >> 4];
		count += _OneCountTbl[val & 0xf];
	}
	return count;
}

void bitsetzero(unsigned char *bits, int bitp, int len)
{
	unsigned char	*d;
	int		n, mask;

	d = bits + (bitp >> 3);
	mask = bitp & 0x07;
	n = len + mask;
	mask = 0xff >> mask;
	if(n > 7) {
		*d++ &= ~mask; n -= 8;
		mask = 0xff;
	}
	while(n > 7) {
		*d++ = 0x00; n -= 8;
	}
	if(n) {
		mask &= 0xff << (8-n);
		*d &= ~mask;
	}
}

void bitsetone(unsigned char *bits, int bitp, int len)
{
	unsigned char	*d;
	int		n, mask;

	d = bits + (bitp >> 3);
	mask = bitp & 0x07;
	n = len + mask;
	mask = 0xff >> mask;
	if(n > 7) {
		*d++ |= mask; n -= 8;
		mask = 0xff;
	}
	while(n > 7) {
		*d++ = 0xff; n -= 8;
	}
	if(n) {
		mask &= 0xff << (8-n);
		*d |= mask;
	}
}

void bitset(unsigned char *bits, int bitp, int val, int len)
{
	unsigned char	*d;
	int		n, mask;

	d = bits + (bitp >> 3);
	mask = bitp & 0x07;
	n = len + mask;
	mask = 0xff >> mask;
	if(val) {
		if(n > 7) {
			*d++ |= mask; n -= 8;
			mask = 0xff;
		}
		while(n > 7) {
			*d++ = 0xff; n -= 8;
		}
		if(n) {
			mask &= 0xff << (8-n);
			*d |= mask;
		}
	} else {
		if(n > 7) {
			*d++ &= ~mask; n -= 8;
			mask = 0xff;
		}
		while(n > 7) {
			*d++ = 0x00; n -= 8;
		}
		if(n) {
			mask &= 0xff << (8-n);
			*d &= ~mask;
		}
	}
}

void bitcpy(unsigned char *dst, int dstp, unsigned char *src, int srcp, int len)
{
	unsigned char	*s, *d;
	int		n, val, shift, mask;

	s = src + (srcp >> 3);
	d = dst + (dstp >> 3);
	mask = dstp & 0x07;
	n = len + mask;
	shift = (srcp & 0x07) - mask;
	mask = 0xff >> mask;
	if(shift >= 0) {
		val = *s++; val <<= 8; val |= *s++; val <<= shift; 
		if(n > 7) {
			*d &= ~mask;
			*d++ |= (val >> 8) & mask; n -= 8;
			mask = 0xff;
		}
		while(n > 7) {
			val <<= 8 - shift; val |= *s++; val <<= shift;
			*d++ = val >> 8; n -= 8;
		}
		if(n) {
			val <<= 8 - shift; val |= *s++; val <<= shift;
			mask &= 0xff << (8-n);
			*d &= ~mask;
			*d |= (val >> 8) & mask;
		}
	} else {
		shift = -shift;
		val = *s++; val <<= 8; val |= *s++; val <<= 8-shift; 
		if(n > 7) {
			*d &= ~mask;
			*d++ |= (val >> 16) & mask; n -= 8;
			mask = 0xff;
		} else {
			val >>= 8;
		}
		while(n > 7) {
			*d++ = val >> 8; n -= 8;
			val <<= shift; val |= *s++; val <<= 8-shift;
		}
		if(n) {
			mask &= 0xff << (8-n);
			*d &= ~mask;
			*d |= (val >> 8) & mask;
		}
	}
}

unsigned char bitreverse(unsigned char byte)
{
	unsigned char	rb;

	if(byte & 0x01) rb = 0x80;
	else		    rb = 0x00;
	if(byte & 0x02) rb |= 0x40;
	if(byte & 0x04) rb |= 0x20;
	if(byte & 0x08) rb |= 0x10;
	if(byte & 0x10) rb |= 0x08;
	if(byte & 0x20) rb |= 0x04;
	if(byte & 0x40) rb |= 0x02;
	if(byte & 0x80) rb |= 0x01;
	return rb;
}

void getTmpRandFileName(char *fileName)
{
	unsigned char	buf[8];

	memrand(buf, 8);
	strcpy(fileName, "/tmp/");
	bin2hexstr(buf, 8, fileName+5);
}

