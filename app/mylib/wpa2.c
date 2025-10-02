#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "dhcp.h"
#include "nwp.h"
#include "wpactrl.h"
#include "wpanet.h"
#include "wpaprim.h"
#include "wpa.h"

#define SCAN_TIMEOUT		20000
#define MAX_WPASCAN_SZ		32

unsigned char	wpaState, wpaScanning, wpaConnScanning;
static int		spipe_fds[2];
WPA_STATUS	*wpastat, _wpastat;
WPA_NET		wpanets[MAX_WPA_NET_SZ];
WPA_SCAN	wpascans[MAX_WPASCAN_SZ];
int			wpascanCount;
unsigned long	wpaTimer;
static void		*cbContext;
static void		(*onConnected)(void *, void *);
static void		(*onDisconnected)(void *, void *);
static void		(*onInterfaceDisabled)(void *);


int wpaWpaState(void)
{
	return (int)wpastat->wpaState;
}

int wpaStateIsConnected(void)
{
	NETWORK_PORT		*nwp;

	nwp = nwpsGet(0);
	if(wpastat->wpaState == WPA_COMPLETED && nwpState(nwp) >= S_NWP_READY) return 1;
	else	return 0;
}

unsigned char *wpaBssid(void)
{
	return wpastat->bssid;
}

void wpaSetScanning(int enable)
{
	wpaScanning = enable;
	if(enable > 1) wpaConnScanning = 1;
	else	wpaConnScanning = 0;
}

void WpaInit(void *context, void (*ponConnected)(void *, void *), void (*ponDisconnected)(void *, void *), void(*ponInterfaceDisabled)(void *))
{
	cbContext				= context;
	onConnected				= ponConnected;
	onDisconnected			= ponDisconnected;
	onInterfaceDisabled		= ponInterfaceDisabled;
}

void _WpaInit(void)
{
	wpaState = 0;
	wpastat = &_wpastat;
	memset(wpastat->bssid, 0, 6);
	wpastat->wpaState = WPA_INACTIVE;
	wpastat->discCause = 0;
	wpactrlInit();
}

int  _WpaScanStart(void);

void _WpaOpen(vid)
{
	int		rval;

	rval = wpactrlOpen("wlan0");
	if(!rval) {
printf("WPA connected...\n");
		wpanetsOpen(wpanets);
		wpastat->wpaState = WPA_INACTIVE;
		_WpaScanStart();
		wpaSetScanning(1);
		wpaState = 1;
	} else {
printf("WPA open failed...\n");
	}
	wpaTimer = MS_TIMER;
}

void _WpaClose(void)
{
	if(wpaState) {
printf("WPA disconnected...\n");
		wpanetsClose(wpanets);
		wpactrlClose();
		wpaState = 0;
	}
	wpaTimer = MS_TIMER;
}

void wpaAddNetwork(WPA_NET_CFG *wpanet_cfg)
{
	unsigned char	buf[12];

	buf[0] = 1;
	memcpy(buf+2, &wpanet_cfg, sizeof(void *));
	write(spipe_fds[1], buf, 10);
}

void wpaSelectNetwork(WPA_NET_CFG *wpanet_cfg)
{
	unsigned char	buf[12];

	buf[0] = 2;
	memcpy(buf+2, &wpanet_cfg, sizeof(void *));
	write(spipe_fds[1], buf, 10);
}

void wpaRemoveNetwork(WPA_NET_CFG *wpanet_cfg)
{
	unsigned char	buf[12];

	buf[0] = 3;
	memcpy(buf+2, &wpanet_cfg, sizeof(void *));
	write(spipe_fds[1], buf, 10);
}

static pthread_t	_thread;
static int			threadid;

#define MAX_WPABUF_SZ		2048
unsigned char	monWpaBuf[MAX_WPABUF_SZ];

void WpaThreadInit(void *(*ThreadMain)(void *))
{
	threadid = 1;
	pthread_create(&_thread, NULL, ThreadMain, (void *)&threadid);
}

int  _WpaScanStart(void);
void _WpaEventReceived(unsigned char *buf, int size);
void _WpaAddNetwork(WPA_NET_CFG *wpanet_cfg);
void _WpaSelectNetwork(WPA_NET_CFG *wpanet_cfg);
void _WpaRemoveNetwork(WPA_NET_CFG *wpanet_cfg);

