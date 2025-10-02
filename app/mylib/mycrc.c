#include "mycrc.h"

unsigned char  crc16tab_h[256];
unsigned char  crc16tab_l[256];
unsigned char  crc8tab[256];


void crc8Init(void)
{
	// terms of polynomial defining this crc(except x^8) 
//	const unsigned char poly = 0x07;	// ATM-8 HEC 0x83=x^8 + x^2 + x^1 + 1
//	const unsigned char poly = 0xd5;	// CRC-8     0xea=x^8 + x^7 + x^6 + x^4 + x^2 + 1
//	const unsigned char poly = 0x2f;	// C2        0x97=x^8 + x^5 + x^3 + x^2 + x^1 + 1
	const unsigned char poly = 0x4d;	//           0xa6=x^8 + x^6 + x^3 + x^2 + 1
	unsigned char	temp;
	int		i, j;

	for(i = 0;i < 256;i++) {
		temp = i;
		for(j = 0; j < 8;j++) {
			if(temp & 0x80) temp = (temp << 1) ^ poly;
			else	temp <<= 1;
		}
 		crc8tab[i] = temp;
    }
}

void crc16Init(void)
{
	const unsigned short poly = 0x1021;		// 
    unsigned short	crc, c;
    int		i, j;

	for(i = 0;i < 256;i++) {
		crc = 0;
		c   = (unsigned short)(i << 8);
		for(j = 0;j < 8;j++) {
			if((crc ^ c) & 0x8000) crc = (crc << 1) ^ poly;
            else	crc <<= 1;
			c <<= 1;
		}
		//for(j = 0; j < 8;j++) {
		//	if(crc &  0x8000) crc = (crc << 1) ^ poly);
		//	else	crc <<= 1;
		//}
		crc16tab_h[i] = (unsigned char)(crc >> 8);
		crc16tab_l[i] = (unsigned char)crc;
    }
}

int crc8Check(unsigned char *buf, int length)
{
	unsigned char	c;
	int		i, len;

	len = length - 1;
	for(i = 0, c = 0xff;i < len;i++) c = crc8tab[c ^ buf[i]];
	if(c == buf[len]) i = 0;
	else	i = -1;
	return i;
}

int crc16Check(unsigned char *buf, int length)
{
	unsigned char	c, c_h, c_l;
	int		i, len;
 
	len = length - 2;
	c_h = 0xff; c_l = 0xff;	// CRC High - CRC Low
	for(i = 0;i < len;i++) {
		c = c_h ^ buf[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
	}
	if(c_h == buf[i] && c_l == buf[i+1]) i = 0;
	else	i = -1;
	return i;
}

