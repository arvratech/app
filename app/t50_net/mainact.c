#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "fsuser.h"
#include "hw.h"
#include "cam.h"
#include "crcred.h"
#include "psu.h"
#include "slvmlib.h"
#include "slvm.h"
#include "sysdep.h"
#include "syscfg.h"
#include "schecfg.h"
#include "tacfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "uv.h"
#include "cf.h"
#include "svrnet.h"
#include "slvnet.h"
#include "ctfont.h"
#include "viewprim.h"
#include "dlg.h"
#include "fpm.h"
#include "func.h"
#include "exec.h"
#include "audio.h"
#include "wallpaper.h"
#include "actprim.h"
#include "intrusion.h"
#include "topprim.h"
#include "admprim.h"
#include "nilib.h"
#include "sysprim.h"
#include "appact.h"

extern int	sleepTimer, authCount;
extern unsigned char  gFuncState, gFuncKey, gFuncTimer;

static void	*sysView, *alertView;
void	*clockView, *funcView, *funcBtns[5];
unsigned long	tabupTimer;
FP_USER	*ReqUser, _ReqUser;
int		wallpaperId, wallpaperPos;

void MainOnStart(GACTIVITY *act);
void MainOnAppMessage(GACTIVITY *act, unsigned char *msg);
void MainOnTimer(GACTIVITY *act, int timerID);
void MainOnClick(GACTIVITY *act, void *view);
void MainOnDown(GACTIVITY *act, void *view, CGPoint *pt);
void MainOnTabUp(GACTIVITY *act, void *view);
void MainOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent);
void MainOnFlying(GACTIVITY *act, void *view, CGPoint *pt);
void MainUpdate(GACTIVITY *act);


void MainOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rt;
	char	*p, temp[32];
	int		x, y, w, h, font;

printf("MainOnCreate...\n");
	wallpaperId = 0;
	activitySetOnStart(act, MainOnStart);
	activitySetOnActivityResult(act, MainOnActivityResult);
	activitySetOnAppMessage(act, MainOnAppMessage);
	activitySetOnTimer(act, MainOnTimer);
	activitySetOnClick(act, MainOnClick);
	activitySetOnDown(act, MainOnDown);
	activitySetOnTabUp(act, MainOnTabUp);
	activitySetOnFlying(act, MainOnFlying);
	wnd = activityWindow(act);
	activitySetViewBuffer(act, ViewAllocBuffer());
PrintBuffer("MainAct");
	UIframe(wnd, &rt);
	w = h = 28;
	sysView = ViewAddChild(wnd, 0, 0, 0, rt.width, h+2);
	x = 50; y = 2;
	ViewAddImageViewAlphaMask(wnd, TAG_LOCK_ICON, NULL, x, y, w, h); x += w + (w>>2);
	ViewAddImageViewAlphaMask(wnd, TAG_DOOR_ICON, NULL, x, y, w, h); x += w + (w>>2);
	if(syscfgNetworkType(NULL)) {
		ViewAddImageViewAlphaMask(wnd, TAG_WIFI_ICON, NULL, x, y, w, h); x += w + (w>>2);
	}
	if(devDisplayModel(NULL)) {
		ViewAddImageViewAlphaMask(wnd, TAG_CAM_ICON, NULL, x, y, w, h); x += w + (w>>2);
		ViewAddImageViewAlphaMask(wnd, TAG_BLE_ICON, NULL, x, y, w, h); x += w + (w>>2);
	}
	p = temp;
	strcpy(p, "9999."); p += 5;
	w = devId(NULL); devSetId(NULL, 99); devGetDeviceName(p); devSetId(NULL, w);
	font = 20;
	ftSetSize(font);
	w = ftTextWidth(temp);
	x = rt.width - 8 - w; y = 0;
	v = ViewAddLabel(wnd, TAG_SLVNET_TEXT, NULL, x, y, w, h, UITextAlignmentRight);
	UIsetFont(v, font);

//	v = ViewAddLabel(sysView, TAG_POWER_TEXT, NULL, x, y, w, h, UITextAlignmentRight);
	y = rt.height >> 3;
	clockView = ViewAddClock(wnd, TAG_CLOCK_VIEW, TAG_CLOCK_BASE, y);
