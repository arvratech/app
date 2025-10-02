#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include "dhcp_priv.h"


unsigned long miliTimer(void)
{
	struct timespec	tspec;
	unsigned long	val;

	clock_gettime(CLOCK_MONOTONIC, &tspec);
	val = tspec.tv_sec * 1000;
	val += tspec.tv_nsec / 1000000;
	return val;
}

#define MS_TIMER	miliTimer()

int ipAddrIsNull(unsigned char *ipAddr)
{
	if(!ipAddr[0] && !ipAddr[1] && !ipAddr[2] && !ipAddr[3]) return 1;
	else	return 0;
} 

void ipAddrSetNull(unsigned char *ipAddr)
{
	ipAddr[0] = ipAddr[1] = ipAddr[2] = ipAddr[3] = 0;
}

int checkIpAddr(unsigned char *ip, int subnetmask)
{
	if(!ip[0]) return -1;
	else	return 0;
}

void BYTEtoLONG(unsigned char *pbVal, unsigned long *plVal)
{
	long lVal1, lVal2, lVal3, lVal4;
	lVal1 = (long)(*pbVal) << 24;
	lVal2 = (long)(*(pbVal+1)) << 16;
	lVal3 = (long)(*(pbVal+2)) << 8;
	lVal4 = (long)(*(pbVal+3));
	*plVal = lVal1+lVal2+lVal3+lVal4;
}

void LONGtoBYTE(unsigned long lVal, unsigned char *pbVal)
{
	*pbVal = (unsigned char)(lVal >> 24);
	*(pbVal+1) = (unsigned char)(lVal >> 16);
	*(pbVal+2) = (unsigned char)(lVal >> 8);
	*(pbVal+3) = (unsigned char)lVal;
}

void prt_ip(char *msg, unsigned char *ip)
{
	printf("%s: %d.%d.%d.%d\n", msg, (int)ip[0], (int)ip[1], (int)ip[2], (int)ip[3]);
}

void prt_ip2(char *msg, unsigned char *myip, unsigned char *otip)
{
	printf("%s: My:%d.%d.%d.%d Another:%d.%d.%d.%d\n", msg, (int)myip[0], (int)myip[1], (int)myip[2], (int)myip[3], (int)otip[0], (int)otip[1], (int)otip[2], (int)otip[3]);
}

#define DHCP_DEBUG		1

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

static char	*_hostName = "Arvratech";
static DHCP				*dhcp, _dhcp;
UDP_DHCP_PACKET			_txPkt, _rxPkt;

int  code_DHCP_DISCOVER(DHCP *dhcp, UDP_DHCP_PACKET *pkt);
int  _ParseDHCPMSG(DHCP *dhcp, unsigned char *buf, int length, unsigned char *ipAddr, unsigned short ipPort);
unsigned long miliTimer(void);


