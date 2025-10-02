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
#include <netinet/udp.h>
#include <netinet/ip.h>
#include "prim.h"
#include "sll.h"

int			sll_errno;


int sllInitClient(void)
{
	int		s;

	s = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	if(s < 0) {
		_sllError(SLL_SYSERR, "sllInitClient() error");
	}
	return s;
}

void sllSetAddr(struct sockaddr *saddr, int ifIndex, unsigned char *macAddr)
{
	struct sockaddr_ll *addr;

	addr = (struct sockaddr_ll *)saddr;
	memset(addr, 0, sizeof(struct sockaddr_ll));
	addr->sll_family	= AF_PACKET;
	addr->sll_protocol	= htons(ETH_P_IP);
	addr->sll_ifindex	= ifIndex;
	if(macAddr) {
		memcpy(addr->sll_addr, macAddr, 6);
		addr->sll_halen	= 6;
	}
}

void sllGetAddr(struct sockaddr *saddr, int *ifIndex, unsigned char *macAddr)
{
	struct sockaddr_ll *addr;

	addr = (struct sockaddr_ll *)saddr;
	if(ifIndex) *ifIndex = addr->sll_ifindex;
	if(macAddr) memcpy(macAddr, addr->sll_addr, 6);
}

static int checksum(void *addr, int count)
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

void sllEncodePacket(unsigned char *srcAddr, unsigned short srcPort, unsigned char *dstAddr, unsigned short dstPort, void *buf, int length)
{
	struct iphdr	*ip;
	struct udphdr	*udp;
	unsigned char	*p;
	unsigned short	slen;
	int		size, payload;

	slen = length;
	p = (unsigned char *)buf;
	ip = (struct iphdr	*)p; p += sizeof(struct iphdr);
	udp = (struct udphdr *)p;
	memset(ip, 0, sizeof(struct iphdr));
	memset(udp, 0, sizeof(struct udphdr));
	ip->protocol	= IPPROTO_UDP;
	memcpy(&ip->saddr, srcAddr, 4);
	memcpy(&ip->daddr, dstAddr, 4);
	udp->source		= htons(srcPort);
	udp->dest		= htons(dstPort);
	udp->len		= htons(slen - sizeof(struct iphdr));	// cheat on the psuedo-header
	ip->tot_len		= udp->len;
	udp->check		= checksum(buf, length);
	ip->tot_len		= htons(slen);
	ip->ihl			= sizeof(struct iphdr) >> 2;
	ip->version		= IPVERSION;
	ip->ttl			= IPDEFTTL;
	ip->check		= checksum(ip, sizeof(struct iphdr));
}

int sllDecodePacket(unsigned char *srcAddr, unsigned short *srcPort, unsigned char *dstAddr, unsigned short *dstPort, void *buf, int length)
{
	struct iphdr	*ip;
	struct udphdr	*udp;
	unsigned char	*p;
	unsigned long	src, dst;
	unsigned short	check;
	int		len;

	p = (unsigned char *)buf;
	ip = (struct iphdr	*)p; p += sizeof(struct iphdr);
	udp = (struct udphdr *)p;
	if(length < (int) (sizeof(struct iphdr) + sizeof(struct udphdr))) {
		_sllError(SLL_ERROR, "sllDecodePacket(): message too short");
		return -1;
	}
	len = ntohs(ip->tot_len);
	if(length < len) {
		_sllError(SLL_ERROR, "sllDecodePacket(): truncated packet");
		return -1;
	}
	// ignore any extra garbage bytes
	// Make sure its the right packet for us, and that it passes sanity checks */
	if(ip->protocol != IPPROTO_UDP || ip->version != IPVERSION || ip->ihl != sizeof(struct iphdr) >> 2
			|| ntohs(udp->len) != (short)(len - sizeof(struct iphdr))) {
		_sllError(SLL_ERROR, "sllDecodePacket(): unrelated/bogus packet");
		return -1;
	}
	// check IP checksum
	check = ip->check;
	ip->check = 0;
	if(check != checksum(ip, sizeof(struct iphdr))) {
		_sllError(SLL_ERROR, "sllDecodePacket(): bad IP header checksum");
		return -1;
	}
	// verify the UDP checksum by replacing the header with a psuedo header
	src = ip->saddr;
	dst = ip->daddr;
	check = udp->check;
	udp->check = 0;
	memset(ip, 0, sizeof(struct iphdr));
	ip->protocol = IPPROTO_UDP;
	ip->saddr = src;
	ip->daddr = dst;
	ip->tot_len = udp->len; // cheat on the psuedo-header
	if(check && check != checksum(buf, len)) {
		_sllError(SLL_ERROR, "sllDecodePacket(): bad UDP checksum");
		return -1;
	}
	memcpy(srcAddr, &ip->saddr, 4);
	memcpy(dstAddr, &ip->daddr, 4);
	*srcPort = ntohs(udp->source);
	*dstPort = ntohs(udp->dest);
	return 0;
}

static char	_err_msg[256];

void sllError(char *s)
{
	if(s && s[0]) {
		if(_err_msg[0]) printf("%s: %s\n", s, _err_msg);
		else	printf("%s\n", s);
	} else if(_err_msg[0]) printf("%s\n", _err_msg);
}

char *sllStrError()
{
	return _err_msg;
}

#include	<stdarg.h>

void _sllError(int errnum, ...)
{
	va_list	args;
	char	*p, *fmt;
	int		status, len;

	va_start(args, errnum);
	fmt = va_arg(args, char *);
	vsprintf(_err_msg, fmt, args);
	va_end(args);
	p = _err_msg + strlen(_err_msg);
	sll_errno = errnum;
	if(errnum == SLL_SYSERR) {
		sprintf(p, " %d: ", errno); p += strlen(p);
		strcpy(p, strerror(errno));
    }
}

