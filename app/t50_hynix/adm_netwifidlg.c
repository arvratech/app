#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
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
#include "dlg.h"
#include "pref.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "wpanet.h"
#include "wpaprim.h"
#include "appact.h"


void AdmNetWifiDlgOnCreate(GACTIVITY *act);
void AdmNetWifiDlgOnDestroy(GACTIVITY *act);
void AdmNetWifiDlgOnTimer(GACTIVITY *act, int timerID);
void AdmNetWifiDlgOnClick(GACTIVITY *act, void *view);
void AdmNetWifiDlgOnAppMessage(GACTIVITY *act, unsigned char *msg);
void AdmNetWifiDlgOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
int  AdmNetWifiDlgOnNumberOfRowsInTableView(void *self, void *tblv);
void *AdmNetWifiDlgOnCellForRowAtIndex(void *self, void *tblv, int index);
void AdmNetWifiDlgOnDidSelectRowAtIndex(void *self, void *tblv, int index);
static void		*tblv;
static WPA_NET_CFG	*wpanet_cfg;


void AdmNetWifiDlgStart(int requestCode, void *wpanetCfg)
{
	GACTIVITY	*act;

	act = CreateActivity(AdmNetWifiDlgOnCreate);
	wpanet_cfg = (WPA_NET_CFG *)wpanetCfg;
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, requestCode, NULL, 0);
}

void AdmNetWifiDlgOnCreate(GACTIVITY *act)
{
	void	*wnd, *btn;
	CGRect	rect;
	char	temp[80];
	int		rval, i, x, y, w, h;

	activitySetOnDestroy(act, AdmNetWifiDlgOnDestroy);
	activitySetOnClick(act, AdmNetWifiDlgOnClick);
	activitySetOnAppMessage(act, AdmNetWifiDlgOnAppMessage);
	activitySetOnActivityResult(act, AdmNetWifiDlgOnActivityResult);
	tblv = AdmActInitWithTableButtonView(act, xmenu_network_wifi[6]);
	wnd = activityWindow(act);
	btn = UIviewWithTag(wnd, TAG_OK_BTN);
	UIsetTitleColor(btn, TABLE_GREY);
	UIsetUserInteractionEnabled(btn, FALSE);
	UIsetOnNumberOfRowsInTableView(tblv, AdmNetWifiDlgOnNumberOfRowsInTableView);
	UIsetOnCellForRowAtIndex(tblv, AdmNetWifiDlgOnCellForRowAtIndex);
	UIsetOnDidSelectRowAtIndex(tblv, AdmNetWifiDlgOnDidSelectRowAtIndex);
	UIreloadData(tblv);
}

void AdmNetWifiDlgOnDestroy(GACTIVITY *act)
{
	AdmActExit(act);
}

void AdmNetWifiDlgOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_BACK_BTN:
	case TAG_CANCEL_BTN:
printf("Cancel Button...\n");
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	case TAG_OK_BTN:
printf("OK Button...\n");
		DestroyActivityForResult(act, RESULT_OK, NULL);
		break;
	}
}

void AdmNetWifiDlgOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
}

void AdmNetWifiDlgOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	void	*wnd;
	void	*cell, *lbl, *btn;
	unsigned char	IPAddr[4];
	char	*p, key[36], value[64];
	UIColor	color;
	BOOL	bVal;
	int		rval, val;

	wnd = activityWindow(act);
	if(resultCode != RESULT_OK) return;
	switch(requestCode) {
	case ACT_PREF+1:
		get_keyvalue(intent, key, value);
printf("[%s] [%s]\n", value, wpanet_cfg->password);
		if(strcmp(value, wpanet_cfg->password)) {
			strcpy(wpanet_cfg->password, value);
			cell = UIcellForRowAtIndex(tblv, 1);
			lbl = UIdetailTextLabel(cell);
			plainstr2password(value, key);
			UIsetText(lbl, key);
			if(strlen(wpanet_cfg->password) < 8) bVal = FALSE;
			else	bVal = TRUE; 
			btn = UIviewWithTag(wnd, TAG_OK_BTN);
			UIsetUserInteractionEnabled(btn, bVal);
			if(bVal) color = TABLE_GREY_50;
			else	 color = TABLE_GREY;
			UIsetTitleColor(btn, color);
		}
		break;
	case ACT_PREF+3:
		get_keyvalue(intent, key, value);
		addr2inet(value, IPAddr);
		if(memcmp(IPAddr, wpanet_cfg->ipAddress, 4)) {
			memcpy(wpanet_cfg->ipAddress, IPAddr, 4);
			cell = UIcellForRowAtIndex(tblv, 3);
			lbl = UIdetailTextLabel(cell);
			UIsetText(lbl, inet2addr(wpanet_cfg->ipAddress));
		}
		break;
	case ACT_PREF+4:
		get_keyvalue(intent, key, value);
		addr2inet(value, IPAddr);
		if(memcmp(IPAddr, wpanet_cfg->subnetMask, 4)) {
			memcpy(wpanet_cfg->subnetMask, IPAddr, 4);
			cell = UIcellForRowAtIndex(tblv, 4);
			lbl = UIdetailTextLabel(cell);
			UIsetText(lbl, inet2addr(wpanet_cfg->subnetMask));
		}
		break;
	case ACT_PREF+5:
		get_keyvalue(intent, key, value);
		addr2inet(value, IPAddr);
		if(memcmp(IPAddr, wpanet_cfg->defaultGateway, 4)) {
			memcpy(wpanet_cfg->defaultGateway, IPAddr, 4);
			cell = UIcellForRowAtIndex(tblv, 5);
			lbl = UIdetailTextLabel(cell);
			UIsetText(lbl, inet2addr(wpanet_cfg->defaultGateway));
		}
		break;
	}
}

