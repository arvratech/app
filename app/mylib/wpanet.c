#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "rtc.h"
#include "prim.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "wpactrl.h"
#include "wpanet.h"

static unsigned char _ipAddress[4]  =	{ 192, 168, 2, 2 };
static unsigned char _subnetMask[4] = 	{ 255, 255, 255, 0 };


void wpanetSetDefault(WPA_NET_CFG *cfg)
{
	cfg->enable			= 0;
	memset(cfg->bssid, 0xff, 6);
	cfg->ssid[0]		= 0;
	cfg->password[0]	= 0;
	cfg->auth			= 0;
	cfg->option			= WPA_IP_DHCP_ENABLE;
	cfg->extdata		= 0;
	memcpy(cfg->ipAddress, _ipAddress, 4);
	memcpy(cfg->subnetMask, _subnetMask, 4);
	memcpy(cfg->defaultGateway, _ipAddress, 4); cfg->defaultGateway[3] = 1; 
	memset(cfg->dnsServers[0], 0, 4);
	memset(cfg->dnsServers[1], 0, 4);
	memset(cfg->dhcpIpAddress, 0, 4);
}

int wpanetEncode(WPA_NET_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->enable;
	memcpy(p, cfg->bssid, 6); p += 6;
	memcpy_pad(p, cfg->ssid, 32); p += 32;	
	memcpy_pad(p, cfg->password, 32); p += 32;	
	*p++ = cfg->auth;
	*p++ = cfg->option;
	*p++ = cfg->extdata;
	memcpy(p, cfg->ipAddress, 4); p += 4;
	memcpy(p, cfg->subnetMask, 4); p += 4;
	memcpy(p, cfg->defaultGateway, 4); p += 4;
	memcpy(p, cfg->dhcpIpAddress, 4); p += 4;
 	return p - (unsigned char *)buf;
}

int wpanetDecode(WPA_NET_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->enable		= *p++;
	memcpy(cfg->bssid, p, 6); p += 6;
	memcpy_chop(cfg->ssid, p, 32); p += 32;	
	memcpy_chop(cfg->password, p, 32); p += 32;	
	cfg->auth		= *p++;
	cfg->option		= *p++;
	cfg->extdata	= *p++;
	memcpy(cfg->ipAddress, p, 4); p += 4;
	memcpy(cfg->subnetMask, p, 4); p += 4;
	memcpy(cfg->defaultGateway, p, 4); p += 4;
	memcpy(cfg->dhcpIpAddress, p, 4); p += 4;
 	return p - (unsigned char *)buf;
}

int wpanetDecode2(WPA_NET_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->enable		= *p++;
	memcpy(cfg->bssid, p, 6); p += 6;
	memcpy_chop(cfg->ssid, p, 32); p += 32;	
	memcpy_chop(cfg->password, p, 32); p += 32;	
	cfg->auth		= *p++;
	cfg->option		= *p++;
	memcpy(cfg->ipAddress, p, 4); p += 4;
	memcpy(cfg->subnetMask, p, 4); p += 4;
	memcpy(cfg->defaultGateway, p, 4); p += 4;
	return p - (unsigned char *)buf;
}

static void _AddNetwork(WPA_NET *wpanet);
static void _EnableNetwork(WPA_NET *wpanet);

void wpanetsOpen(WPA_NET *wpanets)
{
	WPA_NET		*wn;
	WPA_NET_CFG	*wn_cfg;
	int		i, count;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++) {
		wn_cfg = &sys_cfg->wpanets[i];
		wn->cfg		= wn_cfg;
		wn->netID	= -1;
		wn->status	= 0;
		if(wn_cfg->enable) {
printf("wpanets[%d]: %s %s\n", i, mac2addr(wn_cfg->bssid), wn_cfg->ssid);
			_AddNetwork(wn);
			//if(wn->netID >= 0) _EnableNetwork(wn);
		}
		wn->wpascan	= NULL;
	}
}

void wpanetsClose(WPA_NET *wpanets)
{
	WPA_NET		*wn;
	char	cmd[128], rsp[512];
	int		i, rval;

	sprintf(cmd, "REMOVE_NETWORK ALL");
	rval = wpactrlRequest(cmd, rsp, 512);
	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++) {
		wn->netID	= -1;
		wn->status	= 0;
	}
}

int wpanetsGetNetworkCount(WPA_NET *wpanets)
{
	WPA_NET		*wn;
	int		i, count;

	count = 0;
	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID >= 0) count++;
	return count;
}

