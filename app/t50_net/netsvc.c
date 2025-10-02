#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include "defs.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "user.h"
#include "hw.h"
#include "cf.h"
#include "syscfg.h"
#include "dev.h"
#include "net.h"
#include "netsvc.h"

#define NET_PKT_SZ		512
#define NET_MAX_SZ		2048


void netsvcInit(NET_SVC *svc, int svcType, unsigned char *data)
{
//	svc->svcType	= svcType;
//	svc->state		= 0;
	svc->data		= data;
}

void netsvcCodeReq(NET_SVC *svc, unsigned char *buf)
{
	int		len;

	buf[0] = T_REQ;
	buf[1] = svc->invokeId;
	buf[2] = svc->serviceChoice;
	buf[3] = 0;
	len = svc->dataLength;
	memcpy(buf+4, svc->data, len);
	svc->dataLength = 0;
	svc->svcLength = len + 4;
}

void netsvcCodeReqMor(NET_SVC *svc, unsigned char *buf)
{
	int		len;

	buf[0] = T_REQ_MOR;
	buf[1] = svc->invokeId;
	buf[2] = svc->serviceChoice;
	buf[3] = 0;
	if(svc->dataLength > NET_PKT_SZ) len = NET_PKT_SZ;
	else	len = svc->dataLength; 
	memcpy(buf+4, svc->data, len);
	svc->dataLength -= len;
	svc->svcLength = len + 4;
}

void netsvcCodeRsp(NET_SVC *svc, unsigned char *buf)
{
	int		len;

	buf[0] = T_RSP;
	buf[1] = svc->invokeId;
	buf[2] = svc->serviceChoice;
	buf[3] = 0;
	len = svc->dataLength;
	memcpy(buf+4, svc->data, len);
	svc->dataLength = 0;
	svc->svcLength = len + 4;
}

void netsvcCodeRspMor(NET_SVC *svc, unsigned char *buf)
{
	int		len;

	buf[0] = T_RSP_MOR;
	buf[1] = svc->invokeId;
	buf[2] = svc->serviceChoice;
	buf[3] = 0;
	if(svc->dataLength > NET_PKT_SZ) len = NET_PKT_SZ;
	else	len = svc->dataLength; 
	memcpy(buf+4, svc->data, len);
	svc->dataLength -= len;
	svc->svcLength = len + 4;
}

void netsvcCodeAck(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ACK;
	buf[1] = svc->invokeId;
	buf[2] = buf[3] = 0;
	svc->svcLength = 4;
}

void netsvcCodeAckSrv(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ACK_SRV;
	buf[1] = svc->invokeId;
	buf[2] = buf[3] = 0;
	svc->svcLength = 4;
}

void netsvcCodeError(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ERROR;
	buf[1] = svc->invokeId;
	buf[2] = svc->errorClass;
	buf[3] = svc->errorCode;
	svc->svcLength = 4;
}

void netsvcCodeReject(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ERROR;
	buf[1] = svc->invokeId;
	buf[2] = svc->errorCode;
	buf[3] = 0;
	svc->svcLength = 4;
}

void netsvcCodeAbort(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ABORT;
	buf[1] = svc->invokeId;
	buf[2] = svc->errorCode;
	buf[3] = 0;
	svc->svcLength = 4;
}

void netsvcCodeAbortSrv(NET_SVC *svc, unsigned char *buf)
{
	buf[0] = T_ABORT_SRV;
	buf[1] = svc->invokeId;
	buf[2] = svc->errorCode;
	buf[3] = 0;
	svc->svcLength = 4;
}

