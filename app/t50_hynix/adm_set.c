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
#include "cam.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "prefspec.h"
#include "pref.h"
#include "viewprim.h"
#include "dlg.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actalert.h"
#include "ble.h"
#include "av.h"
#include "appact.h"


BOOL _IsEnableCam(void *self)
{
	if(camMode(self) == 2) return TRUE;
	else	return FALSE;
}

void *MainLoop(void);

static void _SetEnableCam(void *self, BOOL enable)
{
	int		rval;

	if(enable) {
		camSetMode(self, 2);
		AvStartSource();
		//if(!rval) camRunFrameRate(camFrameRate(NULL));
	} else {
		camSetMode(self, 1);
		AvStopSource();
	}
}


static void _AdmCamOnCreate(void *ps)
{
	AvStartSourcePreview(0, 0, 0, 0);
//AvRecordMpeg4("temp.mp4");
}

static void _AdmCamOnDestroy(void *ps)
{
	AvStopSourcePreview();
//AvStopRecordMpeg4();
}

static void _SetEnableBle(void *self, BOOL enable)
{
	bleSetEnable(NULL, enable);	
	blemWrite();
}

void AdmSet(void *pss)
{
	void	*ps;
	char	temp[64];
	BOOL	bVal;
	int		i, val;

	ps = PSaddScreen(pss, xmenu_setup[0], AdmSetAdmin);
	ps = PSaddScreen(pss, xmenu_setup[1], AdmSetDateTime);
	ps = PSaddScreen(pss, xmenu_setup[2], AdmSetDisplay);
	ps = PSaddScreen(pss, xmenu_setup[3], AdmSetSound);
	ps = PSaddSpinner(pss, xmenu_setup[4], syscfgLanguage, syscfgSetLanguage);
	PSspinnerAddTitleValue(ps, xmenu_lang[0], LANG_ENG);
	PSspinnerAddTitleValue(ps, xmenu_lang[1], LANG_KOR);
	PSaddScreen(pss, xmenu_setup[6], AdmSetPowerMgmt);
	ps = PSaddSwitch(pss, xmenu_setup[7], _IsEnableCam, _SetEnableCam, AdmSetCam);
	PSsetOnCreate(ps, _AdmCamOnCreate);
	PSsetOnDestroy(ps, _AdmCamOnDestroy);
	ps = PSaddSwitch(pss, xmenu_setup[8], bleEnable, _SetEnableBle, AdmSetBle);
	ps = PSaddSwitch(pss, xmenu_setup[9], devEnableIpPhone, devSetEnableIpPhone, AdmSetIpPhone);
	PSaddStepper(pss, xmenu_setup[10], syscfgScRfoNormalLevel, syscfgSetScRfoNormalLevel, 0, 14, 1, "");
//for(i = 0;i < 9;i++) PSaddScreen(pss, xmenu_setup[i], AdmSetDisplayStart);
}

void _AdmSetAdminChanged(void *pss, int index, void *act)
{
	void	*ps2;
	int		inputType;

	if(index == 1) {
		if(syscfgAdminPwdDigitsOnly(sys_cfg)) inputType = TI_NUMERIC_PASSWORD;
		else	inputType = TI_ALPHANUMERIC_PASSWORD;
		ps2 = PSobjectAtIndex(pss, 0);
		PStextFieldSetInputTypeAtIndex(ps2, 0, inputType);
		PStextFieldSetInputTypeAtIndex(ps2, 1, inputType);
	}
}

void AdmSetAdmin(void *pss)
{
	void	*ps;
	int		inputType;
	
	PSsetOnPrefChanged(pss, _AdmSetAdminChanged);
	ps = PSaddTextField(pss, PStitle(pss), AdmDoneValidatePIN);
	if(syscfgAdminPwdDigitsOnly(sys_cfg)) inputType = TI_NUMERIC_PASSWORD;
	else	inputType = TI_ALPHANUMERIC_PASSWORD;
	PStextFieldAddItem(ps, GetPromptMsg(M_ENTER_PASSWORD), syscfgAdminPwd, syscfgSetAdminPwd, inputType, 8);
	PStextFieldAddItem(ps, GetPromptMsg(M_VERIFY_PASSWORD), NULL, NULL, inputType, 8);
	PStextFieldSetValueAtIndex(ps, 1, syscfgAdminPwd(sys_cfg));
	ps = PSaddCheck(pss, xmenu_admin[1], syscfgAdminPwdDigitsOnly, syscfgSetAdminPwdDigitsOnly);
}

