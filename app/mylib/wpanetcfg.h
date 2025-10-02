#ifndef _WPANET_CFG_H_
#define _WPANET_CFG_H_


#define MAX_WPA_NET_SZ		8
#define WPA_IP_DHCP_ENABLE	0x01


typedef struct _WPA_NET_CFG {
	unsigned char	enable;
	char			bssid[6];
	char			ssid[33];
	char			password[33];
	unsigned char	auth;
	unsigned char	option;
	unsigned char	extdata;
	unsigned char	ipAddress[4];
	unsigned char	subnetMask[4];
	unsigned char	defaultGateway[4];
	unsigned char	dnsServers[2][4];
	unsigned char	dhcpIpAddress[4];
} WPA_NET_CFG;


void wpanetSetDefault(WPA_NET_CFG *cfg);
int  wpanetEncode(WPA_NET_CFG *cfg, void *buf);
int  wpanetDecode(WPA_NET_CFG *cfg, void *buf);
int  wpanetDecode2(WPA_NET_CFG *cfg, void *buf);


#endif

