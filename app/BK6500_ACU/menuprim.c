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
#include "gfont.h"
#include "symbol.h"
#include "lcdc.h"
#include "sche.h"
#include "usbd.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "sclib.h"
#include "user.h"
#include "cr.h"
#include "pno.h"
#include "msg.h"
#include "evt.h"
#include "ment.h"
#include "wnds.h"
#include "menuprim.h"
#include "topprim.h"

extern int		gRequest;
unsigned char	gAdminConsole;

GMENU_ITEM	gMenuItems[256];
int			gMenuItemCount;
extern GWND		*wndMsg;
extern GLABEL	*lblMsg;


void MenuSetAdminConsole(int Enable)
{
	if(Enable) gAdminConsole = 1;
	else	gAdminConsole = 0;
}

int MenuGetAdminConsole(void)
{
	return (int)gAdminConsole;
}

void MenuInitialize(GMENU *menu, char *Title, char *menuStr[], int ItemCount)
{
	GMENU_ITEM	*item, *startItem;
	int		i;
	
	if(menuStr && ItemCount) { 	
		startItem = gMenuItems + gMenuItemCount;
		for(i = 0, item = startItem;i < ItemCount;i++, item++) {
			item->Status = GMENU_ITEM_ENABLE;
			strcpy(item->Text, menuStr[i]);
			item->IconFile = 0;
			item->PutCallBack = NULL;
		}
		gMenuItemCount += ItemCount;
	} else	startItem = NULL;
	menuInitialize(menu, Title, startItem, ItemCount);
	menuSetSize(menu, 0, 0, lcdGetWidth(), lcdGetHeight()-32);
}

void MenuTerminate(GMENU *menu)
{
	gMenuItemCount -= menu->ItemCount;
	menu->ItemCount = 0;
}

char *MenuGetItemText(GMENU *menu, int Item)
{
	return menu->Items[Item].Text;
}

void MenuClearItems(GMENU *menu)
{
	gMenuItemCount -= menu->ItemCount;
	menu->Items = NULL;
	menu->ItemCount = 0;
}

void MenuAddItem(GMENU *menu, char *Text, int IconFile, int Status)
{
	GMENU_ITEM	*item;

	item = gMenuItems + gMenuItemCount;
	if(!menu->ItemCount) menu->Items = item;
	item->Status = Status;
	if(Text) strcpy(item->Text, Text);
	else	item->Text[0] = 0;
	item->IconFile = (unsigned short)IconFile;
	item->PutCallBack = NULL;
	gMenuItemCount++;
	menu->ItemCount++;
}

void MenuAddItemCallBack(GMENU *menu, int Status, void (*PutCallBack)())
{
	GMENU_ITEM	*item;

	item = gMenuItems + gMenuItemCount;
	if(!menu->ItemCount) menu->Items = item;
	item->Status = Status;
	item->Text[0] = 0;
	item->IconFile = (unsigned char)0;
	item->PutCallBack = PutCallBack;
	gMenuItemCount++;
	menu->ItemCount++;
}

void MenuShow(GMENU *menu)
{
	if(MenuGetAdminConsole()) menucShow(menu);
	else {
		_DrawLowerButtons(BTN_CANCEL, BTN_UP_ARROW, BTN_DOWN_ARROW, BTN_OK, 0);
		menuShow(menu);
	}
}

int MenuPopup(GMENU *menu)
{
	int		rval;

	if(MenuGetAdminConsole()) {
		rval = menucPopup(menu);
		if(menu_errno) rval = GWND_ERROR;
	} else {
		rval = menuPopup(menu);
	}
	if(rval == GWND_TIMEOUT) ResultMsg((GWND *)menu, menu->Title, R_INPUT_TIME_OVER);
	return rval;	
}

int MenuPopupEx(GMENU *menu, int Wizard, int Timeout)
{
	int		rval;

	if(MenuGetAdminConsole()) {
		rval = menucPopup(menu);
		if(menu_errno) rval = GWND_ERROR;
	} else {
		rval = menuPopupEx(menu, Wizard, Timeout);
	}
	if(rval == GWND_TIMEOUT) ResultMsg((GWND *)menu, menu->Title, R_INPUT_TIME_OVER);
	return rval;	
}

