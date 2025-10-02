#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "..\BootLoader\flash.h"
#include "cotask.h"
#include "rtc.h"
#include "sysdep.h"
#include "syscfg.h"
#include "iodev.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "gfont.h"
#include "user.h"
#include "sdi.h"
#include "ff.h"
#include "sclib.h"
#include "msg.h"
#include "lang.h"
#include "usbd.h"
#include "fsuser.h"
#include "fsar.h"
#include "cr.h"
#include "fpapi_new.h"
#include "ta.h"
#ifndef STAND_ALONE
#include "cli.h"
#endif
#include "menuprim.h"
#include "wnds.h"
#include "topprim.h"
#include "desktop.h"
#include "admprim.h"
#include "adm.h"

#ifndef STAND_ALONE
extern UNIT	_gUnits[];
#endif
extern CREDENTIAL_READER _gCredentialReaders[];
extern FP_USER	*gUser;
extern SYS_CFG	*tmp_cfg, _tmp_cfg;

void MenuViewAllSettings(GMENU *ParentMenu, char *Title);
void MenuInitDevice(GMENU *ParentMenu, char *Title);
void MenuImportFromSDMemory(GMENU *ParentMenu, char *Title);
void MenuExportToSDMemory(GMENU *ParentMenu, char *Title);
void MenuSafelyRemoveSDMemory(GMENU *ParentMenu, char *Title);
#ifndef STAND_ALONE
void MenuCommStatistics(GMENU *ParentMenu, char *Title);
#endif
void MenuReinitializeDevice(GMENU *ParentMenu, char *Title);
void MenuTestSystem(GMENU *ParentMenu, char *Title);


void MenuAdm_System(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
#ifndef STAND_ALONE
	rval = 7;
#else
	rval = 6;
#endif
	MenuInitialize(menu, Title, xmenu_system, rval);
	menu->MenuStatus |= GMENU_HIDDEN_ITEM;
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: MenuViewAllSettings(menu, ItemText); break;
		case 2: MenuInitDevice(menu, ItemText); break;
		case 3: MenuImportFromSDMemory(menu, ItemText); break;
		case 4: MenuExportToSDMemory(menu, ItemText); break;
		case 5: MenuSafelyRemoveSDMemory(menu, ItemText); break;
#ifndef STAND_ALONE
		case 6: MenuCommStatistics(menu, ItemText); break;
#endif
		case 7: MenuReinitializeDevice(menu, ItemText); break;
		case 99: MenuTestSystem(menu, "Test"); break;
		}
	}
	MenuTerminate(menu);
}

void _AddAdmUserStatistics(GMENU *menu);
void _AddAdmAccessControlSettings(GMENU *menu);
void _AddAdmNetworkSettings(GMENU *menu);
void _AddAdmSetupSettings(GMENU *menu);

void MenuViewAllSettings(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	temp[40], temp2[20];
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);

	sprintf(temp, "%s=%s", xmenu_other[0], sys_cfg->Device.FirmwareVersion+7);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	memcpy(temp2, sys_cfg->Device.FirmwareVersion, 6); temp2[6] = 0;
	sprintf(temp, "%s=%s", xmenu_other[1], temp2);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);

	_AddAdmUserStatistics(menu);
	_AddAdmAccessControlSettings(menu);
	_AddAdmNetworkSettings(menu);
	_AddAdmSetupSettings(menu);

//	rval = MenuPopupGlobalExitOnly(menu);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
}

void MenuReinitializeDevice(GMENU *ParentMenu, char *Title)
{
	char	temp[80];
	int		rval;

	sprintf(temp, "%s", GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, temp);
	if(rval == IDYES) {
		ResultMsg((GWND *)ParentMenu, Title, R_SYSTEM_RESTART);
		ResetSystem();
	}
}

#include "adm_sys_init.c"
#include "adm_sys_sd.c"
#ifndef STAND_ALONE
#include "adm_sys_stat.c"
#endif
#include "adm_sys_test.c"