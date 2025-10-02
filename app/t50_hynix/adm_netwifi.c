#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h" 
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "nilib.h"
#include "wpanet.h"
#include "wpaprim.h"
#include "wpa.h"
#include "appact.h"

extern WPA_STATUS	*wpastat;
extern WPA_NET		wpanets[];
extern WPA_SCAN		wpascans[];
extern int			wpascanCount;

void AdmNetWifiOnCreate(GACTIVITY *act);
void AdmNetWifiOnStart(GACTIVITY *act);
void AdmNetWifiOnDestroy(GACTIVITY *act);
void AdmNetWifiOnClick(GACTIVITY *act, void *view);
void AdmNetWifiOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmNetWifiOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmNetWifiOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmNetWifiOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmNetWifiOnDidSelectRowAtIndex(void *self, void *tblv, int index);

static void	*tblv;
static WPA_NET_CFG	*wpanetcfgReq, _wpanetcfgReq;
static WPA_NET		*wpanetReq;


void AdmNetWifi(int requestCode, void *ps)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmNetWifiOnCreate);
	activitySetTimeout(act, 30);
	appStartActivity(act, NULL, 0);
}

void AdmNetWifiOnCreate(GACTIVITY *act)
{
	activitySetOnStart(act, AdmNetWifiOnStart);
	activitySetOnDestroy(act, AdmNetWifiOnDestroy);
	activitySetOnClick(act, AdmNetWifiOnClick);
	activitySetOnAppMessage(act, AdmNetWifiOnAppMessage);
	activitySetOnActivityResult(act, AdmNetWifiOnActivityResult);
	tblv = AdmActInitWithTableView(act, xmenu_network[1]);
	UIsetOnNumberOfRowsInTableView(tblv, AdmNetWifiOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmNetWifiOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmNetWifiOnDidSelectRowAtIndex);
	wpanetcfgReq = &_wpanetcfgReq;
	UIreloadData(tblv);
}

void AdmNetWifiOnStart(GACTIVITY *act)
{
printf("AdmNetWifiOnStart.....\n");
	wpaSetScanning(2);
}

void AdmNetWifiOnDestroy(GACTIVITY *act)
{
printf("WifiOnDestroy...\n");
	wpaSetScanning(1);
	AdmActExit(act);
}

void AdmNetWifiOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_PREF_BACK:
		DestroyActivity(act);
		break;
	}
}

void AdmNetWifiRefreshState(void *self, void *tblv);

void AdmNetWifiOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd, *cell;
	unsigned char	buf[12];
	unsigned short	sVal;
	int		rval, index;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_WPA_SCAN_RESULTS:
		UIreloadData(tblv);
		break;
	case GM_WPA_STATE_CHANGED:
		if(msg[2]) UIreloadData(tblv);
		else	AdmNetWifiRefreshState(act, tblv);
		break;
	}
}

void AdmNetWifiOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	WPA_NET		*wpanet;
	void	*wnd;
	void	*cell, *lbl;
	char	*p, key[32], value[64];
	int		rval, val, rem, con;

	wnd = activityWindow(act);
printf("AdmNetWifiOnActivityResult: req=%d result=%d\n", requestCode, resultCode);
	rem = con = 0;
	switch(requestCode) {
	case ACT_PREF:
		if(resultCode == RESULT_FIRST_USER+1) rem = 1;		// Forget
		memcpy(wpanetcfgReq->bssid, wpanetBssid(wpanetReq), 6);
		break;
	case ACT_PREF+1:
		if(resultCode == RESULT_FIRST_USER+2) rem = 1;	 	// Forget
		else if(resultCode == RESULT_FIRST_USER) con = 1;	// Connect
		memcpy(wpanetcfgReq->bssid, wpanetBssid(wpanetReq), 6);
		break;
	case ACT_PREF+2:
		if(resultCode == RESULT_OK) {
printf("bssid=[%s] ssid=[%s] password=[%s]\n", mac2addr(wpanetcfgReq->bssid), wpanetcfgReq->ssid, wpanetcfgReq->password);
			wpaAddNetwork(wpanetcfgReq);
//printf("added: netID=%d status=%d\n", (int)wpanet->netID, (int)wpanet->status);
			con = 1;
		}
		break;
	}
	if(rem) {
		wpaRemoveNetwork(wpanetcfgReq);
	}
	if(con) {
		wpaSelectNetwork(wpanetcfgReq);
//printf("selected: netID=%d status=%d\n", (int)wpanetReq->netID, (int)wpanetReq->status);
		AdmNetWifiRefreshState(act, tblv);
	}
	if(rem || con) AdmNetWifiRefreshState(act, tblv);
}

