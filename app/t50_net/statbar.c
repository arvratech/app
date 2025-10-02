#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "dlg.h"
#include "viewprim.h"
#include "actprim.h"
#include "lcdc.h"
#include "dev.h"
#include "hw.h"
#include "psu.h"
#include "ctfont.h"
#include "slvnet.h"
#include "wpaprim.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "appact.h"

#define STATBAR_TIMEOUT		15000

void StatBarOnCreate(GACTIVITY *act);
void StatBarOnStart(GACTIVITY *act);
void StatBarOnDestroy(GACTIVITY *act);
void StatBarOnTimer(GACTIVITY *act, int timerID);
void StatBarOnClick(GACTIVITY *act, void *view);
void StatBarOnAppMessage(GACTIVITY *act, unsigned char *msg);
void StatBarOnDown(GACTIVITY *act, void *view, CGPoint *pt);
void StatBarOnTabUp(GACTIVITY *act, void *view);
void StatBarOnScroll(GACTIVITY *act, void *view, CGPoint *pt);
void StatBarOnFlying(GACTIVITY *act, void *view, CGPoint *pt);

static int	sbY, sbMinHeight, sbMidHeight, sbMaxHeight;


void StatBarStart(int y)
{
	GACTIVITY	*act;

	act = CreateActivity(StatBarOnCreate);
	sbY = y;
	activitySetTimeout(act, 30);
	appStartActivityForResult(act, ACT_STATBAR, NULL, 0);
}

void StatBarOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rect;
	char	*p, temp[128];
	int		x, y, w, h, dy, val;

	activitySetOnDestroy(act, StatBarOnDestroy);
	activitySetOnTimer(act, StatBarOnTimer);
	activitySetOnClick(act, StatBarOnClick);
	activitySetOnAppMessage(act, StatBarOnAppMessage);
	activitySetOnDown(act, StatBarOnDown);
	activitySetOnTabUp(act, StatBarOnTabUp);
	activitySetOnScroll(act, StatBarOnScroll);
	activitySetOnFlying(act, StatBarOnFlying);
	activitySetViewBuffer(act, ViewAllocBuffer());
//PrintBuffer("StatBar");
	wnd = activityWindow(act);
	act->touchView = wnd;	// for activity change	
	sbMinHeight = 40; sbMidHeight = 100; sbMaxHeight = 350;
	x = 20; dy = 10; w = 220;
	UIframe(wnd, &rect);
	rect.x = 0; rect.y = 0; rect.height = sbMinHeight;
	UIsetFrame(wnd, &rect);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600);
	y = dy;
	v = ViewAddTitleImageTextButton(wnd, TAG_CONFIG_BTN, x, y, "settings.bmp", xmenu_top_title);
	UIframe(v, &rect);
	y += rect.height + dy;
	ftSetSize(20);
	h = ftHeight();
	// Model
	sprintf(temp, "%s: %s", admModelTitle(), devModelName(sys_cfg));
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	y += h + dy;
	// Firmware Version
	sprintf(temp, "%s: %s", admFirmwareVersionTitle(), syscfgFirmwareVersionName(sys_cfg));
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	y += h + dy;
 // Device Id
	p = temp;
	sprintf(p, "%s: ", admDeviceIdTitle()); p += strlen(p); devGetDeviceFullName(p);
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	y += h + dy;
	p = temp;
	sprintf(p, "%s: ", admSlvnetRemoteTitle()); p += strlen(p);
	if(SlvnetIsConnected()) sprintf(p, "%d", SlvnetMasterDevId());
	else    strcpy(p, wpaStateName(2));
    v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
    y += h + dy;
	// AC voltage 
	val = psuAcVoltageNow(NULL);
	sprintf(temp, "%s: %d.%02dv", admInputVoltageTitle(), val/1000, (val%1000)/10);
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	y += h + dy;
	// POE status
	sprintf(temp, "POE: %s", psuPoeStatusName(NULL));
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	y += h + dy;
	// Battery status
	p = temp;
	sprintf(p, "%s:", admBatteryTitle());
	p += strlen(p);
	val = psuBatStatus(NULL); 
	sprintf(p, " %s", psuBatStatusName(NULL)); 
	if(val) {
		p += strlen(p);
		sprintf(p, " %d%%", psuBatCapacity(NULL));
		p += strlen(p);
		val = psuBatVoltageNow(NULL);
		sprintf(p, " %d.%02dv", val/1000, (val%1000)/10);
	}
	v = ViewAddLabel(wnd, 0, temp, x, y, w, h, UITextAlignmentLeft);
	UItimerStart(0, STATBAR_TIMEOUT, 0);
}