#ifdef _HYNIX
	x = 24; y += 90; w = rt.width - x - x; h = rt.height - y - 168;
printf("w=%d h=%d %d %d\n", w, h, _scale(w), _scale(h));
	alertView = ViewAddImageViewBitmap(wnd, 0, NULL, x, y, w, h);
	w = 64; x = (rt.width - w) >> 1; y = rt.height - w;
//	v = ViewAddButtonAlphaMask(wnd, TAG_PHONE_BTN, "phone.bmp", x, y, w, w, w-16);
#else
	x = 30; y += 120; w = rt.width - x - x; h = rt.height - y - 84;
	alertView = ViewAddChild(wnd, 0, x, y, w, h);
	UIsetOpaque(alertView, TRUE); 
	UIsetBackgroundColor(alertView, redColor);
	y = h - 80 - 80;
	v = ViewAddImageViewAlphaMask(alertView, 1, "door.bmp", (w-y)>>1, 40, y, y);
	UIsetTintColor(v, blueColor);
	v = ViewAddLabel(alertView, 2, NULL, 0, h-80, w, 28, UITextAlignmentCenter);
	UIsetFont(v, 26);
	UIsetTextColor(v, blackColor));
	UIsetHidden(alertView, TRUE);
	w = 64; x = (rt.width - w) >> 1; y = rt.height - w;
	v = ViewAddButtonAlphaMask(wnd, TAG_KEYPAD_BTN, "keypad.bmp", x, y, w, w, w-16);
#endif
/*
	FuncKeyInitialize();
	w = 160;
	x = (rt.width - w) >> 1; y = 160;
	v = ViewAddImageViewAlphaMask(wnd, TAG_FUNC_LBL, "login.bmp", x, y, w, w);
	ViewRefreshFuncLabel(wnd, (int)gFuncKey);
	w = 48;
	x = rt.width / 4; y = rt.height - w;
	v = ViewAddButtonAlphaMask(wnd, TAG_FUNC_BTN+1, "login.bmp", x-(w>>1), y, w, w, w-16);
	x = rt.width - x;
	v = ViewAddButtonAlphaMask(wnd, TAG_FUNC_BTN+2, "logout.bmp", x-(w>>1), y, w, w, w-16);
*/
//v = ViewAddImageViewJpeg(wnd, 0, "testimgp.jpg", 50, 350, 227, 149);
	tabupTimer = rtcMiliTimer();
	wallpaperPos = 0;
	MainUpdate(act);
}

void MainShutdown(void);
void AuthIdentify(GACTIVITY *act, int clearTop);
void MainAuthVerify(GACTIVITY *act);
void MainAuthCapture(GACTIVITY *act);
void MainAuthResult(GACTIVITY *act);
void MainAuthFailed(GACTIVITY *act);
void MainTopMsg(GACTIVITY *act, int msgIndex);

void MainOnStart(GACTIVITY *act)
{
	int		ss, n, idx;

//printf("MainOnStart: req=%d result=%d intent=[%s]\n", act->requestCode, act->resultCode, act->intent);
	sleepTimer = 0;
	//wnd = activityWindow(act);
	//cr = crsGet(0);
	idx = 0;
	if(devStatus(NULL)) {
		ss = devSystemStatus(NULL);
		if(ss == SS_SHUTDOWN) {
			n = devShutdownReason(NULL);
			if(n == G_WARM_RESTART || n == G_COLD_RESTART) idx = R_SYSTEM_RESTART;
			else if(n == G_POWER_FAIL) idx = R_POWER_FAIL;
			else if(n == G_POWER_OFF) idx = R_POWER_OFF;
			else if(n == G_USER_SHUTDOWN) {
				fsCloseFiles();
				MainShutdown();
				usleep(250000);
				return;
			}
		} else if(ss != SS_OPERATIONAL) idx = R_SLAVE_INITIALIZING;
	} else if(tamperPresentValue(NULL)) idx = R_TAMPER_ALARM;
	if(idx > 0) {
printf("MainTopMsg: %d\n", idx);
		MainTopMsg(act, idx);
		act->requestCode = 0;
	}
	switch(act->requestCode) {
	case ACT_AUTHREQUEST:
	case ACT_AUTHCAPTURE:
	case ACT_AUTHRESULT:
		if(act->resultCode == RESULT_OK) MainAuthFailed(act);
		break;
	case ACT_ADMPASSWORD:
		if(act->resultCode == RESULT_OK) MainTopMsg(act, R_PW_MISMATCHED);
		break;
	case ACT_USERID:
		if(act->resultCode == RESULT_OK) MainTopMsg(act, R_ACU_TIMEOUT);
		break;
	}
	act->requestCode = 0;
}