int AdmNetWifiDlgOnNumberOfRowsInTableView(void *self, void *tblv)
{
	int		rval;

	if(wpanet_cfg->option & WPA_IP_DHCP_ENABLE) rval = 3;
	else	rval = 6;
	return rval;
}

void *AdmNetWifiDlgOnCellForRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell, *lbl1, *lbl2;
	char	temp[128];
	UITableViewCellStyle	style;
	UITableViewCellAccessoryType	accessoryType;
	BOOL	bVal;
	int		val;

	if(index == 2) {
		style = UITableViewCellStyleDefault;
		if(wpanet_cfg->option & WPA_IP_DHCP_ENABLE) accessoryType = UITableViewCellAccessoryTypeCheckmark;
		else	accessoryType = UITableViewCellAccessoryTypeUncheckmark;
	} else {
		style = UITableViewCellStyleSubtitle;
		accessoryType = UITableViewCellAccessoryTypeNone;
	}
	cell = AdmInitTableViewCell(tblv, style, accessoryType, NULL);
	lbl1 = UItextLabel(cell);
	if(index != 2) {
		lbl2 = UIdetailTextLabel(cell);
	}
	switch(index) {
	case 0:
		UIsetText(lbl1, xmenu_network_wifi[0]);
		UIsetText(lbl2, wpanet_cfg->ssid);
		UIsetUserInteractionEnabled(cell, FALSE);
		break;
	case 1:
		UIsetText(lbl1, GetPromptMsg(M_ENTER_PASSWORD));
		plainstr2password(wpanet_cfg->password, temp);
		UIsetText(lbl2, temp);
		break;
	case 2:
		UIsetText(lbl1, xmenu_network_tcpip[1]);
		break;
	case 3:
		UIsetText(lbl1, xmenu_network_tcpip[2]);
		UIsetText(lbl2, inet2addr(wpanet_cfg->ipAddress));
		break;
	case 4:
		UIsetText(lbl1, xmenu_network_tcpip[3]);
		UIsetText(lbl2, inet2addr(wpanet_cfg->subnetMask));
		break;
	case 5:
		UIsetText(lbl1, xmenu_network_tcpip[4]);
		UIsetText(lbl2, inet2addr(wpanet_cfg->defaultGateway));
		break;
	}
	return cell;
}

void AdmNetWifiDlgOnDidSelectRowAtIndex(void *self, void *tblv, int index)
{
	void	*cell;
	char	temp[128];
	int		val;

	switch(index) {
	case 1:
		sprintf(temp, "WiFi=0\n%s=\n%d,32\n", GetTableViewCellText(tblv, index), TI_ALPHANUMERIC_PASSWORD);
		TextInputDialog(ACT_PREF+index, temp, NULL);
		break;
	case 2:
		cell = UIcellForRowAtIndex(tblv, index);
		if(UIaccessoryType(cell) == UITableViewCellAccessoryTypeCheckmark) {
			wpanet_cfg->option &= ~WPA_IP_DHCP_ENABLE;
			UIsetAccessoryType(cell, UITableViewCellAccessoryTypeUncheckmark);
		} else {
			wpanet_cfg->option |= WPA_IP_DHCP_ENABLE;
			UIsetAccessoryType(cell, UITableViewCellAccessoryTypeCheckmark);
		}
		UIreloadData(tblv);
		break;
	case 3:
		sprintf(temp, "WiFi=0\n%s=%s\n%d,15\n", GetTableViewCellText(tblv, index), inet2addr(wpanet_cfg->ipAddress), TI_IP_ADDRESS);
		TextInputDialog(ACT_PREF+index, temp, AdmDoneValidateIpAddress);
		break;
	case 4:
		sprintf(temp, "WiFi=0\n%s=%s\n%d,15\n", GetTableViewCellText(tblv, index), inet2addr(wpanet_cfg->subnetMask), TI_IP_ADDRESS);
		TextInputDialog(ACT_PREF+index, temp, AdmDoneValidateIpAddress);
		break;
	case 5:
		sprintf(temp, "WiFi=0\n%s=%s\n%d,15\n", GetTableViewCellText(tblv, index), inet2addr(wpanet_cfg->defaultGateway), TI_IP_ADDRESS);
		TextInputDialog(ACT_PREF+index, temp, AdmDoneValidateIpAddress);
		break;
	}
}

