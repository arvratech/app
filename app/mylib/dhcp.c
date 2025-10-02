#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include "defs.h"
#include "rtc.h"
#include "prim.h"
#include "sin.h"
#include "sll.h"
#include "cbuf.h"
#include "syscfg.h"
#include "uv.h"
#include "nilib.h"
#include "arpping.h"
#include "dhcp_private.h"
#include "dhcp.h"

//#define DHCP_DEBUG		1
//#define RAW_SOCKET		1

//#ifdef DHCP_DEBUG
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
//#endif

static char	*_hostName = "Arvratech";
static DHCP				*dhcp, _dhcp;
static unsigned long	txBuf[144], rxBuf[144];		// 576 bytes
static uv_timer_t		*timerDhcp, _timerDhcp;
static uv_udp_t			*udpDhcp, _udpDhcp;
static uv_udp_send_t	udpsend;
static int				rawSock;


void dhcpInit(void)
{
	dhcp = &_dhcp;
	dhcp->state = S_DHCP_NULL;
	dhcp->onBound = dhcp->onUnbound = NULL;
}

void *MainLoop(void);
static void _ToState(DHCP *dhcp, int state);
static void _InitMsgTimer(DHCP *dhcp);

void dhcpOpen(int ifIndex, unsigned char *macAddr, unsigned char *ipAddr)
{
	int		state;

	memset(dhcp, 0, sizeof(DHCP));
	dhcp->ifIndex = ifIndex;
	memcpy(dhcp->macAddr, macAddr, 6);
	dhcp->xid = rand();
	udpDhcp = &_udpDhcp;
	timerDhcp = &_timerDhcp;
	uv_timer_init((uv_loop_t *)MainLoop(), timerDhcp);
	dhcp->fd = -1;
	if(ipAddr && !ipIsNull(ipAddr)) {
		memcpy(dhcp->ipAddr, ipAddr, 4);
		state = S_DHCP_INIT_REBOOT;
	} else {
	   state = S_DHCP_INIT;
	}
	_ToState(dhcp, state);
	_InitMsgTimer(dhcp);
#ifdef DHCP_DEBUG
printf("%lu dhcpOpen: %s => %s\n", MS_TIMER, inet2addr(dhcp->ipAddr), dhcps(dhcp->state));
#endif
}

void dhcpSetCallback(void *context, void (*onBound)(void *), void (*onUnbound)(void *))
{
	dhcp->cbContext	= context;
	dhcp->onBound	= onBound;
	dhcp->onUnbound	= onUnbound;
}

static void _DhcpSendRELEASE_DECLINE(DHCP *dhcp, int msgtype);
static void _DhcpUdpClose(DHCP *dhcp);

