#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_pkt.h"
#include "defs_obj.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "dev.h"
#include "user.h"
#include "cam.h"
#include "netudp.h"
#include "svrnet.h"

SYS_CFG		*tmp_cfg, _tmp_cfg;


void SvrProcessConfRequest(NET_UDP *reqnet, NET_UDP *rspnet)
{
	switch(reqnet->head0) {
	case P_READ_PROPERTY:		ReadProperty(reqnet, rspnet); break;
	case P_WRITE_PROPERTY:		WriteProperty(reqnet, rspnet); break;
	default:	netudpCodeReject(rspnet, reqnet, R_UNKNOWN_COMMAND);
	}
}

void ReadProperty(NET_UDP *reqnet, NET_UDP *rspnet)
{
	unsigned char	*p, *data;
	long	nId;
	int		rval, result;

	if(reqnet->dataLength < 4) {
		netudpCodeReject(rspnet, reqnet, R_PARAMETER_ERROR);
		return;
	}
	data = reqnet->data;
	result = 0;
	PACK3toID(data+1, &nId);
	p = rspnet->data; p += 5;
printf("ReadObjectProperty: ID=%d %02x-%02x-%02x-%02x-%02x Data=%d\n", nId, (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], reqnet->dataLength-5);
	switch(data[0]) {
	case OT_CREDENTIAL_READER:
		switch(data[4]) {
		case OP_CAMERA:
			rval = sys_cfg->camera.runState;
			//if(camCodecState()) sys_cfg->camera.runState = 1;
			//else	sys_cfg->camera.runState = 0;
			p += camEncode(&sys_cfg->camera, p);
			sys_cfg->camera.runState = (unsigned char)rval;
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(result) {
		netudpCodeError(rspnet, reqnet, 1, result);
	} else {
		memcpy(rspnet->data, reqnet->data, 5);
		rspnet->dataLength = p - rspnet->data;
		netudpCodeResponse(rspnet, reqnet);
	}
}

void *MainLoop(void);

void WriteProperty(NET_UDP *reqnet, NET_UDP *rspnet)
{
	unsigned char	*p, *data;
	long	nId;
	int		rval, result, nSize, cam, mode;

	if(reqnet->dataLength < 5) {
		netudpCodeReject(rspnet, reqnet, R_PARAMETER_ERROR);
		return;
	}
	data = reqnet->data;
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	result = 0;
	PACK3toID(data+1, &nId);
	p = data + 5;
	nSize = reqnet->dataLength - 5;
printf("WriteProperty: ID=%d %02x-%02x-%02x-%02x-%02x Data=%d\n", nId, (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], nSize);
	cam = 0;
	switch(data[0]) {
	case OT_CREDENTIAL_READER:
		switch(data[4]) {
		case OP_CAMERA:
			if(nSize < 4) result = R_PARAMETER_ERROR;
			else {
				camDecode(&tmp_cfg->camera, p);
//cprintf("OP_CAMERA: %d %d %d 0x%02x\n", (int)tmp_cfg->Camera.Mode, (int)tmp_cfg->Camera.RunState, (int)tmp_cfg->Camera.Channel, (int)tmp_cfg->Camera.Option);
				if(!camValidate(&tmp_cfg->camera)) result = R_PARAMETER_ERROR;
				else	cam = 1;
			}
			break;
		case OP_SEND_CAM_IMAGE:
			if(nSize < 1) result = R_PARAMETER_ERROR;
			else {
				rval = *p;
				if(rval) camjpgOpenServer();
				else	 camjpgCloseServer();
			}
			break;	
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
//cprintf("result=%d\n", result);
	if(cam) {
		if(syscfgCompare(sys_cfg, tmp_cfg)) {
			rval = syscfgWrite(tmp_cfg);
			if(rval < 0) result = R_R_SYSTEM_ERROR;
			else	syscfgCopy(sys_cfg, tmp_cfg);
		}
		if(!result) {
			mode = camMode(NULL);
			/*
			if(mode == 2) {
				rval = camRun(MainLoop());
				if(!rval) camRunFrameRate(camFrameRate(NULL));
			} else if(mode == 1) camStop();
			else	camStop();
			*/
		}
	}
	if(result) {
		netudpCodeError(rspnet, reqnet, 1, result);
	} else {
		rspnet->dataLength = 0;
		netudpCodeResponse(rspnet, reqnet);
	}
}