void *WpaTask(void *arg)
{
	unsigned char	buf[80];
	WPA_NET_CFG		*wpanet_cfg;
	int		rval, pval, timeout;

	_WpaInit();
	rval = pipeOpen(spipe_fds);
	wpaTimer = MS_TIMER;
printf("WpaNetTask: pipe()=%d\n", rval);
	while(1) {
		switch(wpaState) {
		case 0:
			if((MS_TIMER-wpaTimer) > 5000) _WpaOpen();
			break;
		case 1:
			rval = MS_TIMER - wpaTimer;
			if(rval < SCAN_TIMEOUT) timeout = SCAN_TIMEOUT - rval;
			else	timeout = 1; 
			pval = wpactrlPipePoll(spipe_fds[0], timeout);
//printf("wpaPoill=0x%x\n", pval);
			if(pval & 2) {
				rval = read(spipe_fds[0], buf, 10);
				if(rval >= 0) {
					memcpy(&wpanet_cfg, buf+2, sizeof(void *));
					switch(buf[0]) {
					case 1:	_WpaAddNetwork(wpanet_cfg); break;
					case 2:	_WpaSelectNetwork(wpanet_cfg); break;
					case 3:	_WpaRemoveNetwork(wpanet_cfg); break;
					}
				}
			}
			if(pval & 1) {
				rval = wpactrlEventRead(monWpaBuf, MAX_WPABUF_SZ);
				if(rval <= 0) {			// error
					printf("%s\n", sinStrError());
					_WpaClose();
				} else {
					_WpaEventReceived(monWpaBuf, rval);
				}
			}
//			if(wpaScanning && (MS_TIMER-wpaTimer) >= SCAN_TIMEOUT && (wpastat->wpaState >= WPA_COMPLETED || wpastat->wpaState == WPA_INACTIVE || wpastat->wpaState == WPA_DISCONNECTED)) {
			if(wpaScanning && (MS_TIMER-wpaTimer) >= SCAN_TIMEOUT && (wpaConnScanning && wpastat->wpaState >= WPA_COMPLETED || (wpastat->wpaState == WPA_INACTIVE || wpastat->wpaState == WPA_DISCONNECTED))) {
				_WpaScanStart();
			}
			break;
		}
	}
	return arg;
}

int _WpaScanStart(void)
{
	char	temp[80];
	int		rval, count;

	rval = wpactrlRequest("SCAN", temp, 80);
	wpaTimer = MS_TIMER;
	return rval;
}

void _WpaScanResults(void)
{
	unsigned char	msg[12];

	if(wpaScanning && (wpaConnScanning && wpastat->wpaState >= WPA_COMPLETED || (wpastat->wpaState == WPA_INACTIVE || wpastat->wpaState == WPA_DISCONNECTED))) {
		wpascanCount = wpascansScanResults(wpascans, MAX_WPASCAN_SZ);
		wpascansMoveTopWithBssid(wpascans, wpascanCount, wpastat->bssid);
		wpanetsLinkScanning(wpanets, wpascans, wpascanCount);
		wpanetsDeselectNetwork(wpanets);
printf("ScanCount=%d\n", wpascanCount);
		msg[0] = GM_WPA_SCAN_RESULTS; memset(msg+1, 9, 0);
		appPostMessage(msg);
	}
}

void _WpaEventReceived(unsigned char *buf, int size)
{
	WPA_NET		*wpanet;
	WPA_NET_CFG	*wpanet_cfg;
	unsigned char	bssid[8], msg[12], addr[4];
	char	*p, temp[80], temp2[80];
	int		rval;

	msg[0] = msg[2] = 0;
	buf[size] = 0;
	if(buf[0] == '<') {
printf("[%s]\n", (char *)buf);
		p = buf + 3;
		p = read_token(p, temp);
		if(!strcmp(temp, "Trying")) {

// <3>Trying to associate with 88:36:6c:08:2c:fe (SSID='secuvine' freq=2452 MHz)
			p = read_token(p, temp);
			p = read_token(p, temp2);
			if(!strcmp(temp, "to") && !strcmp(temp2, "associate")) {
				p = read_token(p, temp);
				p = read_token(p, temp);
				addr2mac(temp, wpastat->bssid);
				p = read_token_ch(p, temp2, '\047');
				p = read_token_ch(p, temp2, '\047');	// ssid
				wpastat->wpaState = WPA_ASSOCIATING; msg[0] = 1;
				msg[2] = wpascansMoveTopWithBssid(wpascans, wpascanCount, wpastat->bssid);
				wpanetsLinkScanning(wpanets, wpascans, wpascanCount);
printf("WPA_ASSOCIATING...[%s] [%s]\n", temp, temp2);
			}
		} else if(!strcmp(temp, "Associated")) {
// <3>Associated with 88:36:6c:08:2c:5e
			wpastat->wpaState = WPA_4WAY_HANDSHAKE; msg[0] = 1;
printf("WPA_ASSOCIATED...[%s]\n", mac2addr(wpastat->bssid));
		} else if(!strcmp(temp, WPA_EVENT_CONNECTED)) {
// <3>CTRL-EVENT-CONNECTED - Connection to 88:36:6c:08:2c:5e completed (auth) [id=0 id_str=]
			wpastat->wpaState = WPA_COMPLETED; msg[0] = 1;
printf("WPA_COMPLETED..[%s]\n", mac2addr(wpastat->bssid));
			wpanet = wpanetsFindWithBssid(wpanets, wpastat->bssid);
			if(onConnected) (*onConnected)(cbContext, wpanet);
		} else if(!strcmp(temp, WPA_EVENT_DISCONNECTED)) {
printf("WPA_DISCONNECTED...[%s]\n", mac2addr(wpastat->bssid));
			wpanet = wpanetsFindWithBssid(wpanets, wpastat->bssid);
			if(wpastat->wpaState < WPA_COMPLETED) wpastat->discCause = 1;
			else {
				wpastat->discCause = 0;
				if(onDisconnected) (*onDisconnected)(cbContext, wpanet);
			}
			wpanetsDeselectNetwork(wpanets);
			if(wpanetsGetNetworkCount(wpanets) > 0) wpastat->wpaState = WPA_DISCONNECTED;
			else	wpastat->wpaState = WPA_INACTIVE;
			msg[0] = 1;
		} else if(!memcmp(buf+3, "Failed to initiate AP scan", 26)) {
			//rval = wpanetsStatus();	// WPA_DISCONNECTED
			if(wpastat->wpaState != WPA_INTERFACE_DISABLED) {
				wpastat->wpaState = WPA_INTERFACE_DISABLED;
				if(onInterfaceDisabled) (*onInterfaceDisabled)(cbContext);
			}
		} else if(!strcmp(temp, WPA_EVENT_SCAN_RESULTS)) {
			_WpaScanResults();
		} else if(!strcmp(temp, WPA_EVENT_SCAN_FAILED)) {
			//wpaScanResult = 2;
		}
	}
	if(msg[0]) {
		msg[0] = GM_WPA_STATE_CHANGED; msg[1] = wpastat->wpaState; memset(msg+3, 7, 0);
		appPostMessage(msg);
	}
}

