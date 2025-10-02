#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "uv.h"
#include "nilib.h"
#include "arpping.h"
#include "dhcp_private.h"
#include "dhcp.h"

#define DHCP_DEBUG		1

#ifdef DHCP_DEBUG
char dhcpStateMsg[10][12] = {
	"NULL",
	"INIT",
	"INIT_REBOOT",
	"SELECTING",
	"REQUESTING",
	"REBOOTING",
	"BOUNDING",
	"BOUND",
	"RENEWING",
	"REBINDING"
};
#define dhcps(state)	dhcpStateMsg[state]
#endif

static char	*_hostName = "Arvratech";
static DHCP				*dhcp, _dhcp;
static unsigned long	txBuf[137], rxBuf[137];		// 548 bytes
static uv_timer_t		*timerDhcp, _timerDhcp;
static uv_udp_t			*udpDhcp, _udpDhcp;
static uv_udp_send_t	udpsend;


void dhcpInit(void)
{
	dhcp = &_dhcp;
	dhcp->state = S_DHCP_NULL;
}

void *MainGetLoop(void);
static void _InitMsgTimer(DHCP *dhcp);

void dhcpOpen(unsigned char *macAddr, unsigned char *ipAddr)
{
	memset(dhcp, 0, sizeof(DHCP));
	memcpy(dhcp->macAddr, macAddr, 6);
	dhcp->xid = rand();
	udpDhcp = &_udpDhcp;
	timerDhcp = &_timerDhcp;
	uv_timer_init((uv_loop_t *)MainGetLoop(), timerDhcp);
	dhcp->fd = -1;
	if(ipAddr && !ipAddrIsNull(ipAddr)) {
		memcpy(dhcp->ipAddr, ipAddr, 4);
		dhcp->state = S_DHCP_INIT_REBOOT;
	} else {
	   dhcp->state = S_DHCP_INIT;
	}
	_InitMsgTimer(dhcp);
#ifdef DHCP_DEBUG
printf("%lu dhcpOpen: %d.%d.%d.%d => %s\n", MS_TIMER, (int)ipAddr[0], (int)ipAddr[1], (int)ipAddr[2], (int)ipAddr[3], dhcps(dhcp->state));
#endif
}

static void send_DHCP_RELEASE_DECLINE(DHCP *dhcp, int msgtype);
static void _DhcpUdpClose(DHCP *dhcp);

void dhcpClose(int status)
{
	if(dhcp->state > S_DHCP_NULL) {
		if(status == 0  && dhcp->state >= S_DHCP_BOUND) {
			send_DHCP_RELEASE_DECLINE(dhcp, 0);
		}
		_DhcpUdpClose(dhcp);
		uv_timer_stop(timerDhcp);
		dhcp->state = S_DHCP_NULL;
#ifdef DHCP_DEBUG
printf("dhcpClose\n");
#endif
	}
}

int dhcpIsOpened(void)
{
	int		rval;

	if(dhcp->state > S_DHCP_NULL) rval = 1;
	else	rval = 0;
	return rval;
}

int dhcpIsBound(void)
{
	int		rval;

	if(dhcp->state >= S_DHCP_BOUND) rval = 1;
	else	rval = 0;
	return rval;
}

int dhcpGetDnsServers(unsigned char dnsServers[][4])
{
	int		i;

	for(i = 0;i < dhcp->dnsServerCnt;i++)
		memcpy(dnsServers[i], dhcp->dnsServers[i], 4);
	return dhcp->dnsServerCnt;
}

void _OnDhcpTimer(uv_timer_t *handle);

static void _DhcpTimerStart(int timeout)
{
	uv_timer_start(timerDhcp, _OnDhcpTimer, timeout, 0)	;
}

static void _InitMsgTimer(DHCP *dhcp)
{
	unsigned long	data;
	int		val;

	data = rand();
	val = data & 0xfff; val -= 2048;
	val = 3000 + val;		// 952 msec .. 5047 msec 
	_DhcpTimerStart(val);
}

static void _RetryMsgTimer(DHCP *dhcp)
{
	unsigned long	data;
	int		val;
	
	data = rand();
	val = data & 0x3ff;  val -= 512;
	val = 27000 + val;		// 6488 msec .. 7511 msec
	_DhcpTimerStart(val);
}

static void _DhcpUdpError(DHCP *dhcp, int status)
{
	_DhcpUdpClose(dhcp);
}

