#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "dirent.h"
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "sysprim.h"
#include "gactivity.h"
#include "gapp.h"
#include "dlg.h"
#include "pref.h"
#include "psmem.h"
#include "file.h"
#include "hw.h"
#include "slvmlib.h"
#include "slvm.h"
#include "tftpc.h"
#include "admtitle.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "appact.h"

static int		_execCode;


void AdmSysConfirm(int requestCode, void *ps)
{
	char	temp[80];

	PSsetResultCode(ps, 0);
	sprintf(temp, "%s=%s\n", PStitle(ps), GetPromptMsg(M_ARE_YOU_REALLY));  
	AlertDialog(requestCode, temp, TRUE); 
}

void AdmSysConfirm2(int requestCode, void *ps)
{
	char	temp[100], temp2[80];

	if(!SlvnetIsConnected()) {
		GetResultMsgNoLine(R_NETWORK_NOT_OBTAINABLE, temp2);  
		sprintf(temp, "%s=%s\n", PStitle(ps), temp2);  
		AlertDialog(requestCode-900, temp, FALSE); 
	} else {
		PSsetResultCode(ps, 0);
		sprintf(temp, "%s=%s\n", PStitle(ps), GetPromptMsg(M_ARE_YOU_REALLY));  
		AlertDialog(requestCode, temp, TRUE); 
	}
}

void MainReinitializeDevice(void);

void _AdmSysReinitializeDevice(void)
{
printf("ReinitializeDevice.....\n");
	MainReinitializeDevice();
}

#include <signal.h>

void _AdmSysPowerOffDevice(void)
{
	int		i;

printf("PowerOffDevice.....\n");
//sigExitHandler(SIGINT);
	slvmResetMaster(2);
	MainToShutdown(G_POWER_OFF);
}

static void _ProcessingAlertDialog(int clearTop, char *title)
{
	char	*p, buf[80], temp[80];

	p = GetResultMsg(R_PROCESSING);
	strcpy_swap(temp, p, '\n', '&');
	sprintf(buf, "%s=%s\n", title, temp);
	if(clearTop) AlertClearTopDialog(0, buf, FALSE);
	else	AlertDialog(0, buf, FALSE); 
}

static void _SuccessAlertDialog(int clearTop, char *title)
{
	char	*p, buf[80], temp[80];

	p = GetResultMsg(R_COMPLETED);
	strcpy_swap(temp, p, '\n', '&');
	sprintf(buf, "%s=%s\n", title, temp);
	if(clearTop) AlertClearTopDialog(0, buf, FALSE);
	else	AlertDialog(0, buf, FALSE); 
}

static void _FailAlertDialog(int clearTop, char *title)
{
	char	*p, buf[128], temp[80];

	p = GetResultMsg(R_FAILED);
	strcpy_swap(temp, p, '\n', '&');
	sprintf(buf, "%s=%s\n", title, temp);
	if(clearTop) AlertClearTopDialog(0, buf, FALSE);
	else	AlertDialog(0, buf, FALSE); 
}

static void *dlgact;
static char tmpFileName[24];
static char local[80], remote[80], target[80];

void AdmSysDownloading(int status)
{
	unsigned char	msg[12];
	int		downSize;

	if(status == 1) {
		downSize = tftpcDownloadSize();
		if(downSize == 0) {
			msg[0] = GM_USER+1; msg[1] = 0;
			appPostMessage(msg);
		} else {
			msg[0] = GM_USER; msg[1] = downSize * 100 / tftpcTransferSize();
			appPostMessage(msg);
		}
	} else if(status == 2) {
		msg[0] = GM_USER+2; msg[1] = 0;
		appPostMessage(msg);
	} else if(status == 3) {
		activitySetParam2(dlgact, tftpcErrorString());
		msg[0] = GM_USER+2; msg[1] = 2;
		appPostMessage(msg);
	}
}
		
void _AdmSysDownloadWallPaper(void *ps, int index, int code)
{
	unsigned long   size;

printf("Download Wallpaper...\n");
	if(code == 0) {
		sprintf(remote, "wallpaper/w%03d%02d.bmp", SlvnetMasterDevId(), devId(NULL));
		getTmpRandFileName(local);
		sprintf(target, "wallpaper/%s", _bgscreenName);
		dlgact = ProgressDialog(ACT_PREF+index, PStitle(ps), "");
		activitySetParam1(dlgact, remote);
		fileSize(target, &size);
		tftpcGet(local, remote, target, size, AdmSysDownloading);
	} else {
		if(tftpcResult() == 0) UpdateWallpaper();
	}
}

