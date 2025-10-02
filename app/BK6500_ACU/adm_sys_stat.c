#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "sysdep.h"
#include "syscfg.h"
#include "sio.h"
#include "sinport.h"
#include "slavenet.h"
#include "menuprim.h"
#include "wnds.h"
#include "topprim.h"
#include "desktop.h"
#include "admprim.h"
#include "adm.h"


void MenuViewCommStatistics(GMENU *ParentMenu, char *Title);

void MenuCommStatistics(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText, temp[64];
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_comm_stats, 2);
	menu->MenuStatus |= GMENU_HIDDEN_ITEM;
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuViewCommStatistics(menu, ItemText);
			break;
		case 2:
			sprintf(temp, "%s", GetPromptMsg(M_ARE_YOU_REALLY));
			rval = WndMessageBoxYesNo((GWND *)ParentMenu, ItemText, temp);
			if(rval == IDYES) {
				SlaveSioResetCommStatistics();
				ResultMsg((GWND *)ParentMenu, ItemText, R_PROCESSED);
			}		
			break;
		}
	}
	MenuTerminate(menu);
}

void MenuViewCommStatistics(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	SLAVE	*slv;
	unsigned char	IDAddr;
	char	*p, temp[60];
	int		i, rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);

	sprintf(temp, "RS-485");
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, " Overrun Error: %ld", sioGetOverrunError(0));
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, " Paraty Error  : %ld", sioGetParityError(0));
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, " Frame Error  : %ld", sioGetFrameError(0));
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, " Break Detect  : %ld", sioGetBreakDetect(0));
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "Slaves: CommErr CRCErr Timeout");
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	temp[0] = ' ';
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) {
			if(unitGetEnable(i)) {
				IDAddr = 0x60 + i;
				slv = SioGetSlave(IDAddr);
				if(slv) {
					p = temp + 1;
					GetUnitName(i, p); p += strlen(p); *p++ = ':'; sprintf(p, " %ld %ld %ld", slv->CommErrorCount, slv->CRCErrorCount, slv->TimeoutCount);
					MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				}	
			}
		} else {
			if(i > 1 && !(i & 1) && (unitGetEnable(i) || unitGetEnable(i+1))) {
				IDAddr = i;
				slv = SioGetSlave(IDAddr);
				if(slv) {
					p = temp + 1;
					GetUnitName(i, p); p += strlen(p); p -= 2; *p++ = ' '; *p++ = ' '; *p++ = ' '; *p++ = ':'; sprintf(p, " %ld %ld %ld", slv->CommErrorCount, slv->CRCErrorCount, slv->TimeoutCount);
					MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				}	
			}
			if(crGetType(i)) {
				IDAddr = 0x20 + i;
				slv = SioGetSlave(IDAddr);
				if(slv) {
					p = temp + 1;
					GetUnitName(i, p); p += strlen(p); *p++ = ':'; sprintf(p, " %ld %ld %ld", slv->CommErrorCount, slv->CRCErrorCount, slv->TimeoutCount);
					MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
				}	
			}
		}
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		if(iounitGetEnable(i)) {
			IDAddr = 0x40 + i;
			slv = SioGetSlave(IDAddr);
			if(slv) {
				p = temp + 1;
				GetIOUnitName(i, p); p += strlen(p); *p++ = ':'; sprintf(p, " %ld %ld %ld", slv->CommErrorCount, slv->CRCErrorCount, slv->TimeoutCount);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
			}	
		}
	}

//	rval = MenuPopupGlobalExitOnly(menu);
	rval = MenuPopup(menu);
	MenuTerminate(menu);
}