void _OnDhcpUdpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	buf->base = (unsigned char *)rxBuf;
	buf->len  = sizeof(RIP_MSG);
}

void _DhcpUdpReceived(DHCP *dhcp, unsigned char *buf, int size, unsigned char *ipAddr, unsigned short ipPort);

void _OnDhcpUdpRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *buf, const struct sockaddr *saddr, unsigned flags)
{
	struct sockaddr_in *addr;
	unsigned char	ipAddr[4];
	unsigned short	ipPort;
	int		rval;

	if(nread > 0) {
		rval = nread;
		addr = (struct sockaddr_in *)saddr;
		memcpy(ipAddr, &addr->sin_addr.s_addr, 4);
		ipPort = ntohs(addr->sin_port);
printf("dhcp received: %d...\n", nread);
		_DhcpUdpReceived(dhcp, buf->base, nread, ipAddr, ipPort);
	} else {
printf("_OnDhcpUdpRecv error: %s\n", uv_err_name(nread));
		_DhcpUdpError(dhcp, nread);
	}
}

void _OnDhcpUdpSend(uv_udp_send_t *req, int status)
{
	if(status) {
printf("_OnDhcpUdpSend error: %s\n", uv_err_name(status));
		_DhcpUdpError(dhcp, status);
	}
}

static void _DhcpUdpOpen(DHCP *dhcp)
{
	struct sockaddr_in	_addr;
	char	*str;
	int		fd, rval, addrlen;

	fd = uv__socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
printf("_DhcpUdpOpen: uv_socket error: %s\n", strerror(errno));
		_DhcpUdpError(dhcp, fd);
		return;
	}
	str = niIfName();
printf("__DhcpOpen bind interface: %s\n", str);
	rval = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, str, strlen(str)+1);
if(rval < 0) printf("_DhcpUdpOpen: setsocketopt error: %s\n", strerror(errno));
	rval = uv_udp_init(MainGetLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port		  = htons(DHCP_CLIENT_PORT);
	rval = uv_udp_bind(udpDhcp, (const struct sockaddr *)&_addr, 0);
if(rval < 0) printf("_DhcpUdpOpen: uv_udp_bind error: %s\n", strerror(errno));
	rval = uv_udp_recv_start(udpDhcp, _OnDhcpUdpAlloc, _OnDhcpUdpRecv);
	uv_udp_set_broadcast(udpDhcp, 1);
	dhcp->fd = fd;
printf("_DhcpUdpOpen: %d\n", fd);
}

/*
static void _DhcpUdpOpen(DHCP *dhcp)
{
	struct sockaddr_in	_addr;
	struct ifreq		_interface;
	int		fd, rval, addrlen;

	fd = uv__socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
printf("_DhcpUdpOpen: uv_socket error: %s\n", strerror(errno));
		_DhcpUdpError(dhcp, fd);
		return;
	}
	strncpy(_interface.ifr_ifrn.ifrn_name, niIfName(), IFNAMSIZ);
	setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&_interface, sizeof(_interface));
	rval = uv_udp_init(MainGetLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	_addr.sin_family = AF_INET;
	_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	_addr.sin_port		  = htons(DHCP_CLIENT_PORT);
	rval = uv_udp_bind(udpDhcp, (const struct sockaddr *)&_addr, 0);
	rval = uv_udp_recv_start(udpDhcp, _OnDhcpUdpAlloc, _OnDhcpUdpRecv);
	uv_udp_set_broadcast(udpDhcp, 1);
	dhcp->fd = fd;
printf("_DhcpUdpOpen: %d\n", fd);
}
*/

static void _DhcpUdpOpen2(DHCP *dhcp)
{
	struct sockaddr_in	_saddr;
	char	*str;
	int		fd, rval, addrlen;

	fd = uv__socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
printf("_DhcpUdpOpen: uv_socket error: %s\n", strerror(errno));
		_DhcpUdpError(dhcp, fd);
		return;
	}
	str = niIfName();
printf("__DhcpOpen bind interface: %s\n", str);
	rval = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, str, strlen(str)+1);
