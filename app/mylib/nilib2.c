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
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "sysprim.h"
#include "wpanet.h"
#include "wpa.h"
#include "dhcp.h"
#include "nilib2.h"

static char		_ifName[8];


char *niIfName(void)
{
	return _ifName;
}

void niSetIfName(char *ifName)
{
	strcpy(_ifName, ifName);
}

// Return	-1:error  0:link down  1:link up
int niLinkState(void)
{
	struct ifreq	ifr;
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, _ifName);
	rval = ioctl(fd, SIOCGIFFLAGS, &ifr);
	close(fd);
	if(rval < 0) rval = -1;
	else {
		// if unpluged LAN cable, IFF_RUNNING is zero ocassionally.
		//if((ifr.ifr_flags & IFF_UP) && (ifr.ifr_flags & IFF_RUNNING)) rval = 1;
		if(ifr.ifr_flags & IFF_UP) rval = 1;
		else	rval = 0;
//printf("link=[");
//if(ifr.ifr_flags & IFF_UP) printf("IFF_UP ");
//if(ifr.ifr_flags & IFF_RUNNING) printf("IFF_RUNNING");
//printf("]\n");
	}
	return rval;
}

static void _NiUnbound(void)
{
	char	temp[512];

printf("NiUnbound...\n");
#ifdef _A20
	ExecPipeFinal(temp, "/sbin/ip", "ip", "-4", "addr", "flush", "dev", _ifName, NULL); 
	ExecPipeFinal(temp, "/sbin/ip", "ip", "-4", "route", "flush", "dev", _ifName, NULL); 
#else
	ExecPipeFinal(temp, "/sbin/ifconfig", "ifconfig", _ifName, "0.0.0.0", NULL); 
#endif
}

void niUp(void)
{
	char	temp[512];

printf("ifconfig %s up\n", _ifName);
	ExecPipeFinal(temp, "/sbin/ifconfig", "ifconfig", _ifName, "up", NULL); 
	_NiUnbound();
}

void niDown(void)
{
	char	temp[512];

printf("ifconfig %s down\n", _ifName);
	ExecPipeFinal(temp, "/sbin/ifconfig", "ifconfig", _ifName, "down", NULL); 
}

void niUnbound(void)
{
	_NiUnbound();
}

int niGetInterfaceIndex(int *ifIndex)
{
	struct ifreq	ifr;
	struct sockaddr_in *sin;
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, _ifName);
	rval = ioctl(fd, SIOCGIFINDEX, &ifr);
	if(rval < 0) {
		printf("IOCGIFINDEX error: %s\n", strerror(errno));
		close(fd);
		return -1;
	}
	*ifIndex = ifr.ifr_ifindex;	// adapter index;
	close(fd);
printf("ifIndex=%d\n", *ifIndex);
	return 0;
}

int niGetMacAddress(unsigned char *macAddr)
{
	struct ifreq	ifr;
	struct sockaddr_in *sin;
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, _ifName);
	rval = ioctl(fd, SIOCGIFHWADDR, &ifr);
	if(rval < 0) {
		printf("SIOCGIFHWADDR error: %s", strerror(errno));
		close(fd);
		return -1;
	}
	memcpy(macAddr, ifr.ifr_hwaddr.sa_data, 6);
	close(fd);
printf("MacAddress=%s\n", mac2addr(macAddr));
	return 0;
}

int niGetIpAddress(unsigned char *ipAddr, unsigned char *subnet, unsigned char *bcast)
{
	struct ifreq	ifr;
	struct sockaddr_in *addr;
	char	*p, temp[256], temp2[64];
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, _ifName);
	if(ipAddr) {
		rval = ioctl(fd, SIOCGIFADDR, &ifr);
		if(rval < 0) {
			printf("SIOCGIFADDR error: %s", strerror(errno));
			close(fd);
			return -1;
		}
		addr = (struct sockaddr_in *)&ifr.ifr_addr;
		memcpy(ipAddr, &addr->sin_addr.s_addr, 4);
		//*lp = sin->sin_addr.s_addr;
printf("ipAddr=%s\n", inet_ntoa(addr->sin_addr));
	}
	if(subnet) {
		rval = ioctl(fd, SIOCGIFNETMASK, &ifr);
		if(rval < 0) {
			printf("SIOCGIFNETMASK error: %s", strerror(errno));
			close(fd);
			return -1;
		}
		addr = (struct sockaddr_in *)&ifr.ifr_addr;	// == (struct sockaddr_in *)&ifr.ifr_netmask;
		memcpy(subnet, &addr->sin_addr.s_addr, 4);
printf("subnet=%s\n", inet_ntoa(addr->sin_addr));
	}
	if(bcast) {
		rval = ioctl(fd, SIOCGIFBRDADDR, &ifr);
		if(rval < 0) {
			printf("SIOCGIFBRDADDR error: %s", strerror(errno));
			close(fd);
			return -1;
		}
		addr = (struct sockaddr_in *)&ifr.ifr_addr;	// == (struct sockaddr_in *)&ifr.ifr_broadaddr;
		memcpy(bcast, &addr->sin_addr.s_addr, 4);
printf("bcast=%s\n", inet_ntoa(addr->sin_addr));
	}
	close(fd);
	return 0;
}

