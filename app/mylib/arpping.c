#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/if_ether.h>
#include <net/if_arp.h>
#include <netinet/in.h>
#include "arpping.h"

#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"

typedef struct _ARP_MSG {
	struct ethhdr ethhdr;		// Ethernet header
	unsigned short htype;		// hardware type (must be ARPHRD_ETHER)
	unsigned short ptype;		// protocol type (must be ETH_P_IP)
	unsigned char  hlen;		// hardware address length (must be 6)
	unsigned char  plen;		// protocol address length (must be 4)
	unsigned short operation;	// ARP opcode
	unsigned char  sHaddr[6];	// sender's hardware address
	unsigned char  sInaddr[4];	// sender's IP address
	unsigned char  tHaddr[6];	// target's hardware address
	unsigned char  tInaddr[4];	// target's IP address
	unsigned char  pad[18];		// pad for min. Ethernet payload (60 bytes)
} ARP_MSG;


int spackArpOpen(void)
{
	int		fd, rval, optval;


	fd = socket(PF_PACKET, SOCK_PACKET, htons(ETH_P_ARP));
	if(fd < 0) {
		printf("spackArpOpen: socket() error: %s\n", strerror(errno));
		return -1;
	}
	optval = 1;
	rval = setsockopt(fd, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
	if(rval < 0) {
		printf("spackArpOpen: setsockopt(SO_BROADCAST) error: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	return fd;
}

void spackArpClose(int fd)
{
	close(fd);
}

int spackArpSend(int fd, char *ifName, unsigned char *sMacAddr, unsigned char *sIpAddr, unsigned char *tIpAddr)
{
	ARP_MSG		*arp, _arp;
	struct sockaddr		_saddr;
	int		rval, addrlen;

	arp = &_arp;
	memset(arp, 0, sizeof(ARP_MSG));
	memcpy(arp->ethhdr.h_dest, MAC_BCAST_ADDR, 6);	// MAC DA 
	memcpy(arp->ethhdr.h_source, sMacAddr, 6);		// MAC SA
	arp->ethhdr.h_proto = htons(ETH_P_ARP);			// protocol type
	arp->htype = htons(ARPHRD_ETHER);				// Hardware type
	arp->ptype = htons(ETH_P_IP);					// Protocol type: ARP message
	arp->hlen = 6;									// Hardware address length
	arp->plen = 4;									// Protocol address length
	arp->operation = htons(ARPOP_REQUEST);			// Operation
	memcpy(arp->sHaddr, sMacAddr, 6);				// Sender hardware address
	memcpy(arp->sInaddr, sIpAddr, 4);				// Sender protocol address
	memset(arp->tHaddr, 0, 6);						// Target hardware address
	memcpy(arp->tInaddr, tIpAddr, 4);				// Target protocol address
printf("Tx Source: %s %s\n", mac2addr(arp->sHaddr), inet2addr(arp->sInaddr));
printf("Tx Target: %s %s\n", mac2addr(arp->tHaddr), inet2addr(arp->tInaddr));
	addrlen = sizeof(struct sockaddr);
	memset(&_saddr, 0, addrlen);
	strcpy(_saddr.sa_data, ifName);
	rval = sendto(fd, arp, sizeof(ARP_MSG), 0, &_saddr, addrlen);
	if(rval < 0) {
		printf("spackArpSend: sendto() error: %s\n", strerror(errno));
		return -1;
	}
	// if O_NONBLOCK is set, sendmsg(fd) return -1, ENOBUFS. sendto() return success.
	rval = fcntl(fd, F_GETFL, 0);
    rval |= O_NONBLOCK;
    fcntl(fd, F_SETFL, rval);
	return 0;
}

/*
 *              Source					         Target
 *   192.168.2.84(?)
 * Tx  00:20:17:04:17:2f 0.0.0.0		00:00:00:00:00:00 192.168.2.84
 * Rx  d8:80:39:96:c8:7e 192.168.2.84	00:20:17:04:17:2f 0.0.0.0
 *   192.168.2.85(My PC)
 * Tx  00:20:17:04:17:2f 0.0.0.0		00:00:00:00:00:00 192.168.2.85
 * Rx  70:85:c2:06:ad:53 192.168.2.85	00:20:17:04:17:2f 0.0.0.0
 *   192.168.2.67(Linux PC)
 * Tx  00:20:17:04:17:2f 0.0.0.0		00:00:00:00:00:00 192.168.2.67
 * Rx  bc:5f:f4:80:01:f7 192.168.2.67	00:20:17:04:17:2f 0.0.0.0
 *   192.168.2.3(?)
 * Tx  00:20:17:04:17:2f 0.0.0.0		00:00:00:00:00:00 192.168.2.3
 * Rx  00:50:c2:34:ae:84 192.168.2.3	00:20:17:04:17:2f 192.168.2.1	// not 0.0.0.0
 */
int spackArpRecv(int fd, char *ifName, unsigned char *sMacAddr, unsigned char *sIpAddr, unsigned char *tMacAddr, unsigned char *tIpAddr)
{
	ARP_MSG		*arp, _arp;
	struct sockaddr		_saddr;
	unsigned char	*p;
	int		rval, addrlen;

	arp = &_arp;
	while(1) {
		addrlen = sizeof(struct sockaddr);
		memset(&_saddr, 0, addrlen);
		strcpy(_saddr.sa_data, ifName);
		rval = recvfrom(fd, arp, sizeof(ARP_MSG), 0, &_saddr, &addrlen);
		if(rval < 0) {
			if(errno == EAGAIN || errno == EWOULDBLOCK) rval = 0;
			else {
				printf("spackArpSend: recvfrom() error: %s\n", strerror(errno));
			}
		}
		if(rval <= 0) break;
/*
printf("Rx=%d op=0x%04x", rval, (int)arp->operation);
printf(" from %s", mac2addr(arp->ethhdr.h_source));
printf(" to %s\n", mac2addr(arp->ethhdr.h_dest));
printf("Rx Source: %s %s\n", mac2addr(arp->sHaddr), inet2addr(arp->sInaddr));
printf("Rx Target: %s %s\n", mac2addr(arp->tHaddr), inet2addr(arp->tInaddr));
*/
		// because above case 4
		//if(arp->operation == htons(ARPOP_REPLY) && !memcmp(sMacAddr, arp->tHaddr, 6) && !memcmp(sIpAddr, arp->tInaddr, 4) && !memcmp(tIpAddr, arp->sInaddr, 4)) {
		if(arp->operation == htons(ARPOP_REPLY) && !memcmp(sMacAddr, arp->tHaddr, 6) && !memcmp(tIpAddr, arp->sInaddr, 4)) {
			memcpy(tMacAddr, arp->sHaddr, 4);
			rval = 1;
			break;
		}
	}
	return rval;
}

/*
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif

int _DhcpUdpRawOpen(DHCP *dhcp)
{
	struct sockaddr_ll	_addr;
	int		fd, rval, addrlen;

	fd = spackUdpOpen();
	if(fd < 0) {
		return -1;
	}
//	spackAddr((struct sockaddr *)_&addr, ifIndex, macAddr);
	rval = uv_udp_init(MainGetLoop(), udpDhcp);
	rval = uv_udp_open(udpDhcp, fd);
	spackAddr((struct sockaddr *)&_addr, dhcp->ifIndex, MAC_BCAST_ADDR);
	rval = bind(fd, (struct sockaddr *)&_addr, sizeof(struct sockaddr_ll));
	if(fd < 0) {
		printf("bind() error:  %s\n", strerror(errno));
		close(fd);
		return -1;
	}
//	udpDhcp->flags |= UV_HANDLE_BOUND;
	rval = uv_udp_recv_start(udpDhcp, _DhcpAllocCb, _DhcpRecvCb);
	dhcp->fd = fd;
	return 0;
}

int _DhcpUdpRawSend(DHCP *dhcp, unsigned char *data, int dataLen)
{
	struct sockaddr_ll	_addr;
	uv_buf_t	uvbuf;
	int		size, rval;

	spackAddr((struct sockaddr *)&_addr, dhcp->ifIndex, MAC_BCAST_ADDR);
	size = spackPayload((unsigned char *)txBuf, INADDR_ANY, (unsigned short)DHCP_CLIENT_PORT, INADDR_BROADCAST, (unsigned short)DHCP_SERVER_PORT, data, dataLen);
	uvbuf.base = (unsigned char *)txBuf; uvbuf.len = size;
	rval = uv_udp_send(&udpsend, udpDhcp, &uvbuf, 1, (const struct sockaddr *)&_addr, _DhcpSendCb);
	if(rval) {
		//SlvnetUdpClose();
	} else {
		//_Slvnettimer(rval);
	}
	return rval;
}
*/
