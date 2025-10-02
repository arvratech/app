#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "msg.h"
#include "psu.h"
#include "slvmlib.h"
#include "slvm.h"
#include "dev.h"
#include "uv.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "appdev.h"

static uv_timer_t	_timerShut;
void MainToShutdown(int reason);
static void _PsuAddEvent(int evtId);
static void _ToPowerFail(void);


static void _OnPsuShutTimer(uv_timer_t *handle)
{
printf("battery timeout\n");
	_ToPowerFail();
}

static void _ToPowerNormal(void)
{
	ACCESS_DOOR		*ad;
	unsigned char	msg[12];

printf("### AC power OK ###\n");
	uv_timer_stop(&_timerShut);
	if(!devEnableBatteryLock(NULL)) {
		ad = adsGet(0);
		if(ad && adPriority(ad) > 6) {
		//	ad->hymo = 0;
			ad->hymo = 7;
			slvmDoorCommand(PV_UNLOCK, 0);
		}
	}
	msg[0] = GM_ALERT_CHANGED; memset(msg+1, 0, 9);
	appPostMessage(msg);
}

static void _ToBatPower(void)
{
	ACCESS_DOOR		*ad;
	unsigned char	msg[12];

	uv_timer_init(uv_default_loop(), &_timerShut);
	uv_timer_start(&_timerShut, _OnPsuShutTimer, 300*1000, 0);
	audioFlush();
	msg[0] = 124; msg[1] = 60; msg[2] = 0;
	audioPost(msg);
	fsCloseEvent();
	fsCloseCamEvt();
	sync();
	_PsuAddEvent(E_BATTERY_POWER);
	msg[0] = GM_ALERT_CHANGED; memset(msg+1, 0, 9);
	appPostMessage(msg);
	if(!devEnableBatteryLock(NULL)) {
		ad = adsGet(0);
		if(ad && adPriority(ad) > 6) {
			ad->hymo = 7;
			slvmDoorCommand(PV_UNLOCK, 0);
		}
	}
printf("### bat power ###\n");
}

static void _ToPowerFail(void)
{
printf("### power off ###\n");
	_PsuAddEvent(E_POWER_FAIL);
	slvmResetMaster(2);
	MainToShutdown(G_POWER_OFF);
}

void OnPsuAcAlarmStateChanged(void)
{
	unsigned char	msg[12];
	int		rval, acAlarmState, batAlarmState, poeStatus;

	acAlarmState = psuAcAlarmState(NULL);
	batAlarmState = psuBatAlarmState(NULL);
	poeStatus = psuPoeStatus(NULL);
printf("OnAcAlarmStateChanged: %d poe=%d bat=%d\n", acAlarmState, poeStatus, batAlarmState);
	if(acAlarmState) _PsuAddEvent(E_AC_POWER_ALARM);
	else	_PsuAddEvent(E_AC_POWER_NORMAL);
	if(acAlarmState && !poeStatus) {
		if(!batAlarmState && devEnableBattery(NULL)) {
			_ToBatPower();
		} else {
			_ToPowerFail();
		}
	} else if(!acAlarmState && !poeStatus) {
		_ToPowerNormal();
	}
	msg[0] = GM_POWER_LEVEL; memset(msg+1, 0, 9);
	appPostMessage(msg);
}

void OnPsuBatAlarmStateChanged(void)
{
	int		batAlarmState;

	batAlarmState = psuBatAlarmState(NULL);
printf("OnBatAlarmStateChanged: %d\n", batAlarmState);
//	if(batAlarmState) {    //2024.02.06 khan modify...
//		_ToPowerFail();
//	}
}

void OnPsuPoeStatusChanged(void)
{
	int		poeStatus, acAlarmState, batAlarmState;

	poeStatus = psuPoeStatus(NULL);
	acAlarmState = psuAcAlarmState(NULL);
	batAlarmState = psuBatAlarmState(NULL);
printf("OnPoeStatusChanged: %d ac=%d bat=%d\n", poeStatus, acAlarmState, batAlarmState);
	if(poeStatus) _PsuAddEvent(E_POE_PRESENT);	
	else	_PsuAddEvent(E_POE_NOT_PRESENT);	
	if(!poeStatus && acAlarmState) {
		if(!batAlarmState && devEnableBattery(NULL)) {
			_ToBatPower();
		} else {
			_ToPowerFail();
		}
	} else if(poeStatus && acAlarmState) {
		_ToPowerNormal();
	}
}

int PsuBatPower(void)
{
	if(psuAcAlarmState(NULL) && !psuPoeStatus(NULL)) return 1;
	else	return 0;
}

static void _PsuAddEvent(int evtId)
{
	unsigned char	ctm[8], evtData[16];
	char	temp[16];
	int		ac, dc;

	rtcGetDateTime(ctm);
	ac = psuAcVoltageNow(NULL);
	dc = psuBatVoltageNow(NULL);
	sprintf(temp, "%02d%02d%01d%02d%02d", ac/1000, (ac%1000)/10, psuPoeStatus(NULL), dc/1000, (dc%1000)/10);
	digits2card(temp, evtData);
	EventAdd(OT_ACCESS_POINT, devId(NULL), evtId, ctm, evtData);
}

