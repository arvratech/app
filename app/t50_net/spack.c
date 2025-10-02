#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <features.h>
#if __GLIBC__ >=2 && __GLIBC_MINOR >= 1
#include <netpacket/packet.h>
#include <net/ethernet.h>
...
#else
#include <asm/types.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#endif
#include <netinet/ip.h>
#include <netinet/udp.h>
#include "spack.h"


// Compute Internet Checksum
static unsigned short _Checksum(void *addr, int count)
{
	register int	sum;
	unsigned short	*p, tmp;

	sum = 0;
	p = (unsigned short *)addr;
	while(count > 1) {
		sum += *p++;
		count -= 2;
	}
	if(count > 0) {
		// Make sure that the left-over byte is added correctly both
		// with little and big endian hosts
		*(unsigned char *)(&tmp) = * (unsigned char *)p;
		sum += tmp;
	}
	//  Fold 32-bit sum to 16 bits
	while(sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}

int spackUdpOpen(void)
{
	int		fd;

	fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(fd < 0) {
		printf("spackUdpOpen: socket() error: %s\n", strerror(errno));
		return -1;
	}
	return fd;
}

void spackAddr(struct sockaddr *saddr, int ifIndex, unsigned char *macAddr)
{
	struct sockaddr_ll	*addr;
	unsigned long	*lp;

	addr = (struct sockaddr_ll *)saddr;
	memset(addr, 0, sizeof(*addr));
	addr->sll_family	= AF_PACKET;
	addr->sll_protocol	= htons(ETH_P_IP);
	addr->sll_ifindex	= ifIndex;
	addr->sll_halen		= 6;
	memcpy(addr->sll_addr, macAddr, 6);
}

int spackPayload(unsigned char *buf, unsigned long srcIpAddr, unsigned short srcIpPort, 
					unsigned long dstIpAddr, unsigned short dstIpPort, unsigned char *apdu, int apduLen)
{
	struct iphdr	*ip;
	struct udphdr	*udp;
	unsigned char	*p;
	int		size;

	p = buf;
	ip = (struct iphdr *)p; p += sizeof(struct iphdr);
	udp = (struct udphdr *)p; p += sizeof(struct udphdr);
	memcpy(p, apdu, apduLen);
	memset(ip, 0, sizeof(struct iphdr));
	ip->saddr		= htonl(srcIpAddr);
	ip->daddr		= htonl(dstIpAddr);
	ip->protocol	= IPPROTO_UDP;
	ip->tot_len		= htons(sizeof(struct udphdr) + apduLen);
	// iph do the same role of psudo UDP header because the other zero fields don't effect to checksum. 
	udp->source		= htons(srcIpPort);
	udp->dest		= htons(dstIpPort);
	udp->len		= ip->tot_len;
	size = udp->len + sizeof(struct iphdr);
	udp->check		= _Checksum(buf, size);
	ip->tot_len		= htons(size);
	ip->ihl			= sizeof(struct iphdr) >> 2;
	ip->version		= IPVERSION;
	ip->ttl			= IPDEFTTL;
	ip->check		= _Checksum(ip, sizeof(struct iphdr));
	return size;
}
		
int spackPayload2(unsigned char *buf, unsigned char *srcIpAddr, unsigned short srcIpPort, 
					unsigned char *dstIpAddr, unsigned short dstIpPort, unsigned char *apdu, int apduLen)
{ 
	unsigned long	srcAddr, dstAddr;

	srcAddr = (srcIpAddr[0] << 24) | (srcIpAddr[1] << 16) | (srcIpAddr[2] << 8) | srcIpAddr[3];
	dstAddr = (dstIpAddr[0] << 24) | (dstIpAddr[1] << 16) | (dstIpAddr[2] << 8) | dstIpAddr[3];
	return spackPayload(buf, srcAddr, srcIpPort, dstAddr, dstIpPort, apdu, apduLen); 
}