void StatBarOnDestroy(GACTIVITY *act)
{
	UItimerStop(0);
	AdmActExit(act);
}

void StatBarOnTimer(GACTIVITY *act, int timerID)
{
	if(timerID == 0) {
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

void StatBarOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
//printf("StatBarOnCLick: tag=%d\n", tag);
	switch(tag) {
	case TAG_CONFIG_BTN:
		UItimerStop(0);
		AdmPassword(ACT_ADMPASSWORD);
		break;
	}
}

void AuthIdentify(GACTIVITY *act, int clearTop);

void StatBarOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	//wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		AuthIdentify(act, 1);
		break;
	case GM_TAMPER_NORMAL:
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	}
}

void StatBarOnDown(GACTIVITY *act, void *view, CGPoint *pt)
{
	sbY = sbMaxHeight;
	UItimerStop(0);
}

void _StatBarFinal(GACTIVITY *act, int y)
{
	void	*wnd;
	CGRect	rt;
	int		h, oh;

	if(y > sbMidHeight) {
		wnd = activityWindow(act);
		UIframe(wnd, &rt);
		oh = rt.height; h = sbMaxHeight;
		if(h != oh) {
			rt.height = h;
			UIsetFrame(wnd, &rt);
			h -= oh;
			rt.y = rt.height - h; rt.height = h;
			UIsetNeedsDisplayInRect(wnd, &rt); 
		}
		UItimerStop(0); UItimerStart(0, STATBAR_TIMEOUT, 0);
	} else {
		 DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
	}
}

void StatBarOnTabUp(GACTIVITY *act, void *view)
{
//printf("%u statBar: onTabup\n", MS_TIMER);
	_StatBarFinal(act, sbY);
}

void StatBarOnScroll(GACTIVITY *act, void *view, CGPoint *pt)
{
	void	*wnd;
	CGRect	rt, rt2;
	int		h, oh;

//printf("%u statBar: onScroll: dy=%d\n", MS_TIMER, (int)pt->y);
	sbY += pt->y;
	wnd = activityWindow(act);
	UIframe(wnd, &rt);
	oh = rt.height;
	if(sbY < sbMinHeight) h = sbMinHeight;
	else if(sbY > sbMaxHeight) h = sbMaxHeight;
	else	h = sbY;
	if(h != oh) {
		rt.height = h;
		UIsetFrame(wnd, &rt);
		h -= oh;
		if(h < 0) {
			rt2.x = rt2.y = 0; rt2.width = rt2.height = 0;
			UIsetNeedsDisplayInRect(wnd, &rt2);	// non-display 
			rt.y = rt.height; rt.height = -h;
//printf("drawAct [%d,%d %d %d].....\n", rt.x, rt.y, rt.width, rt.height);
			appDrawActivity(&rt); 
		} else {
			rt.y = rt.height - h; rt.height = h;
//printf("reDisplay [%d,%d %d %d].....\n", rt.x, rt.y, rt.width, rt.height);
			UIsetNeedsDisplayInRect(wnd, &rt); 
		}
	}
//printf("%u statBar: onScroll: end\n", MS_TIMER);
}

void StatBarOnFlying(GACTIVITY *act, void *view, CGPoint *pt)
{
	int		y;

//printf("%u statBar: onFlying: dy=%d\n", MS_TIMER, (int)pt->y);
	if(pt->y > 700) y = sbMidHeight+1;
	else if(pt->y < -700) y = sbMidHeight-1;
	else	y = sbY + (pt->y >> 1);
	_StatBarFinal(act, y);
//printf("%u statBar: onFlying: end\n", MS_TIMER);
}

