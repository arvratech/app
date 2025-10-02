#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "gfont.h"
#include "gfonteng.h"
#include "symbol.h"
#include "bmpc.h"
#include "lcdc.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "lang.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "sdi.h"
#include "user.h"
#include "pno.h"
#include "cr.h"
#include "acad.h"
#include "acap.h"
#include "ta.h"
#include "evt.h"
#include "menuprim.h"
#include "wnds.h"
#include "adm.h"
#include "devprim.h"
#include "topprim.h"
#include "desktop.h"

extern int		gRequest, gIOStatus;
extern unsigned char gSound;
static GWND 	*wndResult, _wndResult, *wndStat, _wndStat;
static GLABEL	*lblStat, _lblStat;
static unsigned char _minute, desktop_flag;
static int		DesktopColor;


void WndSetBlackWhiteMode(GWND *wnd)
{
	wndSetBackGroundColor(wnd, RGB(0,0,0));
	wndSetBackGroundTextColor(wnd, RGB(255,255,255));
	wndSetTitleBarColor(wnd, RGB(0,0,0));
	wndSetTitleBarTextColor(wnd, RGB(255,255,255));
}

void EditSetBlackWhiteMode(GEDIT *edt)
{
	editSetBackGroundColor(edt, RGB(0,0,0));
	editSetForeGroundColor(edt, RGB(255,255,255));
}

void LabelSetBlackWhiteMode(GLABEL *lbl)
{
	labelSetForeGroundColor(lbl, RGB(255,255,255));
}

void DesktopInitialize(void)
{
	DesktopColor = RGB(220,220,220);
	wndResult = &_wndResult;
	wndInitialize(wndResult, NULL, 0, 32, lcdGetWidth(), lcdGetHeight()-64); 
	wndModifyStyle(wndResult, WS_CAPTION | WS_BORDER | WS_VISIBLE, 0);
	wndStat = &_wndStat;
	wndInitialize(wndStat, NULL, 60, lcdGetHeight()-32-46, 250, 46); 
	wndModifyStyle(wndStat, WS_CAPTION | WS_VISIBLE | WS_BORDER, 0);
	lblStat = &_lblStat;
	labelInitialize(lblStat, wndStat, 4, 0, 246, 46);
	labelSetLinePitch(lblStat, 0);
	labelSetWindowText(lblStat, "");
	DesktopSetColorMode();
	desktop_flag = 0x01;
}

int DesktopGetColor(void)
{
	return DesktopColor;
}

void DesktopSetColorMode(void)
{
	if(IsBlackWhiteMode()) { 
		WndSetBlackWhiteMode(wndResult);
		WndSetBlackWhiteMode(wndStat);
	} else {
		wndSetDefaultColor(wndResult);
		wndSetDefaultColor(wndStat);
		wndSetBackGroundColor(wndStat, RGB(255,128,96));
		wndSetTitleBarTextColor(wndStat, RGB(0,0,0));
	}
}

int DesktopIsIdle(void)
{
	if(wndIsVisible(wndResult)) return 0;
	else	return 1;
}	

int DesktopProcess(void)
{
	int		rval;
	
	rval = 0;
	if(wndIsVisible(wndResult) && wndResult->Timer && DS_TIMER > wndResult->Timer) {	
		DesktopDrawMain();
		rval = 1;
	} 
	return rval;
}

void DesktopDrawAll(void)
{
	lcdHoldRefresh();
	DesktopDrawBackGround();
	DesktopDrawClock();
	if(wndIsVisible(wndStat)) DesktopDrawStat();
	wndHideWindow(wndResult);
	DesktopSetIconsVisible(1);	
	DesktopDrawIcons();
	MenuSetViewEnable(1);
	MenuViewMain();
	lcdRefresh(0, 0, lcdGetWidth(), lcdGetHeight());
}

void DesktopDrawMain(void)
{
	lcdHoldRefresh();
	if(wndIsVisible(wndResult)) DesktopRestoreWindow(wndResult);
	wndHideWindow(wndResult);		
	if(wndIsVisible(wndStat)) DesktopDrawStat();
	MenuSetViewEnable(1);
	MenuViewMain();
	lcdRefresh(0, 24, lcdGetWidth(), lcdGetHeight()-24);	
}

