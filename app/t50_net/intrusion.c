#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "asciidefs.h"
#include "prim.h"
#include "NSEnum.h"
#include "rtc.h"
#include "syscfg.h"
#include "dev.h"
#include "uv.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "intrusion.h"

#define INTRUSION_NONE		0
#define INTRUSION_DISARMED	1
#define INTRUSION_ARMED		2

extern unsigned char	_devStatus;

static unsigned char	_tamper, _intrusionMode, _intrusion;
static unsigned char	_tamperTrackValueTime[6];
static unsigned char	_tamperChangeOfStateTime[6];
static unsigned char	_intrusionTrackValueTime[6];
static unsigned char	_intrusionChangeOfStateTime[6];
static unsigned char	_intrusionChangeOfModeTime[6];
static uv_timer_t		*timerIntrusion, _timerIntrusion;
static void (*_OnTamperPresentValueChanged)(void *);
static void (*_OnIntrusionValueChanged)(void *, int, int);


void tamperInit(void)
{
	_tamper = 0;
	rtcGetDateTime(_tamperTrackValueTime);
	rtcGetDateTime(_tamperChangeOfStateTime);
}

unsigned char *tamperTrackValueTime(void *self)
{
	return _tamperTrackValueTime;
}

unsigned char *tamperChangeOfStateTime(void *self)
{
	return _tamperChangeOfStateTime;
}

void tamperSetOnPresentValueChanged(void *self, void (*onPresentValueChanged)(void *))
{
	_OnTamperPresentValueChanged = onPresentValueChanged;
}

BOOL tamperIsEnable(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & CR_TAMPER) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void tamperSetEnable(void *self, BOOL bVal)
{
	int		tv;

	if(tamperIsEnable(self) != bVal) {
		if(bVal) sys_cfg->devOption |= CR_TAMPER;
		else	 sys_cfg->devOption &= ~CR_TAMPER;
		tamperClearPresentValue(self);
	}
}

int tamperTrackValue(void *self)
{
	int		tv;

	if(_tamper & 0x01) tv = 1; else tv = 0;
	return tv;
}

int tamperPresentValue(void *self)
{
	int		pv;

	if(_tamper & 0x02) pv = 1; else pv = 0;
	return pv;
}

void TamperOnTrackValueChanged(void *self, int trackValue)
{
	int		oldPv, newPv;

	newPv = trackValue;
	oldPv = tamperTrackValue(self);
printf("%lu TamperOnTrackValueChanged: enable=%d %d => %d\n", SEC_TIMER, tamperIsEnable(NULL), oldPv, newPv);
	if(newPv == oldPv) return;
	if(newPv) _tamper |= 0x01;
	else	  _tamper &= 0xfe;
	rtcGetDateTime(_tamperTrackValueTime);
	if(!tamperIsEnable(NULL) || devTestMode(NULL)) return;
	oldPv = tamperPresentValue(self);
	if(oldPv != newPv) {
		if(newPv) _tamper |= 0x02;
		else	  _tamper &= 0xfd;
		rtcGetDateTime(_tamperChangeOfStateTime);
		if(_OnTamperPresentValueChanged) (*_OnTamperPresentValueChanged)(self);
	}
}

int tamperClearPresentValue(void *self)
{
	if(tamperPresentValue(self)) {
		_tamper &= 0xfd;
		rtcGetDateTime(_tamperChangeOfStateTime);
		if(_OnTamperPresentValueChanged) (*_OnTamperPresentValueChanged)(self);
	}
}

void *MainLoop(void);

void intrusionInit(void)
{
	timerIntrusion = &_timerIntrusion;
	uv_timer_init(MainLoop(), timerIntrusion);
	_intrusionMode = INTRUSION_NONE;
	_intrusion = 0;
	rtcGetDateTime(_intrusionChangeOfStateTime);
	memcpy(_intrusionTrackValueTime, _intrusionChangeOfStateTime, 6);
	memcpy(_intrusionChangeOfModeTime, _intrusionChangeOfStateTime, 6);
	intrusionDisarm(NULL);
}

unsigned char *intrusionTrackValueTime(void *self)
{
	return _intrusionTrackValueTime;
}

unsigned char *intrusionChangeOfStateTime(void *self)
{
	return _intrusionChangeOfStateTime;
}

unsigned char *intrusionChangeOfModeTime(void *self)
{
	return _intrusionChangeOfModeTime;
}

