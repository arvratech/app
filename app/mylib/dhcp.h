#ifndef _DHCP_H_
#define _DHCP_H_


void dhcpInit(void);
void dhcpOpen(int ifIndex, unsigned char *macAddr, unsigned char *ipAddr);
void dhcpClose(int status);
void dhcpSetCallback(void *context, void (*onBound)(void *), void (*onUnbound)(void *));
int  dhcpIsOpened(void);
int  dhcpIsBound(void);
unsigned char *dhcpIpAddress(void);
unsigned char *dhcpSubnetMask(void);
unsigned char *dhcpDefaultGateway(void);
int  dhcpGetDnsServers(unsigned char dnsServers[][4]);


#endif