void MainLockState(void *wnd);
void MainDoorState(void *wnd);
void MainSvrnetState(void *wnd);
void MainWifiState(void *wnd);
void MainCamState(void *wnd);
void MainBleState(void *wnd);
void MainDeviceID(void *wnd);
void MainSlvnetState(void *wnd);
void MainPowerLevel(void *wnd);
void MainAlertView(void *view);
void MainSwapWallpaper(void *wnd);

void MainUpdate(GACTIVITY *act)
{
	void	*wnd;

	wnd = activityWindow(act);
	MainSwapWallpaper(wnd);
	UItimerStart(0, 500, 1);
	sleepTimer = 0;
	ClockViewResize(clockView, TAG_CLOCK_BASE);
	ClockViewUpdate(clockView, TAG_CLOCK_BASE);
	MainLockState(wnd);
	MainDoorState(wnd);
	if(devDisplayModel(NULL)) {
		MainCamState(wnd);
		MainBleState(wnd);
	}
	MainSlvnetState(wnd);
//	if(syscfgNetworkType(NULL)) MainWifiState(wnd);
	MainAlertView(alertView);
}

void MainOnActivityResult(GACTIVITY *act, int requestCode, int resultCode, char *intent)
{
	act->requestCode = requestCode; act->resultCode = resultCode;
	if(intent && intent[0]) strcpy(act->intent, intent); else act->intent[0] = 0;		
	if(requestCode == ACT_ADM) AdmRootExit();
	MainUpdate(act);
}

/*
void _OnWallpaperTimer(uv_timer_t *handle)
{
	CGRect	rt;
	int		alpha, delta;

	delta = 25;
	if(wallpaperLoop < 8) {
		alpha = 255 - delta * (wallpaperLoop+1);
	} else if(wallpaperLoop == 8) {
		alpha = wallpaperNext(wallpaperPos);
		if(alpha > 0) wallpaperPos++;
		else	wallpaperPos = 0;
		wallpaperInit(wallpaperPos);
		alpha = 255 - delta * (16 - wallpaperLoop);
	} else if(wallpaperLoop >= 16) {
		wallpaperDraw(NULL);
		MainUpdate(appCurrentActivity());
	} else {
		alpha = 255 - delta * (16 - wallpaperLoop);
	}
	if(wallpaperLoop < 16) {
		rt.x = rt.y = 0; rt.width = lcdPixelWidth(); rt.height = lcdPixelHeight();
		lcdClipToRect(&rt);
		lcdFadeoutBackgroundScreen(alpha);
		uv_timer_start(timerWallpaper, _OnWallpaperTimer, 10, 0);
	}
	wallpaperLoop++;
}
*/
void MainOnTimer(GACTIVITY *act, int timerId)
{
	void	*wnd;
	int		mode, val;

	wnd = activityWindow(act);
	switch(timerId) {
	case 0:
		ClockViewUpdate(clockView, TAG_CLOCK_BASE);
		if(ta_cfg->funcKeyTimer) {
			gFuncTimer++;
			val = ta_cfg->funcKeyTimer << 1;
			if(gFuncTimer >= val) {
				mode = gFuncKey;
				FuncKeyReset();
				if(mode != gFuncKey) ViewRefreshFuncLabel(wnd, (int)gFuncKey);
			}
		}
		/*
		mode = funcResetState();
		if(gFuncState != mode) {
			gFuncState = mode;
			FuncKeyInitialize();
			ViewRefreshFuncLabel(funcView, (int)gFuncKey);
		}
		*/
		sleepTimer++;
		val = syscfgSleepWaitTime(NULL) << 1;
		if(sleepTimer >= val) {
			mode = syscfgSleepMode(NULL);
			if(wallpaperId == 0 && !psuBatPower() && !intrusionMode(NULL)) {
				if(mode == 1 || (mode == 2 && mpFileCount() > 0)) {
					wdPing();
					SleepStart();
				} else if(mode == 0 && wallpaperNext(wallpaperId) >= 0) {
					BgScreenStart();
				}
			}
		}
		break;
	}
}

