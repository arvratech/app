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
#include "av.h"
#include "appact.h"

static int		mpIndex, mpCount, destroyReq, sleepMode;
static void		*wndSleep;

void SleepOnCreate(GACTIVITY *act);
void SleepOnDestroy(GACTIVITY *act);
void SleepOnStart(GACTIVITY *act);
void SleepOnAppMessage(GACTIVITY *act, unsigned char *msg);
void SleepOnTabUp(GACTIVITY *act, void *view);
void SleepOnFlying(GACTIVITY *act, void *view, CGPoint *pt);


void SleepStart(void)
{
	GACTIVITY	*act;

	act = CreateActivity(SleepOnCreate);
	sleepMode = syscfgSleepMode(NULL);
	appStartActivityForResult(act, ACT_MP, NULL, 0);
}

void SleepOnCreate(GACTIVITY *act)
{
	void	*wnd;

	activitySetOnDestroy(act, SleepOnDestroy);
	activitySetOnStart(act, SleepOnStart);
	activitySetOnAppMessage(act, SleepOnAppMessage);
	activitySetOnTabUp(act, SleepOnTabUp);
	activitySetOnFlying(act, SleepOnFlying);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wndSleep = wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, TABLE_CYAN_600);
}

void SleepOnDestroy(GACTIVITY *act)
{
	if(sleepMode == 1) {
printf("onDestroy: .....\n");
		WakeupSleepMode();
	} else if(!destroyReq) {
		AvStopPlayMpeg4();
		audioContinue();
	}
	AdmActExit(act);
}

void SleepOnStopped(AV_PLAYER *player);

void SleepPlay(void)
{
	CGRect	rt;
	char	path[128], fileName[64];
	int		rval, idx;

	idx = mpIndex;
	do {
		mpIndex++;
		if(mpIndex >= mpCount) mpIndex = 0;
		rval = mpGetFile(mpIndex, fileName);
		if(rval > 0) {
			mpFullPath(path, fileName);
			rt.x = rt.y = 0; rt.width = lcdPixelWidth(); rt.height = lcdPixelHeight();
			AvPlayMpeg4(path, syscfgMediaSoundVolume(NULL), &rt, SleepOnStopped);
			break;
		}
	} while(mpIndex != idx) ;
}

void SleepOnStopped(AV_PLAYER *player)
{
	AvStoppedPlayMpeg4();
	UIsetNeedsDisplay(wndSleep);
	SleepPlay();
}

void SleepOnStart(GACTIVITY *act)
{
	if(sleepMode == 1) {
		EnterSleepMode();
	} else {
		mpArrangeFile();
		mpCount = mpFileCount();
		if(mpCount > 0) {
			mpIndex = mpCount - 1;
			SleepPlay();
		}
	}
}

void SleepOnStop(GACTIVITY *act)
{
	if(sleepMode == 1) {
		WakeupSleepMode();
	} else {
		AvStopPlayMpeg4();
		audioContinue();
	}
}

void SleepOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;

printf("sleep: onMessage=%d %d\n", (int)msg[0], msg[0]-0x80);
	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		SleepOnStop(act);
		AuthIdentify(act, 1);
		break;
	case GM_TAMPER_NORMAL:
	case GM_AD_LOCK_CHANGED:
	case GM_AD_DOOR_CHANGED:
	case GM_ALERT_CHANGED:
		if(!activityIsFinishing(act)) {
			SleepOnStop(act);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		}
		break;
	case GM_FILE_CHANGED:
		if(sleepMode != 1) {
			AvStopPlayMpeg4();
			SleepOnStart(act);
		}
		break;
	}
printf(".....\n");
}

void SleepOnTabUp(GACTIVITY *act, void *view)
{
printf("SleepOnTabUp...\n");
	SleepOnStop(act);
	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
}

void SleepOnFlying(GACTIVITY *act, void *view, CGPoint *pt)
{
printf("SleepOnFlying...\n");
	SleepOnStop(act);
	DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
}