int MenuPopupGlobalExitOnly(GMENU *menu)
{
	int		rval;

	if(MenuGetAdminConsole()) {
		rval = menucPopup(menu);
		if(menu_errno) rval = GWND_ERROR;
	} else {
		rval = menuPopupGlobalExitOnly(menu);
	}
	if(rval == GWND_TIMEOUT) ResultMsg((GWND *)menu, menu->Title, R_INPUT_TIME_OVER);
	return rval;
}

unsigned char	*gIDs;

void _ListUserCallBack(GMENU *menu, int Item, int x, int y, void *Context)
{
	long		nID;
	char	*p, temp[32];
	int		font_h;

	font_h = GetFontHeight();
	p = temp;	
	sprintf(p, "[%06d] ", Item+1); p += strlen(temp);
	userDecodeID(&nID, gIDs + (Item << 2));
	sprintf(temp, "%ld", nID);		
	lcdPutText(x, y, temp, RGB(0,0,0));
}

int MenuUsers(GWND *ParentWnd, char *Title, unsigned char *dIDs, int UserSize, int curItem, int ViewOnly)
{
	GMENU	*menu, _menu;
	int		rval;

	gIDs = dIDs;
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, UserSize);
	if(ViewOnly) menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetSelectNumber(menu, 0);
	menuSetCurrentItem(menu, curItem);
	menuSetPutItemCallBack(menu, _ListUserCallBack);
	rval = MenuPopup(menu);
	return rval;
}

int _GetMenuItemHeight(void);

void _SetMenuProperSize(GMENU *menu)
{
	RECT	rect;
	int		i, x, y, w, h, fh, val;

	fh = GetFontHeight();
	if(menu->Title) w = GetStringWidth(menu->Title);
	else	w = 0;
	for(i = 0;i < menu->ItemCount;i++) {
		val = GetStringWidth(menu->Items[i].Text);
		if(menu->Items[i].IconFile) val += fh + 2;
		if(val > w) w = val;
	}
	if(menu->MenuStatus & GMENU_PREFIX_NUMBER) w += 16;
	if(menu->MenuType == GMENU_TYPE_CHECK) w += fh + 2;
	w += 8;		// border(4) + extra(4)
	wndGetClientArea(menu->ParentWnd, &rect);
	val = (rect.Width >> 1) + 40;
	if(w < val) w = val;
	x = rect.x + ((rect.Width - w) >> 1);
	y = rect.y + 8;
	h = rect.Height - 8;
	menuSetSize(menu, x, y, w, h);
	if(menu->PageCount > menu->ItemCount) {
		val = (menu->PageCount - menu->ItemCount) * _GetMenuItemHeight();
		h -= val; y += val >> 1;
		menuSetSize(menu, x, y, w, h);
	}
}

int WndMessageBoxYesNo(GWND *ParentWnd, char *Title, char *Text)
{
	int		rval;

	if(MenuGetAdminConsole()) {
		rval = wndcMessageBoxYesNo(Text);
		if(rval == 1) rval = IDYES;
		else if(rval == 2) rval = IDNO;
	} else {
		rval = wndMessageBox(ParentWnd, Text, Title, MB_YESNO);
	}
	return rval;	
}

void _SetSelectMenuColor(GMENU *menu)
{
//	menu->BackGroundColor = RGB( 64,252,164);	// 255 152 196
//	menu->TitleBarColor	  = RGB(  0,200, 80);	// 200  50   0
}

// Return	-1:Timeout, System error  0:Cancelled  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuRadio(GWND *ParentWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Section)
{
	GMENU	*menu, _menu;
	int		rval, NewVal, curItem;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title, menuStr, menuSize);
	menuSetParentWnd(menu, ParentWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(FromZero) curItem = NewVal; else curItem = NewVal - 1; 
	menuSetCurrentItem(menu, curItem);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	if(FromZero) rval--;
	NewVal = rval;
	return WndSaveByte(ParentWnd, Title, pVal, NewVal, Section);
}

