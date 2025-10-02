#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "syscfg.h"
#include "iodev.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "user.h"
#include "cr.h"
#include "gfont.h"
#include "user.h"
#include "msg.h"
#include "lang.h"
#include "usbd.h"
#include "menuprim.h"
#include "wnds.h"
#include "devprim.h"
#include "topprim.h"
#include "desktop.h"
#include "adm.h"

extern CREDENTIAL_READER _gCredentialReaders[];
extern FP_USER	*gUser;


// Return	-2=Error  -1=Timeout  0=Cancelled  1=Invalid data/Fail(non-matched)  2=OK(matched)
int MenuAdminAuth(void)
{
	CREDENTIAL_READER *cr;
	char	Title[32], pwd[20];
	int		rval, msg_no;

	cr = &_gCredentialReaders[0];
	strcpy(Title, xmenu_top_title);
//	if(apGetFPReader(ap) && !CheckTmplTopHeader(sys_cfg->AdminFPTemplate)) ap->WantMode |= USER_FP;
	if(MenuGetAdminConsole()) {
		rval = menucInputPassword(NULL, pwd, 8);			
	} else { 
		rval = DesktopEnterPassword(Title, pwd);
	}	
	if(rval > 0) {
		if(!strcmp(sys_cfg->AdminPIN, pwd)) rval = 2;
		else {
			rval = 1;
			if(MenuGetAdminConsole()) ResultMsg(NULL, Title, msg_no);
			else	DesktopResult(Title, R_PW_MISMATCHED);
		}
	} else {
		DesktopDrawMain();
	}
	return rval;
}

void MenuAdm_User(GMENU *ParentMenu, char *Title);
void MenuAdm_AccessControl(GMENU *ParentMenu, char *Title);
void MenuAdm_Network(GMENU *ParentMenu, char *Title);
void MenuAdm_Setup(GMENU *ParentMenu, char *Title);
void MenuAdm_System(GMENU *ParentMenu, char *Title);

void MenuAdm(void)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval, i;

	DesktopEnterMenu();
	menu = &_menu;
	MenuInitialize(menu, NULL, NULL, 0);	// menu, Title, MenuStr, Size
	while(1) {
		MenuClearItems(menu);
		menuSetWindowText(menu, xmenu_top_title);
		for(i = 0;i < 5;i++) MenuAddItem(menu, xmenu_top[i], 0, GMENU_ITEM_ENABLE);	
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuAdm_User(menu, ItemText);
			break;
		case 2:
			MenuAdm_AccessControl(menu, ItemText);	
			break;
		case 3:
			MenuAdm_Network(menu, ItemText);
			break;
		case 4:
			MenuAdm_Setup(menu, ItemText);
			break;
		case 5:
			MenuAdm_System(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
	DesktopExitMenu();
}

void _SetOptionYesNo(char *buf, char *MsgString, unsigned char Data, int OptionFlag)
{
	if(Data & OptionFlag) sprintf(buf, "%s=Y", MsgString);
	else	sprintf(buf, "%s=N", MsgString);
}

void _SetValueYesNo(char *buf, char *MsgString, unsigned char Data)
{
	if(Data) sprintf(buf, "%s=Y", MsgString);
	else	sprintf(buf, "%s=N", MsgString);
}

void _SetValueAsterisk(char *buf, char *MsgString, unsigned char Data)
{
	if(Data) sprintf(buf, "%s*", MsgString);
	else	sprintf(buf, "%s", MsgString);
}
