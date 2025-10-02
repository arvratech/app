#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "vox.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "dlg.h"
#include "topprim.h"
#include "admprim.h"
#include "actprim.h"
#include "exec.h"
#include "appact.h"


void BgScreenOnCreate(GACTIVITY *act);
void BgScreenOnDestroy(GACTIVITY *act);
void BgScreenOnAppMessage(GACTIVITY *act, unsigned char *msg);
void BgScreenOnTimer(GACTIVITY *act, int timerID);;
void BgScreenOnDown(GACTIVITY *act, void *view, CGPoint *pt);
void BgScreenOnTabUp(GACTIVITY *act, void *view);
void BgScreenOnFlying(GACTIVITY *act, void *view, CGPoint *pt);

extern int	wallpaperPos;
int		wallpaperLoop;


void BgScreenStart(void)
{
	GACTIVITY	*act;

	act = CreateActivity(BgScreenOnCreate);
	appStartActivityForResult(act, ACT_MP, NULL, 0);
}

void BgScreenOnCreate(GACTIVITY *act)
{
	void	*wnd;

	activitySetOnDestroy(act, BgScreenOnDestroy);
	activitySetOnAppMessage(act, BgScreenOnAppMessage);
	activitySetOnTimer(act, BgScreenOnTimer);
	activitySetOnDown(act, BgScreenOnDown);
	activitySetOnTabUp(act, BgScreenOnTabUp);
	activitySetOnFlying(act, BgScreenOnFlying);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, FALSE);
	wallpaperLoop = 0;
	UItimerStart(1, 5, 0);
}

void BgScreenOnDestroy(GACTIVITY *act)
{
	wallpaperDarken();
	AdmActExit(act);
}

void BgScreenOnTimer(GACTIVITY *act, int timerId)
{
	CGRect	rt;
	int		alpha, delta;

	if(timerId == 1) {
		wallpaperLoop++;
		if(wallpaperLoop == 1) {
			wallpaperPos = wallpaperChange(wallpaperPos);
			UItimerStart(1, 5, 0);
		} else {
			delta = 3;
			alpha = delta * wallpaperLoop;
			if(alpha >= 255) {
				UItimerStop(1);
				DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
			} else {
				wallpaperAnimate(wallpaperPos, alpha);
				UItimerStart(1, 30, 0);
			}
		}
	}
}

void BgScreenOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		UItimerStop(1); ;
printf("##### BgScreen #####\n");
		AuthIdentify(act, 1);
		break;
	case GM_TAMPER_NORMAL:
		UItimerStop(1);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	}
}

void BgScreenOnDown(GACTIVITY *act, void *view, CGPoint *pt)
{
printf("BgScreenOnDown...\n");
	UItimerStop(1);
}

void BgScreenOnTabUp(GACTIVITY *act, void *view)
{
printf("BgScreenOnTabUp...\n");
	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
}

void BgScreenOnFlying(GACTIVITY *act, void *view, CGPoint *pt)
{
printf("BgScreenOnFlying...\n");
	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
}