// Return	-1:Timeout, System error  0:Cancelled  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuNoYes(GWND *ParentWnd, char *Title, unsigned char *pVal, int Mask, int Section)
{
	GMENU	*menu, _menu;
	int		rval, val, NewVal;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_noyes, 2);
	menuSetParentWnd(menu, ParentWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(NewVal & Mask) val = 1; else val = 0;
	menuSetCurrentItem(menu, val);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	rval--;
	if(rval) NewVal |= Mask;  else NewVal &= ~Mask;
	return WndSaveByte(ParentWnd, Title, pVal, NewVal, Section);
}

// Return	-1:Timeout, System error  0:Cancelled  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuYesNo(GWND *ParentWnd, char *Title, unsigned char *pVal, int Mask, int Section)
{
	GMENU	*menu, _menu;
	int		rval, val, NewVal;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_yesno, 2);
	menuSetParentWnd(menu, ParentWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(NewVal & Mask) val = 0; else val = 1;
	menuSetCurrentItem(menu, val);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	rval--;
	if(rval) NewVal &= ~Mask;  else NewVal |= Mask; 
	return WndSaveByte(ParentWnd, Title, pVal, NewVal, Section);
}

// Return	-1:Timeout, System error  0:Cancelled  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuCheck(GWND *ParentWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int Section)
{
	GMENU	*menu, _menu;
	int		i, rval, NewVal, mask;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, ParentWnd);
	NewVal = *pVal;	mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		MenuAddItem(menu, menuStr[i], 0, GMENU_ITEM_ENABLE);
		if(NewVal & mask) menuSetItemChecked(menu, i, 1);
		mask <<= 1;
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	NewVal = 0; mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		if(menuGetItemChecked(menu, i)) NewVal |= mask;
		mask <<= 1;
	}
	return WndSaveByte(ParentWnd, Title, pVal, NewVal, Section);
}

int MenuCheck2(GWND *ParentWnd, char *Title, unsigned long *pVal, char *menuStr[], int menuSize, int Section)
{
	GMENU	*menu, _menu;
	unsigned long	NewVal, mask;
	int		i, rval;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, ParentWnd);
	NewVal = *pVal;	mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		MenuAddItem(menu, menuStr[i], 0, GMENU_ITEM_ENABLE);
		if(NewVal & mask) menuSetItemChecked(menu, i, 1);
		mask <<= 1;
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	NewVal = 0; mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		if(menuGetItemChecked(menu, i)) NewVal |= mask;
		mask <<= 1;
	}
	return WndSaveInteger(ParentWnd, Title, pVal, 4, NewVal, Section);
}

void ScheduleItemChanged(GMENU *menu, int Item)
{
	unsigned char	cVal, *NewVal;
	char	temp[32];
	int		rval;

	if(Item == 3) {
		menuDarken(menu);
		MakeGuideNumberNormal(temp, 2, 254);
		NewVal = (unsigned char *)menuGetContext(menu);
		cVal = *NewVal;	
		rval = WndInputNumber((GWND *)menu, xmenu_other[14], temp, &cVal, 1, 2, 254, R_INVALID_DATA, 0);
		if(rval > 2) {
			*NewVal = cVal;
			n_utoa((unsigned long)cVal, temp);
			menuSetItemText(menu, Item, temp);
		}
		menuShow(menu);
	}
}