WPA_NET *wpanetsFindWithId(WPA_NET *wpanets, int id)
{
	WPA_NET		*wn;
	int		i;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID == id) break;
	if(i >= MAX_WPA_NET_SZ) wn = NULL;
	return wn; 
}

WPA_NET *wpanetsFindWithBssid(WPA_NET *wpanets, unsigned char *bssid)
{
	WPA_NET		*wn;
	WPA_NET_CFG	*wn_cfg;
	int		i;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++) {
		wn_cfg = (WPA_NET_CFG *)wn->cfg;
		if(wn->netID >= 0 && !memcmp(wn_cfg->bssid, bssid, 6)) break;
	}
	if(i >= MAX_WPA_NET_SZ) wn = NULL;
	return wn; 
}

static void _AddNetwork(WPA_NET *wpanet)
{
	char	cmd[128], rsp[512];
	int		rval, netID, auth;
	
	// ADD_NETWORK
	// Add a new network. This command creates a new network with empty configuration.
	// The new network is disabled and once it has been configured it can be enabled
	// with ENABLE_NETWORK command. ADD_NETWORK returns the network id of the new network
	// or FAIL on failure.
	sprintf(cmd, "ADD_NETWORK");
	rval = wpactrlRequest(cmd, rsp, 512);
	if(rval == -2) rval = wpactrlMoreResponse(rsp, 512);
	if(rval < 2) {
		wpanet->netID = -1;
		return;
	}
	netID = atoi(rsp);
	// SET_NETWORK <network id> <variable> <value>
	// Set network variables.
	sprintf(cmd, "SET_NETWORK %d ssid \"%s\"", netID, wpanetSsid(wpanet));
	rval = wpactrlRequest(cmd, rsp, 512);
	auth = wpanetAuth(wpanet);
	if(auth == KEY_NONE || auth == KEY_WEP) {
		sprintf(cmd, "SET_NETWORK %d key_mgmt NONE", netID);
		rval = wpactrlRequest(cmd, rsp, 512);
		if(auth == KEY_WEP) {
			sprintf(cmd, "SET_NETWORK %d wep_key0 \"%s\"", netID, wpanetPassword(wpanet));
			rval = wpactrlRequest(cmd, rsp, 512);
			sprintf(cmd, "SET_NETWORK %d wep_txkeyidx 0", netID);
			rval = wpactrlRequest(cmd, rsp, 512);
		}
	} else {
		sprintf(cmd, "SET_NETWORK %d key_mgmt WPA-PSK", netID);
		rval = wpactrlRequest(cmd, rsp, 512);
		if(auth == KEY_WPA2_PSK_CCMP) sprintf(cmd, "SET_NETWORK %d proto WPA2", netID);
		else	sprintf(cmd, "SET_NETWORK %d proto WPA", netID);
		rval = wpactrlRequest(cmd, rsp, 512);
		if(auth == KEY_WPA_PSK_TKIP) sprintf(cmd, "SET_NETWORK %d pairwise TKIP", netID);
		else	sprintf(cmd, "SET_NETWORK %d pairwise CCMP", netID);
		rval = wpactrlRequest(cmd, rsp, 512);
		sprintf(cmd, "SET_NETWORK %d psk \"%s\"", netID, wpanetPassword(wpanet));
		rval = wpactrlRequest(cmd, rsp, 512);
	}
	wpanet->netID = netID;
}

// ENABLE_NETWORK <network id>
// Enable a network
static void _EnableNetwork(WPA_NET *wpanet)
{
	char	cmd[128], rsp[512];
	int		rval;

	sprintf(cmd, "ENABLE_NETWORK %d", wpanet->netID);
	rval = wpactrlRequest(cmd, rsp, 512);
	wpanet->status = 1;
}

// DISABLE_NETWORK <network id>
// Disable a network
static void _DisableNetwork(WPA_NET *wpanet)
{
	char	cmd[128], rsp[512];
	int		rval;

	sprintf(cmd, "DISABLE_NETWORK %d", wpanet->netID);
	rval = wpactrlRequest(cmd, rsp, 512);
	wpanet->status = 0;
}

// REMOVE_NETWORK <network id>
// Remove a network.
static void _RemoveNetwork(WPA_NET *wpanet)
{
	char	cmd[128], rsp[512];
	int		rval;

	sprintf(cmd, "REMOVE_NETWORK %d", wpanet->netID);
	rval = wpactrlRequest(cmd, rsp, 512);
	wpanet->status = 0;
}

