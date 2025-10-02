#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "gfont.h"
#include "symbol.h"
#include "lcdc.h"
#include "iodev.h"
#include "syscfg.h"
#include "acad.h"
#include "acap.h"
#include "fsuser.h"
#include "gwnd.h"
#include "glabel.h"
#include "gedit.h"
#include "msg.h"
#include "cr.h"
#include "sysdep.h"
#include "menuprim.h"
#include "wnds.h"
#include "topprim.h"
#include "adm.h"

extern FP_USER	*gUser;
extern GWND		*wndMsg;
extern GLABEL	*lblMsg;


void _AddAdmUserStatistics(GMENU *menu)
{
	char	temp[80];

	sprintf(temp, "%s=%d", xmenu_other[2], userfsGetCount());
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_other[3], userfsGetFPTemplateCount());
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
	sprintf(temp, "%s=%d", xmenu_other[23], fsEventSize() >> 4);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);
}

static void _PutAdmUserStatistics(char *Title)
{
	GMENU	*menu, _menu;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	_AddAdmUserStatistics(menu);
	MenuShow(menu);
	MenuTerminate(menu);	
}

void MenuNewUser(GMENU *ParentMenu, char *Title);
void MenuBatchNewUser(GMENU *ParentMenu, char *Title);
void MenuEditUser(GMENU *ParentMenu, char *Title);
void MenuDeleteUser(GMENU *ParentMenu, char *Title);
void MenuDeleteAllUsers(GMENU *ParentMenu, char *Title);

void MenuAdm_User(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_user, 5);	// menu, Title, MenuStr, Size
	while(1) {
		if(MenuGetAdminConsole()) _PutAdmUserStatistics(Title);
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1: MenuNewUser(menu, ItemText); break;
		case 2: MenuBatchNewUser(menu, ItemText); break;
		case 3: MenuEditUser(menu, ItemText); break;
		case 4: MenuDeleteUser(menu, ItemText); break;
		case 5: MenuDeleteAllUsers(menu, ItemText); break;
		}
	}
	MenuTerminate(menu);
}

int  EnrollWizardUser(FP_USER *user, int FromStage, int NewUser, char *Title);

#ifndef STAND_ALONE
void _GetUserFPTemplates(FP_USER *user)
{
	//if(user) userfsGetFPTemplate(user, OldStoredTmpls);
}
#endif

void MenuNewUser(GMENU *ParentMenu, char *Title)
{
	int		rval, mode;

	AdminConsoleUnsupported();
	mode = USER_CARD;
	gUser->ID = 0L;
	while(1) {
		rval = WndEnterAutoNewID((GWND *)ParentMenu, Title, &gUser->ID);
		if(rval < 2) return;
		userSetDefault(gUser);
		gUser->AccessMode = mode;
		_GetUserFPTemplates(NULL);
		EnrollWizardUser(gUser, 1, 1, Title);
		mode = gUser->AccessMode;
		MenuShow(ParentMenu); 
	}
}

void MenuBatchNewUser(GMENU *ParentMenu, char *Title)
{
	int		rval, mode;

	AdminConsoleUnsupported();
	mode = USER_CARD;
	gUser->ID = 0;
	while(1) {
		rval = WndEnterAutoNewID((GWND *)ParentMenu, Title, &gUser->ID);
		if(rval < 2) break;
		userSetDefault(gUser);
		gUser->AccessMode = mode;
		_GetUserFPTemplates(NULL);
		rval = EnrollWizardUser(gUser, 1, 1, Title);
		MenuShow(ParentMenu); 
		if(rval > 1) break;
		mode = gUser->AccessMode;
	}
	if(rval < 2) return;
	mode = gUser->AccessMode;
	while(1) {
		rval = WndEnterAutoNewID((GWND *)ParentMenu, Title, &gUser->ID);
		if(rval < 2) break;
		userSetDefault(gUser);
		gUser->AccessMode = mode;
		_GetUserFPTemplates(NULL);
		EnrollWizardUser(gUser, 2, 1, Title);
		MenuShow(ParentMenu);
	}
}

static void _MakeEditUser(FP_USER *user)
{
	userfsGet(user);
	_GetUserFPTemplates(user);
}

void MenuEditUser(GMENU *ParentMenu, char *Title)
{
	int		rval;

	AdminConsoleUnsupported();
	while(1) {
		rval = WndEnterID((GWND *)ParentMenu, Title, &gUser->ID, 0, 0);
		if(rval < 2) break;
		_MakeEditUser(gUser);
		EnrollWizardUser(gUser, 2, 0, Title);
		MenuShow(ParentMenu);
	}
}