int AdmNetWifiOnNumberOfRowsInTableView(void *self, void *tblv)
{
printf("wpascanCount=%d wpanetsGetCountOfNotInRange=%d\n", wpascanCount, wpanetsGetCountOfNotInRange(wpanets));
	return wpascanCount + wpanetsGetCountOfNotInRange(wpanets);
}

void AdmNetWifiRefreshCellState(void *cell, int index);

void *AdmNetWifiOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	WPA_SCAN	*wpascan;
	WPA_NET		*wpanet;
	char	*p;
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;

	style = UITableViewCellStyleSubtitle;
	accessoryType = UITableViewCellAccessoryTypeNone;
//printf("index=%d wpascanCount=%d\n", index, wpascanCount);
	if(index < wpascanCount) {
		wpascan = &wpascans[index];
		wpanet	= (WPA_NET *)wpascan->wpanet;
	} else {
		wpascan = NULL;
		wpanet = wpanetsGetOfNotInRange(wpanets, index-wpascanCount);
//printf("wpanet=%x\n", wpanet);
	}
	if(wpascan) p = wpascan->ssid;
	else	    p = wpanetSsid(wpanet);
//printf("p=%x\n", p);
	cell = AdmInitTableViewCell(tblv, style, accessoryType, p);
	AdmNetWifiRefreshCellState(cell, index);
	return cell;
}

void AdmNetWifiOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	WPA_NET		*wpanet;
	WPA_SCAN	*wpascan;
	unsigned char	addr[8];
	char	*p, temp[512];
	int		state, reqCode;

	wpaSetScanning(0);
	if(index < wpascanCount) {
		wpascan = &wpascans[index];
		wpanet	= (WPA_NET *)wpascan->wpanet;
	} else {
		wpascan = NULL;
		wpanet = wpanetsGetOfNotInRange(wpanets, index-wpascanCount);
	}
	wpanetReq = wpanet;
	p = temp;
	if(!wpascan) {
		sprintf(p, "title=%s\n", wpanetSsid(wpanet)); p += strlen(p);
		sprintf(p, "%s=%s\n", xmenu_network_wifi[4], wpaSecurityName(wpanetAuth(wpanet))); p += strlen(p);
		sprintf(p, "button=%s\n", xmenu_okcancel[1]); p += strlen(p);
		sprintf(p, "button=%s\n", xmenu_network_wifi[5]); p += strlen(p);
		reqCode = ACT_PREF;
		FieldAlertDialog(reqCode, temp);
	} else if(wpanet) {
		sprintf(p, "title=%s\n", wpascan->ssid); p += strlen(p);
		state = wpastat->wpaState;
		if(state >= WPA_ASSOCIATING && !memcmp(wpascan->bssid, wpastat->bssid, 6)) {
			sprintf(p, "%s=%s\n", xmenu_network_wifi[1], wpaStateName(state)); p += strlen(p);
			sprintf(p, "%s=%d\n", xmenu_network_wifi[2], wpascan->signalLevel); p += strlen(p);
			if(state >= WPA_COMPLETED) { 
				sprintf(p, "%s=%d\n", xmenu_network_wifi[3], wpaFrequency2Channel((int)wpascan->frequency)); p += strlen(p);
			}
			sprintf(p, "%s=%s\n", xmenu_network_wifi[4], wpaSecurityName((int)wpascan->auth)); p += strlen(p);
			if(state >= WPA_COMPLETED && niState() >= S_NI_READY) { 
				niGetIpAddress(addr, NULL, NULL);
				sprintf(p, "%s=%s\n", xmenu_network_tcpip[2], inet2addr(addr)); p += strlen(p);
			}
			sprintf(p, "button=%s\n", xmenu_okcancel[1]); p += strlen(p);
			sprintf(p, "button=%s\n", xmenu_network_wifi[5]); p += strlen(p);
			reqCode = ACT_PREF;
		} else {
			sprintf(p, "%s=%d\n", xmenu_network_wifi[2], wpascan->signalLevel); p += strlen(p);
			sprintf(p, "%s=%s\n", xmenu_network_wifi[4], wpaSecurityName((int)wpascan->auth)); p += strlen(p);
			sprintf(p, "button=%s\n", xmenu_network_wifi[6]); p += strlen(p);
			sprintf(p, "button=%s\n", xmenu_okcancel[1]); p += strlen(p);
			sprintf(p, "button=%s\n", xmenu_network_wifi[5]); p += strlen(p);
			reqCode = ACT_PREF + 1;
		}
		FieldAlertDialog(reqCode, temp);
	} else {
		if(wpascan->auth == KEY_WPS || wpascan->auth >=  KEY_WPA_EAP) reqCode = R_UNSUPPORTED_SECURITY;
		else if(wpanetsGetNetworkCount(wpanets) >= MAX_WPA_NET_SZ) reqCode = R_DATA_ARE_FULL;
		else	reqCode = 0;
		if(reqCode) {
			p = GetResultMsg(reqCode);
			strcpy_swap(temp, p, '\n', '&');
			AlertDialog(0, temp, FALSE);
		} else {
			wpanetcfgReq = &_wpanetcfgReq;
			wpanetSetDefault(wpanetcfgReq);
			memcpy(wpanetcfgReq->bssid, wpascan->bssid, 6);
			strcpy(wpanetcfgReq->ssid, wpascan->ssid);
			wpanetcfgReq->auth = wpascan->auth;
printf("wpanetReq: %x %x\n", wpanetcfgReq, wpanetReq);
printf("AdmNetWifiDlgStart...\n");
			AdmNetWifiDlgStart(ACT_PREF+2, wpanetcfgReq);
		}
	}
}

