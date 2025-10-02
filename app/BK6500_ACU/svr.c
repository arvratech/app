#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
////////////////////////////////
#include "defs.h"
#include "defs_ascii.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "sio.h"
#include "sin.h"
#include "msgsvr.h"
#include "sysdep.h"
#include "svr.h"

extern int		gIOStatus;
extern unsigned char gSvrSioPort;


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
	if(txflg) cprintf("%ld Tx %d [%s]\n", TICK_TIMER, svrSize, temp);
	else	cprintf("%ld Rx %d [%s]\n", TICK_TIMER, svrSize, temp);
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
	size = sinRead(sock, buf, MAX_SVRBUF_SZ, 0);
	while(size > 0) {
		s_count = count;
		count += size;
		if(s_count < 4 && count >= 4) {
			s_count = 0;
			if((buf[0] == ASCII_ACK || buf[0] == ASCII_BS) && buf[1] == ASCII_STX) {
				BYTEtoSHORT(buf+2, &length);
				if(length >= 9 && length <= MAX_SVRBUF_SZ) s_count = 1;
			}
			if(s_count == 0) return -3;
		}
		if(count >= length) return count;
		size = sinRead(sock, buf+count, length-count, 10);
	}
	return size;
}

int svrWritePacket(int sock, unsigned char *buf, int size)
{
	int		rval;

	if(sock) rval = sinWrite(SOCK_SERVER, (char *)buf, size);
	else	rval = sioWrite(gSvrSioPort, (char *)buf, size);
	if(rval > 0) {
#ifdef SVR_MON
svrLog((unsigned char *)buf, size, 1);
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
	if(sock) rval = sinWrite(SOCK_SERVER, buf, size);
	else	rval = sioWrite(gSvrSioPort, buf, size);
#ifdef SVR_MON
if(rval > 0) svrLog(buf, size, 1);
#endif
	return rval;
}

int svrSendDump(int sock, unsigned char *buf, int size)
{
	int		rval;

	if(sock) rval = sinWrite(SOCK_SERVER, buf, size);
	else	rval = sioWrite(gSvrSioPort, buf, size);
	return rval;
}

//#ifdef SVR_MON
void PrintAT(char *buf, int size)
{
	char	*p, temp[100];
	int		i, c;

	p = temp;
	for(i = 0;i < size;i++) {
		c = buf[i];
		if(c == '\r') {
			*p++ = '<'; *p++ = 'C'; *p++ = 'R'; *p++ = '>';
		} else if(c == '\n') {
			*p++ = '<'; *p++ = 'L'; *p++ = 'F'; *p++ = '>';
		} else {
			*p++ = c;
		}
	}
	*p = 0;
	cprintf("%s", temp);
}
//#endif

int svrATCommand(char *buf)
{
	int		rval, timeout;

	//taskDelayTick(20);	// 이것이 없으면 제대로 동작안함
taskDelayTick(1);	// Zigbee
	rval = sioWrite(gSvrSioPort, buf, strlen(buf));
//#ifdef SVR_MON
	PrintAT(buf, rval);
	cprintf("\n");
//#endif
	if(buf[2] == 'A') timeout = 300;	// 2004/11/18 D-Link 모뎀에서 24초 소요되어 15=>30 늘려줌 */
	else	timeout = 20;
	rval = svrATResult(buf, timeout);
//#ifdef SVR_MON
	if(rval <= 0) cprintf("timeout: %d\n", timeout);
	else	cprintf("\n");
//#endif
	return rval;
}
//#include "system_hdr.h"
int svrATResult(char *buf, int timeout)
{
	unsigned long 	timer, f_timer;
	int		c, read_size, exit, lf;

	read_size = exit = lf = 0;
	f_timer = timer = DS_TIMER;
	while(read_size < 64 && !exit) {
		c = sioGetc(gSvrSioPort);
		if(c < 0) {
			if(!read_size && (!timeout || timeout > 0 && DS_TIMER >= f_timer+timeout) || read_size && DS_TIMER >= timer+10L) exit = 1;
			else	taskYield();
		} else {
			timer = DS_TIMER;
			buf[read_size] = c; read_size++;
			if(c == '\n') lf++;
			//if(lf >= 2) exit = 1;
			if(lf >= 3) exit = 1;
		}
	}
//#ifdef SVR_MON
	if(read_size > 0) PrintAT(buf, read_size);
//#endif
	buf[read_size] = 0;
	return read_size;
}
