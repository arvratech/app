#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "gfont.h"
#include "gfonteng.h"
#include "symbol.h"
#include "lcdc.h"
#include "bmpc.h"
#include "key.h"
#include "iodev.h"
#include "gwnd.h"
#include "glabel.h"
#include "gedit.h"
#include "msg.h"
#include "user.h"
#include "fsuser.h"
#include "sche.h"
#include "cr.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "menuprim.h"
#include "topprim.h"
#include "wnds.h"

extern int	menu_errno;
extern int	gRequest;

GWND	*wndMsg, _wndMsg;
GLABEL	*lblMsg, _lblMsg;
GWND	*wndInput, _wndInput;
GLABEL	*lblInput, _lblInput;
GEDIT	*edtInput, _edtInput;
GDATE	*dateInput, _dateInput;
GIPADDR	*ipaddrInput, _ipaddrInput;


void WndsInitialize(void)
{
	wndMsg = &_wndMsg;
	wndInitialize(wndMsg, NULL, 35, 63, 250, 114); 
	lblMsg = &_lblMsg;
	labelInitialize(lblMsg, wndMsg, 10, 10, 220, 46); 

	wndInput = &_wndInput;
	wndInitialize(wndInput, NULL, 60, 63, 200, 114); 
	lblInput = &_lblInput;
	labelInitialize(lblInput, wndInput,  6, 10, 150, 24); 
	edtInput = &_edtInput;
	editInitialize(edtInput,  wndInput, 20, 38, 150, 38); 
	dateInput = &_dateInput;
	dateInitialize(dateInput, wndInput, 20, 38, 150, 38); 
	ipaddrInput = &_ipaddrInput;
	ipaddrInitialize(ipaddrInput, wndInput, 20, 38, 150, 38); 
}

void WizardWndInitialize(GWND *wnd, char *Title)
{
	wndInitialize(wnd, Title, 0, 0, lcdGetWidth(), lcdGetHeight()-32);
}

/*
static int _WndSaveConfig(GMENU *ParentMenu, char *Title, unsigned char *pVal, int NewVal, int Section, int SaveConfig)
{
	int		rval, OldVal;

	OldVal = *pVal;
	*pVal = (unsigned char)NewVal;
	if(NewVal == OldVal) rval = 0;
	else if(!SaveConfig) rval = 1;
	else {
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(ParentMenu, Title, R_DATA_SAVED);
			rval = 1;
		} else {
			*pVal = (unsigned char)OldVal;
			ResultMsg(ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}
*/


// Return	-1:System error  1:OK
int WndSaveConfig(GWND *ParentWnd, char *Title, int Section)
{
	int		rval;

	rval = syscfgWrite(sys_cfg);
	if(rval == 0) {
		ResultMsg(ParentWnd, Title, R_DATA_SAVED);
		rval = 1;
	} else {
		ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
		rval = -1;
	}
	return rval;
}

// modified 2009.5.6
// Return	-1:System error  2:OK(newVal=OldVal) 3:OK(newVal!=OldVal)
int WndSaveByte(GWND *ParentWnd, char *Title, unsigned char *pVal, int NewVal, int Section)
{
	int		rval, OldVal;

	OldVal = *pVal;
	*pVal = (unsigned char)NewVal;
	if(NewVal == OldVal) rval = 2;
	else if(!Section) rval = 3;
	else {
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(ParentWnd, Title, R_DATA_SAVED);
			rval = 3;
		} else {
			*pVal = (unsigned char)OldVal;
			ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}

// Return	-1:System error  2:OK(newVal=OldVal) 3:OK(newVal!=OldVal)
int WndSaveInteger(GWND *ParentWnd, char *Title, void *pVal, int Length, int NewVal, int Section)
{
	int		rval, OldVal;

	OldVal = GetIntValue(pVal, Length);
	SetIntValue(pVal, Length, NewVal);
	if(NewVal == OldVal) rval = 2;
	else if(!Section) rval = 3;
	else {
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(ParentWnd, Title, R_DATA_SAVED);
			rval = 3;
		} else {
			SetIntValue(pVal, Length, OldVal);	
			ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}

// Return	-1:System error  2:OK(newVal=OldVal) 3:OK(newVal!=OldVal)
int WndSaveByteData(GWND *ParentWnd, char *Title, unsigned char *Data, int Length, unsigned char *NewData, int Section)
{
	unsigned char	OldData[32];
	int		rval;

	memcpy(OldData, Data, Length);
	memcpy(Data, NewData, Length);
	if(!n_memcmp(NewData, OldData, Length)) rval = 2;
	else if(!Section) rval = 3;
	else {
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(ParentWnd, Title, R_DATA_SAVED);
			rval = 3;
		} else {
			memcpy(Data, OldData, Length);
			ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}

// Return	-1:System error  2:OK(newVal=OldVal) 3:OK(newVal!=OldVal)
int WndSaveString(GWND *ParentWnd, char *Title, char *Data, char *NewData, int Section)
{
	char	OldData[32];
	int		rval;

	strcpy(OldData, Data);
	strcpy(Data, NewData);
	if(!strcmp(NewData, OldData)) rval = 2;
	else if(!Section) rval = 3;
	else {
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(ParentWnd, Title, R_DATA_SAVED);
			rval = 3;
		} else {
			strcpy(Data, OldData);
			ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
			rval = -1;
		}
	}
	return rval;
}

void _DrawInputDigitDesp(GWND *wnd);

#include "wnds_uid.c"
#include "wnds_num.c"
#include "wnds_date.c"
#include "wnds_ipaddr.c"
#include "wnds_phone.c"
#ifdef STAND_ALONE
#include "wnds_cap.c"
#endif