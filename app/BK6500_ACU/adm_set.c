#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "symbol.h"
#include "lang.h"
#include "bmpc.h"
#include "syscfg.h"
#include "sysdep.h"
#include "acad.h"
#include "acap.h"
#include "user.h"
#include "pno.h"
#include "fsuser.h"
#include "cr.h"
#include "unit.h"
#include "msg.h"
#include "evt.h"
#include "usbd.h"
#include "lcdc.h"
#ifndef STAND_ALONE
#include "cli.h"
#endif
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "topprim.h"
#include "admprim.h"
#include "adm.h"

extern FP_USER	*gUser;
extern int	gRequest;
extern unsigned char	gLocalDateTimerVersion;


void _AddAdmSetupSettings(GMENU *menu)
{
	DEVICE_CFG	*dev_cfg;
	CAM_CFG		*cam_cfg;
	char	*p1, temp[80];
	int		val;

	dev_cfg = &sys_cfg->Device;
	cam_cfg = &sys_cfg->Camera;
	sprintf(temp, "%s=%d", xmenu_monitor[0], (int)dev_cfg->BackLightTime);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_monitor[1], (int)dev_cfg->LCDContrast);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_monitor[2], (int)dev_cfg->LCDBrightness);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	_SetOptionYesNo(temp, xmenu_monitor[3], dev_cfg->DeviceOption, BW_MODE);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	if(dev_cfg->TimeNotation) val = 1; else val = 0;
	sprintf(temp, "%s=%s", xmenu_monitor[4], xmenu_time_notation[val]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	val = dev_cfg->DateNotation;
	sprintf(temp, "%s=%s", xmenu_monitor[5], xmenu_date_notation[val]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);	
	sprintf(temp, "%s=%d", xmenu_sound[0], (int)dev_cfg->UserSoundVolume);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_sound[1], (int)dev_cfg->AdminSoundVolume);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_sound[2], (int)dev_cfg->KeySoundVolume);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	_SetOptionYesNo(temp, xmenu_lockout[0], dev_cfg->DeviceOption, LOCKOUT);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	_SetOptionYesNo(temp, xmenu_lockout[1], dev_cfg->DeviceOption, TAMPER);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s", xmenu_setup[6]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
/*
	temp[0] = ' '; p1 = temp+1;
	sprintf(p1, "%s=%s", xmenu_cam[0], xmenu_cam_mode[cam_cfg->Mode-1]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(p1, "%s=%d", xmenu_cam[1], cam_cfg->Channel);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
*/
}

static void _PutAdmSetupSettings(char *Title)
{
	GMENU	*menu, _menu;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	_AddAdmSetupSettings(menu);
	MenuShow(menu);
	MenuTerminate(menu);	
}

void MenuAdministrator(GMENU *ParentMenu, char *Title);
void WizardAdministrator(GMENU *ParentMenu, char *Title);
void MenuDateTime(GMENU *ParentMenu, char *Title);
void MenuMonitor(GMENU *ParentMenu, char *Title);
void MenuSound(GMENU *ParentMenu, char *Title);
void MenuLockout(GMENU *ParentMenu, char *Title);
void MenuLanguage(GMENU *ParentMenu, char *Title);
//void MenuCam(GMENU *ParentMenu, char *Title);
#ifdef STAND_ALONE
void MenuDoorPhone(GMENU *ParentMenu, char *Title);
#endif