void UserIdOnDismiss(GACTIVITY *act)
{
	void	*cr;
	char	key[32], val[32];
	long	userId;
	int		resultCode;

	resultCode = activityResultCode(act);
	if(resultCode == RESULT_OK) {
		cr = crsGet(0);
		get_keyvalue(act->intent, key, val);
		userId = n_atol(val);
		crCapturedUserId(cr, userId);
		if(SlvnetIsConnected()) AuthIdentify(act, 1);
		else {
			crSetResult(cr, 4);
			DestroyActivityForResult(act, resultCode, NULL);
		}
	} else {
		DestroyActivityForResult(act, resultCode, NULL);
	}
}

int		i2cloop;

void MainOnClick(GACTIVITY *act, void *view)
{
	GACTIVITY	*a;
	ACCT	*acct, _acct;
	char	temp[128];
	int		tag, rval, idx;

	//wnd = activityWindow(act);
	tag = UItag(view);
	switch(tag) {
	case TAG_KEYPAD_BTN:
		UItimerStop(0);
		//pf = cfPinGet();
		sprintf(temp, "ID=0\n%s=\n8\n", GetPromptMsg(M_ENTER_USER_ID));
		a = DigitInputDialog(ACT_USERID, temp, AdmDoneValidateUserID);
		activitySetOnDismiss(a, UserIdOnDismiss);
		break;
	case TAG_FUNC_BTN:
		if(gFuncKey != 1 && taItemIsEnable(0)) {
			gFuncKey = 1;
			ViewRefreshFuncLabel(funcView, (int)gFuncKey);
		}
		gFuncTimer = 0;
		break;
	case TAG_FUNC_BTN+1:
	case TAG_FUNC_BTN+2:
	case TAG_FUNC_BTN+3:
	case TAG_FUNC_BTN+4:
		idx = tag - TAG_FUNC_BTN;
		if(taItemIsEnable(idx)) {
			gFuncKey = idx + 1;
			ViewRefreshFuncLabel(funcView, (int)gFuncKey);
		}
		gFuncTimer = 0;
		break;
	case TAG_PHONE_BTN:
		rval = acctfsGetCount();
		if(devEnableIpPhone(NULL) && rval > 0) {
			UItimerStop(0);
			audioFlush();
			while(audioActive()) ;
			if(rval == 1) {
				acct = &_acct;
				rval = acctfsReadAtIndex(acct, 0);
				sprintf(temp, "name=%s\nurl=%s\n", acct->acctName, acct->url);
				PjsCallStart(ACT_PJSCALL, temp);
			} else {
				PhoneCall(ACT_PHONECALL);
			}
		}
		break;
	}
}

void MainOnDown(GACTIVITY *act, void *view, CGPoint *pt)
{
//printf("%u main: onDown...%d,%d\n", MS_TIMER, pt->x, pt->y);
	//if(sleepMode) {
	//activityClearTouchView(act);
	//view = NULL;
	//}
	if(view == sysView) {
		UItimerStop(0);
		StatBarStart(pt->y);
	} else {
		sleepTimer = 0;	
	}
}

void MainOnTabUp(GACTIVITY *act, void *view)
{
	int		diff;
	
	diff = rtcMiliTimer() - tabupTimer;
	tabupTimer = rtcMiliTimer();
	if(diff < 500) {
printf("%lu main: doubleTab...\n", MS_TIMER);
		if(syscfgSleepMode(NULL) == 1) SleepStart();
	}
}

void MainOnFlying(GACTIVITY *act, void *view, CGPoint *pt)
{
/*
	long	dx, dy;

	dx = pt->x; dy = pt->y;
//	if(dx < 0) dx = -dx; if(dy < 0) dy = -dy;
//	dx += dy;
//	if(authCount > 0 && dx > 500) AuthDoneStart();
	if(dy > 200 && act->touchView == sysView) {
		UItimerStop(0);
		StatBarStart();
	}
*/
}

void MainOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd;
	CREDENTIAL_READER	*cr;

//printf("%u main: onAppMessage...%d\n", MS_TIMER, (int)msg[0]);
	wnd = activityWindow(act);
	cr = crsGet(0);
	switch(msg[0]) {
	case GM_CRED_CAPTURED:
		UItimerStop(0);
		AuthIdentify(act, 0);
		break;
	case GM_FPM_TOUCHED:
printf("fpmTouched.....\n");
//		fpmRequestCapture();
		fpmRequestIdentify();
		break;
	case GM_FPM_RESPONSE:
printf("fpmResponse.....\n");
		//fpmResponseCapture();
		fpmResponseIdentify();
		if(cr->result == 0) {
			UItimerStop(0);
			AuthIdentify(act, 0);
		}
		break;
	case GM_DATETIME_CHANGED:
//printf("MSG DateTime changed...\n");
		ClockViewResize(clockView, TAG_CLOCK_BASE);
		ClockViewUpdate(clockView, TAG_CLOCK_BASE);
		break;
	case GM_SVRNET_CHANGED:
		MainSvrnetState(wnd);
		break;
	case GM_SLVNET_CHANGED:
		MainSlvnetState(wnd);
		break;
	case GM_NI_CHANGED:
		if(!syscfgNetworkType(NULL)) MainSlvnetState(wnd);
		break;
	case GM_AD_LOCK_CHANGED:
printf("LockStateChanged...\n");
		MainSwapWallpaper(wnd);
		MainLockState(wnd);
		break;
	case GM_AD_DOOR_CHANGED:
		MainSwapWallpaper(wnd);
		MainDoorState(wnd);
		break;
	case GM_CAM_CHANGED:
		if(devDisplayModel(NULL)) {
			MainCamState(wnd);
		}	
		break;
	case GM_BLE_CHANGED:
		if(devDisplayModel(NULL)) {
			MainBleState(wnd);
		}
		break;
	case GM_ALERT_CHANGED:
		MainAlertView(alertView);
		break;
	case GM_WALLPAPER_CHANGED:
		MainSwapWallpaper(activityWindow(act));
		break;
	}
}

//buf[0] = 0x30; buf[1] = 8; memcpy(buf+2, cr->data, 8); slvmWriteCredBuffer(buf, 10); AuthResultStart();
void AuthIdentify(GACTIVITY *act, int clearTop)
{
	if(clearTop) AuthRequestRestart();
	else	AuthRequestStart();
}

void MainTopMsg(GACTIVITY *act, int msgIndex)
{
	char	intent[24];

	UItimerStop(0);
	sprintf(intent, "%d", msgIndex);
	TopMsgStart(intent);
}

void MainAuthFailed(GACTIVITY *act)
{
	void	*cr;
	int		msgIndex;

	UItimerStop(0);
	cr = crsGet(0);
	switch(crResult(cr)) {
	case 1:	 msgIndex = R_USER_NOT_FOUND; break;
	case 2:	 msgIndex = R_MSG_SYSTEM_ERROR; break;
	case 4:	 msgIndex = R_ACU_TIMEOUT; break;
	case 5:	 msgIndex = R_NETWORK_FAILED; break;
	case 6:	 msgIndex = R_NOT_ALLOWED_CARD; break;
	default: msgIndex = R_FAILED;
	}
	MainTopMsg(act, msgIndex);
}

void MainLockState(void *wnd)
{
	void	*ad, *img, *imgv;
	char	file[32];
	UIColor	color;
	int		pv;

	ad = adsGet(0);
	if(ad && adIsNonNull(ad)) {
		pv = adPresentValue(ad);
		if(pv == PV_LOCK) strcpy(file, "lock.bmp");
		else	strcpy(file, "lock_unlock.bmp");
		color = enableWhiteColor;
	} else {
		file[0] = 0;
		color = disableWhiteColor;
	}
	imgv = UIviewWithTag(wnd, TAG_LOCK_ICON);
	UIsetTintColor(imgv, color);
	img = UIimage(imgv);
	UIinitWithFile(img, file, IMAGE_ALPHAMASK);
	UIsetImage(imgv, img);
//	if(file1 & rel) lcdPutIconFile(80+8, 8, 16, BTN_NO);
}

