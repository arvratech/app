#ifndef _DHCP_H_
#define _DHCP_H_


void dhcpInit(void);
void dhcpOpen(unsigned char *macAddr, unsigned char *ipAddr);
void dhcpClose(void);
int  dhcpIsOpened(void);
int  dhcpIsBound(void);
int  dhcpGetDnsAddresses(unsigned char dns[][4]);


#endif