if(rval < 0) printf("_DhcpUdpOpen: setsocketopt error: %s\n", strerror(errno));
	rval = uv_udp_init(MainGetLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	_saddr.sin_family		= AF_INET;
	_saddr.sin_addr.s_addr	= htonl(INADDR_ANY);
	_saddr.sin_port			= htons(DHCP_CLIENT_PORT);
	rval = uv_udp_bind(udpDhcp, (const struct sockaddr *)&_saddr, 0);
	rval = uv_udp_recv_start(udpDhcp, _OnDhcpUdpAlloc, _OnDhcpUdpRecv);
	dhcp->fd = fd;
printf("_DhcpUdpOpen2: %d\n", fd);
}

static void _DhcpUdpClose(DHCP *dhcp)
{
	int		rval;

	if(dhcp->fd >= 0) {
printf("_DhcpUdpClose: %d\n", dhcp->fd);
		rval = uv_udp_recv_stop(udpDhcp);
		uv_close((uv_handle_t *)udpDhcp, NULL);
		dhcp->fd = -1;
	}
}

static int _DhcpUdpWrite(DHCP *dhcp, unsigned char *dstIpAddr, unsigned char *data, int dataLen)
{
	struct sockaddr_in	_addr;
	uv_buf_t	uvbuf;
	int		rval;

	_addr.sin_family	= AF_INET;
	memcpy(&_addr.sin_addr.s_addr, dstIpAddr, 4);
	_addr.sin_port	  = htons(DHCP_SERVER_PORT);
	uvbuf.base = data; uvbuf.len = dataLen;
	rval = uv_udp_send(&udpsend, udpDhcp, &uvbuf, 1, (const struct sockaddr *)&_addr, _OnDhcpUdpSend);
	return rval;
}

static void _DhcpUdpSendMsg(DHCP *dhcp, unsigned char *dstIpAddr, unsigned char *data, int dataLen)
{
	uv_buf_t	uvbuf;
	int		size, rval;

	if(dhcp->fd < 0) {
		//if(dhcp->state == S_DHCP_RENEWING) _DhcpUdpOpen2(dhcp);
		if(dhcp->state == S_DHCP_RENEWING) _DhcpUdpOpen(dhcp);
		else	_DhcpUdpOpen(dhcp);
	}
	if(dhcp->fd >= 0) {
		rval = _DhcpUdpWrite(dhcp, dstIpAddr, data, dataLen);
		if(rval < 0) {
printf("DhcpUdpSendMsg: uv_udp_send error: %s\n", uv_err_name(rval));
			_DhcpUdpError(dhcp, rval);
		}
	}
	dhcp->tryCount++;
	_RetryMsgTimer(dhcp);
}
	
#ifdef DHCP_DEBUG
void prt_ip(char *msg, unsigned char *ip)
{
	printf("%s: %d.%d.%d.%d\n", msg, (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);
}

void prt_ip2(char *msg, unsigned char *myip, unsigned char *otip)
{
	printf("%s: My:%d.%d.%d.%d Another:%d.%d.%d.%d\n", msg, (int)myip[0], (int)myip[1], (int)myip[2], (int)myip[3], (int)otip[0], (int)otip[1], (int)otip[2], (int)otip[3]);
}
#endif

static int code_DHCP_DISCOVER(DHCP *dhcp)
{
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	dhcp->xid++;
	pRIPMSG = (RIP_MSG *)txBuf;
	memset(pRIPMSG, 0, sizeof(RIP_MSG));
	pRIPMSG->op			= DHCP_BOOTREQUEST;
	pRIPMSG->htype		= DHCP_HTYPE10MB;
	pRIPMSG->hlen		= DHCP_HLENETHERNET;
	pRIPMSG->hops		= DHCP_HOPS;
	pRIPMSG->xid		= htonl(dhcp->xid);
	pRIPMSG->secs		= htons(DHCP_SECS);
	pRIPMSG->flags		= htons(DHCP_FLAGSBROADCAST);
	memcpy(pRIPMSG->chaddr, dhcp->macAddr, 6);
	i = 0;
	// MAGIC_COOKIE
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 24)& 0xff);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 16)& 0xff);
	pRIPMSG->OPT[i++] = (char)((MAGIC_COOKIE >> 8)& 0xff);
	pRIPMSG->OPT[i++] = (char)(MAGIC_COOKIE& 0xff);
	// DHCP Message Type
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_DISCOVER;
	// Client-identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x01;
	memcpy(&pRIPMSG->OPT[i], dhcp->macAddr, 6); i += 6;
	// Host Name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(_hostName) + 3;	// length of hostname + 3
	strcpy((char *)&pRIPMSG->OPT[i], _hostName); i += strlen(_hostName);
	memcpy(&pRIPMSG->OPT[i], dhcp->macAddr+3, 3); i += 3;
	// Parameter Request List
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x06;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = dns;
	pRIPMSG->OPT[i++] = domainName;
	pRIPMSG->OPT[i++] = dhcpT1value;
	pRIPMSG->OPT[i++] = dhcpT2value;
	// End of this option field
	pRIPMSG->OPT[i++] = endOption;

	// send broadcasting packet
	dhcp->msgType = DHCP_DISCOVER;
	dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