void intrusionSetOnValueChanged(void *self, void (*onValueChanged)(void *, int, int))
{
	_OnIntrusionValueChanged = onValueChanged;
}

BOOL intrusionIsEnable(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & INTRUSION_ALARM) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void intrusionSetEnable(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= INTRUSION_ALARM;
	else	sys_cfg->devOption &= ~INTRUSION_ALARM;
	intrusionDisarm(self);
}

int intrusionTrackValue(void *self)
{
	int		tv;

	if(_intrusion & 0x01) tv = 1; else tv = 0;
	return tv;
}

int intrusionPresentValue(void *self)
{
	int		pv;

	if(_intrusion & 0x02) pv = 1; else pv = 0;
	return pv;
}

int intrusionMode(void *self)
{
	int		mode;

	mode = _intrusionMode;
	if(mode) mode--;
	return mode;
}

void IntrusionTrackValueChanged(void *self, int trackValue)
{
	unsigned char	msg[12];
	int		oldPv, newPv;


	if(!_intrusionMode) return;
	newPv = trackValue;
	oldPv = intrusionTrackValue(self);
printf("%lu IntrusionTrackValueChanged: mode=%d tv=%d => %d\n", SEC_TIMER, (int)_intrusionMode, oldPv, newPv);
	if(newPv == oldPv) return;
	rtcGetDateTime(_intrusionTrackValueTime);
	if(newPv) {
		_intrusion |= 0x01;
		if(_intrusionMode == INTRUSION_ARMED) {
			oldPv = intrusionPresentValue(self);
			if(!oldPv && newPv) {
				_intrusion |= 0x02;
				rtcGetDateTime(_intrusionChangeOfStateTime);
				if(_OnIntrusionValueChanged) (*_OnIntrusionValueChanged)(self, 0, 1);
			}
		}
	} else {
		_intrusion &= 0xfe;
		if(_intrusionMode == INTRUSION_DISARMED) {
			_intrusionMode = INTRUSION_ARMED;
			rtcGetDateTime(_intrusionChangeOfModeTime);
			if(_OnIntrusionValueChanged) (*_OnIntrusionValueChanged)(self, 1, 0);
		}
	}
}

void _OnIntrusionTimer(uv_timer_t *timer)
{
printf("%lu IntrusionOnTimer\n", SEC_TIMER);
	if(_intrusionMode) {
		IntrusionTrackValueChanged(NULL, 0);
	}
}

#define _IntrusionTimerStart(timeout)	uv_timer_start(timerIntrusion, _OnIntrusionTimer, timeout, 0)
#define _IntrusionTimerStop(timeout)	uv_timer_stop(timerIntrusion)

void IntrusionBiAlarmStateChanged(void *self, int alarm)
{
	if(_intrusionMode) {
if(alarm) printf("%u BI(intrusion): on\n", SEC_TIMER); else printf("%u BI(intrusion): off\n", SEC_TIMER); 
		if(alarm) {
			_IntrusionTimerStop();
			IntrusionTrackValueChanged(self, alarm);
		} else {
			_IntrusionTimerStart(120000);
		}
	}
}

void intrusionDisarm(void *self)
{
	void	*cr, *ad, *bi;
	int		mode, pv;

printf("%lu intrusionDisarm...\n", SEC_TIMER);
	mode = pv = 0;
	if(_intrusionMode == INTRUSION_ARMED) {
		if(intrusionPresentValue(self)) {
			_intrusion &= 0xfd;
			rtcGetDateTime(_intrusionChangeOfStateTime);
			pv = 1;
		}
		rtcGetDateTime(_intrusionChangeOfModeTime);
		_intrusionMode = INTRUSION_DISARMED;
		mode = 1;
		if(_OnIntrusionValueChanged) (*_OnIntrusionValueChanged)(self, mode, pv);
	}
	_intrusionMode = INTRUSION_NONE;
	_intrusion = 0;
	if(intrusionIsEnable(self)) {
		ad = _AdsGet(0);
		if(ad && adAlarmState(ad) < 2 && adPresentValue(ad) == 0) {
			_intrusionMode = INTRUSION_DISARMED; _intrusion |= 0x01;
			bi = bisGet(2);
			if(!biRawAlarm(bi)) _IntrusionTimerStart(10000);
		}
	}
}