int main(int argc, char **argv)
{
//	struct sockaddr_in	addr, raddr;
	struct sockaddr_ll	laddr, raddr, rxaddr;
	struct ifreq	ifr;
	UDP_DHCP_PACKET		*pkt;
	char	ifname[8];
	char mesg[] = "Hello World!";
	unsigned long	tm;
	unsigned char	*p, ipAddr[4];
	unsigned short	ipPort;
	int		s, s2, rval, val, size, addrlen;

	dhcp = &_dhcp;
	dhcp->state = S_DHCP_NULL;
	memset(dhcp, 0, sizeof(DHCP));
	dhcp->xid = rand();
	dhcp->fd = -1;
	dhcp->state = S_DHCP_INIT;
//	_InitMsgTimer(dhcp);

	strcpy(ifname, "eth0");
	s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(s < 0) {
		perror("socket error");
		exit(-1);
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, ifname);
	rval = ioctl(s, SIOCGIFINDEX, &ifr);
	if(rval < 0) {
		perror("ioctl(SIOCGIFHWADDR) error");
		close(s);
		exit(-1);
	}
	dhcp->ifIndex = ifr.ifr_ifindex;
printf("ifIndex=%d\n", dhcp->ifIndex);
	rval = ioctl(s, SIOCGIFHWADDR, &ifr);
	if(rval < 0) {
		perror("ioctl(SIOCGIFHWADDR) error");
		close(s);
		exit(-1);
	}
	memcpy(dhcp->macAddr, ifr.ifr_hwaddr.sa_data, 6);
	close(s);
//printf("MacAddress=%s\n", mac2addr(dhcp->macAddr));
	s2 = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	val = 1;
	rval = ioctl(s2, FIONBIO, &val);
	addrlen = sizeof(rxaddr);
	memset(&rxaddr, 0, addrlen);
	rxaddr.sll_family	= AF_PACKET;
	rxaddr.sll_protocol	= htons(ETH_P_IP); 
	rxaddr.sll_ifindex	= dhcp->ifIndex;
	rval = bind(s2, (const struct sockaddr *)&rxaddr, addrlen);
	if(rval < 0) {
		perror("bind() error");
		close(s2);
		exit(-1);
	}

	s = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(s < 0) {
		perror("socket error");
		exit(-1);
	}
	addrlen = sizeof(laddr);
	memset(&laddr, 0, addrlen);
	laddr.sll_family	= AF_PACKET;
	laddr.sll_protocol	= htons(ETH_P_IP); 
	laddr.sll_ifindex	= dhcp->ifIndex;
	//memcpy(laddr.sll_addr, dhcp->macAddr, 6);	// MAC_BCAST_ADDR
	memcpy(laddr.sll_addr, MAC_BCAST_ADDR, 6);	// MAC_BCAST_ADDR
	laddr.sll_halen		= 6;
	rval = bind(s, (const struct sockaddr *)&laddr, addrlen);
	if(rval < 0) {
		perror("bind() error");
		close(s);
		exit(-1);
	}
printf("DHCP ready...\n");
	while(1) {
	 	size = code_DHCP_DISCOVER(dhcp, &_txPkt);
		rval = sendto(s, &_txPkt, sizeof(UDP_DHCP_PACKET), 0, (struct sockaddr *)&laddr, sizeof(laddr));
		if(rval < 0) {
			perror("sendto() error");
			close(s);
			exit(-1);
		}
printf("%lu boadcasted DHCP_DISCOVER...%d %d\n", miliTimer(), sizeof(UDP_DHCP_PACKET), rval);
		addrlen = sizeof(raddr);
		tm = miliTimer();
		pkt = &_rxPkt;
		while(1) {
			rval = read(s2, pkt, sizeof(UDP_DHCP_PACKET));
			//rval = recvfrom(s, pkt, sizeof(UDP_DHCP_PACKET), 0, (struct sockaddr *)&raddr, &addrlen);
			if(rval > 0) {
//printf("%lu Received %d\n", miliTimer(), rval);
				if(rval < sizeof(struct iphdr) + sizeof(struct udphdr)) {
					printf("message too short, ignoring\n");
					rval = 0;
				} else if(rval < sizeof(UDP_DHCP_PACKET)) { 
//					printf("not dhcp message , ignoring\n");
					rval = 0;
				} else if(rval < ntohs(pkt->ip.tot_len)) {
					printf("truncated packet: %d %d\n", rval, ntohs(pkt->ip.tot_len));
					rval = 0;
				} else {
					size = ntohs(pkt->ip.tot_len);
					if(pkt->ip.protocol != IPPROTO_UDP || pkt->ip.version != IPVERSION
							|| pkt->ip.ihl != sizeof(pkt->ip) >> 2 || pkt->udp.dest != htons(DHCP_CLIENT_PORT)
							|| size > (int) sizeof(UDP_DHCP_PACKET)
							|| ntohs(pkt->udp.len) != (short)(size - sizeof(pkt->ip))) {
						printf("%lu unrelated/bogus packet: %d\n", miliTimer(), rval);
						rval = 0;
					}
				}
			} else if(errno == EAGAIN && errno == EWOULDBLOCK) rval = 0;
			if(rval > 0) break;
			if(miliTimer() - tm > 3000) break;
		}
printf("%lu Received DHCP OFFER %d\n", miliTimer(), rval);
if(rval > 0) {
		memcpy(ipAddr, &_rxPkt.ip.saddr, 4);
		ipPort = ntohs(_rxPkt.udp.source);
	_ParseDHCPMSG(dhcp, (unsigned char *)&_rxPkt.msg, rval-sizeof(struct iphdr)-sizeof(struct udphdr),
						ipAddr, ipPort);
}
		sleep(3);
	}
	return 0;
}

