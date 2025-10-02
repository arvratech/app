#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "gfont.h"
#include "symbol.h"
#include "lcdc.h"
#ifndef STAND_ALONE
#include "eng16.h"
#endif
#include "lang.h"
#include "syscfg.h"
#include "unit.h"
#include "bio.h"
#include "acad.h"
#include "acap.h"
#ifndef STAND_ALONE
#include "acaz.h"
#endif
#include "alm.h"
#include "fire.h"
#include "fsuser.h"
#include "msg.h"
#include "ta.h"
#include "evt.h"
#include "sche.h"
#include "usbd.h"
#include "sysdep.h"
#include "cr.h"
#include "fpapi_new.h"
#include "sinport.h"
#ifndef STAND_ALONE
#include "slavenet.h"
#include "cli.h"
#endif
#include "menuprim.h"
#include "wnds.h"
#include "desktop.h"
#include "topprim.h"
#include "toptask.h"
#include "admprim.h"
#include "adm.h"

#ifndef STAND_ALONE
extern UNIT	_gUnits[];
#endif


void _MakeTAItemPrefix(char *Prefix, int Item)
{
	int		size;

	size = taGetItemSize();
	if(size > 10) {
		if(Item == 0) {
			if(GetBidi()) strcpy(Prefix, " No"); else strcpy(Prefix, "No ");
		} else	sprintf(Prefix, "F%02d", Item);
	} else {
		if(Item == 0) sprintf(Prefix, "No");
		else	sprintf(Prefix, "F%d", Item);		
	}
}

void _MakeConsoleTAItem(char *Text, int Item)
{
	char	*p;

	p = Text;
	if(taItemIsEnable(Item)) *p++ = '*';
	else	*p++ = ' ';
	strcpy(p, taGetTextAt(taGetItemIndex(Item)));
	p += strlen(p);
	*p = 0;
}

unsigned short	BmpBuffer[2048];
static int	_TAItemOffset;

void _PutNonConsoleTAItem(GMENU *menu, int Item, int x, int y)
{
	BMPC		*bmpD, _bmpD, *bmpS, _bmpS;
	unsigned short	buf[192];
	char	Prefix[8], Text[32];
	int		fh, Index, fColor, File;

	if(menu->MenuStatus & GMENU_PREFIX_NUMBER) {
		bmpD = &_bmpD; bmpS = &_bmpS; 
		bmpcInit(bmpD, buf, 12, 16);
		bmpcInit(bmpS, digits_8x16[Item+1], 8, 16);
		bmpcClear(bmpD, RGB(0,40,128));
		bmpcPutMask(bmpD, 2, 0, RGB(255,255,255), bmpS);
		lcdPut(x+2, y+3, bmpD);
		x += bmpcGetWidth(bmpD) + 4;
	} else	x += 8;
	fColor = menu->BackGroundTextColor;
	Index = Item - _TAItemOffset;
	_MakeTAItemPrefix(Prefix, Index);	
	if(taItemIsEnable(Index)) File = BTN_CHECK_ON; else File = BTN_CHECK_OFF;
	strcpy(Text, taGetTextAt(taGetItemIndex(Index)));
	x += lcdPutText(x, y+1, Prefix, fColor);
	fh = GetFontHeight();
	lcdPutIconFile(x, y+1, fh, File); x += fh+2;
	x += lcdPutText(x, y+1, Text, fColor);
}

void _AddAccessPoint(GMENU *menu, int ID, int doorcfg)
{
}

void _AddAccessDoor(GMENU *menu, int ID, int doorcfg)
{
}

void _AddReadersSetting(GMENU *menu, int doorcfg)
{
}

void _AddAdmAccessControlSettings(GMENU *menu)
{
}

static void _PutAdmAccessControlSettings(char *Title)
{
	GMENU	*menu, _menu;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	_AddAdmAccessControlSettings(menu);
	MenuShow(menu);
	MenuTerminate(menu);	
}

#include "toptask.h"

#ifndef STAND_ALONE

void MenuHardwareConfiguration(GMENU *ParentMenu, char *Title);
void MenuAllReaders(GMENU *ParentMenu, char *Title);
void MenuAccessDoors(GMENU *ParentMenu, char *Title);
void MenuAccessPoints(GMENU *ParentMenu, char *Title);
void MenuBinaryInputs(GMENU *ParentMenu, char *Title);
void MenuBinaryOutputs(GMENU *ParentMenu, char *Title);
void MenuAccessZonesPassback(GMENU *ParentMenu, char *Title);
void MenuAlarms(GMENU *ParentMenu, char *Title);
void MenuXInputOutputs(GMENU *ParentMenu, char *Title);

void MenuAdm_AccessControl(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_access_control, 9);
	while(1) {
		if(MenuGetAdminConsole()) _PutAdmAccessControlSettings(Title);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: MenuHardwareConfiguration(menu, ItemText); break;
		case 2: MenuAllReaders(menu, ItemText); break;
		case 3: MenuAccessDoors(menu, ItemText); break;
		case 4: MenuAccessPoints(menu, ItemText); break;
		case 5: MenuBinaryInputs(menu, ItemText); break;
		case 6: MenuBinaryOutputs(menu, ItemText); break;
		case 7: MenuAccessZonesPassback(menu, ItemText); break;
		case 8: MenuAlarms(menu, ItemText); break;
		case 9: MenuXInputOutputs(menu, ItemText); break;			
		}
	}
	MenuTerminate(menu);
}

