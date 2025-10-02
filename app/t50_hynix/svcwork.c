#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "defs.h"
#include "rtc.h"
#include "cbuf.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "ad.h"
#include "subdev.h"
#include "appact.h"
#include "netsvc.h"
#include "svcwork.h"


void _SvcWorkConf(NET_SVC *svc);
void _SvcWorkConfReqMore(NET_SVC *svc);
void _SvcWorkConfRspMore(NET_SVC *svc);

void *_SvcWorkMain(void *arg)
{
	void	*subdev;
	NET_SVC	*svc;
	unsigned char	msg[4];
	int		rval, id;

	while(1) {
		rval = _GetSvcWorkMessage(msg);
		if(rval > 0) {
			id = msg[0];
			if(id >= 0 && id < 32) {
				subdev = subdevsGet(id);
				svc = subdevConfSvc(subdev);
				if(msg[1] == 3) _SvcWorkConfRspMore(svc);
				else if(msg[1] == 2) _SvcWorkConfReqMore(svc);
				else	_SvcWorkConf(svc);
			}				
		}
	}
	return arg;
}

void _SvcWorkConf(NET_SVC *svc)
{
/*
	rspnet = (NET *)tsm->SvcNet;
	rspnet->Address		= tsm->Address;
	rspnet->InvokeID	= tsm->InvokeID;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	gResetFlag = 0;
	switch(tsm->ServiceChoice) {
	case P_READ_OBJECT:			ReadObject(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY:		ReadProperty(tsm, reqnet, rspnet); break;
	case P_READ_STATUS:			ReadStatus(tsm, reqnet, rspnet); break;
	case P_WRITE_OBJECT:		WriteObject(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY:		WriteProperty(tsm, reqnet, rspnet); break;
	case P_DELETE_OBJECT:		DeleteObject(tsm, reqnet, rspnet); break;
	case P_READ_OBJECT_LIST:	ReadObjectList(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY_LIST:	ReadPropertyList(tsm, reqnet, rspnet); break;
	case P_READ_STATUS_LIST:	ReadStatusList(tsm, reqnet, rspnet); break;
	case P_WRITE_OBJECT_LIST:	WriteObjectList(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY_LIST:	WritePropertyList(tsm, reqnet, rspnet); break;
	case P_READ_FILE:			ReadFile(tsm, reqnet, rspnet); break;
	case P_WRITE_FILE:			WriteFile(tsm, reqnet, rspnet); break;
	case P_REINITIALIZE_DEVICE:	ReinitializeDevice(tsm, reqnet, rspnet); break;	
#ifndef STAND_ALONE
	case P_REINITIALIZE_CREDENTIAL_READER:	ReinitializeCredentialReader(tsm, reqnet, rspnet); break;
#endif
	default:	netCodeReject(rspnet, R_UNKNOWN_COMMAND);
	}
	tsm->SvcPending = 1;
	if(gResetFlag) {
		taskDelay(3);	// wait for send Response
		ResetSystem();
	}
*/
}

void _SvcWorkConfReqMore(NET_SVC *svc)
{
/*
	rspnet = (NET *)tsm->SvcNet;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	switch(tsm->ServiceChoice) {
	case P_WRITE_OBJECT_LIST:	WriteObjectList(tsm, reqnet, rspnet); break;
	case P_WRITE_PROPERTY_LIST:	WritePropertyList(tsm, reqnet, rspnet); break;
	case P_WRITE_FILE:			WriteFile(tsm, reqnet, rspnet); break;
	default:		netCodeReject(rspnet, R_UNKNOWN_COMMAND);
	}
	tsm->SvcPending = 1;
*/
}

void _SvcWorkConfRspMore(NET_SVC *svc)
{
/*
	rspnet = (NET *)tsm->SvcNet;
	rspnet->DataLength	= 0;
	sUser = &_sUser;
	switch(tsm->ServiceChoice) {
	case P_READ_OBJECT_LIST:	ReadObjectList(tsm, reqnet, rspnet); break;
	case P_READ_PROPERTY_LIST:	ReadPropertyList(tsm, reqnet, rspnet); break;
	case P_READ_STATUS_LIST:	ReadStatusList(tsm, reqnet, rspnet); break;
	case P_READ_FILE:			ReadFile(tsm, reqnet, rspnet); break;
	default:		netCodeAbortSrv(rspnet, R_UNKNOWN_COMMAND);
	}
	tsm->SvcPending = 1;
*/
}

/*
#include "svcwork_read.c"
#include "svcwork_write.c"
#include "svcwork_del.c"
#include "svcwork_rdlist.c"
#include "svcwork_wrlist.c"
#include "svcwork_etc.c"
*/