#ifdef DHCP_DEBUG
//	printf("%u Tx:DHCP_DISCOVER\n", MS_TIMER);
#endif
	return sizeof(RIP_MSG);
}

// This function sends DHCP REQUEST message to DHCP server.
// state	0:SELECTING,REQUESTING 1:INIT_REBOOT,REBOOTING 2:BOUND,RENEWING 3:REBINDING
static int code_DHCP_REQUEST(DHCP *dhcp, int state)
{
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	dhcp->xid++;
	pRIPMSG = (RIP_MSG *)txBuf;
	memset(pRIPMSG, 0, sizeof(RIP_MSG));
	pRIPMSG->op			= DHCP_BOOTREQUEST;
	pRIPMSG->htype		= DHCP_HTYPE10MB;
	pRIPMSG->hlen		= DHCP_HLENETHERNET;
	pRIPMSG->hops		= DHCP_HOPS;
	pRIPMSG->xid		= htonl(dhcp->xid);
	pRIPMSG->secs		= htons(DHCP_SECS);
	if(state == S_DHCP_RENEWING) pRIPMSG->flags = 0;	// for Unicast
	else	pRIPMSG->flags = htons(DHCP_FLAGSBROADCAST);
	memcpy(pRIPMSG->chaddr, dhcp->macAddr, 6);
	i = 0;
	/* MAGIC_COOKIE */
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 24) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 16) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 8) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)(MAGIC_COOKIE & 0xff);
	// DHCP Message Type
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = DHCP_REQUEST;
	// Client-identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x01;
	memcpy(&pRIPMSG->OPT[i], dhcp->macAddr, 6); i += 6;
	// Requested Address
	if(state < S_DHCP_BOUND) {
		pRIPMSG->OPT[i++] = dhcpRequestedIPaddr;
		pRIPMSG->OPT[i++] = 0x04;
		memcpy(&pRIPMSG->OPT[i], dhcp->ipAddr, 4); i += 4;
	} else {
		memcpy(pRIPMSG->ciaddr, dhcp->ipAddr, 4);
	}
	// Server Identifier
	if(state == S_DHCP_REQUESTING) {
		pRIPMSG->OPT[i++] = dhcpServerIdentifier;
		pRIPMSG->OPT[i++] = 0x04;
		memcpy(&pRIPMSG->OPT[i], dhcp->svrAddr, 4); i += 4;
	}
	// Host Name
	pRIPMSG->OPT[i++] = hostName;
	pRIPMSG->OPT[i++] = strlen(_hostName) + 3;	// length of hostname + 3
	strcpy((char *)&(pRIPMSG->OPT[i]), _hostName); i += strlen(_hostName);
	memcpy(&pRIPMSG->OPT[i], dhcp->macAddr+3, 3); i += 3;
	// Parameter Request List
	pRIPMSG->OPT[i++] = dhcpParamRequest;
	pRIPMSG->OPT[i++] = 0x08;
	pRIPMSG->OPT[i++] = subnetMask;
	pRIPMSG->OPT[i++] = routersOnSubnet;
	pRIPMSG->OPT[i++] = dns;
	pRIPMSG->OPT[i++] = domainName;
	pRIPMSG->OPT[i++] = dhcpT1value;
	pRIPMSG->OPT[i++] = dhcpT2value;
	pRIPMSG->OPT[i++] = performRouterDiscovery;
	pRIPMSG->OPT[i++] = staticRoute;
	// End of this option field
	pRIPMSG->OPT[i++] = endOption;

	dhcp->msgType = DHCP_REQUEST;
	//if(state == S_DHCP_RENEWING) memcpy(dhcp->toAddr, dhcp->svrAddr, 4); 
	//else	dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
