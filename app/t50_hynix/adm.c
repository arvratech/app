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
#include "sysdep.h"
#include "syscfg.h"
#include "schecfg.h"
#include "tacfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "prefspec.h"
#include "psmem.h"
#include "dlg.h"
#include "acu.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "pref.h"
#include "slvmlib.h"
#include "slvm.h"
#include "blem.h"
#include "appact.h"

SYS_CFG		*uisys_cfg, _uisys_cfg;
SCHE_CFG	*uische_cfg, _uische_cfg;
TA_CFG		*uita_cfg, _uita_cfg;


void AdmStart(void *pss)
{
	void	*ps;

	ps = PSaddScreen(pss, xmenu_top[0], NULL);
	PSsetUserAction(ps, AdmMasterPassword);
	PSscreenSetImageFile(ps, "users.bmp");
	ps = PSaddScreen(pss, xmenu_top[1], AdmHwDevStart);
	PSscreenSetImageFile(ps, "hardware.bmp");
	ps = PSaddScreen(pss, xmenu_top[3], AdmNet);
	PSscreenSetImageFile(ps, "network.bmp");
	ps = PSaddScreen(pss, xmenu_top[4], AdmSet);
	PSscreenSetImageFile(ps, "dev_settings.bmp");
	ps = PSaddScreen(pss, xmenu_top[5], AdmSys);
	PSscreenSetImageFile(ps, "system.bmp");
}

PS_SCREEN	_psscreen;

void AdmRoot(int requestCode)
{
	void	*ps;

printf("AdmRoot...\n");
	uisys_cfg = &_uisys_cfg;
	syscfgCopy(uisys_cfg, sys_cfg);
	uische_cfg = &_uische_cfg;
	schecfgCopy(uische_cfg, sche_cfg);
	uita_cfg = &_uita_cfg;
	tacfgCopy(uita_cfg, ta_cfg);
	ps = &_psscreen;
	PSscreenInit(ps, xmenu_top_title, AdmStart);
	PrefRecreate(requestCode, ps);
}

void AdmRootExit(void)
{
	BOOL	bVal;
	int		rval;

printf("AdmSaveConfig...\n");
	if(tacfgCompare(ta_cfg, uita_cfg)) {
		rval = tacfgWrite(ta_cfg);
	}
	if(schecfgCompare(sche_cfg, uische_cfg)) {
		rval = schecfgWrite(sche_cfg);
	}
	if(syscfgCompare(sys_cfg, uisys_cfg)) {
		rval = syscfgWrite(sys_cfg);
		slvmWriteLocalIoConfig();
		usleep(300);
		slvmWriteReaderConfig();
		usleep(300);
#ifndef _ESP32
		blemWrite();
#endif
		if(uisys_cfg->devId != sys_cfg->devId || memcmp(uisys_cfg->masterIpAddress, sys_cfg->masterIpAddress, 4)
			|| uisys_cfg->masterIpPort != sys_cfg->masterIpPort) SlvnetSetAddress();
		if(uisys_cfg->networkOption & USE_DHCP) bVal = TRUE; else bVal = FALSE;
		if(syscfgUseDhcp(sys_cfg) != bVal || !syscfgUseDhcp(sys_cfg) && 
				(memcmp(uisys_cfg->ipAddress, syscfgIpAddress(sys_cfg), 4)
				|| memcmp(uisys_cfg->subnetMask, syscfgSubnetMask(sys_cfg), 4)
				|| memcmp(uisys_cfg->gatewayIpAddress, syscfgGatewayIpAddress(sys_cfg), 4))) {
printf("Network Changed...\n");
			SlvnetSetAddress();
			niApplyChanges();
		}
	}
}

void PasswordOnDismiss(GACTIVITY *act)
{
	char	key[32], val[32];
	int		resultCode;

	resultCode = activityResultCode(act);
printf("PasswordOnDismiss...%x resultCode=%d\n", act, resultCode);
	if(resultCode == RESULT_OK) {
		get_keyvalue(act->intent, key, val);
		if(strcmp(val, sys_cfg->adminPassword)) DestroyActivityForResult(act, resultCode, NULL);
		else	AdmRoot(ACT_ADM);
	} else {
printf("PasswordOnDismiss...cancelled\n");
		DestroyActivityForResult(act, resultCode, NULL);
	}
}

void AdmPassword(int requestCode)
{
	void	*act;
	char	temp[80];

	if(devAdminPass(NULL)) {
printf("Bypassing password...\n");
		 AdmRoot(ACT_ADM);
	} else {
printf("AdmPassword...\n");
		if(sys_cfg->devOption & ADMPWD_DIGITS_ONLY)
			sprintf(temp, "Pwd=0\n%s=\n%d,8\n", GetPromptMsg(M_ENTER_PASSWORD), TI_NUMERIC_PASSWORD);
		else	
			sprintf(temp, "Pwd=0\n%s=\n%d,8\n", GetPromptMsg(M_ENTER_PASSWORD), TI_ALPHANUMERIC_PASSWORD);
		act = PasswordClearTopDialog(requestCode, temp, NULL);
		activitySetOnDismiss(act, PasswordOnDismiss);
	}
}

void MasterPasswordOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	char	temp[80];
	int		val;

	if(status) AcuRequestDestroy(act);
	else {
		if(buf[0]) val = R_FAILED;
		else {
			bcd2string(buf+1, 8, temp);
printf("result=%d [%s][%s]\n", status, temp, act->intent);
			if(strcmp(act->intent, temp)) val = R_PW_MISMATCHED;
			else	val = 0;
		}
		if(val) AcuRequestSetResult(act, val);
		else {
			UItimerStop(1);
			AdmClearTopUsr(0, activityPsContext(act));
		}
	}
}

void MasterPasswordOnDismiss(GACTIVITY *act)
{
	void	*ps;
	char	key[32], val[32];
	int		resultCode;

	resultCode = activityResultCode(act);
	if(resultCode == RESULT_OK) {
printf("MasterPasswordOnDismiss: OK\n");
		activitySetOnDismiss(act, NULL);
		get_keyvalue(act->intent, key, val);
		AcuRequestRestart(val, MasterPasswordOnResult);
		acuReqReadPassword();
	} else {
printf("MasterPasswordOnDismiss: Canceleld\n");
		DestroyActivityForResult(act, resultCode, NULL);
	}
}

void AdmMasterPassword(int requestCode, void *ps)
{
	void	*act;
	char	temp[80];

	sprintf(temp, "Pwd=0\n%s=\n%d,8\n", admMasterPasswordTitle(), TI_NUMERIC_PASSWORD);
	act = PasswordDialog(requestCode, temp, NULL);
	activitySetPsContext(act, ps);
	activitySetOnDismiss(act, MasterPasswordOnDismiss);
}

