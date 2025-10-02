#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "syscfg.h"
#include "fsuser.h"
#include "cr.h"
#include "dev.h"
#include "sclib.h"
#include "msg.h"
#include "slvnet.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "topprim.h"
#include "viewprim.h"
#include "authmst.h"

extern unsigned char gFuncKey, reqCmd;

void _GetEvent(CREDENTIAL_READER *cr);

static unsigned long	authTimer;

void acuReqIdentify(CREDENTIAL_READER *cr)
{
	unsigned char   *p, *p0, *s;
	int     rval, i, val, size;

    p = p0 = SlvnetConfReqBuf();
	*p++ = reqCmd = 0x30;
	*p++ = gFuncKey;
	*p++ = cr->credType;
	if(cr->credType == CREDENTIAL_CARD) {
		s = cr->data;
		val = cr->dataLength;
		for(i = 0;val >= 9;i++) {
			*p++ = cr->readerFormat;
			memcpy(p, s, 9); p += 9;
			s += 9; val -= 9;
		}
	} else {
		memcpy(p, cr->data, cr->dataLength); p += cr->dataLength;
	}
	SlvnetSetConfReqTxSize(p - p0, 0);
//  cr->state = 1;
	authTimer = MS_TIMER;
//printf("Identify: data=%d\n", cr->dataLength);
}

void acuCnfIdentify(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *s;
	unsigned long	ltime;
	int		rval, val, size;

	cr->evtbuf[0] = 0;
	p = SlvnetConfReqBuf();
	rval = SlvnetConfReqRxSize();
//printf("Rx %d [%02x", rval, (int)p[0]); for(val = 1;val < rval;val++) printf("-%02x", (int)p[val]); printf("]\n");
	if(rval >= 5) {
		cr->result		= *p++;
		cr->accessEvent	= *p++;
		val				= *p++;
		if(val) memcpy(cr->data, cr->data+9*val, 9);
		cr->captureMode	= *p++;
		cr->sessionNo	= *p++;
		rval -= 5;
		if(rval >= 3) {
			PACK3toID(p, &cr->userId); p += 3;
			rval -= 3;
			if(rval >= 27) {
				memcpy_chop(cr->userName, p, 20); p += 20;
				bcd2string(p, 14, cr->userExternalId); p += 7;
				rval -= 27;				
			} else {
				cr->userName[0] = cr->userExternalId[0] = 0;
			}
			if(rval >= 4 && cr->accessEvent) {
				BYTEtoLONG(p, &ltime);
				longtime2datetime(ltime, cr->accessTime);
				_GetEvent(cr);
			}
		} else {
			cr->userId = 0; cr->userName[0] = cr->userExternalId[0] = 0;
		}
		//user->width = user->height = 0;
//printf("%ld _CnfIdentify: result=%d event=%d\n", MS_TIMER, (int)cr->result, (int)cr->accessEvent);
printf("%lu _CnfIdentify: %ums result=%d event=%d\n", MS_TIMER, MS_TIMER-authTimer, (int)cr->result, (int)cr->accessEvent);
	} else {
		cr->result = 3;
printf("%lu _CnfIdentify: error len=%d\n", MS_TIMER, rval);
	}
}

//	cr->result = 3;
//printf("%ld _CnfIdentifySend: Timeout\n", DS_TIMER);
void acuReqVerify(CREDENTIAL_READER *cr)
{
	unsigned char   *p, *p0, *s;
	int     rval, i, val;

	p = p0 = SlvnetConfReqBuf();
	*p++ = reqCmd = 0x31;
	*p++ = cr->sessionNo;
	*p++ = cr->credType;
	if(cr->credType == CREDENTIAL_CARD) {
		s = cr->data;
		val = cr->dataLength;
		for(i = 0;val >= 9;i++) {
			*p++ = cr->readerFormat;
			memcpy(p, s, 9); p += 9;
			s += 9; val -= 9;
		}
	} else {
		memcpy(p, cr->data, cr->dataLength); p += cr->dataLength;
	}
	SlvnetSetConfReqTxSize(p - p0, 0);
//printf("_VerifySendReceive\nTx [%02x", (int)p[0]); for(val = 1;val < cr->dataLength+3;val++) printf("-%02x", (int)p[val]); printf("]\n");
//printf("%ld _ReqVerify: data=%d\n", MS_TIMER, cr->dataLength);
}

void acuCnfVerify(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *s;
	unsigned long	ltime;
	int		rval, val;

	cr->evtbuf[0] = 0;
	p = SlvnetConfReqBuf();
	rval = SlvnetConfReqRxSize();
//if(rval > 0) { printf("Rx [%02x", (int)p[0]); for(val = 1;val < rval;val++) printf("-%02x", (int)p[val]);  printf("]\n"); }
	if(rval >= 4) {
		cr->result		= *p++;
		cr->accessEvent	= *p++;
		val				= *p++;
		if(val) memcpy(cr->data, cr->data+9*val, 9);
		cr->captureMode	= *p++;
printf("%ld _CnfVerify: result=%d event=%d\n", MS_TIMER, (int)cr->result, (int)cr->accessEvent);
		rval -= 4;
		if(rval >= 4 && cr->accessEvent) {
			BYTEtoLONG(p, &ltime);
			longtime2datetime(ltime, cr->accessTime);
			_GetEvent(cr);
		}
	} else {
		cr->result = 3;
printf("%ld _CnfVerify: len=%d\n", MS_TIMER, rval);
	}
}

#define OT_ACCESS_POINT				0x31

void _GetEvent(CREDENTIAL_READER *cr)
{
	char	data[20];
	unsigned long	ltime;

	cr->evtbuf[0] = OT_ACCESS_POINT; cr->evtbuf[1] = devId(NULL); cr->evtbuf[2] = cr->accessEvent;
	datetime2longtime(cr->accessTime, &ltime);
	LONGtoBYTE(ltime, cr->evtbuf+3);
	if(cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_CARD || cr->accessEvent == E_ACCESS_GRANTED_ALL
            || cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_EMV_CARD || cr->accessEvent == E_ACCESS_GRANTED_ALL_EMV) {
		memcpy(cr->evtbuf+7, cr->data, 9);
	} else if(cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_FP) {
		memset(cr->evtbuf+7, 0x00, 9);
	} else {
		utoa02((int)gFuncKey, data); sprintf(data+2, "%u", cr->userId);
		digits2card(data, cr->evtbuf+7);
	}
//printf("Event: id=%d Event=%03d", (int)cr->evtbuf[1], (int)cr->evtbuf[2]);
//longtime2datetime(AccessTime, ctm); printf(" %02d%02d%02d%02d%02d%02d", (int)ctm[0], (int)ctm[1], (int)ctm[2], (int)ctm[3], (int)ctm[4], (int)ctm[5]);
//string_co2db(cr->evtbuf+7, 8, data);	printf(" [%s]\n", data);  	
}