void DesktopClearMain(void)
{
	MenuSetViewEnable(0);
	lcdSetClipRect(0, 24, lcdGetWidth(), lcdGetHeight()-24);
	if(wndIsVisible(wndStat)) DesktopDrawStat();
	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

void DesktopEnterMenu(void)
{
	DesktopSetIconsVisible(0);
//	MentUserNormal(120);	// µùµ¿
}

void DesktopExitMenu(void)
{
//	MentUserNormal(121);	// ¶¯-	
	DesktopDrawAll();
}

void DesktopInitClock(void)
{
}

static void _DrawClock(unsigned char *ctm)
{
	char	*p, temp[20];

	_minute = ctm[4];
	p = temp;
	utoa02((unsigned int)ctm[0], p); p += 2;
	*p++ = '/';
	utoa02((unsigned int)ctm[1], p); p += 2;
	*p++ = '/';
	utoa02((unsigned int)ctm[2], p); p += 2;
	*p++ = ' ';
	utoa02((unsigned int)ctm[3], p); p += 2;
	*p++ = ':';
	utoa02((unsigned int)ctm[4], p); p += 2;
	*p = 0;
	lcdFillRect(180, 2, 136, 20, DesktopColor);
	lcdPutText(180, 2, temp, 0);		
}

void DesktopDrawClock(void)
{
	unsigned char	ctm[8];

	rtcGetDateTime(ctm);
	_DrawClock(ctm);
}

void DesktopOnClockBlink(void)
{
	unsigned char	ctm[8];

	rtcGetDateTime(ctm);
	if(!(gRequest & G_DATE_TIME_UPDATED) && _minute == ctm[4]) {
		lcdPutEngChar(180+108, 2, ':', 0); 	
	} else {
		if(gRequest & G_DATE_TIME_UPDATED) gRequest &= ~G_DATE_TIME_UPDATED;
		_DrawClock(ctm);
	}
}

void DesktopOffClockBlink(void)
{
	lcdFillRect(180+108, 2, 6, 20, DesktopColor);
}

void DesktopSetStat(char *Text)
{
	RECT	rect;
	int		w, h;
		
	if(!Text || !Text[0]) {
		wndHideWindow(wndStat);
		labelSetWindowText(lblStat, "");
	} else {
		labelSetWindowText(lblStat, Text);
		labelGetTextExtent(lblStat, &rect);
		w = rect.Width + 2; h = rect.Height + 4;
		labelSetSize(lblStat, 4, 0, w, h);
		w += 8;
		wndSetSize(wndStat, lcdGetWidth()-w-8, lcdGetHeight()-32-h, w, h);
		wndShowWindow(wndStat);	
	}
}

void DesktopDrawStat(void)
{	
	wndDrawWindow(wndStat);
	labelDrawWindow(lblStat);
}

#include "fsuser.h"
#include "jpeg_api.h"

char remote_login[16] = "Remote Login...";	// width=146

void DesktopRemoteLogin(void)
{
	GWND	*wnd, _wnd;
	RECT	rect;
	int		x, y;

	DesktopClearMain();
	wnd = &_wnd;
	wndInitialize(wnd, NULL, 5, 86, lcdGetWidth()-10, 92);
	wndModifyStyle(wnd, WS_CAPTION | WS_VISIBLE, WS_BORDER);
	wndGetClientArea(wnd, &rect);
	x = rect.x + 2; y = rect.y + 22;
	wndDrawWindow(wnd);
	lcdPutTextDouble(x, y, remote_login, wndGetBackGroundTextColor(wnd), MASK_HORIZONTAL_DOUBLE|MASK_VERTICAL_DOUBLE);
}

void DesktopRestoreWindow(GWND *wnd)
{
	RECT	rect;
	
	wndGetDrawArea(wnd, &rect);
	lcdSetClipRect(rect.x, rect.y, rect.Width, rect.Height);
	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

void DesktopRestoreRect(RECT *rect)
{
	lcdSetClipRect(rect->x, rect->y, rect->Width, rect->Height);
	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

void DesktopRestoreArea(int x, int y, int Width, int Height)
{
	lcdSetClipRect(x, y, Width, Height);
	DesktopDrawBackGround();
	lcdSetDefaultClipRect();
}

void DesktopDrawBackGround(void)
{
	if(IsBlackWhiteMode()) lcdClear(RGB(0,0,0));
	else	lcdClear(DesktopColor);	
}

#include "desktop_modu.c" 
#include "desktop_icon.c"