#else

void MenuHardwareConfiguration(GMENU *ParentMenu, char *Title);
void MenuAccessDoor(GMENU *ParentMenu, char *Title, int adID);
void MenuAccessPoint(GMENU *ParentMenu, char *Title, int adID, int apID);
void MenuBinaryInputs(GMENU *ParentMenu, char *Title);
void MenuBinaryOutputs(GMENU *ParentMenu, char *Title);
void MenuAlarms(GMENU *ParentMenu, char *Title);

void MenuAdm_AccessControl(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[24];
	int		rval, ad;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {
		if(MenuGetAdminConsole()) _PutAdmAccessControlSettings(Title);
		MenuClearItems(menu);
		MenuAddItem(menu, xmenu_access_control[0], 0, GMENU_ITEM_ENABLE);
		if(apGetAssignedDoor(0) >= 0) {
			ad = 1; GetAccessDoorName(0, temp);
		} else {
			ad = 0; GetAccessPointName(0, temp);
		}			
		MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_access_control[4], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_access_control[5], 0, GMENU_ITEM_ENABLE);
		MenuAddItem(menu, xmenu_access_control[7], 0, GMENU_ITEM_ENABLE);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: MenuHardwareConfiguration(menu, ItemText); break;
		case 2: if(ad) MenuAccessDoor(menu, ItemText, 0); else MenuAccessPoint(menu, ItemText, -1, 0); break;
		case 3: MenuBinaryInputs(menu, ItemText); break;
		case 4: MenuBinaryOutputs(menu, ItemText); break;
		case 5: MenuAlarms(menu, ItemText); break;
		}
	}
	MenuTerminate(menu);
}

void MenuHardwareConfiguration(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, &xmenu_hw_config[6], 1);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuRadio((GWND *)menu, ItemText, &sys_cfg->AccessDoors[0].Enable, xmenu_inuse, 2, 1, 1);
			break;
		}
	}
	MenuTerminate(menu);
}

#endif

void MenuFireZones(GMENU *ParentMenu, char *Title);
void MenuFirePoints(GMENU *ParentMenu, char *Title);
void MenuAlarmActions(GMENU *ParentMenu, char *Title);
void MenuOutputCommands(GMENU *ParentMenu, char *Title);

void MenuAlarms(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_alarm, 4);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuFireZones(menu, ItemText);
			break;
		case 2:
			MenuFirePoints(menu, ItemText);
			break;
		case 3:
			MenuAlarmActions(menu, ItemText);
			break;
		case 4:
			MenuOutputCommands(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

#ifndef STAND_ALONE

void MenuIOUnitInputOutputs(GMENU *ParentMenu, char *Title, int ID);

void MenuXInputOutputs(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, *p, temp[32];
	int		rval, i, Item, type, IDs[MAX_IOUNIT_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		for(Item = i = 0;i < MAX_IOUNIT_SZ;i++) {
			if(iounitGetEnable(i)) {
				p = temp;
				GetIOUnitName(i, p); p += strlen(p); *p++ = ' ';
				type = iounitGetType(i);
				if(type == 0) strcpy(p, xmenu_iounit_type[0]); else strcpy(p, xmenu_iounit_type[1]);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = i; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		MenuIOUnitInputOutputs(menu, ItemText, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

void MenuBinaryInput(GMENU *ParentMenu, char *Title, int adID, int biID);
void MenuBinaryOutput(GMENU *ParentMenu, char *Title, int adID, int boID);

void MenuIOUnitInputOutputs(GMENU *ParentMenu, char *Title, int ID)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[32];
	int		rval, i, ioID, Item, type, IDs[MAX_IOUNIT_BI_SZ];

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	while(1) {	
		MenuClearItems(menu);
		type = iounitGetType(ID);
		if(type == 0) {
			ioID = iounitGetBinaryInput(ID);
			for(Item = i = 0;i < MAX_IOUNIT_BI_SZ;i++, ioID++) {
				GetBinaryInputName(ioID, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = ioID; Item++;
			}
		} else {
			ioID = iounitGetBinaryOutput(ID);
			for(Item = i = 0;i < MAX_IOUNIT_BO_SZ;i++, ioID++) {
				GetBinaryOutputName(ioID, temp);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				IDs[Item] = ioID; Item++;
			}
		}
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		if(type == 0) MenuBinaryInput(menu, ItemText, -1, IDs[rval-1]);
		else	MenuBinaryOutput(menu, ItemText, -1, IDs[rval-1]);
	}
	MenuTerminate(menu);
}

#endif

#ifndef STAND_ALONE
#include "adm_ac_hw.c"
#include "adm_ac_hw_item.c"
#include "adm_ac_hw_view.c"
#endif
#include "adm_ac_cr.c"
#include "adm_ac_ad.c"
#include "adm_ac_ap.c"
#include "adm_ac_in.c"
#include "adm_ac_out.c"
#ifndef STAND_ALONE
#include "adm_ac_az.c"
#endif
#include "adm_ac_fz.c"
#include "adm_ac_act.c"
#include "adm_ac_oc.c"
#include "adm_ac_ta.c"
#ifdef STAND_ALONE
//#include "adm_ac_meals.c"
#endif