void AdmNetWifiRefreshState(void *self, void *tblv)
{
	void	*cell, *cells[16];;
	int		i, index, count;

	count = UIvisibleCells(tblv, cells);
	index = UIindexForCell(tblv, cells[0]);
	for(i = 0;i < count;i++) {
		cell = cells[i];
		AdmNetWifiRefreshCellState(cell, index+i);
	}
}

void AdmNetWifiRefreshCellState(void *cell, int index)
{
	WPA_SCAN	*wpascan;
	WPA_NET		*wpanet;
	void	*lbl;
	char	*p, temp[80], temp2[40];
	int		state;

	if(index < wpascanCount) {
		wpascan = &wpascans[index];
		wpanet	= (WPA_NET *)wpascan->wpanet;
	} else {
		wpascan = NULL;
		wpanet = wpanetsGetOfNotInRange(wpanets, index-wpascanCount);
	}
	lbl = UIdetailTextLabel(cell);
	if(!wpascan) {
		p = xmenu_wifi_state[2];		// Not in range
	} else if(wpanet) {
		state = wpastat->wpaState;
		if(state >= WPA_ASSOCIATING && !memcmp(wpascan->bssid, wpastat->bssid, 6)) {
			p = wpaStateName(state);
		} else if(state == WPA_DISCONNECTED && wpastat->discCause && !memcmp(wpascan->bssid, wpastat->bssid, 6)) {
			p = xmenu_wifi_state[4];	// Authentication error
		} else if(wpanet->status) {
			wpaGetSecuredName((int)wpascan->auth, temp2);
			sprintf(temp, "%s %s", xmenu_wifi_state[1], temp2);	// Saved Secured with xxx
			p = temp;
		} else {
			p = xmenu_wifi_state[0];	// Disabled
		}
	} else {
		wpaGetSecuredName((int)wpascan->auth, temp);	// Secured with xxx
		p = temp;
	}
	UIsetText(lbl, p);
	UIsetNeedsDisplay(cell);
}

void AdmNetRefreshWifiState(void *cell)
{
	void	*lbl;
	char	temp[80];
	int		val;

	lbl = UIdetailTextLabel(cell);
	val = wpastat->wpaState;
	if(val < WPA_ASSOCIATING) sprintf(temp, "%s", wpaStateName(val));
	else	sprintf(temp, "%s %s", wpaStateName(val), wpastat->ssid);
	UIsetText(lbl, temp);
	UIsetNeedsDisplay(cell);
}

