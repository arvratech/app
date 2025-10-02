#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "spimlib.h"
#include "slvmlib.h"
#include "slvm.h"
#include "user.h"
#include "psu.h"
#include "hw.h"
#include "cam.h"
#include "aa.h"
#include "crcred.h"
#include "cf.h"
#include "sysdep.h"
#include "syscfg.h"
#include "schecfg.h"
#include "tacfg.h"
#include "audio.h"
#include "gactivity.h"
#include "gapp.h"
#include "gesture.h"
#include "svrnet.h"
#include "slvnet.h"
#include "nilib.h"
#include "dhcp.h"
#include "wpanet.h"
#include "wpactrl.h"
#include "viewprim.h"
#include "psmem.h"
#include "actprim.h"
#include "epolllib.h"
#include "inputlib.h"
#include "slvm.h"
#include "blem.h"
#include "fpm.h"
#include "evt.h"
#include "uv.h"
#include "av.h"
#include "psu.h"
#include "jpegdev.h"
#include "fsuser.h"
#include "beacon.h"
#include "pjsualib.h"
#include "pjsua_app.h"
#include "scthread.h"
#include "intrusion.h"
#include "appact.h"
#include "appdev.h"
#include "maininit.h"
#include "maininput.h"

uv_timer_t	*timerMain, _timerMain;
uv_work_t	_workMain;
int			slvm_fd;
int			blem_fd;
int			wdFd, gRequest;
unsigned char	gFuncState, gFuncKey, gFuncTimer, gSlaveState, fpmTouch, gEvtPending, gDevReset;
unsigned char	sleepMode, bleViews, poeLoop, acState;
int		sleepTimer, authCount;

void InitializeHardware(void);
void _OnTouchScreen(uv_poll_t *handle, int status, int events);
void _OnMainTimer(uv_timer_t *handle);
void _OnBeaconsTimer(uv_timer_t *handle);
void AppSockClose(void);
void MainPostPipe(unsigned char *buf, int length);


int MainInit(void)
{
	void	*loop;
	int		rval;

	rtcOpen();
	rval = lcdOpen();
	if(rval < 0) {
		printf("=== lcdOpen error ===\n");
		return -1;
	}
	lcdOnBacklight();
	lcdSetBrightness(syscfgLcdBrightness(NULL));
	lcdSetContrast(syscfgLcdContrast(NULL));
	usleep(250);
	rtcSync();
	schecfgInitialize();
	tacfgInitialize();
	rval = fsOpenFiles(GetUserFileSize());
	gRequest = 0;
	authCount = 0;
	sleepMode = 0; sleepTimer = 0;
	poeLoop = acState = 0;
	loop = uv_default_loop();
	InitializeHardware();
devSetMode(NULL, 1);
//camSetEventWithCamImage(NULL, TRUE);
//camSetMode(NULL, 1);
	evtInit();
	jpegDevOpen();
//	pjsuaInit(on_call_state, on_call_media_state);
	AvOpen();
	audioInit();
    slvm_fd = -1;
	blem_fd = -1;
	slvm_fd = slvmOpen();
#ifdef _ESP32
	blem_fd = iowmEventOpen();
#else
	blem_fd = spimOpen();
#endif
	rval = MainOpenSlvm();
	if(rval < 0) {
		return -1;
	}
#ifndef _ESP32
	rval = MainOpenBlem();
	if(rval < 0) {
		return -1;
	}
#endif
	rval = MainOpenPipe();
	if(rval < 0) {
		return -1;
	}
	rval = MainOpenSio();
	if(rval < 0) {
		return -1;
	}
usleep(9000);
	initRand();
	dhcpInit();
	initUIKit();
	initUITimers(loop);
	InitializeActivitys();
	ViewInitBuffer();
	PSInitBuffer();
	appInit(loop, MainOnCreate);
	appSetSoundSetting((int)sys_cfg->systemSoundVolume, (int)sys_cfg->mediaOption);
	tamperInit();
	intrusionInit();
	gestureInit(loop);
	if(devDisplayModel(NULL)) {
		beaconsInit(_OnBeaconsTimer);
	}
	wdOpen();
	timerMain = &_timerMain;
	uv_timer_init(loop, timerMain);
	uv_timer_start(timerMain, _OnMainTimer, 20, 0);
	uv_init_async_threads();
	if(syscfgNetworkType(NULL)) {
		SlvnetSioInit();
	} else {
		niInit(AppSockClose);
		SlvnetUdpInit();
		SvrnetUdpInit();
	}
/*
	if(devDisplayModel(NULL)) {
		if(camMode(NULL) == 2) {
			rval = avStreamInputStart(NULL);
			if(!rval) camRunFrameRate(camFrameRate(NULL));
		}
	}
*/
	usleep(2500);
	scThreadCreate();
	return 0;
}

