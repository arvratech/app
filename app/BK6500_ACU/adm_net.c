#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "lcdc.h"
#include "gfont.h"
#include "iodev.h"
#include "sin.h"
#include "key.h"
#include "symbol.h"
#include "bmpc.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "user.h"
#include "cr.h"
#include "msg.h"
#include "evt.h"
#include "usbd.h"
#include "sysdep.h"
#include "sinport.h"
#ifndef STAND_ALONE
#include "slavenet.h"
#endif
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "topprim.h"
#include "toptask.h"
#include "ping.h"
#include "adm.h"

extern int	gRequest;


void _AddAdmNetworkSettings(GMENU *menu)
{
	NETWORK_PORT_CFG	*nwp_cfg;
	char	*p, temp[80];
	int		val;

	nwp_cfg = &sys_cfg->NetworkPort;
	sprintf(temp, "%s=%d", xmenu_network[0], (int)sys_cfg->DeviceID);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=", xmenu_network[1]); 
	p = temp + strlen(temp); MACAddr2Str(syscfgGetMACAddress(), p);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	val = nwp_cfg->NetworkType;
	sprintf(temp, "%s=%s", xmenu_network[2], xmenu_network_mode[val-1]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	if(val == 1 || val == 2) {
		sprintf(temp, "%s=%s", xmenu_network_serial[0], xmenu_serial_speed[nwp_cfg->SerialSpeed-1]);
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=%s", xmenu_network_serial[1], xmenu_serial_port[nwp_cfg->SerialPort]);
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	} else {		
		_SetOptionYesNo(temp, xmenu_network_tcpip[0], nwp_cfg->TCPIPOption, 0x01);
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=", xmenu_network_tcpip[1]);
		p = temp + strlen(temp); strcpy(p, inet2addr(nwp_cfg->IPAddress));
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=", xmenu_network_tcpip[2]);
		p = temp + strlen(temp); strcpy(p, inet2addr(nwp_cfg->SubnetMask));
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=", xmenu_network_tcpip[3]);
		p = temp + strlen(temp); strcpy(p, inet2addr(nwp_cfg->GatewayIPAddress));
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=", xmenu_network_tcpip[4]);
		p = temp + strlen(temp); strcpy(p, inet2addr(sys_cfg->ServerIPAddress));
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		sprintf(temp, "%s=%d", xmenu_network_tcpip[5], (int)sys_cfg->ServerPort);
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	}
}

static void _PutAdmNetworkSettings(char *Title)
{
	GMENU	*menu, _menu;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	_AddAdmNetworkSettings(menu);
	MenuShow(menu);
	MenuTerminate(menu);	
}

void MenuIPAddress(char *Title);
void WizardIPAddress(char *Title);
void MenuServerSetup(char *Title);
void WizardServerSetup(char *Title);
void MenuPingTest(GMENU *ParentMenu, char *Title);

void MenuAdm_Network(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	NETWORK_PORT_CFG	*nwp_cfg;
	char	*ItemText, buf[32];
	int		rval, i;

	nwp_cfg = &sys_cfg->NetworkPort;
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		if(MenuGetAdminConsole()) _PutAdmNetworkSettings(Title);
		MenuClearItems(menu);
		for(i = 0;i < 3;i++) MenuAddItem(menu, xmenu_network[i], 0, GMENU_ITEM_ENABLE);
		if(nwp_cfg->NetworkType == NETWORK_TCPIP) {
			for(i = 3;i < 6;i++) MenuAddItem(menu, xmenu_network[i], 0, GMENU_ITEM_ENABLE);
		} else {
			for(i = 0;i < 2;i++) MenuAddItem(menu, xmenu_network_serial[i], 0, GMENU_ITEM_ENABLE);
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			WndInputNumber((GWND *)menu, ItemText, NULL, &sys_cfg->DeviceID, 2, 1, 999, R_INVALID_TID, 1);
			break;
		case 2:
			MACAddr2Str(syscfgGetMACAddress(), buf);
			ResultMsgEx((GWND *)menu, ItemText, -1, MB_ICONNONE, "%s", buf);
			break;
		case 3:
			MenuRadio((GWND *)menu, ItemText, &nwp_cfg->NetworkType, xmenu_network_mode, 3, 0, 1);
			break;
		case 4:
			if(nwp_cfg->NetworkType == NETWORK_TCPIP) {
				if(MenuGetAdminConsole()) MenuIPAddress(ItemText);
				else	WizardIPAddress(ItemText);
			} else {
				rval = MenuRadio((GWND *)menu, ItemText, &nwp_cfg->SerialSpeed, xmenu_serial_speed, 5, 0, 1);
				if(rval > 2) gRequest |= G_SERIAL_SPEED_CHANGED;
			}
			break;
		case 5:
			if(nwp_cfg->NetworkType == NETWORK_TCPIP) {
				if(MenuGetAdminConsole()) MenuServerSetup(ItemText);
				else	WizardServerSetup(ItemText);
			} else {
				rval = MenuRadio((GWND *)menu, ItemText, &nwp_cfg->SerialPort, xmenu_serial_port, 2, 1, 1);
				if(rval > 2) gRequest |= G_SERIAL_SPEED_CHANGED;
			}
			break;
		case 6:
			MenuPingTest(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

int _SaveDynamicIPAddress(GMENU *ParentMenu, char *Title, int NewVal, int InMenu)
{
	NETWORK_PORT_CFG	*nwp_cfg;
	unsigned char	IPAddr[4];
	int		rval, OldVal;

	nwp_cfg = &sys_cfg->NetworkPort;
	OldVal = nwp_cfg->TCPIPOption;
	if(OldVal != NewVal) {
		memcpy(IPAddr, nwp_cfg->IPAddress, 4);
		nwp_cfg->TCPIPOption = NewVal;
		if(NewVal & 0x01) memset(nwp_cfg->IPAddress, 0, 4);
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			if(InMenu) ResultMsg((GWND *)ParentMenu, Title, R_DATA_SAVED);
			rval = 1;
		} else {
			nwp_cfg->TCPIPOption = OldVal;
			memcpy(nwp_cfg->IPAddress, IPAddr, 4);
			if(InMenu) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	} else	rval = 0;
	return rval;
}

void MenuIPAddress(char *Title)
{
	GMENU	*menu, _menu;
	NETWORK_PORT_CFG	*nwp_cfg;
	char	*ItemText;
	unsigned char	cVal, IPAddr[4];
	int		rval;

	nwp_cfg = &sys_cfg->NetworkPort;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_network_tcpip, 4);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			cVal = nwp_cfg->TCPIPOption;
			rval = MenuNoYes((GWND *)menu, ItemText, &cVal, 0x01, 0);
			if(rval > 2) _SaveDynamicIPAddress(menu, ItemText, (int)cVal, 1);
			break;
		case 2:
			if(nwp_cfg->TCPIPOption & 0x01) {
				sockGetIfConfig(IPAddr, NULL, NULL);
				WndViewIPAddress((GWND *)menu, ItemText, IPAddr);
			} else {
				rval = WndInputIPAddress((GWND *)menu, ItemText, nwp_cfg->IPAddress, 0, 1);
				if(rval > 2) gRequest |= G_TCP_IP_CHANGED;
			}
			break;
		case 3:
			if(nwp_cfg->TCPIPOption & 0x01) {
				sockGetIfConfig(NULL, IPAddr, NULL);
				WndViewIPAddress((GWND *)menu, ItemText, IPAddr);
			} else {
				rval = WndInputIPAddress((GWND *)menu, ItemText, nwp_cfg->SubnetMask, 1, 1);
				if(rval > 2) gRequest |= G_TCP_IP_CHANGED;
			}
			break;
		case 4:
			if(nwp_cfg->TCPIPOption & 0x01) {
				sockGetIfConfig(NULL, NULL, IPAddr);
				WndViewIPAddress((GWND *)menu, ItemText, IPAddr);
			} else {
				rval = WndInputIPAddress((GWND *)menu, ItemText, nwp_cfg->GatewayIPAddress, 0, 1);
				if(rval > 2) gRequest |= G_TCP_IP_CHANGED;
			}
			break;
		}
	}
	MenuTerminate(menu);
}

int _DynamicIPAddress(GWND *WizardWnd, int *Wizard, unsigned char *NewOption);
int _ProcessIPAddress(GWND *WizardWnd, int *Wizard, unsigned char NewOption, unsigned char *NewIPAddr);
int _SaveIPAddress(GWND *WizardWnd, char *Title, unsigned char NewOption, unsigned char *NewIPAddr);

void WizardIPAddress(char *Title)
{
	GWND		*wnd, _wnd;
	NETWORK_PORT_CFG	*nwp_cfg;
	unsigned char	NewOption, NewIPAddr[12];
	int		rval, Stage, Wizard;

	nwp_cfg = &sys_cfg->NetworkPort;
	NewOption = nwp_cfg->TCPIPOption;
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 2) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _DynamicIPAddress(wnd, &Wizard, &NewOption); break;
		case 2:	
			if(NewOption & 0x01) sockGetIfConfig(NewIPAddr, NewIPAddr+4, NewIPAddr+8);
			else {
				memcpy(NewIPAddr, nwp_cfg->IPAddress, 4);
				memcpy(NewIPAddr+4, nwp_cfg->SubnetMask, 4);
				memcpy(NewIPAddr+8, nwp_cfg->GatewayIPAddress, 4);
			}
			rval = _ProcessIPAddress(wnd, &Wizard, NewOption, NewIPAddr); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_SaveIPAddress(wnd, Title, NewOption, NewIPAddr);
				Stage = 0;
			}
		}
		taskYield();
	}
}