void _AdmSysDownloadFirmware(void *ps, int index, int code)
{
	char	temp[256];
	unsigned long   size;

printf("DownloadFirmware...%d\n", code);
	if(code == 0) {
		strcpy(remote, _firmwareName);
		getTmpRandFileName(local);
		sprintf(target, "/root/app/%s.out", devFirmwareName(NULL));
		sprintf(temp, "/root/app/%s", devFirmwareName(NULL));
		dlgact = ProgressDialog(ACT_PREF+index, PStitle(ps), "");
		activitySetParam1(dlgact, remote);
		fileSize(temp, &size);
		tftpcGet(local, remote, target, size, AdmSysDownloading);
	} else {
		if(tftpcResult() == 0) {
			ExecPipeFinal(temp, "/bin/chmod", "chmod", "a+x", target, NULL);
		}
	}
}

void _AdmSysDownloadKernel(void *ps, int index, int code)
{
	char	temp[256];
	unsigned long   size;

printf("DownloadKernel...%d\n", code);
	if(code == 0) {
		strcpy(remote, _kernelName);
		getTmpRandFileName(local);
		sprintf(target, "/boot/%s", _kernelName);
		dlgact = (GACTIVITY *)ProgressDialog(ACT_PREF+index, PStitle(ps), "");
		activitySetParam1(dlgact, remote);
		activitySetTimeout(dlgact, 80);
		fileSize(target, &size);
		tftpcGet(local, remote, target, size, AdmSysDownloading);
	} else {
		if(tftpcResult() == 0) {
			ExecPipeFinal(temp, "/bin/chmod", "chmod", "a+x", target, NULL);
		}
	}
}

void _AdmSysDownloadVideo(void *ps, int index, int code)
{
	char	temp[256];
	unsigned char	buf[12];
	unsigned long	size;

printf("DownloadVideo...%d\n", code);
	if(code == 0) {
        sprintf(remote, "video/%s", _bgvideoName);
        sprintf(local, "video/%s_", _bgvideoName);
		dlgact = (GACTIVITY *)ProgressDialog(ACT_PREF+index, PStitle(ps), "");
		activitySetParam1(dlgact, remote);
		activitySetTimeout(dlgact, 80);
		fileSize(remote, &size);
		tftpcGet(local, remote, NULL, size, AdmSysDownloading);
	} else {
		if(tftpcResult() == 0) {
			buf[0] = GM_FILE_CHANGED; memset(buf+1, 0, 9);
			appPostMessage(buf);
		}
	}
}

void _AdmSysChanged(void *pss, int index, void *act)
{
	void	*ps;
	int		code;

printf("AdmSysChanged: %d\n", index);
	ps = PSobjectAtIndex(pss, index);
	code = PSresultCode(ps);
	PSsetResultCode(ps, code+1);
	switch(index) {
	case 6: _AdmSysReinitializeDevice(); break;
	case 7: _AdmSysPowerOffDevice(); break;
	}
}

void AdmSysView(void *pss);
void AdmSysInit(void *pss);
void AdmSysTest(void *pss);
void AdmSysImport(void *pss);
void AdmSysExport(void *pss);
void AdmSysDownload(void *pss);

void AdmSys(void *pss)
{
	void	*ps;

	PSsetOnPrefChanged(pss, _AdmSysChanged);
	PSaddScreen(pss, xmenu_system[1], AdmSysView);  
	PSaddScreen(pss, xmenu_system[2], AdmSysInit);  
	PSaddScreen(pss, xmenu_system[3], AdmSysTest);  
	PSaddScreen(pss, xmenu_system[4], AdmSysImport);  
	PSaddScreen(pss, xmenu_system[5], AdmSysExport);
	PSaddScreen(pss, xmenu_system[6], AdmSysDownload);  
	PSaddButton(pss, xmenu_system[7], AdmSysConfirm);  
	PSaddButton(pss, xmenu_system[8], AdmSysConfirm);  
}