void niSetIpAddress(unsigned char *ipAddr, unsigned char *subnet)
{
	char	temp[1024], ip[16], sn[16];

	strcpy(ip, inet2addr(ipAddr));
	strcpy(sn, inet2addr(subnet));
	ExecPipeFinal(temp, "/sbin/ifconfig", "ifconfig", _ifName, ip, "netmask", sn, NULL); 
printf("SetIpAddress=%s %s\n", ip, sn);
}

void niGetDefaultGateway(unsigned char *gwAddr)
{
	char	*p, temp[1024], token[80];
	int		rval;

#ifdef _A20
	//default via 192.168.2.1 dev eth0  proto static  metric 1024
	ExecPipe("/sbin/ip", "ip", "-4", "route", "show", "dev", _ifName, NULL); 
	ExecPipeFinal(temp, "/bin/grep", "grep", "^default", NULL); 
	p = temp; rval = 0;
	if(p[0]) {
		p = read_token(p, token);
		if(p[0]) { 
			p = read_token(p, token);
			if(p[0]) {
				read_token(p, token);
				rval = 1;
			}
		}
	}
#else
	ExecPipe("/sbin/route", "route", NULL); 
	ExecPipeFinal(temp, "/bin/grep", "grep", "^default", NULL); 
	p = temp; rval = 0;
	if(p[0]) {
		p = read_token(p, token);
		if(p[0]) { 
			p = read_token(p, token);
			rval = 1;
		}
	}
#endif
	if(rval) addr2inet(token, gwAddr);
	else	 ipSetNull(gwAddr);
printf("GetDefaultGateway=%s\n", inet2addr(gwAddr));
}

void niSetDefaultGateway(unsigned char *gwAddr)
{
	char	temp[1024], gw[20];

#ifdef _A20
	ExecPipeFinal(temp, "/sbin/ip", "ip", "-4", "route", "flush", "exact", "0.0.0.0/0", "dev", _ifName, NULL); 
	ExecPipeFinal(temp, "/sbin/ip", "ip", "-4", "route", "add", "default", "via", inet2addr(gwAddr), "proto", "static", "metric", "1024", "dev", _ifName, NULL); 
#else
	strcpy(gw, inet2addr(gwAddr));
	ExecPipeFinal(temp, "/sbin/route", "route", "del", "default", "gw", "0.0.0.0", "dev", _ifName, NULL); 
	ExecPipeFinal(temp, "/sbin/route", "route", "add", "default", "gw", gw, "dev", _ifName, NULL); 
#endif
printf("SetDefaultGateway=%s\n", inet2addr(gwAddr));
}

static char *nameserver = "nameserver";

int niAddDnsServers(unsigned char dnsServers[][4], int count)
{
	FILE	*fp;
	unsigned char	mark[16];
	char	*p, addr[20], buf[256], key[256], val[256];
	int		i, rval, size;
	
	fp = fopen("/etc/resolv.conf", "a+");
	if(!fp) {
		printf("niAddDnsServers: fopen error: %s\n", strerror(errno)); 
		return -1;
	}
	for(i = 0;i < count;i++) mark[i] = 0;
	fseek(fp, SEEK_SET, 0L);
	while(fgets(buf, 256, fp)) {
		get_keyvalue_space(buf, key, val);
		if(strcmp(key, nameserver)) continue;
		for(i = 0;i < count;i++) {
			strcpy(addr, inet2addr(dnsServers[i]));	
			if(!strcmp(val, addr)) {
				mark[i] = 1;
				break;
			}
		}
	}
	for(i = 0;i < count;i++) {
		if(!mark[0]) {
			p = buf;
			strcpy(p, nameserver); p += strlen(p); *p++ = ' ';
			strcpy(p, inet2addr(dnsServers[i])); p += strlen(p); *p++ = '\n';
			size = p - buf;
			rval = fwrite(buf, 1, size, fp);
		}
	}
	fclose(fp);
	return 0;
}
		