void netsvcResponding(NET_SVC *svc, unsigned char *buf, int length)
{
	int		type, result;

	if(buf[0] == T_RESULT) {
		type = svc->type;
		if(svc->svcWork) {
			result = T_RESULT;
			svc->svcWork = 0;
		} else {
printf("unexpected call result received\n");
			svc->svcLength = 0;
			return;
		}
	} else {
		type = buf[0];
		if(type == T_REQ || type == T_REQ_MOR) {
			if(!svc->type || svc->invokeId != buf[1]) {
				svc->invokeId	= buf[1];
				svc->dataLength	= 0;
				svc->error		= 0;
				svc->svcWork	= 0;
			}
			memcpy(svc->data+svc->dataLength, buf+4, length-4);
			svc->dataLength += length - 4;
		} else {
			if(!svc->type || svc->invokeId != buf[1]) {
if(!svc->type) printf("unexpected service received\n");
else	printf("invokeID is different : %d %d\n", (int)svc->invokeId, (int)buf[1]);
				svc->svcLength = 0;
				return;
			}
		}
		svc->type = type;
		result = 0;
	}
	buf[0] = 0;
	switch(type) {
	case T_REQ:
	case T_ACK:
		if(!result && type == T_REQ) {
			svc->svcWork = 1;
		} else if(!result && svc->position && svc->dataLength < NET_PKT_SZ) {
			svc->svcWork = 3;
		} else if(svc->error) {
			if(type == T_REQ) netsvcCodeError(svc, buf);
			else	netsvcCodeAbortSrv(svc, buf);
			svc->type = 0;
		} else if(svc->position || svc->dataLength > NET_PKT_SZ) {
			netsvcCodeRspMor(svc, buf);
		} else {
			netsvcCodeRsp(svc, buf);
			svc->type = 0;
		}
		break;
	case T_REQ_MOR:
		if(!result && svc->dataLength + NET_PKT_SZ > NET_MAX_SZ) {
			svc->svcWork = 2;
		} else if(svc->error) {
			netsvcCodeAbortSrv(svc, buf);
			svc->type = 0;
		} else {
			netsvcCodeAck(svc, buf);
		}
		break;
	case T_ABORT:
		svc->type = 0;
		svc->svcLength = 0;
		break;
	default:
		svc->type = 0;
		svc->svcLength = 0;
	}
}

void netsvcRequesting(NET_SVC *svc, unsigned char *buf, int length)
{
/*
	int		type, result;

	if(buf[0] == T_RESULT) {
		type = svc->type;
		if(type) result = T_RESULT; 
		else {
printf("unexpected call result received\n");
			return;
		}
	} else {
		type = buf[0];
		if(!svc->state || svc->invokeId != buf[1]) {
if(!svc->state) printf("unexpected service received\n");
else	printf("invokeID is different : %d %d\n", (int)svc->invokeId, (int)buf[1]);
			return;
		}
		if(type == T_RSP || type == T_RSP_MOR) {
			memcpy(svc->buf+svc->length, buf+4, length-4);
			svc->length += length - 4;
		}
		result = 0;
	}
	buf[0] = 0;
	switch(type) {
	case 0:
	case T_ACK:
		if(!result && type == 0) {
			svc->type = type;
			reqCall(svc);
		} else if(!result && type == T_ACK && svc->position && svc->length < NET_PKT_SZ) {
			svc->type = type;
			reqTxMoreCall(svc);
		} else if(result && svc->error) {
			netsvcCodeAbortSrv(svc, buf);
			svc->state = 0;
		} else if(svc->position || svc->length > NET_PKT_SZ) {
			netsvcCodeReqMor(svc, buf);
			svc->state = 1;
		} else {
			netsvcCodeReq(svc, buf);
			svc->state = 2;
		}
		break;
	case T_RSP:
	case T_RSP_MOR:
		if(!result && svc->length + NET_PKT_SZ > NET_MAX_SZ) {
			svc->type = type;
			reqRxMoreCall(svc);
		} else if(result && svc->error) {
			netsvcCodeAbortSrv(svc, buf);
			svc->state = 0;
		} else {
			netsvcCodeAck(svc, buf);
			if(type == T_RSP) svc->state = 0;
			else	svc->state = 2;
		}
		break;
	case T_ERROR:
	case T_REJECT:
	case T_ABORT:
		svc->state = 0;
		break;
	}
*/
}