#ifdef DHCP_DEBUG
//printf("%u Tx:%d.%d.%d.%d DHCP_REQUEST: state=%d\n", MS_TIMER, (int)dhcp->toAddr[0], (int)dhcp->toAddr[1], (int)dhcp->toAddr[2], (int)dhcp->toAddr[3], state);
#endif
	return sizeof(RIP_MSG);
}

static void send_DHCP_RELEASE_DECLINE(DHCP *dhcp, int msgtype)	/* 0:RELEASE  NonZero:DECLINE */
{
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	dhcp->xid++;
	pRIPMSG = (RIP_MSG *)txBuf;
	memset(pRIPMSG, 0, sizeof(RIP_MSG));
	pRIPMSG->op			= DHCP_BOOTREQUEST;
	pRIPMSG->htype		= DHCP_HTYPE10MB;
	pRIPMSG->hlen		= DHCP_HLENETHERNET;
	pRIPMSG->hops		= DHCP_HOPS;
	pRIPMSG->xid		= htonl(dhcp->xid);
	pRIPMSG->secs		= htons(DHCP_SECS);
	pRIPMSG->flags		= 0;	//DHCP_FLAGSBROADCAST;
	memcpy(pRIPMSG->chaddr, dhcp->macAddr, 6);
	i = 0;
	// MAGIC_COOKIE
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 24) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 16) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)((MAGIC_COOKIE >> 8) & 0xff);
	pRIPMSG->OPT[i++] = (unsigned char)(MAGIC_COOKIE & 0xff);
	// DHCP Message Type
	pRIPMSG->OPT[i++] = dhcpMessageType;
	pRIPMSG->OPT[i++] = 0x01;
	pRIPMSG->OPT[i++] = (msgtype ? DHCP_DECLINE : DHCP_RELEASE);
	// Client-identifier
	pRIPMSG->OPT[i++] = dhcpClientIdentifier;
	pRIPMSG->OPT[i++] = 0x07;
	pRIPMSG->OPT[i++] = 0x01;
	memcpy(&pRIPMSG->OPT[i], dhcp->macAddr, 6); i += 6;
	// Server Identifier
	pRIPMSG->OPT[i++] = dhcpServerIdentifier;
	pRIPMSG->OPT[i++] = 0x04;
	memcpy(&pRIPMSG->OPT[i], dhcp->svrAddr, 4); i += 4;
	// Requested Address
	if(msgtype)	{
		pRIPMSG->OPT[i++] = dhcpRequestedIPaddr;
		pRIPMSG->OPT[i++] = 0x04;
		memcpy(&pRIPMSG->OPT[i], dhcp->ipAddr, 4); i += 4;
	}
	// End of this option field
	pRIPMSG->OPT[i++] = endOption;

	if(msgtype) {
		dhcp->msgType = DHCP_DECLINE;
		dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
	} else{
		dhcp->msgType = DHCP_RELEASE;
		//memcpy(dhcp->toAddr, dhcp->svrAddr, 4);
dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
	}
#ifdef DHCP_DEBUG
if(msgtype) printf("%u Tx:%d.%d.%d.%d DHCP_DECLINE\n", MS_TIMER, (int)dhcp->toAddr[0], (int)dhcp->toAddr[1], (int)dhcp->toAddr[2], (int)dhcp->toAddr[3]);
else		printf("%u Tx:%d.%d.%d.%d DHCP_RELEASE\n", MS_TIMER, (int)dhcp->toAddr[0], (int)dhcp->toAddr[1], (int)dhcp->toAddr[2], (int)dhcp->toAddr[3]);
#endif
	rval = _DhcpUdpWrite(dhcp, dhcp->toAddr, (unsigned char *)txBuf, sizeof(RIP_MSG));
	if(rval < 0) {
		if(msgtype) printf("%u sendDHCP_DECLINE: uv_udp_send error: %s\n", MS_TIMER, uv_err_name(rval));
		else	printf("%u sendDHCP_RELEASE: uv_udp_send error: %s\n", MS_TIMER, uv_err_name(rval));
		_DhcpUdpError(dhcp, rval);
	}
}