/*
int niAddDnsServers(void)
{
	FILE	*fp;
	unsigned char	src[2000], dst[2000], ipAddr[4], dns[4][4];
	char	*s, *d, *p, temp[128], key[32], val[32];
	int		i, size, size2, len, count;

	if(dhcpIsOpened()) count = dhcpGetDnsServers(dns);
	else	count = 0;
//	if(!checkIpAddr(syscfgDnsIpAddress(NULL), 0)) {
//		memcpy(dns[count], syscfgDnsIpAddress(NULL), 4);
//		count++;
//	}
printf("niAddDnsServers: %d\n", count);
	fp = fopen("/etc/resolv.conf", "r");
	if(!fp && errno != ENOENT) {
		printf("niAddDnsServers: fopen error: %s\n", strerror(errno)); 
		return -1;
	}
	d = (char *)dst;
	if(fp) {
		size = fread(src, 1, 2000, fp);
printf("fread=%d\n", size);
		if(size < 0) {
			printf("niAddDnsServers: fread error: %s\n", strerror(errno)); 
			fclose(fp);
			return -1;
		}
		fclose(fp);
		src[size] = 0;
		s = (char *)src;
		while(1) {
			s = strcpy_chr(temp, s, '\n');
			if(!s) break;
			get_keyvalue_space(temp, key, val);
			if(strcmp(key, nameserver)) {
				len = strlen(temp);
				memcpy(d, temp, len); d += len; *d++ = '\n';
			}
		}
	} else {
		size = -1;
	}
	for(i = 0;i < count;i++) {
		len = strlen(nameserver);
		memcpy(d, nameserver, len); d += len; *d++ = ' ';
		p = inet2addr(dns[i]);
		len = strlen(p);
		memcpy(d, p, len); d += len; *d++ = '\n';
	}
	size2 = d - (char *)dst;
	if(size != size2 || size > 0 && memcmp(src, dst, size)) {
		fp = fopen("/etc/resolv.conf", "w");
		if(!fp) {
			printf("niAddDnsServers: fopen error: %s\n", strerror(errno)); 
			return -1;
		}
		size = fwrite(dst, 1, size2, fp);
printf("resolv.conf fwrite: %d\n", size);
		if(size < 0) {
			printf("niAddDnsServers: fwrite error: %s\n", strerror(errno)); 
			fclose(fp);
			return -1;
		}
		fclose(fp);
		sync();
	}
	return 0;
}

int niUp(void)
{
	struct ifreq	ifr;
	struct sockaddr_in *sin;
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	strcpy(ifr.ifr_name, _ifName);
	rval = ioctl(fd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	rval = ioctl(fd, SIOCSIFFLAGS, &ifr);
	close(fd);
	return 0;
}

int niSetIpAddress(unsigned char *ipAddr, unsigned char *subnet, unsigned char *bcast)
{
	struct ifreq	ifr;
	struct sockaddr_in *addr;
	int		fd, rval;

	fd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
	if(fd < 0) {
		printf("socket open error: %s", strerror(errno));
		return -1;
	}
	memset(&ifr, 0, sizeof(struct ifreq));
	ifr.ifr_addr.sa_family = AF_INET;
	strcpy(ifr.ifr_name, _ifName);
	if(ipAddr) {
		addr = (struct sockaddr_in *)&ifr.ifr_addr;
		memcpy(&addr->sin_addr.s_addr, ipAddr, 4);
		rval = ioctl(fd, SIOCSIFADDR, &ifr); 
		if(rval < 0) {
			printf("SIOCSIFADDR error: %s", strerror(errno));
			close(fd);
			return -1;
		}
	}
	if(subnet) {
		addr = (struct sockaddr_in *)&ifr.ifr_addr;
		memcpy(&addr->sin_addr.s_addr, subnet, 4);
		rval = ioctl(fd, SIOCSIFNETMASK, &ifr);
		if(rval < 0) {
			printf("SIOCSIFNETMASK error: %s", strerror(errno));
			close(fd);
			return -1;
		}
	}
	if(bcast) {
		addr = (struct sockaddr_in *)&ifr.ifr_addr;
		memcpy(&addr->sin_addr.s_addr, bcast, 4);
		rval = ioctl(fd, SIOCSIFBRDADDR, &ifr);
		if(rval < 0) {
			printf("SIOCSIFBRDADDR error: %s", strerror(errno));
			close(fd);
			return -1;
		}
	}
	close(fd);
	return 0;
}
	
void niGetMACAddress(char *addr)
{
	char	*p, temp[256], temp2[64];

	ExecPipe("/sbin/ifconfig", "ifconfig", "-a", NULL); 
	ExecPipeFinal(temp, "/bin/grep", "grep", "^wlan0", NULL); 
	// wlan0     Link encap:Ethernet  HWaddr 00:E0:4C:12:24:5A
	p = temp;
	do { 
		p = read_token(p, temp2);
	} while(p) ; 
	strcpy(addr, temp2);
}

void niSetDhcpIpAddress(void)
{
	char	temp[256], optif[8];

printf("Set DHCP...\n");
	optif[0] = '-'; optif[1] = 'i'; strcpy(optif+2, _ifName);
	//ExecPipeFinal(temp, "/sbin/udhcpc", "udhcpc", optif, "-T1", NULL); 
	ExecPipeFinal(temp, "/sbin/udhcpc", "udhcpc", optif, "-bq", NULL); 
printf("udhcpc [%s]\n", temp);
}

*/

