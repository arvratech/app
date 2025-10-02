#include "sc_mifare_crc.h"


static unsigned short _CrcUpdateCcitt(unsigned short crc, unsigned char dat)
{
	dat ^= ((unsigned char)crc) & 0xff;
	dat ^= dat << 4;
	crc = (crc >> 8)^(((unsigned short)dat) << 8)^(((unsigned short) dat) << 3)^(((unsigned short)dat) >> 4);
	return crc;
}

unsigned short crcCalculateCcitt(unsigned short preloadValue, const unsigned char* buf, unsigned short length)
{
	unsigned short	crc;
	int		i;

	crc =  preloadValue;
	for(i = 0;i < length;i++) crc = _CrcUpdateCcitt(crc, buf[i]);
	return crc;
}