// This function parses the reply message from DHCP server.
// @return	success - return type, fail - 0
static int _ParseDHCPMSG(DHCP *dhcp, unsigned char *buf, int length, unsigned char *ipAddr, unsigned short ipPort)
{
	RIP_MSG		*pRIPMSG;
	unsigned char	*p, *e;
	unsigned short	svr_port;
	unsigned long	leased_time;
	int		c, type, opt_len, exitflg;

	pRIPMSG = (RIP_MSG *)buf;
	if(length <= 0) return 0;
#ifdef DHCP_DEBUG
printf("%u Received %d From %d.%d.%d.%d.%d\n", MS_TIMER, length, ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3], (int)ipPort);
#endif
	if(dhcp->tryCount == 0) {
#ifdef DHCP_DEBUG
printf("Unexpected received, ignore it\n");
#endif
		return 0;
	}
	if(ipPort != DHCP_SERVER_PORT) {
#ifdef DHCP_DEBUG
printf("Not DHCP server port: %d\n", (int)ipPort);
#endif		
		return 0;
	}
	if(pRIPMSG->op != DHCP_BOOTREPLY) {
#ifdef DHCP_DEBUG
printf("Not DHCP Message\n");
#endif
		return 0;
	}
	if(memcmp(pRIPMSG->chaddr, dhcp->macAddr, 6) || pRIPMSG->xid != htonl(dhcp->xid)) {
//#ifdef DHCP_DEBUG
printf("No My DHCP Message. This message is ignored. xid=%x,%x\n", ntohl(pRIPMSG->xid), dhcp->xid);
//#endif
		return 0;
	}
	if(!ipAddrIsNull(dhcp->realSvrAddr)) {
		if(memcmp(ipAddr, dhcp->realSvrAddr, 4)) {
#ifdef DHCP_DEBUG
prt_ip2("Another DHCP sever send a response message. This is ignored.", dhcp->realSvrAddr, ipAddr);
#endif
			return 0;
		}
	} else {
		memcpy(dhcp->realSvrAddr, ipAddr, 4);
	}
#ifdef DHCP_DEBUG
prt_ip("IPAddr(yiaddr)", pRIPMSG->yiaddr);	// delay copy to dhcp->ipAddr until endOption
#endif
	type = 0;
	p = (unsigned char *)&pRIPMSG->op;
	p += 240;
	e = p + (length - 240);
	exitflg = 0;
	dhcp->dnsServerCnt = 0;
	while(p < e && !exitflg) {
		c = *p++;
		switch(c) {
		case endOption :
			memcpy(dhcp->ipAddr, pRIPMSG->yiaddr, 4);
#ifdef DHCP_DEBUG
printf("endOption: type=%d\n", type);
#endif
			return type;
		case padOption :
			break;
		case dhcpMessageType :
			opt_len = *p++;
			type = *p;
#ifdef DHCP_DEBUG
printf("dhcpMessageType: 0x%x\n", type);
#endif
			break;
		case subnetMask :
			opt_len = *p++;
			memcpy(dhcp->subnet, p, 4);
#ifdef DHCP_DEBUG
prt_ip("subnetMask", dhcp->subnet);
#endif
			break;
		case routersOnSubnet :
			opt_len = *p++;
			memcpy(dhcp->gwAddr, p, 4);
#ifdef DHCP_DEBUG
prt_ip("routersOnSubnet", dhcp->gwAddr);
#endif
			break;
		case dns:
			opt_len = *p++;
			if(dhcp->dnsServerCnt < 4) {
				memcpy(dhcp->dnsServers[dhcp->dnsServerCnt], p, 4);
				dhcp->dnsServerCnt++;
			}
#ifdef DHCP_DEBUG
prt_ip("dns", p);
#endif
			break;
		case dhcpIPaddrLeaseTime:
			opt_len = *p++;
			BYTEtoLONG(p, &leased_time);	// lease_time = n_ntohl(*unsigned long *)p); => Pabort(alignment)
#ifdef DHCP_DEBUG
printf("dhcp LeasedTime: 0x%08x(%d hours)\n", leased_time, leased_time/3600);
#endif
			if(leased_time == 0xffffffff) dhcp->leasedTime = 3660;	// one hour
			else 	dhcp->leasedTime = leased_time;
//dhcp->leasedTime = 600;
			dhcp->leasedTimer = MS_TIMER / 1000;
			break;
		case dhcpServerIdentifier:
			opt_len = *p++;
//			if(*((unsigned long*)DHCP_SIP) == 0 || !memcmp(DHCP_REAL_SIP, svr_addr, 4) || !memcmp(DHCP_SIP, svr_addr, 4)) {
			if(ipAddrIsNull(dhcp->svrAddr) || !memcmp(p, dhcp->svrAddr, 4)) {
				memcpy(dhcp->svrAddr, p, 4);
#ifdef DHCP_DEBUG
prt_ip("dhcpServerIdentifier", dhcp->svrAddr);
#endif
			} else {
				exitflg = 1;
#ifdef DHCP_DEBUG
prt_ip2("Another DHCP server dhcpServerIdentifier. This message is ignored.", dhcp->svrAddr, p);
#endif
			}
			break;
		default :
			opt_len = *p++;
		}
		p += opt_len;
	}
	return 0;
}