void MainDoorState(void *wnd)
{
	void	*ad, *bi, *img, *imgv;
	char	file[32];
	UIColor	color;
	int		id, alarmState;

	ad = adsGet(0);
	if(ad && adIsNonNull(ad)) {
		alarmState = adAlarmState(ad);
//printf("adAlarmState=%d\n", alarmState);
		id = adIoSet(ad);
		bi = bisGet(IO2BI(id)+1);
		if(!biIsNonNull(bi) || !biIsDoorType(bi)) {
			strcpy(file, "door.bmp"); color = disableWhiteColor;
		} else {
			 if(alarmState == 3) strcpy(file, "door_forcedopen.bmp");
			else if(alarmState == 2) strcpy(file, "door_otl.bmp");
			else if(biPresentValue(bi)) strcpy(file, "door_opened.bmp");
			else	strcpy(file, "door.bmp");
			color = enableWhiteColor;
		}
	} else {
		file[0] = 0;
		color = disableWhiteColor;
	}
	imgv = UIviewWithTag(wnd, TAG_DOOR_ICON);
	UIsetTintColor(imgv, color);
	img = UIimage(imgv);
	UIinitWithFile(img, file, IMAGE_ALPHAMASK);
	UIsetImage(imgv, img);
}

void MainSvrnetState(void *wnd)
{
	void	*img, *imgv;
	UIColor	color;

	imgv = UIviewWithTag(wnd, TAG_SVRNET_ICON);
	img = UIimage(imgv);
	if(SvrnetIsConnected()) {
		UIinitWithFile(img, "tcp.bmp", IMAGE_ALPHAMASK);
		color = enableWhiteColor;
	} else {
		UIinitWithFile(img, "tcp_fail.bmp", IMAGE_ALPHAMASK);
		color = redColor;
	}
	UIsetTintColor(imgv, color);
	UIsetImage(imgv, img);
}

void MainWifiState(void *wnd)
{
	void	*img, *imgv;
	UIColor	color;

printf("wifiState=%d...\n", niState());
	imgv = UIviewWithTag(wnd, TAG_WIFI_ICON);
	img = UIimage(imgv);
	if(niState() >= 2) {
		UIinitWithFile(img, "wifi.bmp", IMAGE_ALPHAMASK);
		color = enableWhiteColor;
	} else {
		UIinitWithFile(img, "wifi.bmp", IMAGE_ALPHAMASK);
		color = redColor;
	}
	UIsetTintColor(imgv, color);
	UIsetImage(imgv, img);
}

void MainCamState(void *wnd)
{
	void	*img, *imgv;
	char	file[32];
	UIColor	color;

	if(camMode(NULL) == 2) {
		strcpy(file, "cam.bmp");
		if(camCodecState()) {
			color = 0xffbccad8;
		} else {
			color = redColor;
		}
	} else {
		file[0] = 0;
		color = disableWhiteColor;
	}
	imgv = UIviewWithTag(wnd, TAG_CAM_ICON);
	UIsetTintColor(imgv, color);
	img = UIimage(imgv);
	UIinitWithFile(img, file, IMAGE_ALPHAMASK);
	UIsetImage(imgv, img);
}

void MainBleState(void *wnd)
{
	void	*img, *imgv;
	char	file[32];
	UIColor	color;

	if(bleEnable(NULL)) {
		strcpy(file, "bluetooth.bmp");
		color = 0xffbccad8;
	} else {
		file[0] = 0;
		color = disableWhiteColor;
	}
	imgv = UIviewWithTag(wnd, TAG_BLE_ICON);
	UIsetTintColor(imgv, color);
	img = UIimage(imgv);
	UIinitWithFile(img, file, IMAGE_ALPHAMASK);
	UIsetImage(imgv, img);
}

void MainDeviceID(void *wnd)
{
	void	*v;
	char	temp[16];

    devGetDeviceName(temp);
	v = UIviewWithTag(wnd, TAG_SLVNET_TEXT);
	UIsetTextColor(v, enableWhiteColor);
	UIsetText(v, temp);
}