void ServerSinChanged(void);

void MenuServerSetup(char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, &xmenu_network_tcpip[4], 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			rval = WndInputIPAddress((GWND *)menu, ItemText, sys_cfg->ServerIPAddress, 0, 1);
			if(rval > 2) ServerSinChanged();
			break;
		case 2:
			rval = WndInputNumber((GWND *)menu, ItemText, NULL, &sys_cfg->ServerPort, 2, 1, 65536, R_INVALID_SERVER_PORT, 1);
			if(rval > 2) ServerSinChanged();
			break;
		}
	}
	MenuTerminate(menu);
}

int _DynamicIPAddress(GWND *WizardWnd, int *Wizard, unsigned char *NewOption)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuNoYesWizard(WizardWnd, xmenu_network_tcpip[0], NewOption, 0x01, *Wizard);
	return rval;
}

int _ProcessIPAddress(GWND *WizardWnd, int *Wizard, unsigned char NewOption, unsigned char *IPAddr)
{
	GLABEL	*lblInput1, _lblInput1, *lblInput2, _lblInput2, *lblInput3, _lblInput3;
	GIPADDR	*ipaddrInput1, _ipaddrInput1, *ipaddrInput2, _ipaddrInput2, *ipaddrInput3, _ipaddrInput3;
	RECT	rect;
	int		rval, c, x, y;

	*Wizard &= ~GWND_WIZARD_NEXT;
	wndDrawClientArea(WizardWnd);
	lblInput1 = &_lblInput1;
	y = 10;
	labelInitialize(lblInput1, WizardWnd, 6, y, 150, 24);
	y += 23; 
	ipaddrInput1 = &_ipaddrInput1;
	ipaddrInitialize(ipaddrInput1, WizardWnd, 32, y, 176, 26); 
	y += 29;
	ipaddrSetFont(ipaddrInput1, GWND_NORMAL_FONT);
	if(NewOption & 0x01) ipaddrModifyStyle(ipaddrInput1, ES_PASSWORD|ES_WILDDIGIT, ES_NUMBER|ES_READONLY);
	else	ipaddrModifyStyle(ipaddrInput1, ES_PASSWORD|ES_WILDDIGIT||ES_READONLY, ES_NUMBER);
	labelSetWindowText(lblInput1, xmenu_network_tcpip[1]); 
	memcpy(ipaddrInput1->Address, IPAddr, 4);

	lblInput2 = &_lblInput2;
	labelInitialize(lblInput2, WizardWnd, 6, y, 150, 24); 
	y += 23;
	ipaddrInput2 = &_ipaddrInput2;
	ipaddrInitialize(ipaddrInput2, WizardWnd, 32, y, 176, 26); 
	y += 29;
	ipaddrSetFont(ipaddrInput2, GWND_NORMAL_FONT);
	if(NewOption & 0x01) ipaddrModifyStyle(ipaddrInput2, ES_PASSWORD|ES_WILDDIGIT, ES_NUMBER|ES_READONLY);
	else	ipaddrModifyStyle(ipaddrInput2, ES_PASSWORD|ES_WILDDIGIT||ES_READONLY, ES_NUMBER);
	ipaddrSetSubnetMask(ipaddrInput2, 1);
	labelSetWindowText(lblInput2, xmenu_network_tcpip[2]); 
	memcpy(ipaddrInput2->Address, IPAddr+4, 4);

	lblInput3 = &_lblInput3;
	labelInitialize(lblInput3, WizardWnd, 6, y, 150, 24); 
	y += 23;
	ipaddrInput3 = &_ipaddrInput3;
	ipaddrInitialize(ipaddrInput3, WizardWnd, 32, y, 176, 26); 
	y += 29;
	ipaddrSetFont(ipaddrInput3, GWND_NORMAL_FONT);
	if(NewOption & 0x01) ipaddrModifyStyle(ipaddrInput3, ES_PASSWORD|ES_WILDDIGIT, ES_NUMBER|ES_READONLY);
	else	ipaddrModifyStyle(ipaddrInput3, ES_PASSWORD|ES_WILDDIGIT||ES_READONLY, ES_NUMBER);
	labelSetWindowText(lblInput3, xmenu_network_tcpip[3]); 
	memcpy(ipaddrInput3->Address, IPAddr+8, 4);

	while(1) {
		labelDrawWindow(lblInput1);
		ipaddrDrawWindow(ipaddrInput1);
		labelDrawWindow(lblInput2);
		ipaddrDrawWindow(ipaddrInput2);
		labelDrawWindow(lblInput3);
		ipaddrDrawWindow(ipaddrInput3);					
		if(!(NewOption & 0x01)) {	
			wndGetClientArea(WizardWnd, &rect);
			x = rect.x + 208; c = rect.y + 80;
			lcdPutIconFile(x, c, 24, BTN_PERIOD); x += 22;
			lcdPutIconFile(x, c, 24, BTN_ASTERISK); x += 32;
			lcdPutIconFile(x, c, 24, BTN_BACKSPACE); x += 24;
			lcdPutIconFile(x, c, 24, BTN_SHARP);
		}
		if(NewOption & 0x01) {	
			rval = editReadOnlyWindow(&ipaddrInput3->Edit, *Wizard);
			if(rval > 0) c = ipaddrInput3->Edit.LastKey;
		} else {
			rval = ipaddrProcessWindow(ipaddrInput1, *Wizard);
			if(rval > 0) {
				ipaddrDrawWindow(ipaddrInput1); c = ipaddrInput1->Edit.LastKey;
			}
			if(rval > 0 && c == KEY_OK) {
				rval = ipaddrProcessWindow(ipaddrInput2, *Wizard);
				if(rval > 0) {
					ipaddrDrawWindow(ipaddrInput2); c = ipaddrInput2->Edit.LastKey;
				}
			}
			if(rval > 0 && c == KEY_OK) {
				rval = ipaddrProcessWindow(ipaddrInput3, *Wizard);
				if(rval > 0) {
					ipaddrDrawWindow(ipaddrInput3); c = ipaddrInput3->Edit.LastKey;
				}
			}
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4 || c == KEY_OK && (NewOption & 0x01)) {
			if(!(NewOption & 0x01)) {
				if(CheckIPAddr(ipaddrInput1->Address, 0)) {
					ResultMsg(WizardWnd, xmenu_network_tcpip[1], R_INVALID_IP_ADDRESS);
				} else if(CheckIPAddr(ipaddrInput2->Address, 1)) {
					ResultMsg(WizardWnd, xmenu_network_tcpip[2], R_INVALID_IP_ADDRESS);
				} else if(CheckIPAddr(ipaddrInput3->Address, 0)) {
					ResultMsg(WizardWnd, xmenu_network_tcpip[3], R_INVALID_IP_ADDRESS);
				} else {
					memcpy(IPAddr,   ipaddrInput1->Address, 4);
					memcpy(IPAddr+4, ipaddrInput2->Address, 4);
					memcpy(IPAddr+8, ipaddrInput3->Address, 4);
					if(c == KEY_F1) rval = 1; else rval = 2;
				}
			} else {
				if(c == KEY_F1) rval = 1; else rval = 2;
			}	
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _SaveIPAddress(GWND *WizardWnd, char *Title, unsigned char NewOption, unsigned char *NewIPAddr)
{
	NETWORK_PORT_CFG	*nwp_cfg;	
	unsigned char	OldOption, OldIPAddr[12];
	int		rval;

	nwp_cfg = &sys_cfg->NetworkPort;
	OldOption = nwp_cfg->TCPIPOption;
	if(!(NewOption & 0x01)) {
		memcpy(OldIPAddr, nwp_cfg->IPAddress, 4);
		memcpy(OldIPAddr+4, nwp_cfg->SubnetMask, 4);
		memcpy(OldIPAddr+8, nwp_cfg->GatewayIPAddress, 4);
	}
	if(NewOption != OldOption || !(NewOption & 0x01) && n_memcmp(NewIPAddr, OldIPAddr, 12)) {
		nwp_cfg->TCPIPOption = NewOption;
		if(!(NewOption & 0x01)) {
			memcpy(nwp_cfg->IPAddress, NewIPAddr, 4);
			memcpy(nwp_cfg->SubnetMask, NewIPAddr+4, 4);
			memcpy(nwp_cfg->GatewayIPAddress, NewIPAddr+8, 4);
		}
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) {
			nwp_cfg->TCPIPOption = OldOption;
			if(!(NewOption & 0x01)) {
				memcpy(nwp_cfg->IPAddress, OldIPAddr, 4);
				memcpy(nwp_cfg->SubnetMask, OldIPAddr+4, 4);
				memcpy(nwp_cfg->GatewayIPAddress, OldIPAddr+8, 4);
			}
			rval = -1;
		} else {
			gRequest |= G_TCP_IP_CHANGED;
			rval = 1;
		}
	} else	rval = 0;
	return rval;
}

int _ProcessServerSetup(GWND *WizardWnd, int *Wizard, unsigned char *IPAddr, unsigned short *Port, char *MsgIPAddr, char *MsgPort);
int _SaveServerSetup(GWND *WizardWnd, char *Title, unsigned char *NewIPAddr, unsigned short NewPort);

void WizardServerSetup(char *Title)
{
	GWND		*wnd, _wnd;
	unsigned char	NewIPAddr[4];
	unsigned short	NewPort;
	int		rval, Stage, Wizard;

	NewPort = sys_cfg->ServerPort;
	memcpy(NewIPAddr, sys_cfg->ServerIPAddress, 4);
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		switch(Stage) {
		case 1:	rval = _ProcessServerSetup(wnd, &Wizard, NewIPAddr, &NewPort, xmenu_network_tcpip[4], xmenu_network_tcpip[5]); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			_SaveServerSetup(wnd, Title, NewIPAddr, NewPort);
			Stage = 0;
		}
		taskYield();
	}
}

int _ProcessServerSetup(GWND *WizardWnd, int *Wizard, unsigned char *IPAddr, unsigned short *Port, char *MsgIPAddr, char *MsgPort)
{
	GLABEL	*lblAddr, _lblAddr, *lblPort, _lblPort, *lblMsg, _lblMsg;
	GEDIT	*edtPort, _edtPort;
	GIPADDR	*ipaddrServer, _ipaddrServer;
	RECT	rect;
	unsigned long	NewVal;
	char	buf[64];
	int		rval, c, x, y;

	*Wizard &= ~GWND_WIZARD_NEXT;
	wndDrawClientArea(WizardWnd);
	lblAddr = &_lblAddr;
	y = 32;
	labelInitialize(lblAddr, WizardWnd, 6, y, 150, 24);
	y += 23;
	ipaddrServer = &_ipaddrServer;
	ipaddrInitialize(ipaddrServer, WizardWnd, 32, y, 176, 26); 
	ipaddrSetFont(ipaddrServer, GWND_NORMAL_FONT);
	ipaddrModifyStyle(ipaddrServer, ES_PASSWORD|ES_WILDDIGIT||ES_READONLY, ES_NUMBER);
	labelSetWindowText(lblAddr, MsgIPAddr); 
	memcpy(ipaddrServer->Address, IPAddr, 4);
	wndGetClientArea(WizardWnd, &rect);
	y += 40;
	lblPort = &_lblPort;
	labelInitialize(lblPort, WizardWnd, 6, y, 80, 24);
	labelSetWindowText(lblPort, MsgPort);
	y += 23;
	edtPort = &_edtPort;
	editInitialize(edtPort, WizardWnd, 32, y, 76, 26); 
	editSetFont(edtPort, GWND_NORMAL_FONT);
	editLimitText(edtPort, 5);
	editModifyStyle(edtPort, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	sprintf(edtPort->Text, "%d", (int)*Port);
	lblMsg = &_lblMsg;
	labelInitialize(lblMsg, WizardWnd, 130, y+1, 160, 24);
	MakeGuideNumberNormal(buf, 1, 65535);
	labelSetWindowText(lblMsg, buf);
	while(1) {
		labelDrawWindow(lblAddr);
		ipaddrDrawWindow(ipaddrServer);
		x = rect.x + 208; y = rect.y + 57;
		lcdPutIconFile(x, y, 24, BTN_PERIOD); x += 22;
		lcdPutIconFile(x, y, 24, BTN_ASTERISK); x += 32;
		lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 24;
		lcdPutIconFile(x, y, 24, BTN_SHARP);
		labelDrawWindow(lblPort);
		editDrawWindow(edtPort);
		labelDrawWindow(lblMsg);
		x = rect.x + 240; y = rect.y + 120;
		lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 24;
		lcdPutIconFile(x, y, 24, BTN_SHARP);
		rval = ipaddrProcessWindow(ipaddrServer, *Wizard);
		if(rval > 0) {
			ipaddrDrawWindow(ipaddrServer); c = ipaddrServer->Edit.LastKey;
		}
		if(rval > 0 && c == KEY_OK) {
			rval = editProcessWindow(edtPort, *Wizard);
			if(rval > 0) {
				editDrawWindow(edtPort); c = edtPort->LastKey;
			}
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4) {
			NewVal = n_check_atou(edtPort->Text);
			if(CheckIPAddr(ipaddrServer->Address, 0)) {
				ResultMsg(WizardWnd, MsgIPAddr, R_INVALID_IP_ADDRESS);
			} else if(NewVal < 1 || NewVal > 65535) {
				ResultMsg(WizardWnd, MsgPort, R_INVALID_SERVER_PORT);
			} else {
				*Port = (unsigned short)NewVal;	
				memcpy(IPAddr, ipaddrServer->Address, 4);
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _SaveServerSetup(GWND *WizardWnd, char *Title, unsigned char *NewIPAddr, unsigned short NewPort)
{
	unsigned char	OldIPAddr[4];
	unsigned short	OldPort;
	int		rval;

	OldPort = sys_cfg->ServerPort;
	memcpy(OldIPAddr, sys_cfg->ServerIPAddress, 4);
	if(NewPort != OldPort || n_memcmp(NewIPAddr, OldIPAddr, 4)) {
		memcpy(sys_cfg->ServerIPAddress, NewIPAddr, 4);
		sys_cfg->ServerPort = NewPort;
		rval = WndSaveConfig(WizardWnd, Title, 1);
		if(rval < 0) {
			sys_cfg->ServerPort = OldPort;
			memcpy(sys_cfg->ServerIPAddress, OldIPAddr, 4);
			rval = -1;
		} else {
			rval = 1;
			ServerSinChanged();
		}
	} else	rval = 0;
	return rval;
}

void _GetSmallDigit(int Index, unsigned char *buf);

static unsigned char	IPAddr[4];

/* Return
 *	1: ARP error
 *	2: Loss(timeout)
 *	3: Checksum error
 *	4: Destination unreachable
 *	5: Time exceed error
 *	6: Unknown message
 */

void MenuPingTest(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	RECT	rect;
	char	data[32], temp[32];
	unsigned char	Addr[4];
	unsigned long	timer;
	int		rval, i, x, y, s;

	AdminConsoleUnsupported();
	wnd = &_wnd;
	wndInitialize(wnd, Title, 0, 0, lcdGetWidth(), lcdGetHeight()-32); 
	wndDrawWindow(wnd);

	if(IPIsNull(IPAddr)) memcpy(IPAddr, sys_cfg->ServerIPAddress, 4);
	strcpy(temp, "Ping address");
	strcpy(data, inet2addr(IPAddr));
	rval = WndInputIPString((GWND *)ParentMenu, temp, data, 0);
	if(rval <= 0) return;
	rval = addr2inet(data, Addr);
	if(rval >= 0 && IPIsNull(Addr) || IPIsBroadcast(Addr)) rval = -1;
	if(rval < 0) {
		 ResultMsg((GWND *)ParentMenu, Title, R_INVALID_IP_ADDRESS);
		 return;
	}
	memcpy(IPAddr, Addr, 4);
	strcpy(data, inet2addr(IPAddr));
	wndDrawClientArea(wnd);
	_DrawLowerButtons(0, 0, 0, 0, 0);
	wndGetClientArea(wnd, &rect);
	x = rect.x + 8; y = rect.y + 8;
	s = 3;
	for(i = 0;i < 5;i++) {
		rval = sinSocket(s, SIN_ICMP, 0);
		timer = DS_TIMER;
		rval = SendPing(s, (int)sys_cfg->DeviceID, IPAddr); 
		sinClose(s);
		switch(rval) {
		case 0:		strcpy(data, "Reply OK"); break;
		case 1: 	strcpy(data, "Non Destination"); break;
		case 2:		strcpy(data, "Timed out"); break;
		case 3:		strcpy(data, "Checksum error"); break;
		case 4:		strcpy(data, "Dest. unreachable"); break;
		case 5:		strcpy(data, "Time exceed error"); break;
		default:	strcpy(data, "Unknown message");
		}
		lcdPutText(x, y + i*26, data, wndGetBackGroundTextColor(wnd));
		rval = DS_TIMER - timer;
		if(rval < 10) taskDelay(10-rval);
	}
	keyGet();
	wndProcessWindow(wnd, 0, 150);
}