void MenuDeleteUser(GMENU *ParentMenu, char *Title)
{
	long	nID;
	char	*p, buf[80];
	int		rval;

	AdminConsoleUnsupported();
	while(1) {
		rval = WndEnterID((GWND *)ParentMenu, Title, &nID, 0, 0);
		if(rval < 2) break;
		p = buf;
		if(GetBidi()) sprintf(p, "%ld :ID", nID);
		else	sprintf(p, "ID: %ld", nID);
		p += strlen(p); *p++ = '\n'; strcpy(p, xmsgs[M_DETETE_CONFIRM]);
		menuShow(ParentMenu);
		rval = wndMessageBox((GWND *)ParentMenu, buf, Title, MB_YESNO);
		if(rval == IDYES) {
			ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING); if(menu_errno) return;
			wdtResetLong();
			rval = userfsRemove(nID);
			wdtReset();
			if(rval == 0) ResultMsg((GWND *)ParentMenu, Title, R_USER_NOT_FOUND);
			else if(rval < 0) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			else	ResultMsg((GWND *)ParentMenu, Title, R_USER_DELETED);
		}
	}
}

void MenuDeleteAllUsers(GMENU *ParentMenu, char *Title)
{
	long	nID;
	char	*p, buf[80];
	int		rval;

	AdminConsoleUnsupported();
	p = buf;
	rval = userfsGetCount();
	sprintf(p, "* %d", rval);
	p += strlen(p); *p++ = '\n'; strcpy(p, xmsgs[M_DETETE_CONFIRM]);
	menuShow(ParentMenu);
	rval = wndMessageBox((GWND *)ParentMenu, buf, Title, MB_YESNO);
	if(rval == IDYES) {
		ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING); if(menu_errno) return;
		wdtResetLong();
		rval = userfsRemoveAll();
		wdtReset();
		if(rval == 0) ResultMsg((GWND *)ParentMenu, Title, R_USER_NOT_FOUND);
		else if(rval < 0) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		else	ResultMsg((GWND *)ParentMenu, Title, R_ALL_USER_DELETED);
	}
}
/*
#include "key.h"
#include "cam.h"
#include "jpeg_api.h"


void _EnrollUserPhoto(FP_USER *user, char *Title);

void MenuEnrollUserPhoto(GMENU *ParentMenu, char *Title)
{
	char	temp[60];
	int		rval;

	AdminConsoleUnsupported();
	while(1) {	
		rval = WndEnterID((GWND *)ParentMenu, Title, &gUser->ID, 0, 0);
		if(rval < 2) break;
		sprintf(temp, "%s: %ld", Title, gUser->ID);
		_EnrollUserPhoto(gUser, temp);
		MenuShow(ParentMenu);
	}
}

unsigned char _Photo[38400];
int CaptureUserPhoto(JPEG *jpeg, unsigned char *JPEGBuffer, unsigned char *YUVBuffer);

void _EnrollUserPhoto(FP_USER *user, char *Title)
{
	GWND	*wnd, _wnd;
	BMPC	*bmpLcd, _bmpLcd, *bmpPhoto, _bmpPhoto;
	JPEG	*jpeg, _jpeg;
	unsigned char	*p, *YUVBuffer;
	unsigned long	CamFrameCount, timer;
	char	temp[60];
	int		c, rval, size, c_state, p_state, runned;
	
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	_DrawLowerButtons(BTN_CANCEL, 0, 0, 0, 0);
	timer = DS_TIMER + 50;
	c_state = camGetCodecState();	
	if(c_state == CAM_STOPPED || c_state == CAM_NULL) {
		runned = 0;
		rval = RunCam();
	} else {
		runned = 1;
		rval = 1;
	}
	if(rval > 0) {
		camRunPreview();
		while(1) {
			PromptMsg(wnd, Title, M_WAIT_A_MOMENT);
			c = keyGet();
			c_state = camGetCodecState();
			p_state = camGetPreviewState();
			if(c == KEY_F1 || c == KEY_CANCEL) rval = 0;
			else if(c_state == CAM_RUNNING && p_state == CAM_RUNNING) rval = 2;
			else if(DS_TIMER > timer) rval = 1;
			else	rval = 9;
			if(rval < 9) break;
			taskYield();
		}
	}
	if(rval < 2 && rval) {	// -1:Error 1:Timeout
		GetResultWithoutNewLine(temp, R_FAILED);
		ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "Camera isn't exist\n%s", temp);
	}
	if(rval < 2) {
		if(rval >= 0) {
			if(!runned) camStopCodec();
			camStopPreview();
		}
		return;
	}
	p = GetMainBuffer();
	bmpLcd = &_bmpLcd;
	bmpcInit(bmpLcd, p, lcdGetHeight(), lcdGetWidth());
	p += bmpcGetHeight(bmpLcd)*bmpcGetWidth(bmpLcd) * 2;
	bmpPhoto = &_bmpPhoto;
	bmpcInit(bmpPhoto, p, 120, 160);
	p += bmpcGetHeight(bmpPhoto)*bmpcGetWidth(bmpPhoto) * 2;
	YUVBuffer = p; p += 115200 + 4096;
	user->JPEGBuffer = p;
	jpeg = &_jpeg;
	jpeg->Width = bmpcGetWidth(bmpPhoto); jpeg->Height = bmpcGetHeight(bmpPhoto);
	wndDrawWindow(wnd);
	_DrawLowerButtons(BTN_CANCEL, 0, CAM_RUNNING_ICON, 0, 0);
	size = 0;
	CamFrameCount = 0L;
	rval = userfsGetPhoto(user);
	if(rval > 0 && user->Width && user->Height) {
		jpeg->Width = user->Width; jpeg->Height = user->Height; jpeg->QuantityLevel = user->QuantityLevel;	
		jpegDecodeBodyToYUV420(jpeg, user->JPEGBuffer, YUVBuffer);
		bmpcPutYUV420(bmpPhoto, YUVBuffer);
		lcdPut(175, 40, bmpPhoto);
	}
	while(1) {
		c_state = camGetCodecState();
		p_state = camGetPreviewState();
		if(c_state != CAM_RUNNING || p_state != CAM_RUNNING) break;
		rval = camCapturePreview(bmpLcd->Buffer, &CamFrameCount, 1);
		if(rval > 0) {
			bmpcResuceHalf(bmpPhoto, bmpLcd);
			lcdPut(25, 40, bmpPhoto);
		}
		c = keyGet();
		if(c == KEY_F1 || c == KEY_CANCEL || size > 0 && (c == KEY_F4 || c == KEY_OK)) break;
		else if(c == KEY_F3) {
			size = CaptureUserPhoto(jpeg, user->JPEGBuffer, YUVBuffer);
cprintf("QL=%d Size=%d\n", (int)jpeg->QuantityLevel, size);
			jpegDecodeBodyToYUV420(jpeg, user->JPEGBuffer, YUVBuffer);
			bmpcPutYUV420(bmpPhoto, YUVBuffer);
			lcdPut(175, 40, bmpPhoto);
			_DrawLowerButtons(BTN_CANCEL, 0, CAM_RUNNING_ICON, BTN_OK, 0);
		}
		taskYield();
	}
	if(!runned) camStopCodec();
	camStopPreview();
	if(c_state != CAM_RUNNING || p_state != CAM_RUNNING) {
		GetResultWithoutNewLine(temp, R_FAILED);
		ResultMsgEx(wnd, Title, 30, MB_ICONERROR, "%s", GetResultMsg(R_FAILED));
	} else if(size > 0 && (c == KEY_F4 || c == KEY_OK)) {
		user->Width = jpeg->Width; user->Height = jpeg->Height; user->QuantityLevel = jpeg->QuantityLevel;
		user->JPEGSize = size;
		user->UserName[0] = 0; user->UserExternalID[0] = 0;
		rval = userfsAddEx(user);
		if(rval < 0) ResultMsg(wnd, Title, R_MSG_SYSTEM_ERROR);
		else	ResultMsg(wnd, Title, R_DATA_SAVED);
	}
}

#define PHOTO_BODY_SZ	2036	// 2048-12

int _CaptureYUV(JPEG *jpeg, unsigned char *YUVBuffer);

int CaptureUserPhoto(JPEG *jpeg, unsigned char *JPEGBuffer, unsigned char *YUVBuffer)
{
	int		size, level;
	
	_CaptureYUV(jpeg, YUVBuffer);
	jpeg->Bits = 24;
	level = 5;	
	do {
		jpeg->QuantityLevel = (unsigned char)level;
		size = jpegEncodeBodyFromYUV420(jpeg, JPEGBuffer, YUVBuffer);
		if(size <= PHOTO_BODY_SZ) break;
		level--;
	} while(level > 0) ;
	if(level > 0) return size;
	else	return 0;
}

int _CaptureYUV(JPEG *jpeg, unsigned char *YUVBuffer)
{
	unsigned long	CamFrameCount;
	unsigned char	*s1, *s2, *d;
	int		i, j, c, w, width, height;
	
	CamFrameCount = 0L;
	d = YUVBuffer;
	s2 = d + 4096;
	camCaptureCodec(s2, &CamFrameCount, 1);
	width = jpeg->Width; height = jpeg->Height;
	w = ((width + 15) >> 4) << 4; 
	for(i = 0;i < height;i++) {
		s1 = s2; s2 += width + width;
		for(j = 0;j < width;j++) {
			c = *s1 + *s2 + *(s1+1) + *(s2+1);
			*d++ = c >> 2;
			s1 += 2; s2 += 2;
		}
		c = *(d-1);
		for( ;j < w;j++) *d++ = c;
	}
	width >>= 1; height >>= 1; w >>= 1;
	for(i = 0;i < height;i++) {
		s1 = s2; s2 += width + width;
		for(j = 0;j < width;j++) {
			c = *s1 + *s2 + *(s1+1) + *(s2+1);
			*d++ = c >> 2;
			s1 += 2; s2 += 2;
		}
		c = *(d-1);
		for( ;j < w;j++) *d++ = c;
	}
	for(i = 0;i < height;i++) {
		s1 = s2; s2 += width + width;
		for(j = 0;j < width;j++) {
			c = *s1 + *s2 + *(s1+1) + *(s2+1);
			*d++ = c >> 2;
			s1 += 2; s2 += 2;
		}
		c = *(d-1);
		for( ;j < w;j++) *d++ = c;
	}
	return 0;
}
*/