void _AdmSysDownloadChanged(void *pss, int index, void *act)
{
	void	*ps;
	int		code;

printf("AdmSysDownloadChanged: %d\n", index);
	ps = PSobjectAtIndex(pss, index);
	code = PSresultCode(ps);
	PSsetResultCode(ps, code+1);
	switch(index) {
	case 0:	_AdmSysDownloadWallPaper(ps, index, code); break;
	case 1:	_AdmSysDownloadFirmware(ps, index, code); break;
	case 2:	_AdmSysDownloadKernel(ps, index, code); break;
	case 3:	_AdmSysDownloadVideo(ps, index, code); break;
	}
}

void AdmSysDownload(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmSysDownloadChanged);
	PSaddButton(pss, xmenu_download[0], AdmSysConfirm); 
	PSaddButton(pss, xmenu_download[1], AdmSysConfirm); 
	PSaddButton(pss, xmenu_download[2], AdmSysConfirm); 
	PSaddButton(pss, xmenu_download[3], AdmSysConfirm); 
}

BOOL _IsEnableCam(void *self);

void AdmSysView(void *pss)
{
	void	*ps;
	char	temp[80];

	PSaddTitle(pss, admModelTitle(), devModelName(sys_cfg), PS_TITLE_STYLE_SUBTITLE);
	getKernelVersion(temp);
	PSaddTitle(pss, admKernelVersionTitle(), temp, PS_TITLE_STYLE_SUBTITLE); 
	PSaddTitle(pss, admFirmwareVersionTitle(), syscfgFirmwareVersionName(NULL), PS_TITLE_STYLE_SUBTITLE); 
	devGetDeviceFullName(temp);
	PSaddTitle(pss, admDeviceIdTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	sprintf(temp, "%d", fsEventSize() >> 4);
	PSaddTitle(pss, admEventCountTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
//	PSaddTitle(pss, xmenu_network[1], xmenu_network_type[syscfgNetworkType(NULL)], PS_TITLE_STYLE_SUBTITLE);
	PSaddTitle(pss, xmenu_setup[2], NULL, PS_TITLE_STYLE_GROUP);
//	sprintf(temp, "%d", syscfgLcdContrast(NULL));
//	PSaddTitle(pss, xmenu_display[1], temp, PS_TITLE_STYLE_SUBTITLE);
	sprintf(temp, "%d", syscfgLcdBrightness(NULL));
	PSaddTitle(pss, xmenu_display[2], temp, PS_TITLE_STYLE_SUBTITLE);
	PSaddTitle(pss, xmenu_setup[3], NULL, PS_TITLE_STYLE_GROUP);
	sprintf(temp, "%d", syscfgUserSoundVolume(NULL));
	PSaddTitle(pss, xmenu_sound[0], temp, PS_TITLE_STYLE_SUBTITLE);
	sprintf(temp, "%d", syscfgSystemSoundVolume(NULL));
	PSaddTitle(pss, xmenu_sound[1], temp, PS_TITLE_STYLE_SUBTITLE);
	if(syscfgKeypadTone(NULL)) strcpy(temp, admYesTitle()); else strcpy(temp, admNoTitle());
	PSaddTitle(pss, xmenu_sound[2], temp, PS_TITLE_STYLE_SUBTITLE);
	if(syscfgTouchSounds(NULL)) strcpy(temp, admYesTitle()); else strcpy(temp, admNoTitle());
	PSaddTitle(pss, xmenu_sound[3], temp, PS_TITLE_STYLE_SUBTITLE);
	if(_IsEnableCam(NULL)) strcpy(temp, admYesTitle()); else strcpy(temp, admNoTitle());
	PSaddTitle(pss, xmenu_setup[7], temp, PS_TITLE_STYLE_SUBTITLE);
	if(bleEnable(NULL)) strcpy(temp, admYesTitle()); else strcpy(temp, admNoTitle());
	PSaddTitle(pss, xmenu_setup[8], temp, PS_TITLE_STYLE_SUBTITLE);
}

void _AdmSysInitChanged(void *pss, int index, void *act)
{
	switch(index) {
	case 0:
		fsRemoveAllEvent();
		break;
	case 1:
		syscfgReset(sys_cfg);
		_AdmSysReinitializeDevice();
		break;
	case 2:
		syscfgReset(sys_cfg);
		userfsRemoveAll();
		fsRemoveAllEvent();		
		mpDeleteAll();
		sync();
		_AdmSysReinitializeDevice();
		break;
	}
}

void AdmSysInit(void *pss)
{
	void	*ps;

	PSsetOnPrefChanged(pss, _AdmSysInitChanged);
	ps = PSaddButton(pss, xmenu_init_config[0], AdmSysConfirm);  
	ps = PSaddButton(pss, xmenu_init_config[1], AdmSysConfirm);  
	ps = PSaddButton(pss, xmenu_init_config[2], AdmSysConfirm);  
}

extern unsigned char pingIpAddress[];
char *PingIpAddressName(void *self);
void PingSetIpAddressName(void *self, char *name);

void AdmSysTest(void *pss)
{
	CREDENTIAL_READER	*cr;
	void	*ps;

//	PSsetOnPrefChanged(pss, _AdmSysTestChanged);
	ps = PSaddScreen(pss, xmenu_test[0], NULL);  
	PSsetUserAction(ps, AdmSysTestRun);
	ps = PSaddScreen(pss, xmenu_test[1], NULL);  
	PSsetUserAction(ps, AdmSysTestRun);
	ps = PSaddScreen(pss, xmenu_test[2], NULL);  
	PSsetUserAction(ps, AdmSysTestRun);
	ps = PSaddScreen(pss, xmenu_test[3], NULL);  
	cr = crsGet(0);
	cr->data[0] = 0; cr->captureMode = USER_CARD;
	PSsetUserAction(ps, AdmUsrCredStart);
	memcpy(pingIpAddress, syscfgServerIpAddress(NULL), 4);
	ps = PSaddTextField(pss, admIpAddressTitle(), AdmDoneValidateIpAddress);
	PStextFieldAddItem(ps, PStitle(ps), PingIpAddressName, PingSetIpAddressName, TI_IP_ADDRESS, 15);
	ps = PSaddScreen(pss, xmenu_network[7], NULL);
	PSsetUserAction(ps, AdmNetPingRun);
}

void *ExecCopyTask(void *arg);
void *KernelCopyTask(void *arg);
void *ResCopyTask(void *arg);

void _AdmSysImportChanged(void *pss, int index, void *act)
{
	void	*ps;
	pthread_t	_thread;
	GACTIVITY	*a;
	char	temp[256], path[128], path2[128];
	int		rval, code;

	ps = PSobjectAtIndex(pss, index);
	code = PSresultCode(ps);
printf("_AdmSysImportChanged: index=%d code=%d\n", index, code);
	PSsetResultCode(ps, code+1);
	switch(index) {
	case 0:
printf("ImportFirmware...%d\n", code);
		if(code == 0) {
			a = (GACTIVITY *)ProgressDialog(ACT_PREF+index, PStitle(pss), "");
			pthread_create(&_thread, NULL, ExecCopyTask, (void *)a);
		} else {
			if(_execCode == 0) {
				GetTargetFirmwareFileName(path);
				unlink(path);
				ExecPipeFinal(temp, "/bin/cp", "cp", tmpFileName, path, NULL);
				unlink(tmpFileName);
				ExecPipeFinal(temp, "/bin/chmod", "chmod", "a+x", path, NULL);
				sync();
				slvmResetMaster(1);	// 0
				MainToShutdown(G_COLD_RESTART);
			}
		}
		break;
	case 1:
printf("ImportKernel...%d\n", code);
		if(code == 0) {
			a = (GACTIVITY *)ProgressDialog(ACT_PREF+index, PStitle(pss), "");
			activitySetTimeout(a, 80);
			pthread_create(&_thread, NULL, KernelCopyTask, (void *)a);
		} else {
			if(_execCode == 0) {
				sprintf(path, "%s/%s", tgtBootDir, _kernelName);
				unlink(path);
				ExecPipeFinal(temp, "/bin/cp", "cp", tmpFileName, path, NULL);
				unlink(tmpFileName);
				ExecPipeFinal(temp, "/bin/chmod", "chmod", "a+x", path, NULL);
				sync();
				slvmResetMaster(1);	// 0
				MainToShutdown(G_COLD_RESTART);
			}
		}
		break;
	case 2:
printf("ImportResources...%d\n", code);
		if(code == 0) {
			a = (GACTIVITY *)ProgressDialog(ACT_PREF+index, PStitle(pss), "");
			pthread_create(&_thread, NULL, ResCopyTask, (void *)a);
		}
		break;
	case 3:
printf("ImportBgScreen...%d\n", code);
		if(code == 0) {
			sprintf(path, "%s/wallpaper/w%03d%02d.bmp", usbAppDir, SlvnetMasterDevId(), devId(NULL));
			getTmpRandFileName(tmpFileName);
			rval = fileCopy(path, tmpFileName, NULL);
			sync();
			if(rval < 0) _FailAlertDialog(0, PStitle(pss));
			else {
				sprintf(path, "%s/wallpaper/%s", tgtAppDir, _bgscreenName);
				unlink(path);
				ExecPipeFinal(temp, "/bin/cp", "cp", tmpFileName, path, NULL);
				unlink(tmpFileName);
				sync();
				UpdateWallpaper();
				_SuccessAlertDialog(0, PStitle(pss));
			}
		}
		break;
	case 4:
		if(code == 0) {
printf("ImportSetupFiles...\n");
			sprintf(path, "%s/cfg", usbAppDir);
			sprintf(path2, "%s/cfg", tgtAppDir);
			rval = dirCopy(path, path2, NULL);
			sync();
			if(rval < 0) _FailAlertDialog(0, PStitle(pss));
			else	_SuccessAlertDialog(0, PStitle(pss));
		}
		break;
	case 5:
		if(code == 0) {
printf("ImportUserFiles...\n");
			fsCloseFiles();
			sprintf(path, "%s/data", usbAppDir);
			sprintf(path2, "%s/data", tgtAppDir);
			rval = dirCopy(path, path2, NULL);
			sync();
			rval = fsOpenFiles(GetUserFileSize());
			sync();
			if(rval < 0) _FailAlertDialog(0, PStitle(pss));
			else	_SuccessAlertDialog(0, PStitle(pss));
		}
		break;
	}
}

static int	usbMounted;

void _AdmSysImexportDestroy(void *pss)
{
	if(usbMounted) {	
		umountUsb();
		usbMounted = 0;
	}
}

void AdmSysImport(void *pss)
{
	char	temp[128];
	int		rval;

	PSsetOnDestroy(pss, _AdmSysImexportDestroy);
	rval = mountUsb();
	if(rval) {
		usbMounted = 0;
		strcpy_swap(temp, GetResultMsg(R_FAILED), '\n', ' ');
		PSaddTitle(pss, "USB", temp, PS_TITLE_STYLE_SUBTITLE);
	} else {
		usbMounted = 1;
		PSsetOnPrefChanged(pss, _AdmSysImportChanged);
		PSaddButton(pss, xmenu_imexport[0], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[1], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[2], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[3], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[4], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[5], AdmSysConfirm);  
	}
}

void _AdmSysExportChanged(void *pss, int index, void *act)
{
	void	*ps;
	char	srcPath[64], dstPath[64];
	int		rval, code;

	ps = PSobjectAtIndex(pss, index);
	code = PSresultCode(ps);
	PSsetResultCode(ps, code+1);
	switch(index) {
	case 0:
		if(code == 0) {
			sprintf(srcPath, "%s/cfg", tgtAppDir);
			sprintf(dstPath, "%s/cfg", usbAppDir);
			rval = dirCopy(srcPath, dstPath, NULL);
			sync();
			if(rval < 0) _FailAlertDialog(0, PStitle(pss));
			else	_SuccessAlertDialog(0, PStitle(pss));
		}
		break;
	case 1:
		if(code == 0) {
			sprintf(srcPath, "%s/data", tgtAppDir);
			sprintf(dstPath, "%s/data", usbAppDir);
			rval = dirCopy(srcPath, dstPath, NULL);
			sync();
			if(rval < 0) _FailAlertDialog(0, PStitle(pss));
			else	_SuccessAlertDialog(0, PStitle(pss));
		}
		break;
	}
}

void AdmSysExport(void *pss)
{
	char	temp[128];
	int		rval;

	PSsetOnDestroy(pss, _AdmSysImexportDestroy);
	rval = mountUsb();
	if(rval) {
		usbMounted = 0;
		strcpy_swap(temp, GetResultMsg(R_FAILED), '\n', ' ');
		PSaddTitle(pss, "USB", temp, PS_TITLE_STYLE_SUBTITLE);
	} else {
		usbMounted = 1;
		PSsetOnPrefChanged(pss, _AdmSysExportChanged);
		PSaddButton(pss, xmenu_imexport[4], AdmSysConfirm);  
		PSaddButton(pss, xmenu_imexport[5], AdmSysConfirm);
	}
}

void *ExecCopyTask(void *arg)
{
	GACTIVITY	*act;
	unsigned char	msg[12];
	char	srcFile[64];
	unsigned long   size;
	int		rval, i;

	usleep(250);
	act = (GACTIVITY *)arg;
	for(i = 0;i < 1;i++) {
		if(i == 0) {
			sprintf(srcFile, "%s/%s", usbAppDir, _firmwareName);
printf("srcFile=[%s]\n", srcFile);
			getTmpRandFileName(tmpFileName);
printf("dstFile=[%s]\n", tmpFileName);
		}
		rval = fileSize(srcFile, &size);
		if(rval < 0) break;
		TaskSetCopySize(size);
		activitySetParam1(act, tmpFileName+5);
		msg[0] = GM_USER+1; msg[1] = 0;
		appPostMessage(msg);
		usleep(30);
		rval = fileCopy(srcFile, tmpFileName, TaskCopying);
		if(rval < 0) break;
	}
	sync();
	msg[0] = GM_USER+2;
	if(rval < 0) msg[1] = 2;
	else	msg[1] = 0;
	appPostMessage(msg);
	_execCode = msg[1];
	return (void *)0;
}

void *KernelCopyTask(void *arg)
{
	GACTIVITY	*act;
	unsigned char	msg[12];
	char	srcFile[64];
	unsigned long   size;
	int		rval, i;

	usleep(250);
	act = (GACTIVITY *)arg;
	for(i = 0;i < 1;i++) {
		if(i == 0) {
			sprintf(srcFile, "%s/%s", usbBootDir, _kernelName);
printf("srcFile=[%s]\n", srcFile);
			getTmpRandFileName(tmpFileName);
printf("dstFile=[%s]\n", tmpFileName);
		}
		rval = fileSize(srcFile, &size);
		if(rval < 0) break;
		TaskSetCopySize(size);
		activitySetParam1(act, tmpFileName+5);
		msg[0] = GM_USER+1; msg[1] = 0;
		appPostMessage(msg);
		usleep(30);
		rval = fileCopy(srcFile, tmpFileName, TaskCopying);
		if(rval < 0) break;
	}
	sync();
	msg[0] = GM_USER+2;
	if(rval < 0) msg[1] = 2;
	else	msg[1] = 0;
	appPostMessage(msg);
	_execCode = msg[1];
	return (void *)0;
}

void *ResCopyTask(void *arg)
{
	GACTIVITY	*act;
	unsigned char	msg[12];
	char	srcPath[64], dstPath[64];
	int		rval, i;

	usleep(250);
	act = (GACTIVITY *)arg;
	for(i = 0;i < 3;i++) {
		if(i == 0) {
			sprintf(srcPath, "%s/res", usbAppDir);
			sprintf(dstPath, "%s/res", tgtAppDir);
		} else if(i == 1) {	
			sprintf(srcPath, "%s/ment", usbAppDir);
			sprintf(dstPath, "%s/ment", tgtAppDir);
		} else {
			sprintf(srcPath, "%s/video", usbAppDir);
			sprintf(dstPath, "%s/video", tgtAppDir);
		}
		rval = dirUsage(srcPath);
		if(rval < 0) {
			msg[0] = GM_USER+2; msg[1] = 1;
			appPostMessage(msg);
			return (void *)0;
		}
		TaskSetCopySize(rval);
		activitySetParam1(act, dstPath);
		msg[0] = GM_USER+1; msg[1] = 0;
		appPostMessage(msg);
		usleep(30);
		rval = dirCopy(srcPath, dstPath, TaskCopying);
		if(rval < 0) break;
	} 
	sync();
	msg[0] = GM_USER+2;
	if(rval < 0) msg[1] = 2;
	else	msg[1] = 0;
	appPostMessage(msg);
	_execCode = msg[1];
	return (void *)0;
}

/*
static void _DirOpenError(char *title, char *dirPath)
{
	char	*p, buf[80], temp[80];

	if(errno == ENOENT)  p = GetResultMsg(R_DIRECTORY_NOT_FOUND);
	else	p = GetResultMsg(R_DIR_OPEN_ERROR);
	strcpy_swap(temp, p, '\n', '&');
	sprintf(buf, "%s=%s%s\n", title, dirPath, temp);
	AlertDialog(0, buf, FALSE); 
}
*/