// SELECT_NETWORK <network id>
// Select a network (disable others)
void _SelectNetwork(WPA_NET *wpanet)
{
	char	cmd[128], rsp[512];
	int		rval;

	sprintf(cmd, "SELECT_NETWORK %d", wpanet->netID);
	rval = wpactrlRequest(cmd, rsp, 512);
}

// STATUS 
// Get the current WPA/EAPOL/EAP status
static int _Status(void)
{
	char	*p, rsp[512], line[256], value[64];
	int		rval;

	rval = wpactrlRequest("STATUS", rsp, 512);
	if(!rval) {
		rval = -1;
		p = rsp;	
		while(p) {
			p = read_linestr(p, line);
			if(!strcmp(line, "wpa_state")) {
				strcpy(value, line+10);
				if(!strcmp(value, "COMPLETED")) rval = WPA_COMPLETED;
				else if(!strcmp(value, "DISCONNECTED")) rval = WPA_DISCONNECTED;
				else if(!strcmp(value, "INTERFACE_DISABLED")) rval = WPA_INTERFACE_DISABLED;
				else if(!strcmp(value, "INACTIVE")) rval = WPA_INACTIVE;
				else	rval = 99;
				break;
			}
		}
	} else {
		rval = -1;
	}
	return rval;
}

int wpanetsStatus(void)
{
	return _Status();
}

// DISABLE_NETWORK <network id>

WPA_NET *wpanetsAddNetwork(WPA_NET *wpanets, WPA_NET_CFG *wpanet_cfg)
{
	WPA_NET		*wn;
	WPA_NET_CFG	*wn_cfg;
	char	cmd[128], rsp[512];
	int		i, rval, netID;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++) {
		wn_cfg = (WPA_NET_CFG *)wn->cfg;
		if(!wn_cfg->enable) break;
	}
	if(i >= MAX_WPA_NET_SZ) wn = NULL;
	else {
		memcpy(wn->cfg, wpanet_cfg, sizeof(WPA_NET_CFG));
		wn_cfg->enable = 1;
		_AddNetwork(wn);
		if(wn->netID >= 0) _EnableNetwork(wn);
	}
	return wn;
}

void wpanetsRemoveNetwork(WPA_NET *wpanets, WPA_NET *wpanet)
{
	_RemoveNetwork(wpanet);
	wpanet->netID = -1;
	wpanetSetDefault(wpanet->cfg);
}

void wpanetsSelectNetwork(WPA_NET *wpanets, WPA_NET *wpanet)
{
	WPA_NET		*wn;
	int		i, rval;

	_SelectNetwork(wpanet);
	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID >= 0 && wn->wpascan) {
			if(wn == wpanet) wn->status = 1;
			else	wn->status = 0;
		}
}

void wpanetsDeselectNetwork(WPA_NET *wpanets)
{
	WPA_NET		*wn;
	char	cmd[128], rsp[512];
	int		i, rval;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID >= 0) {
			if(!wn->status && wn->wpascan) _EnableNetwork(wn);
			else if(wn->status && !wn->wpascan) _DisableNetwork(wn);
		}
}

unsigned char *wpanetBssid(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->bssid;
}

char *wpanetSsid(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->ssid;
}

char *wpanetPassword(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->password;
}

int wpanetAuth(void *self)
{
	WPA_NET		*wpanet = self;

	return (int)wpanet->cfg->auth;
}

int wpanetIpDhcpEnable(void *self)
{
	WPA_NET		*wpanet = self;

	if(wpanet->cfg->option & WPA_IP_DHCP_ENABLE) return 1;
	else	return 0;
}

void wpanetSetIpDhcpEnable(void *self, int enable)
{
	WPA_NET		*wpanet = self;

	if(enable) wpanet->cfg->option |= WPA_IP_DHCP_ENABLE;
	else	   wpanet->cfg->option &= ~WPA_IP_DHCP_ENABLE;
}

unsigned char *wpanetIpAddress(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->ipAddress;
}

void wpanetSetIpAddress(void *self, unsigned char *ipAddress)
{
	WPA_NET		*wpanet = self;

	memcpy(wpanet->cfg->ipAddress, ipAddress, 4); 
}

char *wpanetIpAddressName(void *self)
{
	return inet2addr(wpanetIpAddress(self));
}

void wpanetSetIpAddressName(void *self, char *name)
{
	unsigned char	ipAddress[4];

	addr2inet(name, ipAddress);
	wpanetSetIpAddress(self, ipAddress);
}

