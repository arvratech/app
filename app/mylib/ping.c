#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <arpa/inet.h>
#include "defs.h"
#include "rtc.h"
#include "prim.h"
#include "syscfg.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "uv.h"
#include "ping.h"


static uv_timer_t		*timerPing, _timerPing;
static uv_udp_t			*udpPing, _udpPing;
static uv_udp_send_t	udpsend;
static unsigned char	txBuf[64], rxBuf[84];
static unsigned short	txId, txSeqNo;
static unsigned char	txIpAddr[4];


void _OnPingAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (unsigned char *)rxBuf;
	buf->len  = 84;
}

unsigned short _ICMP_Checksum(unsigned char *buf, int len)
{
	unsigned char	*p;
	unsigned short	answer;
	unsigned long	sum;
	int		c;

	p = buf;
	sum = 0L;
	while(len > 1) {
		c = *p++;
		c <<= 8;
		c += *p++;
		sum += c;
		len -= 2;
	}
	if(len == 1) sum += *p;	// mop up an odd byte, if necessary
	sum = (sum >> 16 ) + (sum & 0xffff);	// add high 16 to low 16
	sum += (sum >> 16);		// add carry ???
	answer = ~sum;			// ones-complement, truncate    
    return answer;
}

/* Return
 *	1: ARP error
 *	2: Loss(timeout)
 *	3: Checksum error
 *	4: Destination unreachable
 *	5: Time exceed error
 *	6: Unknown message		Wiznet단말기에서 자신의 IP로 EchoRequest 보내면 EchoReply에 그대로 리턴(Type=08로 Unknown message)
 */
int _PingDataReceived(unsigned char *buf, int len, unsigned char *ipAddr, unsigned short ipPort)
{
	unsigned short	id, seqNo;
	unsigned char	*p, addr[4];
	int		i, rval;

	buf += 20; len -= 20;
//printf("##### Rx=%d [%02x", len, (int)buf[0]);
//for(i = 1;i < len;i++) printf("-%02x", (int)buf[i]); printf("]\n");
	if(len == 64) {
		BYTEtoSHORT(buf+2, &id);	// checksum
		buf[2] = buf[3] = 0;		// for checksum calculation
		if(id != _ICMP_Checksum(buf, len)) return 3;	// packet's checksum error
		if(buf[0] == 0) {	// if the received packet is ping-reply 
			BYTEtoSHORT(buf+4, &id); BYTEtoSHORT(buf+6, &seqNo);
			if(id != txId || seqNo != txSeqNo || memcmp(ipAddr, txIpAddr, 4)) rval = 6;	// Unknown msg
			else	rval = 0;
		} else if(buf[0] == 3) {	// If the packet is unreachable message      
			rval = 4;		// Destination Unreachable;
		} else if(buf[0] == 11) {	// If the packet is time exceeded message
			rval = 5;		// TTL expired in transit
		} else {			// if the packet is unknown message
//cprintf("Unknown: %d(0x%02x)\n", (int)buf[0], (int)buf[0]);
			rval = 6;	// Unknown Message
		}
	} else	rval = 6;
	return rval;
}

void _PingResult(int result, unsigned char *ipAddr)
{
	unsigned char	msg[12];

	msg[0] = GM_PING_RESULT; msg[1] = result;
	if(ipAddr) {
		memcpy(msg+2, ipAddr, 4); memset(msg+6, 4, 0);
	} else	memset(msg+2, 8, 0);
	appPostMessage(msg);
}

void _OnPingRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *saddr, unsigned flags)
{
	struct sockaddr_in *addr;
	unsigned char	ipAddr[4];
	unsigned short	ipPort;
	int		rval;

	uv_timer_stop(timerPing);
	if(nread > 0) {
		rval = nread;
		addr = (struct sockaddr_in *)saddr;
		memcpy(ipAddr, &addr->sin_addr.s_addr, 4);
		ipPort = ntohs(addr->sin_port);
		rval = _PingDataReceived(buf->base, nread, ipAddr, ipPort);
		_PingResult(rval, ipAddr);		
	} else if(nread < 0) {
printf("Ping recv error: %d\n", nread);
		_PingResult(1, NULL);
	} else {
//printf("Ping recv: size=0\n");
//		_PingResult(1, NULL);
	}
}

void _OnPingSend(uv_udp_send_t *req, int status)
{
	if(status) {
printf("Ping send error: %d\n", status);
		uv_timer_stop(timerPing);
		_PingResult(1, NULL);
	}
}

void _OnPingTimer(uv_timer_t *handle)
{
	_PingResult(2, NULL);
}

int pingSend(unsigned char *ipAddr, int id, int seqNo)
{
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	unsigned char	*p;
	unsigned short	sVal;
	int		i, rval, len;

	p = txBuf;
	*p++ = 0x08;	// Type: ICMP_ECHO
	*p++ = 0x00;	// Code
	*p++ = 0x00; *p++ = 0x00;	// Checksum
	txId = id; SHORTtoBYTE(txId, p); p += 2;			// ID
	txSeqNo = seqNo; SHORTtoBYTE(txSeqNo, p); p += 2;	// Sequence number
	len = 56;
	for(i = 0;i < len;i++) *p++ = 'a' + i % 23;	// fill 'a'-'w' characters into ping-request's data 
	len += 8;
	sVal = _ICMP_Checksum(txBuf, len);
	SHORTtoBYTE(sVal, txBuf+2);	//	 Checksum
	memcpy(txIpAddr, ipAddr, 4);
	_addr.sin_family	= AF_INET;
	memcpy(&_addr.sin_addr.s_addr, ipAddr, 4);
	_addr.sin_port	  = htons(0);
	uvbuf.base = txBuf; uvbuf.len = len;
//printf("##### Tx=%d [%02x", len, (int)txBuf[0]);
//for(i = 1;i < len;i++) printf("-%02x", (int)txBuf[i]); printf("]\n");
	rval = uv_udp_send(&udpsend, udpPing, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnPingSend);
	if(!rval) uv_timer_start(timerPing, _OnPingTimer, 3000, 0);
	return rval;
}

void *MainLoop(void);

int pingOpen(void)
{
	int		fd, rval;

	timerPing = &_timerPing;
	uv_timer_init(MainLoop(), timerPing);
	udpPing = &_udpPing;
	fd = uv__socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if(fd < 0) {
printf("pingOpen: uv_socket error: %s\n", strerror(errno));
		return -1;
	}
	rval = uv_udp_init(MainLoop(), udpPing);
	rval = uv_udp_open(udpPing, fd);
	rval = uv_udp_recv_start(udpPing, _OnPingAlloc, _OnPingRecv);
	return 0;
}

void pingClose(void)
{
	uv_udp_recv_stop(udpPing);
	uv_close((uv_handle_t *)udpPing, NULL);
	uv_timer_stop(timerPing);
}

void pingResultText(char *text, int result, unsigned char *ipAddr)
{
	switch(result) {
	case 0:		sprintf(text, "64 bytes from %s", inet2addr(ipAddr)); break;
	case 1:		sprintf(text, "ARP error"); break;
	case 2:		sprintf(text, "Loss(timeout)"); break;
	case 3:		sprintf(text, "From %s Checksum error", inet2addr(ipAddr)); break;
	case 4:		sprintf(text, "From %s Destination unreachable", inet2addr(ipAddr)); break;
	case 5:		sprintf(text, "From %s Time exceed error", inet2addr(ipAddr)); break;
	case 6:		sprintf(text, "From %s Unknown message", inet2addr(ipAddr)); break;
	default:	text[0] = ' '; text[1] = 0;
	}
}