static void _ToInit(DHCP *dhcp)
{
	int		i;

	if(dhcp->state >= S_DHCP_BOUND) {
		send_DHCP_RELEASE_DECLINE(dhcp, 0);
		niDhcpUnbound();
	}	
	ipAddrSetNull(dhcp->ipAddr);
	ipAddrSetNull(dhcp->subnet);
	ipAddrSetNull(dhcp->gwAddr);
	dhcp->dnsServerCnt = 0;
	dhcp->state = S_DHCP_INIT;
	_InitMsgTimer(dhcp);
}

int		arp_fd;
unsigned long	arp_timer;

static void _ToBounding(DHCP *dhcp, int newIP)
{
	unsigned char	ipAddr[4];
	int		rval;

	dhcp->state = S_DHCP_BOUNDING;
	arp_fd = spackArpOpen();
	ipAddrSetNull(ipAddr);
	rval = spackArpSend(arp_fd, niIfName(), dhcp->macAddr, ipAddr, dhcp->ipAddr);
	arp_timer = MS_TIMER;
	_DhcpTimerStart(100);
}

static void _ToBound(DHCP *dhcp)
{
	dhcp->state = S_DHCP_BOUND;
	_DhcpUdpClose(dhcp);
	niDhcpBound(dhcp->ipAddr, dhcp->subnet, dhcp->gwAddr);
	_DhcpTimerStart(60000);	// one minute
}

static int _CheckLeased(DHCP *dhcp)
{
	unsigned char	macAddr[6], ipAddr[4];
	int		rval;

	memset(ipAddr, 0, 4);
	rval = spackArpRecv(arp_fd, niIfName(), dhcp->macAddr, ipAddr, macAddr, dhcp->ipAddr);
	if(rval < 0) {
		printf("_CheckLeased: ARP error\n");
		rval = 1;
	} else if(rval > 0) {
		if(!memcmp(dhcp->macAddr, macAddr, 6)) {
			printf("_CheckLeased: ARP self...\n");
			rval = 1;
		} else {
			printf("_CheckLeased: ARP conflict...\n");
			rval = -1;
		}
	} else if((MS_TIMER-arp_timer) > 2500) {
//		printf("_CheckLeased: ARP timeout\n");
		rval = 1;
	} else {
		_DhcpTimerStart(100);
		rval = 0;
	}
	if(rval) spackArpClose(arp_fd);
	return rval;
}

