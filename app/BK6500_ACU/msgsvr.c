#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_ascii.h"
#include "prim.h"
#include "msgsvr.h"


int IsCommand(unsigned char *buf)
{
	if(buf[0] == ASCII_ACK) return 1;
	else	return 0;
}

int IsResult(unsigned char *buf)
{
	if(buf[0] == ASCII_BS) return 1;
	else	return 0;
}

int GetDeviceID(unsigned char *buf)
{
	int		DeviceID;

	if(!buf[4] && !buf[5]) DeviceID = 0;
	else	addr2term(buf+4, &DeviceID);
	return DeviceID;
}

int GetCommand(unsigned char *buf)
{
	if(buf[0] != ASCII_ACK) return -1;
	else	return (int)buf[6];
}

int GetResult(unsigned char *buf)
{
	if(buf[0] != ASCII_BS) return -1;
	else	return (int)buf[6];
}

int CodeCommand(unsigned char *buf, int Address, int Command, int DataSize)
{
	unsigned char	*p, c;
	short	length;
	int		i, size;

	p = buf;
	*p++ = ASCII_ACK; *p++ = ASCII_STX;
	length = DataSize + 9;
	SHORTtoBYTE(length, p); p += 2;
	if(Address > 0) {
		term2addr(Address, p); p += 2; 
	} else {
		*p++ = 0; *p++ = 0;
	}
	*p++ = (unsigned char)Command;
	p += DataSize;
	size = p - buf;
	length = size + 2;
	SHORTtoBYTE(length, buf+2);
	for(c = i = 0;i < size;i++) c += buf[i];
	*p++ = c;
	*p++ = ASCII_ETX;
	return (int)length;
}

int CodeResult(unsigned char *buf, int Result, int DataSize)
{
	unsigned char	*p, c;
	short	length;
	int		i, size;

	if(Result) DataSize = 0;
	p = buf;
	*p++ = ASCII_BS; *p++ = ASCII_STX;
	length = 9 + DataSize;
	SHORTtoBYTE(length, p); p += 2;
	p += 2;		// skip address
	*p++ = (unsigned char)Result;
	p += DataSize;
	size = p - buf;
	length = size + 2;
	SHORTtoBYTE(length, buf+2);
	for(c = i = 0;i < size;i++) c += buf[i];
	*p++ = c;
	*p++ = ASCII_ETX;
	return (int)length;
}
