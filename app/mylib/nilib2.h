#ifndef _NI_LIB_H_
#define _NI_LIB_H_


char *niIfName(void);
void niSetIfName(char *ifName);
int  niLinkState(void);
void niUp(void);
void niDown(void);
void niUnbound(void);
int  niGetInterfaceIndex(int *ifIndex);
int  niGetMacAddress(unsigned char *macAddr);
int  niGetIpAddress(unsigned char *ipAddr, unsigned char *subnet, unsigned char *bcast);
void niSetIpAddress(unsigned char *ipAddr, unsigned char *subnet);
void niGetDefaultGateway(unsigned char *gwAddr);
void niSetDefaultGateway(unsigned char *gwAddr);
int  niAddDnsServers(unsigned char dnsServers[][4], int count);


#endif