void AdmSetDate(int requestCode, void *pss)
{
	unsigned char	ctm[8];

	PSsetResultCode(pss, 0);
	rtcGetDate(ctm);
	DatePickerDialog(ACT_PREF+0, (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
}

void AdmSetTime(int requestCode, void *pss)
{
	unsigned char	ctm[8];
	BOOL	bVal;

	PSsetResultCode(pss, 0);
	rtcGetTime(ctm);
	if(syscfgTimeNotation(NULL)) bVal = TRUE; else bVal = FALSE;
	TimePickerDialog(ACT_PREF+1, (int)ctm[0], (int)ctm[1], bVal);
}

void _AdmSetDateTimeChanged(void *pss, int index, void *act)
{
	void	*ps, *ps2;
	unsigned char	ctm[8];
	char	*p, temp[64], key[64], value[64];

	ps = PSobjectAtIndex(pss, index);
	switch(index) {
	case 0:
		p = activityIntent(act);
		get_keyvalue(p, key, value);
		str2date(value, ctm);
		rtcGetTime(ctm+3);
		rtcSetDateTime(ctm);
		sprintf(temp, "%04d/%02d/%02d", (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
printf("ps=%x date=%s\n", ps, temp);
		PSsetSubtitle(ps, temp);
		PrefReloadRowAtIndex(act, index);
		break;
	case 1:
		p = activityIntent(act);
		get_keyvalue(p, key, value);
		str2time(value, ctm+3);
		rtcGetDate(ctm);
		rtcSetDateTime(ctm);
		GetTimeString(temp, (int)ctm[3], (int)ctm[4]);
		PSsetSubtitle(ps, temp);
		PrefReloadRowAtIndex(act, index);
		break;
	case 2:
		PSsetSubtitle(ps, xmenu_time_notation[syscfgTimeNotation(NULL)]);
		PrefReloadRowAtIndex(act, index);
		rtcGetTime(ctm);
		GetTimeString(temp, (int)ctm[0], (int)ctm[1]);
		ps2 = PSobjectAtIndex(pss, 1);
		PSsetSubtitle(ps2, temp);
		PrefReloadRowAtIndex(act, 1);
		break;
	}
}

void AdmSetDateTime(void *pss)
{
	void	*ps;
	unsigned char	ctm[8];
	char	temp[64];

	PSsetOnPrefChanged(pss, _AdmSetDateTimeChanged);
	ps = PSaddScreen(pss, xmenu_datetime[0], NULL);
	PSsetUserAction(ps, AdmSetDate);
	rtcGetDateTime(ctm);
	sprintf(temp, "%04d/%02d/%02d", (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2]);
	PSsetSubtitle(ps, temp);
	ps = PSaddScreen(pss, xmenu_datetime[1], NULL);
	PSsetUserAction(ps, AdmSetTime);
	GetTimeString(temp, (int)ctm[3], (int)ctm[4]);
	PSsetSubtitle(ps, temp);
	ps = PSaddCheck(pss, xmenu_datetime[2], syscfgTimeNotation, syscfgSetTimeNotation);
	PSsetSubtitle(ps, xmenu_time_notation[syscfgTimeNotation(NULL)]);
}

void _AdmSetDisplayChanged(void *pss, int index, void *act)
{
	if(index == 0) {
		lcdSetBrightness(syscfgLcdBrightness(NULL));
	}
}

void AdmSetSleepMode(void *pss)
{
	void	*ps;
	int		i;

	ps = PSaddSpinner(pss, xmenu_sleepmode[0], syscfgSleepMode, syscfgSetSleepMode);
	for(i = 0;i < 3;i++) PSspinnerAddTitleValue(ps, xmenu_sleepact[i], i);
	PSaddStepper(pss, xmenu_sleepmode[1], syscfgSleepWaitTime, syscfgSetSleepWaitTime, 10, 6000, 10, admSecondUnitTitle());
	ps = PSaddScreen(pss, xmenu_sleepmode[2], NULL);
	PSsetUserAction(ps, AdmVf);
}

void AdmSetDisplay(void *pss)
{
	void	*ps;
	
	PSsetOnPrefChanged(pss, _AdmSetDisplayChanged);
//	ps = PSaddSlider(pss, xmenu_display[1], syscfgLcdContrast, syscfgSetLcdContrast, 0, 100);
	ps = PSaddSlider(pss, xmenu_display[2], syscfgLcdBrightness, syscfgSetLcdBrightness, 0, 100);
	PSaddScreen(pss, xmenu_readerop[2], AdmSetSleepMode);
}

void AdmSetSound(void *pss)
{
	void	*ps;

	ps = PSaddSlider(pss, xmenu_sound[0], syscfgUserSoundVolume, syscfgSetUserSoundVolume, 0, 100);
	ps = PSaddSlider(pss, xmenu_sound[1], syscfgMediaSoundVolume, syscfgSetMediaSoundVolume, 0, 100);
	ps = PSaddSlider(pss, xmenu_sound[2], syscfgCallSoundVolume, syscfgSetCallSoundVolume, 0, 100);
	ps = PSaddSlider(pss, xmenu_sound[3], syscfgSystemSoundVolume, syscfgSetSystemSoundVolume, 0, 100);
	ps = PSaddCheck(pss, xmenu_sound[4], syscfgKeypadTone, syscfgSetKeypadTone);
	ps = PSaddCheck(pss, xmenu_sound[5], syscfgTouchSounds, syscfgSetTouchSounds);
}

void AdmSetPowerMgmt(void *pss)
{
	void	*ps;

	ps = PSaddCheck(pss, xmenu_power_mgmt[0], devEnableBattery, devSetEnableBattery);
	ps = PSaddCheck(pss, xmenu_power_mgmt[1], devEnableBatteryLock, devSetEnableBatteryLock);
}

static void _CamSetFrameRate(void *self, int frameRate)
{
	camSetFrameRate(self, frameRate);
	camRunFrameRate(frameRate);
}

static void _CamFrameRateGetValueText(int value, char *text)
{
	sprintf(text, "%d.%02d", 15/value, 15*100/value%100);
}

void AdmSetCam(void *pss)
{
	void	*ps;

	ps = PSaddStepper(pss, xmenu_cam[0], camFrameRate, _CamSetFrameRate, 1, 15, -1, "");
	PSstepperSetGetValueText(ps, _CamFrameRateGetValueText);
	ps = PSaddCheck(pss, xmenu_cam[1], camEventWithCamImage, camSetEventWithCamImage);
}

void _AdmSetBleChanged(void *pss, int index, void *act)
{
	void	*ps;

printf("BleChanged: index=%d...\n", index);
	if(index == 0 || index == 1 || index == 3) {
		if(index == 3) {
			ps = PSobjectAtIndex(pss, 3);
			PSsetBackResultCode(ps, RESULT_CANCELLED);
		}
		blemWrite();
	}
}

void AdmSetBleMacFilter(void *pss);

void AdmSetBle(void *pss)
{
	void	*ps;
	int		i;

	PSsetOnPrefChanged(pss, _AdmSetBleChanged);
	ps = PSaddCheck(pss, xmenu_ble[0], bleEnableBeaconScan, bleSetEnableBeaconScan);
	//ps = PSaddSlider(pss, xmenu_ble[2], bleMinBeaconSignal, bleSetMinBeaconSignal, 0, 100);
	PSaddStepper(pss, xmenu_ble[1], bleMinBeaconSignal, bleSetMinBeaconSignal, -100, -40, 1, "dB");
	PSaddStepper(pss, xmenu_ble[2], bleBeaconIntervalTime, bleSetBeaconIntervalTime, 10, 600, 10, admSecondUnitTitle());
	ps = PSaddScreen(pss, xmenu_ble[3], AdmSetBleMacFilter);
	ps = PSaddScreen(pss, xmenu_ble[4], NULL);
	PSsetUserAction(ps, AdmSetBleBeacon);
}

char *_BleMacFilterName(void *self)
{
	int		index;

	index = PStag(self);
	return bleMacFilterName(NULL, index);
}

void _BleSetMacFilterName(void *self, char *name)
{
	int		index;

	index = PStag(self);
	bleSetMacFilterName(NULL, index, name);
}

void _AdmSetBleMacFilterChanged(void *pss, int index, void *act)
{
printf("BleMacFilterChanged: index=%d...\n", index);
	PSsetBackResultCode(pss, RESULT_OK);
}

void AdmSetBleMacFilter(void *pss)
{
	void	*ps;
	char	temp[80];
	int		i;

	PSsetOnPrefChanged(pss, _AdmSetBleMacFilterChanged);
	for(i = 0;i < 10;i++) {
		sprintf(temp, "%s %d", xmenu_ble[3], i+1);
		ps = PSaddTextField(pss, temp, AdmDoneValidateMacFilter);
		PSsetContext(ps, ps);
		PStextFieldAddItem(ps, PStitle(ps), _BleMacFilterName, _BleSetMacFilterName, TI_HEXANUMERIC, 6);
		PSsetTag(ps, i);
	}
}

void AdmSetIpPhone(void *pss)
{
	void	*ps;

	ps = PSaddScreen(pss, xmenu_phone[0], NULL);
	PSsetUserAction(ps, AdmAcct);
}