int udp_checksum(void *addr, int count)
{
	unsigned short		*p, tmp;
	int		sum;

	p = (unsigned short *)addr;
	//Compute Internet Checksum for "count" bytes
	sum = 0;
	while(count > 1)  {
		// This is the inner loop
		sum += *p++;
		count -= 2;
	}
	//  Add left-over byte, if any
	if (count > 0) {
		// Make sure that the left-over byte is added correctly both
		// with little and big endian hosts
		tmp = 0;
		*(unsigned char *)(&tmp) = *(unsigned char *)p;
		sum += tmp;
	}
	// Fold 32-bit sum to 16 bits
	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	sum = ~sum;
	sum &= 0xffff;
	return sum;
}

int code_DHCP_DISCOVER(DHCP *dhcp, UDP_DHCP_PACKET *pkt)
{
	RIP_MSG		*pRIPMSG;
unsigned char	*p;
	int		i, rval;

	memset(pkt, 0, sizeof(UDP_DHCP_PACKET));
	pkt->ip.protocol	= IPPROTO_UDP;
	pkt->ip.saddr		= INADDR_ANY;
	pkt->ip.daddr		= INADDR_BROADCAST;
	pkt->udp.source		= htons(DHCP_CLIENT_PORT);
	pkt->udp.dest		= htons(DHCP_SERVER_PORT);
	pkt->udp.len		= htons(sizeof(pkt->udp) + sizeof(RIP_MSG));	// cheat on the psuedo-header
	pkt->ip.tot_len		= pkt->udp.len;
//	memcpy(&(pkt->data), payload, sizeof(RIP_MSG));
	pkt->udp.check		= udp_checksum(pkt, sizeof(UDP_DHCP_PACKET));
	pkt->ip.tot_len		= htons(sizeof(UDP_DHCP_PACKET));
	pkt->ip.ihl			= sizeof(pkt->ip) >> 2;
	pkt->ip.version		= IPVERSION;
	pkt->ip.ttl			= IPDEFTTL;
	pkt->ip.check		= udp_checksum(&pkt->ip, sizeof(pkt->ip));
//for(i = 0;i < 28;i += 4) {
//	printf("%02x-%02x-%02x-%02x\n", (int)p[i+0], (int)p[i+1], (int)p[i+2], (int)p[i+3]);
//}
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
	dhcp->toAddr[0] = dhcp->toAddr[1] = dhcp->toAddr[2] = dhcp->toAddr[3] = 255;
#ifdef DHCP_DEBUG
printf("Tx:DHCP_DISCOVER\n");
#endif
	return sizeof(RIP_MSG);
}

int _ParseDHCPMSG(DHCP *dhcp, unsigned char *buf, int length, unsigned char *ipAddr, unsigned short ipPort)
{
	RIP_MSG		*pRIPMSG;
	unsigned char	*p, *e;
	unsigned short	svr_port;
	unsigned long	leased_time;
	int		c, type, opt_len, exitflg;

	pRIPMSG = (RIP_MSG *)buf;
	if(length <= 0) return 0;
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
printf("No My DHCP Message. This message is ignored. xid=%lx,%lx\n", ntohl(pRIPMSG->xid), dhcp->xid);
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