unsigned char *wpanetSubnetMask(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->subnetMask;
}

void wpanetSetSubnetMask(void *self, unsigned char *subnetMask)
{
	WPA_NET		*wpanet = self;

	memcpy(wpanet->cfg->subnetMask, subnetMask, 4); 
}

char *wpanetSubnetMaskName(void *self)
{
	return inet2addr(wpanetSubnetMask(self));
}

void wpanetSetSubnetMaskName(void *self, char *name)
{
	unsigned char	subnetMask[4];

	addr2inet(name, subnetMask);
	wpanetSetSubnetMask(self, subnetMask);
}

unsigned char *wpanetDefaultGateway(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->defaultGateway;
}

void wpanetSetDefaultGateway(void *self, unsigned char *defaultGateway)
{
	WPA_NET		*wpanet = self;

	memcpy(wpanet->cfg->defaultGateway, defaultGateway, 4); 
}

char *wpanetDefaultGatewayName(void *self)
{
	return inet2addr(wpanetDefaultGateway(self));
}

void wpanetSetDefaultGatewayName(void *self, char *name)
{
	unsigned char	defaultGateway[4];

	addr2inet(name, defaultGateway);
	wpanetSetDefaultGateway(self, defaultGateway);
}

unsigned char *wpanetPrimaryDnsServer(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->dnsServers[0];
}

void wpanetSetPrimaryDnsServer(void *self, unsigned char *dnsServer)
{
	WPA_NET		*wpanet = self;

	if(memcmp(wpanet->cfg->dnsServers[0], dnsServer, 4)) wpanet->changesPending = 1; else wpanet->changesPending = 0;
	memcpy(wpanet->cfg->dnsServers[0], dnsServer, 4);
}

char *wpanetPrimaryDnsServerName(void *self)
{
	return inet2addr(wpanetPrimaryDnsServer(self));
}

void wpanetSetPrimaryDnsServerName(void *self, char *name)
{
	unsigned char	dnsServer[4];

	addr2inet(name, dnsServer);
	wpanetSetPrimaryDnsServer(self, dnsServer);
}

unsigned char *wpanetSecondaryDnsServer(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->dnsServers[1];
}

void wpanetSetSecondaryDnsServer(void *self, unsigned char *dnsServer)
{
	WPA_NET		*wpanet = self;

	if(memcmp(wpanet->cfg->dnsServers[1], dnsServer, 4)) wpanet->changesPending = 1; else wpanet->changesPending = 0;
	memcpy(wpanet->cfg->dnsServers[1], dnsServer, 4);
}

char *wpanetSecondaryDnsServerName(void *self)
{
	return inet2addr(wpanetSecondaryDnsServer(self));
}

void wpanetSetSecondaryDnsServerName(void *self, char *name)
{
	unsigned char	dnsServer[4];

	addr2inet(name, dnsServer);
	wpanetSetPrimaryDnsServer(self, dnsServer);
}

unsigned char *wpanetDhcpIpAddress(void *self)
{
	WPA_NET		*wpanet = self;

	return wpanet->cfg->dhcpIpAddress;
}

void wpanetSetDhcpIpAddress(void *self, unsigned char *dhcpIpAddress)
{
	WPA_NET		*wpanet = self;

	memcpy(wpanet->cfg->dhcpIpAddress, dhcpIpAddress, 4); 
}

char *wpanetDhcpIpAddressName(void *self)
{
	return inet2addr(wpanetDhcpIpAddress(self));
}

void wpanetSetDhcpIpAddressName(void *self, char *name)
{
	unsigned char	dhcpIpAddress[4];

	addr2inet(name, dhcpIpAddress);
	wpanetSetDhcpIpAddress(self, dhcpIpAddress);
}

void wpanetsLinkScanning(WPA_NET *wpanets, WPA_SCAN *wpascans, int wpascanCount)
{
	WPA_SCAN	*ws;
	WPA_NET		*wn;
	int		i, j, rval, count;

	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++) wn->wpascan = NULL;
	for(i = 0, ws = wpascans;i < wpascanCount;i++, ws++) {
		for(j = 0, wn = wpanets;j < MAX_WPA_NET_SZ;j++, wn++)
			if(wn->netID >= 0 && !wn->wpascan && !memcmp(ws->bssid, wpanetBssid(wn), 6)) break;
		if(j < MAX_WPA_NET_SZ) {
			ws->wpanet = wn; wn->wpascan = ws;
		} else {
			ws->wpanet = NULL;
		}
	}
}