void _OnDhcpTimer(uv_timer_t *handle)
{
	unsigned char	ipAddr[4];
	unsigned long	timer;
	int		size, rval, state, duration;

	state = dhcp->state;
	size = 0;
	switch(state) {
	case S_DHCP_INIT:
		dhcp->state = S_DHCP_SELECTING; dhcp->tryCount = 0;
	 	size = code_DHCP_DISCOVER(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_DISCOVER(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		break;
	case S_DHCP_INIT_REBOOT:
		dhcp->state = S_DHCP_REBOOTING; dhcp->tryCount = 0;
	 	size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		break;
	case S_DHCP_SELECTING:
		if(dhcp->tryCount < 3) {
			size = code_DHCP_DISCOVER(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_DISCOVER(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, dhcp->tryCount);
#endif
		} else {
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, retryout => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_REQUESTING:
	case S_DHCP_REBOOTING:
		if(dhcp->tryCount < 3) {
			size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, dhcp->tryCount);
#endif
		} else {
			// INIT_REBOOT상태에서 subnet이 다른 경우(예:192.168.2.x망에서 192.168.1.x주소가 있을때)
			// DHCP가 무응답하므로 INIT_REBOOT상태 유지에서 INIT상태 변경으로 수정
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, retryout => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_BOUNDING:
		rval = _CheckLeased(dhcp);
		if(rval > 0) {
			_ToBound(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: Check leased(OK) => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		} else if(rval < 0) {
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: Check leased(conflict), DHCP_DECLINE => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_BOUND:
		timer = MS_TIMER / 1000; duration = dhcp->leasedTime >> 1;
		if((timer-dhcp->leasedTimer) >= duration) {
			dhcp->state = S_DHCP_RENEWING; dhcp->tryCount = 0;
			size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout:T1, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		} else {
#ifdef DHCP_DEBUG
printf("%u %s: remain %d to T1\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			_DhcpTimerStart(60000);	// one minute
		}
		break;
	case S_DHCP_RENEWING:
		if(dhcp->tryCount) {
			if(dhcp->tryCount < 3) {
				size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, (int)dhcp->tryCount);
#endif
			} else {
				dhcp->timer = timer; dhcp->tryCount = 0;
				_DhcpUdpClose(dhcp);
				_DhcpTimerStart(60000);	// one minute
#ifdef DHCP_DEBUG
printf("%u %s: timeout, retryout\n", MS_TIMER, dhcps(state));
#endif
			}
		} else {
			timer = MS_TIMER / 1000; duration = (dhcp->leasedTime * 7) >> 3;
			if((timer-dhcp->leasedTimer) >= duration) {
				dhcp->state = S_DHCP_REBINDING;
				size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout:T2, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
			} else if((timer-dhcp->timer) >= (duration-(dhcp->timer-dhcp->leasedTimer))/2) {
				size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x)\n", MS_TIMER, dhcps(state), dhcp->xid);
#endif
			} else {
#ifdef DHCP_DEBUG
printf("%u %s: remain %d to T2\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			}
		}
		break;
	case S_DHCP_REBINDING:
		if(dhcp->tryCount) {
			if(dhcp->tryCount < 3) {
				size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x): rety=%d\n", MS_TIMER, dhcps(state), dhcp->xid, (int)dhcp->tryCount);
#endif
			} else {
				dhcp->timer = timer; dhcp->tryCount = 0;
				_DhcpUdpClose(dhcp);
				_DhcpTimerStart(60000);	// one minute
#ifdef DHCP_DEBUG
printf("%u %s: timeout, retryout\n", MS_TIMER, dhcps(state));
#endif
			}
		} else {
			timer = MS_TIMER / 1000; duration = dhcp->leasedTime;
			if((timer-dhcp->leasedTimer) >= duration) {
				_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: leased time expired => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
			} else if((timer-dhcp->timer) >= (duration-(dhcp->timer-dhcp->leasedTimer))/2) {
				size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: timeout, DHCP_REQUEST(%x)", MS_TIMER, dhcps(state), dhcp->xid);
#endif
			} else {
#ifdef DHCP_DEBUG
printf("%u %s: remain %d to expired\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			}
		}
		break;
	}
	if(size > 0) _DhcpUdpSendMsg(dhcp, dhcp->toAddr, (unsigned char *)txBuf, size);
}

void _DhcpUdpReceived(DHCP *dhcp, unsigned char *buf, int length, unsigned char *fromIpAddr, unsigned short fromIpPort)
{
	int		rval, size, state, type;

	state = dhcp->state;
	type = _ParseDHCPMSG(dhcp, buf, length, fromIpAddr, fromIpPort);
	size = 0;
	switch(state) {
	case S_DHCP_SELECTING:
		if(type == DHCP_OFFER) {
			dhcp->state = S_DHCP_REQUESTING; dhcp->tryCount = 0;
			size = code_DHCP_REQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%u %s: DHCP_OFFER, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_REQUESTING:
	case S_DHCP_REBOOTING:
	case S_DHCP_RENEWING:
	case S_DHCP_REBINDING:
		switch(type) {
		case DHCP_ACK:
			_ToBounding(dhcp, 1);
#ifdef DHCP_DEBUG
printf("%u %s: DHCP_ACK, Check leased => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
			break;
		case DHCP_NAK:
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%u %s: DHCP_NAK => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
			break;
		}
		break;
	}
	if(size > 0) _DhcpUdpSendMsg(dhcp, dhcp->toAddr, (unsigned char *)txBuf, size);
}