void MainExit(void)
{
printf("MainExit...\n");
	wdClose();
//	svwmResetMaster(1);
//	fsCloseFiles();
	uv_timer_stop(timerMain);
	appExit();
	if(devDisplayModel(NULL)) {
		//avStreamInputStop(streamIn);
	}
	MainCloseSio();
	MainClosePipe();
	MainCloseSlvm();
#ifdef _ESP32
	iowmEventClose(blem_fd);
#else
	MainCloseBlem();
	spimClose(blem_fd);
#endif
	usleep(1000); //modified at 2024.04.09 
	slvmReset(0);
	slvmClose(slvm_fd);
	audioExit();
	AvClose();
//	pjsuaExit();
	jpegDevClose();
//	fsCloseFiles();
printf("Exit.......\n");
}

void uv_threadpool_cleanup(void);

void MainShutdown(void)
{
	void    *loop;

printf("### MainShutdown ###\n");
	loop = MainLoop();
	uv_threadpool_cleanup();
	uv_async_watcher_send(loop);
	uv_stop(loop);
}

void MainTask(void)
{
printf("### uv_run()...\n");
	uv_run(MainLoop(), 0);
printf("### uv_run() exit...\n");
}

void MainStart(void)
{
    unsigned char   buf[12];

    MainStartPipe();
    buf[0] = GM_SS_CHANGED; memset(buf+1, 9, 0);
    appPostMessage(buf);
}

void MainToShutdown(int reason)
{
printf("### ToShutdown: %d ###\n", reason);
	devSetShutdown(NULL, reason);
	MainCloseSlvm();
	MainClosePipe();
	devSetSystemStatus(NULL, SS_SHUTDOWN);
	//if(reason == G_USER_SHUTDOWN) PostMoveHomeActivity();
	//else	MoveHomeActivity();
	MoveHomeActivity();
}

void MainReinitializeDevice(void)
{
    //slvmResetMaster(1);
    MainToShutdown(G_WARM_RESTART);
}

void _OnMainTimer(uv_timer_t *handle)
{
	int		val, timeout, state;

	wdPing();
	MainOnReadSlvm();
#ifndef _ESP32
	if(bleEnable(NULL)) MainOnReadBlem();
#endif
	val = ioGetPoe();
	state = psuAcAlarmState(NULL);
	if(state) {
		if(!acState) {
			poeLoop = 1;
			val = psuPoeStatus(NULL);
		} else if(poeLoop) {
			poeLoop++;
			if(poeLoop < 4) val = psuPoeStatus(NULL);
			else	poeLoop = 0;
		}
	} else {
		if(val) {
			poeLoop = 0;
		} else {
			poeLoop++;
			if(poeLoop < 8) val = psuPoeStatus(NULL);
			else {
				val = 0; poeLoop = 0;
			}
		}
	}
	acState = state;
	PsuOnPoeStatusChanged(NULL, val);
	PsuOnDataChanged();
	if(devSystemStatus(NULL) == SS_OPERATIONAL) timeout = 3000;
	else	timeout = 70;
//printf("%lu timeout...\n", MS_TIMER, timeout);
	uv_timer_start(timerMain, _OnMainTimer, timeout, 0);
	if(gDevReset) MainReinitializeDevice();
}

void MainTimer(int timeout)
{
	uv_timer_start(timerMain, _OnMainTimer, timeout, 0);
}

extern unsigned char    bleUpdated;

void _OnBeaconsTimer(uv_timer_t *handle)
{
	int     rval;

	rval = beaconsScan();
	if(rval) bleUpdated = 1;
	beaconsTimer(5000);
}

uv_work_t *GetMainWork(void)
{
	return &_workMain;
}

void MainSvcPost(int type)
{
	//buf[0] = 0x33;
	//buf[1] = type;
	//write(pipeMain[1], buf, 2);
}

void AppSockClose(void)
{
}