int wpanetsGetCountOfNotInRange(WPA_NET *wpanets)
{
	WPA_NET		*wn;
	int		i, count;

	count = 0;
	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID >= 0 && !wn->wpascan) count++;
	return count; 
}

WPA_NET *wpanetsGetOfNotInRange(WPA_NET *wpanets, int index)
{
	WPA_NET		*wn;
	int		i, count;

	count = 0;
	for(i = 0, wn = wpanets;i < MAX_WPA_NET_SZ;i++, wn++)
		if(wn->netID >= 0 && !wn->wpascan) {
			if(count == index) break;
			count++;
		}
	if(i >= MAX_WPA_NET_SZ) wn = NULL;
	return wn; 
}

int wpascansMoveTopWithBssid(WPA_SCAN *wpascans, int wpascanCount, unsigned char *bssid)
{
	WPA_SCAN	*ws, _ws;
	int		i, rval, count;

	for(i = 0, ws = wpascans;i < wpascanCount;i++, ws++)
		if(!memcmp(ws->bssid, bssid, 6)) break;
	if(i < wpascanCount && i > 0) {
		memcpy(&_ws, &wpascans[0], sizeof(WPA_SCAN));
		memcpy(&wpascans[0], &wpascans[i], sizeof(WPA_SCAN));
		memcpy(&wpascans[i], &_ws, sizeof(WPA_SCAN));
	} else	i = 0;
	return i;
}

int _ParseWpaAuth(char *buf)
{
	char	*s, *d, temp[64];
	int		c, auth, aval;

	auth = KEY_NONE;
	s = buf;
	while(1) {
		c = *s++;
		if(c != '[') break;
		d = temp;
		while((c = *s++) && c != ']') *d++ = c;
		if(c != ']') break;
		*d = 0;
		if(!strcmp(temp, "WPA2-PSK-CCMP") || !strcmp(temp, "WPA2-PSK-TKIP+CCMP")) aval = KEY_WPA2_PSK_CCMP;
		else if(!strcmp(temp, "WPA-PSK-CCMP") || !strcmp(temp, "WPA-PSK-TKIP+CCMP")) aval = KEY_WPA_PSK_CCMP;
		else if(!strcmp(temp, "WPA-PSK-TKIP")) aval = KEY_WPA_PSK_TKIP;
		else if(!memcmp(temp, "WPA-EAP", 7) || !memcmp(temp, "WPA2-EAP", 8)) aval = KEY_WPA_EAP;
		else if(!strcmp(temp, "WEP")) aval = KEY_WEP;
		else if(!strcmp(temp, "WPS")) aval = KEY_WPS;
		else if(!strcmp(temp, "ESS")) aval = KEY_NONE;
		else	aval = KEY_UNKNOWN;
		if(aval > auth) auth = aval;
	}
	return auth;
}

static char		wpatemp[4096];

int wpascansScanResults(WPA_SCAN *wpascans, int wpascanCount)
{
	WPA_SCAN	*wpascan;
	char	*p, *s, line[512], value[80];
	int		rval, count, err;

	rval = wpactrlRequest("SCAN_RESULTS", wpatemp, 4096);
if(rval == -2) rval = wpactrlMoreResponse(wpatemp, 4096);
	if(rval < 3) return 0;
	count = 0;
	wpascan = wpascans;
	p = wpatemp;
	p = read_linestr(p, line);
	err = 0;
	while(p && count < wpascanCount) {
if(err) printf("Error: %d=[%s]\n", count, line);
		err = 1;
		p = read_linestr(p, line);
		s = line;
		s = read_token(s, value);
		addr2mac(value, wpascan->bssid);
		if(!s) continue;
		s = read_token(s, value);
		wpascan->frequency = n_atoi(value);
		if(!s) continue;
		s = read_token(s, value);
		wpascan->signalLevel = n_atoi(value);
		if(!s) continue;
		s = read_token(s, value);
		wpascan->auth = _ParseWpaAuth(value);
//printf("%d [%s] Auth=%d\n", count, value, (int)wpascan->auth);
		if(s)  {
			strcpy(wpascan->ssid, s);
		} else {
			wpascan->ssid[0] = 0;
		}
		wpascan->wpanet = (void *)0;
		count++; wpascan++;
		err = 0;
	}
if(err) printf("Error: %d=[%s]\n", count, line);
	return count;
}

