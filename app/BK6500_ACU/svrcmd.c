#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "lcdc.h"
#include "iodev.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "syscfg.h"
#include "sclib.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#ifndef STAND_ALONE
#include "acaz.h"
#endif
#include "bio.h"
#include "fire.h"
#include "fsuser.h"
#include "sche.h"
#include "fsar.h"
#include "cr.h"
#include "evt.h"
#include "ta.h"
#ifndef STAND_ALONE
#include "cli.h"
#endif
#include "event.h"
#include "net.h"
#include "svr.h"
#include "msg.h"
#include "lang.h"
#include "msgsvr.h"
#include "cr.h"
#include "alm.h"
#include "fpapi_new.h"
#include "sysdep.h"
#include "menuprim.h"
#include "wnds.h"
#include "sinport.h"
#include "slavenet.h"
#include "desktop.h"
#include "devprim.h"
#include "admprim.h"
#include "topprim.h"
#include "svrcmd.h"

extern int 	gRequest, gCommand;
extern NET_TSM	ReqTSMs[], RspTSMs[];


SYS_CFG	*tmp_cfg, _tmp_cfg;
FP_USER	*sUser, _sUser;

int		gResetFlag;

void SvrProcessConfRequest(NET_TSM *tsm, NET *reqnet)
{
	NET		*rspnet;

	rspnet = (NET *)tsm->SvcNet;
	rspnet->Address		= tsm->Address;
	rspnet->InvokeID	= tsm->InvokeID;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	gResetFlag = 0;
	switch(tsm->ServiceChoice) {
	case P_READ_OBJECT:  		 ReadObject(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY:		 ReadProperty(tsm, reqnet, rspnet); break;
	case P_READ_STATUS:			 ReadStatus(tsm, reqnet, rspnet); break;
	case P_WRITE_OBJECT:		 WriteObject(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY:		 WriteProperty(tsm, reqnet, rspnet); break;
	case P_DELETE_OBJECT:		 DeleteObject(tsm, reqnet, rspnet); break;
	case P_READ_OBJECT_LIST:  	 ReadObjectList(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY_LIST:	 ReadPropertyList(tsm, reqnet, rspnet); break;
	case P_READ_STATUS_LIST:	 ReadStatusList(tsm, reqnet, rspnet); break;
	case P_WRITE_OBJECT_LIST:	 WriteObjectList(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY_LIST:	 WritePropertyList(tsm, reqnet, rspnet); break;
	case P_READ_FILE:			 ReadFile(tsm, reqnet, rspnet); break;
	case P_WRITE_FILE:			 WriteFile(tsm, reqnet, rspnet); break;
	case P_REINITIALIZE_DEVICE:	 ReinitializeDevice(tsm, reqnet, rspnet); break;	
#ifndef STAND_ALONE
	case P_REINITIALIZE_CR:		 CrReinitialize(tsm, reqnet, rspnet); break;
	case P_RUN_SCRIPT:			 CrRunScript(tsm, reqnet, rspnet); break;	
	case P_DOWNLOAD_FIRMWARE_CR: CrDownloadFirmware(tsm, reqnet, rspnet); break;	
	case P_DOWNLOAD_WALLPAPER_CR: CrDownloadWallPaper(tsm, reqnet, rspnet); break;	
	case P_DOWNLOAD_KERNEL_CR:	 CrDownloadKernel(tsm, reqnet, rspnet); break;	
	case P_DOWNLOAD_VIDEO_CR:	 CrDownloadVideo(tsm, reqnet, rspnet); break;	
	case P_DOWNLOAD_POLL_CR:	 CrDownloadPoll(tsm, reqnet, rspnet); break;	
#endif
	default:	netCodeReject(rspnet, R_UNKNOWN_COMMAND); tsm->SvcPending = 1;
	}
	if(gResetFlag) {
		taskDelay(3);	// wait for send Response
		ResetSystem();
	}
}

void SvrProcessConfRequestMor(NET_TSM *tsm, NET *reqnet)
{
	NET		*rspnet;

	rspnet = (NET *)tsm->SvcNet;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	switch(tsm->ServiceChoice) {
	case P_WRITE_OBJECT_LIST:	WriteObjectList(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY_LIST:	WritePropertyList(tsm, reqnet, rspnet); break;
	case P_WRITE_FILE:			WriteFile(tsm, reqnet, rspnet); break;
	default:		netCodeReject(rspnet, R_UNKNOWN_COMMAND); tsm->SvcPending = 1;
	}
}

void SvrProcessAck(NET_TSM *tsm, NET *reqnet)
{
	NET		*rspnet;

	rspnet = (NET *)tsm->SvcNet;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	switch(tsm->ServiceChoice) {
	case P_READ_OBJECT_LIST:  	ReadObjectList(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY_LIST:	ReadPropertyList(tsm, reqnet, rspnet); break;
	case P_READ_STATUS_LIST:	ReadStatusList(tsm, reqnet, rspnet); break;
	case P_READ_FILE:			ReadFile(tsm, reqnet, rspnet); break;
	default:		netCodeAbortSrv(rspnet, R_UNKNOWN_COMMAND);	tsm->SvcPending = 1;
	}
}

#include "svrcmd_read.c"
#include "svrcmd_write.c"
#include "svrcmd_del.c"
#include "svrcmd_rdlist.c"
#include "svrcmd_wrlist.c"
#include "svrcmd_etc.c"