void _WpaAddNetwork(WPA_NET_CFG *wpanet_cfg)
{
	WPA_NET	*wpanet;

	wpanet = wpanetsAddNetwork(wpanets, wpanet_cfg);
	wpanetsLinkScanning(wpanets, wpascans, wpascanCount);
	wpanetsSelectNetwork(wpanets, wpanet);
}

void _WpaSelectNetwork(WPA_NET_CFG *wpanet_cfg)
{
	WPA_NET	*wpanet;

	wpanet = wpanetsFindWithBssid(wpanets, wpanet_cfg->bssid);
	wpanetsSelectNetwork(wpanets, wpanet);
}

void _WpaRemoveNetwork(WPA_NET_CFG *wpanet_cfg)
{
	WPA_NET	*wpanet;

	wpanet = wpanetsFindWithBssid(wpanets, wpanet_cfg->bssid);
	wpanetsRemoveNetwork(wpanets, wpanet);
	wpanetsLinkScanning(wpanets, wpascans, wpascanCount);
}

int _ParseWpaState(char *state)
{
	int		rval;

	if(!strcmp(state, "DISCONNECTED")) rval = WPA_DISCONNECTED;
	else if (!strcmp(state, "INACTIVE")) rval = WPA_INACTIVE;
	else if (!strcmp(state, "SCANNING")) rval = WPA_SCANNING;
	else if (!strcmp(state, "AUTHENTICATING")) rval = WPA_AUTHENTICATING;
	else if (!strcmp(state, "ASSOCIATING")) rval = WPA_ASSOCIATING;
	else if (!strcmp(state, "ASSOCIATED")) rval = WPA_ASSOCIATED;
	else if (!strcmp(state, "4WAY_HANDSHAKE")) rval = WPA_4WAY_HANDSHAKE;
	else if (!strcmp(state, "GROUP_HANDSHAKE")) rval = WPA_GROUP_HANDSHAKE;
	else if (!strcmp(state, "COMPLETED"))  rval =  WPA_COMPLETED;
	else	rval = -1;
	return rval;
}

int wpaStatus(void)
{
	char	*p, temp[512], key[64], value[64];
	int		rval;

	rval = wpactrlRequest("STATUS", temp, 512);
	wpastat->ssid[0] = 0;
	wpastat->wpaState = WPA_INACTIVE;
	wpastat->keyMgmt = 0;
	memset(wpastat->ipAddress, 0, 4);
	wpastat->netID = -1;
	wpastat->signalLevel = 0;
	p = temp;
	while(p && (*p)) {
		p = get_keyvalue(p, key, value);
		if(!strcmp(key, "bssid")) addr2mac(value, wpastat->bssid);
		else if(!strcmp(key, "ssid")) strcpy(wpastat->ssid, value);
		else if(!strcmp(key, "id")) wpastat->netID = n_atoi(value);
		//else if(!strcmp(key, "key_mgmt")) strcpy(wpastat->key_mgmt, value);
		else if(!strcmp(key, "wpa_state")) wpastat->wpaState = _ParseWpaState(value);
		else if(!strcmp(key, "signal_level")) wpastat->signalLevel = n_atoi(value);
		else if(!strcmp(key, "ip_address")) addr2inet(value, wpastat->ipAddress);
	}
	return 0;
}