void dhcpClose(int status)
{
	if(dhcp->state > S_DHCP_NULL) {
		if(status == 0  && dhcp->state >= S_DHCP_BOUND) {
			_DhcpSendRELEASE_DECLINE(dhcp, 0);
		}
		_DhcpUdpClose(dhcp);
		uv_timer_stop(timerDhcp);
		_ToState(dhcp, S_DHCP_NULL);
#ifdef DHCP_DEBUG
printf("%lu dhcpClose\n", MS_TIMER);
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

unsigned char *dhcpIpAddress(void)
{
	return dhcp->ipAddr;
}

unsigned char *dhcpSubnetMask(void)
{
	return dhcp->subnet;
}

unsigned char *dhcpDefaultGateway(void)
{
	return dhcp->gwAddr;
}
	
int dhcpGetDnsServers(unsigned char dnsServers[][4])
{
	int		i, count;

	count = dhcp->dnsServerCnt;
	for(i = 0;i < count;i++)
		memcpy(dnsServers[i], dhcp->dnsServers[i], 4);
	return count;
}

static void _OnDhcpTimer(uv_timer_t *handle);

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

static void _OnDhcpUdpSend(uv_udp_send_t *req, int status)
{
	if(status) {
printf("_OnDhcpUdpSend error: %s\n", uv_err_name(status));
		_DhcpUdpError(dhcp, status);
	}
}

static void _OnDhcpUdpAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	UDP_DHCP_PACKET		*pkt;

	pkt = (UDP_DHCP_PACKET *)rxBuf;
	if(rawSock) {
		buf->base = (unsigned char *)pkt;
		buf->len  = sizeof(UDP_DHCP_PACKET);
	} else {
		buf->base = (unsigned char *)&pkt->msg;
		buf->len  = sizeof(RIP_MSG);
	}
}

static void _DhcpReceivedMsg(DHCP *dhcp, void *buf, unsigned char *fromAddr, unsigned char *fromMacAddr);

static void _OnDhcpUdpRecv(uv_udp_t *handle, ssize_t nread, const uv_buf_t *uvbuf, const struct sockaddr *addr, unsigned flags)
{
	UDP_DHCP_PACKET		*pkt;
	unsigned char	*buf, macAddr[8], srcAddr[4], dstAddr[4];
	unsigned short	srcPort, dstPort;
	int		rval, ifIndex;

	if(nread > 0) {
		rval = 0;
		if(rawSock) {
			pkt = (UDP_DHCP_PACKET *)uvbuf->base;
			sllGetAddr((struct sockaddr *)addr, &ifIndex, macAddr);
			if(!ipIsBroadcast(dhcp->toAddr) && memcmp(macAddr, dhcp->svrMacAddr, 6)) {
				printf("unknown MAC address\n");
				rval = -1;
			} else {
				rval = sllDecodePacket(srcAddr, &srcPort, dstAddr, &dstPort, pkt, nread);
				if(rval < 0) {
					printf("%s\n", sllStrError());
				} else  if(srcPort != DHCP_SERVER_PORT || dstPort != DHCP_CLIENT_PORT) {
					printf("unknown port: %d\n", (int)srcPort);
					rval = -1;
				} else if(nread != sizeof(UDP_DHCP_PACKET)) {
					printf("length error: %d\n", nread);
					rval = -1;
				}
				buf = (unsigned char *)&pkt->msg;
			}
		} else {
			sinGetAddr(addr, srcAddr, &srcPort);
			if(srcPort != DHCP_SERVER_PORT) {
				printf("unknown port: %d\n", (int)srcPort);
				rval = -1;
			} else if(nread != sizeof(RIP_MSG)) {
				printf("length error: %d\n", nread);
				rval = -1;
			}
			buf = (unsigned char *)uvbuf->base;;
		}
		if(rval == 0 && dhcp->tryCount == 0) {
#ifdef DHCP_DEBUG
printf("Unexpected received, ignore it\n");
#endif
			rval = -1;
		}
		if(rval == 0) {
#ifdef DHCP_DEBUG
printf("%lu Received %d From %s\n", MS_TIMER, nread, inet2addr(srcAddr));
#endif
			_DhcpReceivedMsg(dhcp, buf, srcAddr, macAddr);
		}
	} else {
//printf("_OnDhcpUdpRecv error: %s\n", uv_err_name(nread));
//		_DhcpUdpError(dhcp, nread);
	}
}

static int _DhcpUdpOpenRaw(DHCP *dhcp)
{
	struct sockaddr_ll	_slladdr;
	char	*str;
	int		fd, rval;

	fd = uv__socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(fd < 0) {
		printf("_DhcpUdpOpenRaw: uv_socket error: %s\n", strerror(errno));
		return - 1;
	}
	rval = uv_udp_init(MainLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	sllSetAddr((struct sockaddr *)&_slladdr, dhcp->ifIndex, NULL);
	rval = uv_udp_bind(udpDhcp, (const struct sockaddr *)&_slladdr, 0);
	if(rval < 0) {
		printf("_DhcpUdpOpenRaw: uv_udp_bind error: %s\n", strerror(errno));
		return -1;
	}
	rval = uv_udp_recv_start(udpDhcp, _OnDhcpUdpAlloc, _OnDhcpUdpRecv);
	dhcp->fd = fd; rawSock = 1;
printf("_DhcpUdpOpenRaw: %d\n", fd);
	return 0;
}

static int _DhcpUdpOpen(DHCP *dhcp)
{
	struct sockaddr_in	_sinaddr;
	char	*str;
	int		fd, rval;

	fd = uv__socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(fd < 0) {
		printf("_DhcpUdpOpen: uv_socket error: %s\n", strerror(errno));
		return -1;
	}
	str = niIfName();
printf("__DhcpOpen bind interface: %s\n", str);
	rval = setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, str, strlen(str)+1);
if(rval < 0) printf("_DhcpUdpOpen: setsocketopt error: %s\n", strerror(errno));
	rval = uv_udp_init(MainLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	sinSetAddr((struct sockaddr *)&_sinaddr, NULL, DHCP_CLIENT_PORT);
	rval = uv_udp_bind(udpDhcp, (const struct sockaddr *)&_sinaddr, 0);
	if(rval < 0) {
		printf("_DhcpUdpOpen: uv_udp_bind error: %s\n", strerror(errno));
		return -1;
	}
	rval = uv_udp_recv_start(udpDhcp, _OnDhcpUdpAlloc, _OnDhcpUdpRecv);
	dhcp->fd = fd; rawSock = 0;
printf("_DhcpUdpOpen: %d\n", fd);
	return 0;
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

static int _DhcpUdpWriteRaw(DHCP *dhcp, unsigned char *dstMacAddr, void *buf, int length)
{
	struct sockaddr_ll	_slladdr;
	uv_buf_t	uvbuf;
	int		rval;

//printf("DhcpUdpWriteRaw...%d\n", length);
	sllSetAddr((struct sockaddr *)&_slladdr, dhcp->ifIndex, dstMacAddr);
	uvbuf.base = buf; uvbuf.len = length;
	rval = uv_udp_send(&udpsend, udpDhcp, &uvbuf, 1, (const struct sockaddr *)&_slladdr, _OnDhcpUdpSend);
	return rval;
}

static int _DhcpUdpWrite(DHCP *dhcp, unsigned char *dstIpAddr, void *buf, int length)
{
	struct sockaddr_in	_sinaddr;
	uv_buf_t	uvbuf;
	int		rval, addrlen;

//printf("DhcpUdpWrite...%d\n", length);
	addrlen = sizeof(_sinaddr);
	memset(&_sinaddr, 0, addrlen);
	_sinaddr.sin_family		= AF_INET;
	memcpy(&_sinaddr.sin_addr.s_addr, dstIpAddr, 4);
	_sinaddr.sin_port		= htons(DHCP_SERVER_PORT);
	uvbuf.base = buf; uvbuf.len = length;
	rval = uv_udp_send(&udpsend, udpDhcp, &uvbuf, 1, (const struct sockaddr *)&_sinaddr, _OnDhcpUdpSend);
	return rval;
}

static void _DhcpUdpSendMsg(DHCP *dhcp)
{
	UDP_DHCP_PACKET		*pkt;
	unsigned char	*sa, *ma;
	int		rval, size, rawsock;

	pkt = (UDP_DHCP_PACKET *)txBuf;
#ifdef RAW_SOCKET
	rawsock = 1;
	if(ipIsBroadcast(dhcp->toAddr))	{
		sa = INADDR_ANY_ADDR;
		ma = MAC_BCAST_ADDR;
	} else {
		sa = dhcp->ipAddr;
		ma = dhcp->svrMacAddr;
	}
#else
	if(ipIsBroadcast(dhcp->toAddr))	{
		rawsock = 1;
		sa = INADDR_ANY_ADDR;
		ma = MAC_BCAST_ADDR;
	} else {
		rawsock = 0;
	}
#endif
	if(dhcp->fd > 0 && rawsock != rawSock) _DhcpUdpClose(dhcp);
	if(rawsock) {
		if(dhcp->fd < 0) _DhcpUdpOpenRaw(dhcp);
		if(dhcp->fd < 0) {
			_DhcpUdpError(dhcp, 1);
			return;
		}
		size = sizeof(UDP_DHCP_PACKET);
		sllEncodePacket(sa, DHCP_CLIENT_PORT, dhcp->toAddr, DHCP_SERVER_PORT, pkt, size);
printf("DhcpUdpWriteRaw...src: %s\n", inet2addr(sa));
printf("DhcpUdpWriteRaw...dst: %s %s\n", mac2addr(ma), inet2addr(dhcp->toAddr));
		rval = _DhcpUdpWriteRaw(dhcp, ma, pkt, size);
	} else {
		if(dhcp->fd < 0) _DhcpUdpOpen(dhcp);
		if(dhcp->fd < 0) {
			_DhcpUdpError(dhcp, 1);
			return;
		}
		size = sizeof(RIP_MSG);
		rval = _DhcpUdpWrite(dhcp, dhcp->toAddr, &pkt->msg, size);
	}
	if(rval < 0) {
printf("DhcpUdpSendMsg: uv_udp_send error: %s\n", uv_err_name(rval));
		_DhcpUdpError(dhcp, rval);
	} else {
		dhcp->tryCount++;
		_RetryMsgTimer(dhcp);
	}
}

void prt_ip(char *msg, unsigned char *ip)
{
	printf("%s: %d.%d.%d.%d\n", msg, (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);
}

void prt_ip2(char *msg, unsigned char *myip, unsigned char *otip)
{
	printf("%s: My:%d.%d.%d.%d Another:%d.%d.%d.%d\n", msg, (int)myip[0], (int)myip[1], (int)myip[2], (int)myip[3], (int)otip[0], (int)otip[1], (int)otip[2], (int)otip[3]);
}

void prt_mac2(char *msg, unsigned char *mymac, unsigned char *otmac)
{
	printf("%s: My:%02x.%02x.%02x.%02x.%02x.%02x Another:%02x.%02x.%02x.%02x.%02x.%02x\n", msg, (int)mymac[0], (int)mymac[1], (int)mymac[2], (int)mymac[3], (int)mymac[4], (int)mymac[5], (int)otmac[0], (int)otmac[1], (int)otmac[2], (int)otmac[3], (int)otmac[4], (int)otmac[5]);
}

static int _DhcpCodeDISCOVER(DHCP *dhcp)
{
	UDP_DHCP_PACKET		*pkt;
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	pkt = (UDP_DHCP_PACKET *)txBuf;
	dhcp->xid++;
	pRIPMSG = &pkt->msg;
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
	ipSetBroadcast(dhcp->toAddr);
	ipSetNull(dhcp->fromAddr);
	return sizeof(RIP_MSG);
}

// This function sends DHCP REQUEST message to DHCP server.
// state	0:SELECTING,REQUESTING 1:INIT_REBOOT,REBOOTING 2:BOUND,RENEWING 3:REBINDING
static int _DhcpCodeREQUEST(DHCP *dhcp, int state)
{
	UDP_DHCP_PACKET		*pkt;
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	pkt = (UDP_DHCP_PACKET *)txBuf;
	dhcp->xid++;
	pRIPMSG = &pkt->msg;
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
	// MAGIC_COOKIE
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
	if(state == S_DHCP_RENEWING) memcpy(dhcp->toAddr, dhcp->svrAddr, 4); 
	else	ipSetBroadcast(dhcp->toAddr);
	if(state == S_DHCP_REBOOTING || state == S_DHCP_REBINDING) ipSetNull(dhcp->fromAddr);
#ifdef DHCP_DEBUG
//printf("%lu Tx:%s DHCP_REQUEST: state=%d\n", MS_TIMER, inet2addr(dhcp->toAddr), state);
#endif
	return sizeof(RIP_MSG);
}

static int _DhcpCodeRELEASE_DECLINE(DHCP *dhcp, int msgtype)	/* 0:RELEASE  NonZero:DECLINE */
{
	UDP_DHCP_PACKET		*pkt;
	RIP_MSG		*pRIPMSG;
	int		i, rval;

	pkt = (UDP_DHCP_PACKET *)txBuf;
	dhcp->xid++;
	pRIPMSG = &pkt->msg;
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
		ipSetBroadcast(dhcp->toAddr);
	} else{
		dhcp->msgType = DHCP_RELEASE;
		memcpy(dhcp->toAddr, dhcp->svrAddr, 4);
//ipSetBroadcast(dhcp->toAddr);
	}
#ifdef DHCP_DEBUG
if(msgtype) printf("%lu Tx:%s DHCP_DECLINE\n", MS_TIMER, inet2addr(dhcp->toAddr));
else		printf("%lu Tx:%s DHCP_RELEASE\n", MS_TIMER, inet2addr(dhcp->toAddr));
#endif
	return sizeof(RIP_MSG);
}

// This function parses the reply message from DHCP server.
static void _DhcpSendRELEASE_DECLINE(DHCP *dhcp, int msgtype)	/* 0:RELEASE  NonZero:DECLINE */
{
	int		rval;

	_DhcpCodeRELEASE_DECLINE(dhcp, msgtype);
	rval = _DhcpUdpWrite(dhcp, dhcp->toAddr, (unsigned char *)txBuf, sizeof(RIP_MSG));
	if(rval < 0) {
		if(msgtype) printf("%lu sendDHCP_DECLINE: uv_udp_send error: %s\n", MS_TIMER, uv_err_name(rval));
		else	printf("%lu sendDHCP_RELEASE: uv_udp_send error: %s\n", MS_TIMER, uv_err_name(rval));
		_DhcpUdpError(dhcp, rval);
	}
}

// This function parses the reply message from DHCP server.
// @return	success - return type, fail - 0
static int _DhcpDecodeMsg(DHCP *dhcp, void *buf, unsigned char *fromAddr, unsigned char *fromMacAddr)
{
	RIP_MSG		*pRIPMSG;
	unsigned char	*p, *e;
	unsigned short	svr_port;
	unsigned long	leased_time;
	int		c, type, opt_len, exitflg;

	if(ipIsNull(dhcp->fromAddr)) {
		memcpy(dhcp->fromAddr, fromAddr, 4);
		memcpy(dhcp->svrMacAddr, fromMacAddr, 6);
#ifdef RAW_SOCKET
	} else if(memcmp(dhcp->svrMacAddr, fromMacAddr, 6)) {
prt_mac2("Another DHCP sever send a response message. This is ignored.", dhcp->svrMacAddr, fromMacAddr);
		return 0;
#endif
	} else if(memcmp(dhcp->fromAddr, fromAddr, 4)) {
prt_ip2("Another DHCP sever send a response message. This is ignored.", dhcp->fromAddr, fromAddr);
		return 0;
	}
	pRIPMSG = (RIP_MSG *)buf;
	if(pRIPMSG->op != DHCP_BOOTREPLY) {
#ifdef DHCP_DEBUG
printf("Not DHCP Message\n");
#endif
		return 0;
	}
	if(memcmp(pRIPMSG->chaddr, dhcp->macAddr, 6) || dhcp->xid != ntohl(pRIPMSG->xid)) {
//#ifdef DHCP_DEBUG
printf("No My DHCP Message. This message is ignored. xid=%x,%x\n", ntohl(pRIPMSG->xid), dhcp->xid);
//#endif
		return 0;
	}
#ifdef DHCP_DEBUG
prt_ip("IPAddr(yiaddr)", pRIPMSG->yiaddr);	// delay copy to dhcp->ipAddr until endOption
#endif
	type = 0;
	p = (unsigned char *)&pRIPMSG->op;
	p += 240;
	e = p + (sizeof(RIP_MSG) - 240);
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
printf("dhcp leasedTime: 0x%08x(%d hours)\n", leased_time, leased_time/3600);
#endif
			if(leased_time == 0xffffffff) dhcp->leasedTime = 3660;	// one hour
			else 	dhcp->leasedTime = leased_time;
			dhcp->leasedTimer = MS_TIMER / 1000;
			break;
		case dhcpServerIdentifier:
			opt_len = *p++;
//			if(*((unsigned long*)DHCP_SIP) == 0 || !memcmp(DHCP_REAL_SIP, svr_addr, 4) || !memcmp(DHCP_SIP, svr_addr, 4)) {
			if(ipIsNull(dhcp->svrAddr) || !memcmp(p, dhcp->svrAddr, 4)) {
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

static void _ToState(DHCP *dhcp, int state)
{
	dhcp->state = state;
	dhcp->tryCount = 0;
}

static void _ToInit(DHCP *dhcp)
{
	int		i;

	if(dhcp->state >= S_DHCP_BOUND) {
		_DhcpSendRELEASE_DECLINE(dhcp, 0);
		if(dhcp->onUnbound) (*dhcp->onUnbound)(dhcp->cbContext);
	}	
	ipSetNull(dhcp->ipAddr);
	ipSetNull(dhcp->subnet);
	ipSetNull(dhcp->gwAddr);
	dhcp->dnsServerCnt = 0;
	_ToState(dhcp, S_DHCP_INIT);
	_InitMsgTimer(dhcp);
}

int		arp_fd;
unsigned long	arp_timer;

static void _ToBounding(DHCP *dhcp)
{
	unsigned char	ipAddr[4];
	int		rval;

	_ToState(dhcp, S_DHCP_BOUNDING);
	arp_fd = spackArpOpen();
	ipSetNull(ipAddr);
	rval = spackArpSend(arp_fd, niIfName(), dhcp->macAddr, ipAddr, dhcp->ipAddr);
	arp_timer = MS_TIMER;
	_DhcpTimerStart(100);
}

static void _ToBound(DHCP *dhcp)
{
	_ToState(dhcp, S_DHCP_BOUND);
	_DhcpUdpClose(dhcp);
	if(dhcp->onBound) (*dhcp->onBound)(dhcp->cbContext);
	_DhcpTimerStart(60000);	// one minute
}

static void _ToBoundIdle(DHCP *dhcp)
{
	dhcp->timer = MS_TIMER / 1000;
	dhcp->tryCount = 0;
	_DhcpUdpClose(dhcp);
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
		printf("_CheckLeased: ARP timeout\n");
		rval = 1;
	} else {
		_DhcpTimerStart(100);
		rval = 0;
	}
	if(rval) spackArpClose(arp_fd);
	return rval;
}

static void _OnDhcpTimer(uv_timer_t *handle)
{
	unsigned char	ipAddr[4];
	unsigned long	timer;
	int		size, rval, state, duration;

	size = 0;
	state = dhcp->state;
	switch(state) {
	case S_DHCP_INIT:
		_ToState(dhcp, S_DHCP_SELECTING);
	 	size = _DhcpCodeDISCOVER(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_DISCOVER(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		break;
	case S_DHCP_INIT_REBOOT:
		_ToState(dhcp, S_DHCP_REBOOTING);
	 	size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		break;
	case S_DHCP_SELECTING:
		if(dhcp->tryCount < 3) {
			size = _DhcpCodeDISCOVER(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_DISCOVER(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, dhcp->tryCount);
#endif
		} else {
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, retryout => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_REQUESTING:
	case S_DHCP_REBOOTING:
		if(dhcp->tryCount < 3) {
			size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, dhcp->tryCount);
#endif
		} else {
			// INIT_REBOOT 상태에서 subnet이 다른 경우(예:192.168.2.x망에서 192.168.1.x주소가 있을때)
			// DHCP가 무응답하므로 INIT_REBOOT 상태 유지에서 INIT 상태 변경으로 수정
			_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, retryout => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
		}
		break;
	case S_DHCP_BOUNDING:
		rval = _CheckLeased(dhcp);
		if(rval > 0) {
			_ToBound(dhcp);
//#ifdef DHCP_DEBUG
printf("%lu %s: check leased(OK) => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
//#endif
		} else if(rval < 0) {
			_DhcpCodeRELEASE_DECLINE(dhcp, 1);	// inserted 2021.1.7
			_DhcpUdpSendMsg(dhcp);
			_ToInit(dhcp);
//#ifdef DHCP_DEBUG
printf("%lu %s: check leased(conflict), DHCP_DECLINE => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
//#endif
		}
		break;
	case S_DHCP_BOUND:
		timer = MS_TIMER / 1000; duration = dhcp->leasedTime >> 1;
		if((timer-dhcp->leasedTimer) >= duration) {
			_ToState(dhcp, S_DHCP_RENEWING);
			size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout:T1, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
		} else {
#ifdef DHCP_DEBUG
printf("%lu %s: remain %d to T1\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			_DhcpTimerStart(60000);	// one minute
		}
		break;
	case S_DHCP_RENEWING:
		if(dhcp->tryCount) {
			if(dhcp->tryCount < 3) {
				size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x): retry=%d\n", MS_TIMER, dhcps(state), dhcp->xid, (int)dhcp->tryCount);
#endif
			} else {
				_ToBoundIdle(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, retryout\n", MS_TIMER, dhcps(state));
#endif
			}
		} else {
			timer = MS_TIMER / 1000; duration = (dhcp->leasedTime * 7) >> 3;
			if((timer-dhcp->leasedTimer) >= duration) {
				_ToState(dhcp, S_DHCP_REBINDING);
				size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout:T2, DHCP_REQUEST(%x) => %s\n", MS_TIMER, dhcps(state), dhcp->xid, dhcps(dhcp->state));
#endif
			} else if((timer-dhcp->timer) >= (duration-(dhcp->timer-dhcp->leasedTimer))/2) {
				size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x)\n", MS_TIMER, dhcps(state), dhcp->xid);
#endif
			} else {
				_ToBoundIdle(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: remain %d to T2\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			}
		}
		break;
	case S_DHCP_REBINDING:
		if(dhcp->tryCount) {
			if(dhcp->tryCount < 3) {
				size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x): rety=%d\n", MS_TIMER, dhcps(state), dhcp->xid, (int)dhcp->tryCount);
#endif
			} else {
				_ToBoundIdle(dhcp);	
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, retryout\n", MS_TIMER, dhcps(state));
#endif
			}
		} else {
			timer = MS_TIMER / 1000; duration = dhcp->leasedTime;
			if((timer-dhcp->leasedTimer) >= duration) {
				_ToInit(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: leased time expired => %s\n", MS_TIMER, dhcps(state), dhcps(dhcp->state));
#endif
			} else if((timer-dhcp->timer) >= (duration-(dhcp->timer-dhcp->leasedTimer))/2) {
				size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
#ifdef DHCP_DEBUG
printf("%lu %s: timeout, DHCP_REQUEST(%x)", MS_TIMER, dhcps(state), dhcp->xid);
#endif
			} else {
				_ToBoundIdle(dhcp);
#ifdef DHCP_DEBUG
printf("%lu %s: remain %d to expired\n", MS_TIMER, dhcps(state), duration - (timer-dhcp->leasedTimer));
#endif
			}
		}
		break;
	}
	if(size > 0) _DhcpUdpSendMsg(dhcp);
}

static void _DhcpReceivedMsg(DHCP *dhcp, void *buf, unsigned char *fromAddr, unsigned char *fromMacAddr)
{
	int		rval, size, state, type;

	type = _DhcpDecodeMsg(dhcp, buf, fromAddr, fromMacAddr);
	state = dhcp->state;
	size = 0;
	switch(state) {
	case S_DHCP_SELECTING:
		if(type == DHCP_OFFER) {
			_ToState(dhcp, S_DHCP_REQUESTING);
			size = _DhcpCodeREQUEST(dhcp, (int)dhcp->state);
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
			_ToBounding(dhcp);
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
	if(size > 0) _DhcpUdpSendMsg(dhcp);
}

