#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
////////////////////////////////
#include "defs.h"
#include "asciidefs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "msgsvr.h"
#include "sysdep.h"
#include "svr.h"

extern int		gIOStatus;


void svrLog(unsigned char *buf, int svrSize, int txflg)
{
#ifdef SVR_MON
	char	*p, temp[128];
	int		i;

	for(p = temp, i = 0;i < svrSize;i++) {
		sprintf(p, "%02x", buf[i]);
		p += 2;
		if(i >= 26) {
			sprintf(p, "...");
			p += 3;
			break;
		}
	}
	*p = 0;
	if(txflg) printf("%ld Tx %d [%s]\n", MS_TIMER, svrSize, temp);
	else	printf("%ld Rx %d [%s]\n", MS_TIMER, svrSize, temp);
#endif
}

int CheckPacketHead(unsigned char *buf, int size)
{
	unsigned short	length;

	if(size != 9) return -1;
	if(buf[0] != ASCII_ACK && buf[0] != ASCII_BS || buf[1] != ASCII_STX) return -1;
	BYTEtoSHORT(buf+2, &length);
	if(length < 9 || length > MAX_SVRBUF_SZ) return -1;
	return (int)length;
}

int CheckPacketTrail(unsigned char *buf, int size)
{
	unsigned char	checksum;
	unsigned short	length;
	int		i;

	BYTEtoSHORT(buf+2, &length);
	if(size != length || buf[size-1] != ASCII_ETX) return R_PACKET_FORMAT_ERROR;
	checksum = 0;
	for(i = 0;i < size-2;i++) checksum += buf[i];
	if(buf[size-2] != checksum) return R_PACKET_CHECKSUM_ERROR;	// CHECKSUM
	return 0;
}

int CheckPacket(unsigned char *buf, int size)
{
	unsigned char	checksum;
	unsigned short	length;
	int		i;

	if(size < 9 || size > MAX_SVRBUF_SZ) return R_PACKET_FORMAT_ERROR;
	if(buf[0] != ASCII_ACK && buf[0] != ASCII_BS || buf[1] != ASCII_STX) return R_PACKET_FORMAT_ERROR;
	BYTEtoSHORT(buf+2, &length);
	if(size != length || buf[size-1] != ASCII_ETX) return R_PACKET_FORMAT_ERROR;
	checksum = 0;
	for(i = 0;i < size-2;i++) checksum += buf[i];
	if(buf[size-2] != checksum) return R_PACKET_CHECKSUM_ERROR;	// CHECKSUM
	return 0;
}

int SockReadPacket(int sock, unsigned char *buf)
{
	unsigned short	length;
	int		size, count, s_count;
	
	length = MAX_SVRBUF_SZ; count = 0;
	size = sinPollRead(sock, buf, MAX_SVRBUF_SZ, 10);
	while(size > 0) {
		s_count = count;
		count += size;
		if(s_count < 4 && count >= 4) {
			s_count = 0;
			if((buf[0] == ASCII_ACK || buf[0] == ASCII_BS) && buf[1] == ASCII_STX) {
				BYTEtoSHORT(buf+2, &length);
				if(length >= 9 && length <= MAX_SVRBUF_SZ) s_count = 1;
			}
			if(s_count == 0) {
				size = -3;
				break;
			}
		}
		if(count >= length) return count;
		size = sinPollRead(sock, buf+count, length-count, 200);
	}
	return size;
}

int svrWritePacket(int sock, unsigned char *buf, int size)
{
	int		rval;

	rval = sinWrite(sock, buf, size);
	if(rval > 0) {
#ifdef SVR_MON
svrLog(buf, size, 1);
#endif
		return 0;
	} else {
		return -1;
	}
}

int svrSendResult(int sock, unsigned char *buf, int Result, int DataSize)
{
	int		rval, size;

	size = CodeResult(buf, Result, DataSize);
	rval = sinWrite(sock, buf, size);
printf("svrSendResult=%d: result=%d size=%d\n", rval, Result, size);
#ifdef SVR_MON
if(rval > 0) svrLog(buf, size, 1);
#endif
	return rval;
}

int svrSendDump(int sock, unsigned char *buf, int size)
{
	int		rval;

	rval = sinWrite(sock, buf, size);
	return rval;
}