// Return	-1:Timeout, System error  0:Cancelled  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuInputSchedule(GWND *ParentWnd, char *Title, unsigned char *pSchedule, int Section)
{
	GMENU	*menu, _menu;
	unsigned char	cVal, NewVal;
	char	temp[2][20]; 
	int		rval, curItem;

	if(ParentWnd->WndType == 1) menuDarken((GMENU *)ParentWnd); 
	menu = &_menu;
	MenuInitialize(menu, Title,NULL, 0);
	menuSetParentWnd(menu, ParentWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	temp[0][0] = '-'; temp[0][1] = 0;
	MenuAddItem(menu, temp[0], 0, GMENU_ITEM_ENABLE);	 
	MenuAddItem(menu, xmenu_other[12], 0, GMENU_ITEM_ENABLE);
	MenuAddItem(menu, xmenu_other[13], 0, GMENU_ITEM_ENABLE);
	cVal = *pSchedule;
	if(cVal == 0xff) {
		curItem = 0; NewVal = 2;
	} else if(cVal < 2) {
		curItem = cVal+1; NewVal = 2;
	} else {
		curItem = 3; NewVal = cVal;
	}
	n_utoa((unsigned long)NewVal, temp[1]);
	MenuAddItem(menu, temp[1], 0, GMENU_ITEM_ENABLE);	
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	menuSetItemChanged(menu, ScheduleItemChanged);
	menuSetContext(menu, &NewVal);
	menuSetCurrentItem(menu, curItem);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	curItem = rval - 1;
	if(curItem == 0) NewVal = 0xff;
	else if(curItem < 3) NewVal = curItem - 1;
	return WndSaveByte(ParentWnd, Title, pSchedule, (int)NewVal, Section);
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int MenuRadioWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Wizard)
{
	GMENU	*menu, _menu;
	int		rval, NewVal, curItem;

	menu = &_menu;
	MenuInitialize(menu, Title, menuStr, menuSize);
	menuSetParentWnd(menu, WizardWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(FromZero) curItem = NewVal;  else curItem = NewVal - 1; 
	menuSetCurrentItem(menu, curItem);
	rval = MenuPopupEx(menu, Wizard, GWND_TIMER);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	if(FromZero) rval--;
	*pVal = (unsigned char)rval;
	if(menu->LastKey == KEY_F1) rval = 1; else if(menu->LastKey == KEY_F4) rval = 2; else rval = 3;
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int MenuRadioWizardEx(GWND *WizardWnd, RECT *rectWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int FromZero, int Wizard)
{
	GMENU	*menu, _menu;
	RECT	rect;
	int		rval, NewVal, curItem;

	menu = &_menu;
	MenuInitialize(menu, Title, menuStr, menuSize);
	menuSetParentWnd(menu, WizardWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	wndGetClientArea(WizardWnd, &rect);
	menuSetSize(menu, rect.x+rectWnd->x, rect.y+rectWnd->y, rectWnd->Width, rectWnd->Height);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(FromZero) curItem = NewVal;  else curItem = NewVal - 1; 
	menuSetCurrentItem(menu, curItem);
	rval = MenuPopupEx(menu, Wizard, GWND_TIMER);
	menuDarkenTitleBar(menu);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	if(FromZero) rval--;
	*pVal = (unsigned char)rval;
	if(menu->LastKey == KEY_F1) rval = 1; else if(menu->LastKey == KEY_F4) rval = 2; else rval = 3;
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int MenuNoYesWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, int Mask, int Wizard)
{
	GMENU	*menu, _menu;
	int		rval, val, NewVal;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_noyes, 2);
	menuSetParentWnd(menu, WizardWnd);
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_RADIO);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	NewVal = *pVal;
	if(NewVal & Mask) val = 1; else val = 0;
	menuSetCurrentItem(menu, val);
	rval = MenuPopupEx(menu, Wizard, GWND_TIMER);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	rval--;
	if(rval) NewVal |= Mask;  else NewVal &= ~Mask;
	*pVal = (unsigned char)NewVal;
	if(menu->LastKey == KEY_F1) rval = 1; else if(menu->LastKey == KEY_F4) rval = 2; else rval = 3;
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int MenuCheckWizard(GWND *WizardWnd, char *Title, unsigned char *pVal, char *menuStr[], int menuSize, int Wizard)
{
	GMENU	*menu, _menu;
	int		i, rval, NewVal, mask;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	menuSetParentWnd(menu, WizardWnd);
	NewVal = *pVal;	mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		MenuAddItem(menu, menuStr[i], 0, GMENU_ITEM_ENABLE);
		if(NewVal & mask) menuSetItemChecked(menu, i, 1);
		mask <<= 1;
	}
	menuModifyStyle(menu, 0L, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	menuSetType(menu, GMENU_TYPE_CHECK);
	menuSetSelectNumber(menu, 0);
	_SetMenuProperSize(menu);
	_SetSelectMenuColor(menu);
	menuSetCurrentItem(menu, 0);
	rval = MenuPopupEx(menu,  Wizard, GWND_TIMER);
	MenuTerminate(menu);
	if(rval <= 0) return rval;
	NewVal = 0; mask = 0x01;
	for(i = 0;i < menuSize;i++) {
		if(menuGetItemChecked(menu, i)) NewVal |= mask;
		mask <<= 1;
	}
	*pVal = (unsigned char)NewVal;
	if(menu->LastKey == KEY_F1) rval = 1; else if(menu->LastKey == KEY_F4) rval = 2; else rval = 3;
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int WndWizard(GWND *wnd, int Wizard)
{
	int		rval, c;
	
	rval = wndProcessWindow(wnd, Wizard, GWND_TIMER);
	if(rval > 0) {
		c = wnd->LastKey;
		if(c == KEY_F1) rval = 1; else if(c == KEY_F4) rval = 2; else rval = 3; 
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int EditWizard(GEDIT *edt, int Wizard)
{
	int		rval, c;
	
	rval = editProcessWindow(edt, Wizard);
	if(rval > 0) {
		editDrawWindow(edt);
		c = edt->LastKey;
		if(c == KEY_F1) rval = 1; else if(c == KEY_F4) rval = 2; else rval = 3; 
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int DateWizard(GDATE *date, int Wizard)
{
	int		rval, c;
	
	rval = dateProcessWindow(date, Wizard);
	if(rval > 0) {
		dateDrawWindow(date);
		c = date->Edit.LastKey;
		if(c == KEY_F1) rval = 1; else if(c == KEY_F4) rval = 2; else rval = 3; 
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int RadioGroupWizard(GRADIO_GROUP *rdGroup, int Wizard)
{
	int		rval, c;
	
	rval = radioGroupProcessWindow(rdGroup, Wizard);
	if(rval > 0) {
		c = radioGroupGetLastKey(rdGroup);
		if(c == KEY_F1) rval = 1; else if(c == KEY_F4) rval = 2; else rval = 3; 
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Back Key  2:Next Key  3:OK Key
int RadioGroupWizardEx(GRADIO_GROUP *rdGroup, int Wizard, void (*ChangedCallBack)(), void *Context)
{
	int		rval, c;

	rval = radioGroupProcessWindowEx(rdGroup, Wizard, ChangedCallBack, Context);
	if(rval > 0) {
		c = radioGroupGetLastKey(rdGroup);
		if(c == KEY_F1) rval = 1; else if(c == KEY_F4) rval = 2; else rval = 3; 
	}
	return rval;
}

/*
// Console Only
// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int MenuInputIPAddress(GWND *ParentWnd, char *Title, unsigned char *IPAddr, int SubnetMask, int Section)
{
	GMENU	*menu, _menu;
	unsigned char	NewIPAddr[4];
	char	buf[20];
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);
	MenuShow(menu); if(menu_errno) return GWND_ERROR;
	memcpy(NewIPAddr, IPAddr, 4);
	cPutTextLine(inet2addr(NewIPAddr)); if(menu_errno) return GWND_ERROR;
	buf[0] = 0;	
	rval = menucInputString("Input: ", buf, 15);
	if(rval == 1) rval = 0;
	if(rval > 0) {
		rval = addr2inet(buf, NewIPAddr);
		if(rval < 0 || CheckIPAddr(NewIPAddr, SubnetMask)) rval = 1;
		else	rval = 4;
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	if(rval < 4) {
		ResultMsg(ParentWnd, Title, R_INVALID_IP_ADDRESS);
		rval = 1;
	} else {
		rval = WndSaveByteData(ParentWnd, Title, IPAddr, 4, NewIPAddr, Section);
		if(rval > 2) gRequest |= G_TCP_IP_CHANGED;
	}
	return rval;
}

*/                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                 