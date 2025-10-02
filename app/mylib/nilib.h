#ifndef _NI_LIB_H_
#define _NI_LIB_H_


// niState definitions
#define S_NI_NULL		0
#define S_NI_IDLE		1
#define S_NI_UNBOUND	2
#define S_NI_READY		3


void niInit(void (*onNiUnbound)(void));
char *niIfName(void);
unsigned char *niMacAddress(void);
int  niIfIndex(void);
int  niState(void);
int  niIsReady(void);
void niChanged(void);
void niFatalError(void);
void niApplyChanges(void);
void niWpaConnected(void *wpanet);
void niWpaDisconnected(void *wpanet);
void niWpaInterfaceDisabled(void);
void niDhcpBound(unsigned char *ipAddr, unsigned char *subnet, unsigned char *gwAddr);
void niDhcpUnbound(void);
void niUnbound(void);
void niUp(void);
void niDown(void);
int  niGetIpAddress(unsigned char *ipAddr, unsigned char *subnet, unsigned char *bcast);
void niSetIpAddress(unsigned char *ipAddr, unsigned char *subnet);
void niGetDefaultGateway(unsigned char *gwAddr);
void niSetDefaultGateway(unsigned char *gwAddr);
int  niAddDnsServers(void);


#endif