void MenuAdm_Setup(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval, i;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_setup, 6);
	while(1) {
		if(MenuGetAdminConsole()) _PutAdmSetupSettings(Title);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			if(MenuGetAdminConsole()) MenuAdministrator(menu, ItemText);
			else	WizardAdministrator(menu, ItemText);
			break;
		case 2:
			MenuDateTime(menu, ItemText);	
			break;
		case 3:
			MenuMonitor(menu, ItemText);
			break;
		case 4:
			MenuSound(menu, ItemText);
			break;
		case 5:
			MenuLockout(menu, ItemText);
			break;
		case 6:
			MenuLanguage(menu, ItemText);
			MenuClearItems(menu);
			menuSetWindowText(menu, xmenu_top[3]);
			for(i = 0;i < 8;i++) MenuAddItem(menu, xmenu_setup[i], 0, GMENU_ITEM_ENABLE);
			DesktopInitClock();	
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuAdministrator(GMENU *ParentMenu, char *Title)
{
	char	buf1[20], buf2[20];
	int		rval, len;
	
	PromptMsg((GWND *)ParentMenu, Title, M_ENTER_PASSWORD); if(menu_errno) return;
	buf1[0] = buf2[0] = 0;
	rval = menucInputPassword(NULL, buf1, 8);
	if(rval < 1) return;
	PromptMsg((GWND *)ParentMenu, Title, M_VERIFY_PASSWORD); if(menu_errno) return;
	rval = menucInputPassword(NULL, buf2, 8);			
	if(rval < 1) return;
	len = strlen(buf1);
	if(!len && strlen(buf2) || len && len < 4) ResultMsg((GWND *)ParentMenu, Title, R_INVALID_DATA);
	else if(len && strcmp(buf1, buf2)) ResultMsg((GWND *)ParentMenu, Title, R_DATA_MISMATCHED);
	else if(strcmp(sys_cfg->AdminPIN, buf1)) {
		strcpy(buf2, sys_cfg->AdminPIN);
		strcpy(sys_cfg->AdminPIN, buf1);
		rval = WndSaveConfig((GWND *)ParentMenu, Title, 1);
		if(rval < 0) strcpy(sys_cfg->AdminPIN, buf2);
#ifndef STAND_ALONE
		else	ClisAdminPINChanged();
#endif
	}
}

int _EnrollPINStage(GWND *WizardWnd, int Wizard, char *Title, FP_USER *user);

void WizardAdministrator(GMENU *ParentMenu, char *Title)
{
	GWND		*wnd, _wnd;
	FP_USER		*user, _user;
	char	temp[16], pin[16];
	int		rval, Stage, Wizard;
	
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	user = &_user;
	string2bcd(sys_cfg->AdminPIN, 8, user->PIN);
	Stage = 1;
	while(Stage) {
		Wizard = 0;		// GWND_WIZARD_NEXT
		switch(Stage) {
		case 1: rval = _EnrollPINStage(wnd, Wizard, xmenu_admin[0], user); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else {
			bcd2string(user->PIN, 8, pin);
			if(strcmp(sys_cfg->AdminPIN, pin)) {
				strcpy(temp, sys_cfg->AdminPIN);
				strcpy(sys_cfg->AdminPIN, pin);
				rval = WndSaveConfig(wnd, Title, 1);
				if(rval < 0) strcpy(sys_cfg->AdminPIN, temp);
#ifndef STAND_ALONE
				else	ClisAdminPINChanged();
#endif
			}
			Stage = 0;
		}
		taskYield();
	}
}

void MenuDateTime(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	unsigned char	cdata[8];
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_datetime, 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			rval = WndInputDate((GWND *)menu, ItemText, cdata);
			if(rval > 1) {
				rtcSetDate(cdata);
#ifndef STAND_ALONE
				ClisLocalDateTimeChanged();
#endif
				
				ResultMsg((GWND *)menu, ItemText, R_DATE_CHANGED);
			}
			break;
		case 2:
			rval = WndInputTime((GWND *)menu, ItemText, cdata);
			if(rval > 1) {
				rtcSetTime(cdata);
#ifndef STAND_ALONE
				ClisLocalDateTimeChanged();
#endif
				ResultMsg((GWND *)menu, ItemText, R_TIME_CHANGED);
			}
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuMonitor(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	DEVICE_CFG	*dev_cfg;
	char	*ItemText, temp[32];
	int		rval;

	dev_cfg = &sys_cfg->Device;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_monitor, 6);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			MakeGuideNumberSecondAlways(temp, 255);
			WndInputNumber((GWND *)menu, ItemText, temp, &dev_cfg->BackLightTime, 1, 0, 255, R_INVALID_TIME_INTERVAL, 1);
			break;
		case 2:
			WndInputAnalog((GWND *)menu, ItemText, &dev_cfg->LCDContrast, 1, 64, lcdSetContrast, 1);
			break;
		case 3:
			WndInputAnalog((GWND *)menu, ItemText, &dev_cfg->LCDBrightness, 1, 32, lcdSetBrightness, 1);
			break;
		case 4:
			rval = MenuNoYes((GWND *)menu, ItemText, &dev_cfg->DeviceOption, BW_MODE, 1);
			if(rval > 2) DesktopSetColorMode();
			break;
		case 5:
			rval = MenuRadio((GWND *)menu, ItemText, &dev_cfg->TimeNotation, xmenu_time_notation, 2, 1, 1);
			break;
		case 6:
			rval = MenuRadio((GWND *)menu, ItemText, &dev_cfg->DateNotation, xmenu_date_notation, 3, 1, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuSound(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	DEVICE_CFG	*dev_cfg;
	char	*ItemText;
	int		rval;

	dev_cfg = &sys_cfg->Device;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_sound, 3);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);		
		switch(rval) {
		case 1:
			WndInputAnalog((GWND *)menu, ItemText, &dev_cfg->UserSoundVolume, 0, 20, NULL, 1);	// Title, pVal, Min, Max, CallBack(), Section
			break;
		case 2:
			WndInputAnalog((GWND *)menu, ItemText, &dev_cfg->AdminSoundVolume, 0, 20, NULL, 1);
			break;
		case 3:
			WndInputAnalog((GWND *)menu, ItemText, &dev_cfg->KeySoundVolume, 0, 20, NULL, 1);
			break;		
		}
	}
	MenuTerminate(menu);
}

void MenuLockout(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	DEVICE_CFG	*dev_cfg;
	char	*ItemText;
	int		rval;

	dev_cfg = &sys_cfg->Device;
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_lockout, 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuNoYes((GWND *)menu, ItemText, &dev_cfg->DeviceOption, LOCKOUT, 1);
			break;
		case 2:
			MenuNoYes((GWND *)menu, ItemText, &dev_cfg->DeviceOption, TAMPER, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuLanguage(GMENU *ParentMenu, char *Title)
{
	unsigned char	cVal;
	int		rval, val, lang;

	AdminConsoleUnsupported();	
	lang = sys_cfg->Device.Language;
	if(lang == LANG_KOR || lang == LANG_TWN) val = 2;
	else	val = 1;
	cVal = (unsigned char)val;
	rval = MenuRadio((GWND *)ParentMenu, Title, &cVal, xmenu_lang, 2, 0, 0);	// Title, pVal, menuStr[], menuSize, FromZero, Section
	if(rval > 2) {
		if(cVal == 2)
#ifdef _TWN
			val = LANG_TWN;
#else
			val = LANG_KOR;
#endif
		else	val = LANG_ENG;
		if(val != sys_cfg->Device.Language) {
			ResultMsg((GWND *)ParentMenu, Title, R_DATA_SAVED);
			ChangeLanguage(val);
		}
	}
}
/*
#include "adm_set_cam.c"
#ifdef STAND_ALONE
#include "adm_set_phone.c"
#include "adm_set_pno.c"
#endif
*/