void MainSlvnetState(void *wnd)
{
	void	*v;
	char	*p, temp[64];
	UIColor		color;

	v = UIviewWithTag(wnd, TAG_SLVNET_TEXT);
	if(v) {
		p = temp;
		if(SlvnetIsConnected()) {
			sprintf(p, "%d.", SlvnetMasterDevId()); p += strlen(p);
			color = enableWhiteColor;
		} else {
			color = redColor;
		}
		GetSlaveFullName(devId(NULL), p);
		UIsetTextColor(v, color);
		UIsetText(v, temp);
	}
}

void MainPowerLevel(void *wnd)
{
	void	*v;
	char	temp[16];

	sprintf(temp, "%dv", psuAcVoltageNow(NULL)/1000000);
	v = UIviewWithTag(sysView, TAG_POWER_TEXT);
	UIsetTextColor(v, enableWhiteColor);
	UIsetText(v, temp);
}

void MainAlertView(void *view)
{
	void	*v, *img;
	char	file[32];

	file[0] = 0;
#ifdef _HYNIX
    if(psuBatPower()) {
		strcpy(file, "co_power_low.bmp");	
	} else if(intrusionMode(NULL)) {
		strcpy(file, "co_armed.bmp");	
	}
	if(file[0]) {
		v = alertView; 
		img = UIimage(v);
		UIinitWithFile(img, file, IMAGE_BITMAP);
		UIsetImage(v, img);
		UIsetHidden(v, FALSE);
#else
	if(intrusionMode(NULL)) {
		if(intrusionPresentValue(NULL)) {
			strcpy(file, "thief.bmp"); color = redColor; strcpy(text, xmenu_armed[3]);	
		} else {	// armed
			strcpy(file, "shield.bmp"); color = purpleColor; strcpy(text, xmenu_armed[2]);	
		}
	}
	v = UIviewWithTag(view, 1);
	if(v && file[0]) {
		UIsetBackgroundColor(view, color);
		img = UIimage(v);
		UIinitWithFile(img, file, IMAGE_ALPHAMASK);
		UIsetImage(v, img);
		v = UIviewWithTag(view, 2);
		UIsetText(v, text);
		UIsetHidden(view, FALSE);
#endif
	} else {
		UIsetHidden(view, TRUE);
	}
}

void MainSwapWallpaper(void *wnd)
{
	CGRect	rt;
	void	*ad;
	UIColor color;
	char	*p, *p2, file[32], text[64];
	int		val;

	val = 0;
	ad = adsGet(0);
	if(ad) {
		val = adAlarmState(ad);
		if(val > 1) val++;
		else {
			val = 0;
			if(adPriority(ad) < 7) {
				if(adPresentValue(ad)) val = 1;
				else	val = 2;
			}
		}
	} 
	p = p2 = NULL; color = 0;
	if(wallpaperId != val) {
		wallpaperId = val;
		switch(val) {
		case 1:
			strcpy(file, "door_opened.bmp");
			p = xauth_msgs[1]; p2 = NULL;
			color = redColor;
			break;
		case 2:
			strcpy(file, "door.bmp");
			p = xauth_msgs[2]; p2 = NULL;
			color = redColor;
			break;
		case 3:
			strcpy(file, "door_otl.bmp");
			p = xalarm_msgs[2]; p2 = xalarm_msgs[3];
			color = yellowColor;
			break;
		case 4:
			strcpy(file, "door_forcedopen.bmp");
			p = xalarm_msgs[0]; p2 = xalarm_msgs[1];
			color = yellowColor;
			break;
		}
		if(val > 0) {
			strcpy_swap(text, p, '\n', ' ');
			wallpaperSwapBitmap(color, file, text, p2);
		} else {
			wallpaperPos = 0;
			wallpaperInit(wallpaperPos);
		}
		wallpaperDraw(NULL);
		UIsetNeedsDisplay(wnd);
		UIframe(wnd, &rt); rt.x = rt.y = 0;
		UIdrawView(wnd, &rt);
	}
}

void UpdateWallpaper(void)
{
	unsigned char	msg[12];

printf("UpdateWallPaper...\n");
	wallpaperId = -1;
	msg[0] = GM_WALLPAPER_CHANGED;
	appPostMessage(msg);
}

int camCodecState(void)
{
	return 0;
}

