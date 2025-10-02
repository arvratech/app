#ifndef _WPA_NET_H_
#define _WPA_NET_H_

#include "wpanetcfg.h"


// wpa state
#define WPA_INTERFACE_DISABLED		0
#define WPA_INACTIVE				1
#define WPA_DISCONNECTED			2
#define WPA_DORMANT					3		/// Android append
#define WPA_SCANNING				4
#define WPA_AUTHENTICATING			5
#define WPA_ASSOCIATING				6
#define WPA_ASSOCIATED				7
#define WPA_4WAY_HANDSHAKE			8
#define WPA_GROUP_HANDSHAKE			9
#define WPA_COMPLETED				10

// wpa authentication
#define KEY_NONE				0
#define KEY_WPS					1
#define KEY_WEP					2
#define KEY_WPA_PSK_TKIP		3
#define KEY_WPA_PSK_CCMP		4
#define KEY_WPA2_PSK_CCMP		5
#define KEY_WPA_EAP				6
#define KEY_UNKNOWN				7


typedef struct _WPA_NET {
	unsigned char	status;		// 0:Disabled 1:Enabled 2:Current
	unsigned char	mark;
	unsigned char	changesPending;
	unsigned char	reserve[1];
	int				netID;
	void			*wpascan;
	WPA_NET_CFG		*cfg;
} WPA_NET;


typedef struct _WPA_SCAN {
	unsigned char	bssid[6];
	char			ssid[33];
	unsigned char	signalLevel;
	unsigned short	frequency;
	unsigned char	auth;
	unsigned char	reserve[1];
	void			*wpanet;
} WPA_SCAN;


typedef struct _WPA_STATUS {
	unsigned char	bssid[6];
	char			ssid[33];
	unsigned char	wpaState;
	unsigned char	signalLevel;
	unsigned char	keyMgmt;
	unsigned char	discCause;
	unsigned char	reserve[1];
	int				netID;
	unsigned char	ipAddress[4];
} WPA_STATUS;


void wpanetsOpen(WPA_NET *wpanets);
void wpanetsClose(WPA_NET *wpanets);
int  wpanetsGetNetworkCount(WPA_NET *wpanets);
WPA_NET *wpanetsFindWithId(WPA_NET *wpanets, int id);
WPA_NET *wpanetsFindWithBssid(WPA_NET *wpanets, unsigned char *bssid);
int  wpanetsStatus(void);
WPA_NET *wpanetsAddNetwork(WPA_NET *wpanets, WPA_NET_CFG *wpanet_cfg);
void wpanetsRemoveNetwork(WPA_NET *wpanets, WPA_NET *wpanet);
void wpanetsSelectNetwork(WPA_NET *wpanets, WPA_NET *wpanet);
void wpanetsDeselectNetwork(WPA_NET *wpanets);

unsigned char *wpanetBssid(void *self);
char *wpanetSsid(void *self);
char *wpanetPassword(void *self);
int  wpanetAuth(void *self);
int  wpanetIpDhcpEnable(void *self);
void wpanetSetIpDhcpEnable(void *self, int enable);
unsigned char *wpanetIpAddress(void *self);
void wpanetSetIpAddress(void *self, unsigned char *ipAddress);
char *wpanetIpAddressName(void *self);
void wpanetSetIpAddressName(void *self, char *name);
unsigned char *wpanetSubnetMask(void *self);
void wpanetSetSubnetMask(void *self, unsigned char *subnetMask);
char *wpanetSubnetMaskName(void *self);
void wpanetSetSubnetMaskName(void *self, char *name);
unsigned char *wpanetDefaultGateway(void *self);
void wpanetSetDefaultGateway(void *self, unsigned char *defaultGateway);
char *wpanetDefaultGatewayName(void *self);
void wpanetSetDefaultGatewayName(void *self, char *name);
unsigned char *wpanetPrimaryDnsServer(void *self);
void wpanetSetPrimaryDnsServer(void *self, unsigned char *ipDnsServer);
char *wpanetPrimaryDnsServerName(void *self);
void wpanetSetPrimaryDnsServerName(void *self, char *name);
unsigned char *wpanetSecondaryDnsServer(void *self);
void wpanetSetSecondaryDnsServer(void *self, unsigned char *ipDnsServer);
char *wpanetSecondaryDnsServerName(void *self);
void wpanetSetSecondaryDnsServerName(void *self, char *name);
unsigned char *wpanetDhcpIpAddress(void *self);
void wpanetSetDhcpIpAddress(void *self, unsigned char *dhcpIpAddress);
char *wpanetDhcpIpAddressName(void *self);
void wpanetSetDhcpIpAddressName(void *self, char *name);

void wpanetsLinkScanning(WPA_NET *wpanets, WPA_SCAN *wpascans, int wpascanCount);
int  wpanetsGetCountOfNotInRange(WPA_NET *wpanets);
WPA_NET *wpanetsGetOfNotInRange(WPA_NET *wpanets, int index);
int  wpascansMoveTopWithBssid(WPA_SCAN *wpascans, int wpascanCount, unsigned char *bssid);
int  wpascansScanResults(WPA_SCAN *wpascans, int wpascanCount);


#endif

