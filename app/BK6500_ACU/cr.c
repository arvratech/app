#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "key.h"
#include "sio.h"
#include "sin.h"
#include "syscfg.h"
#include "iodev.h"
#include "unit.h"
#include "wiegand.h"
#include "bio.h"
#include "event.h"
#include "cr.h"

#include "crcfg.c"

extern unsigned char ServerNetState;

void CardCaptureBeep(void);

static CREDENTIAL_READER  *crs;


void crsInit(CREDENTIAL_READER *g_crs, int size)
{
	CREDENTIAL_READER	*cr;	
	int		i;

	crsSet(g_crs);
	for(i = 0, cr = g_crs;i < size;i++, cr++) {
		cr->ID	= i;
		cr->cfg	= &sys_cfg->CredentialReaders[i];
		crReset(i);
	}
}

void crsSet(CREDENTIAL_READER *g_crs)
{
	crs = g_crs;
}

CREDENTIAL_READER *crsGetAt(int crID)
{
	return &crs[crID];
}

void crReset(int crID)
{
	CREDENTIAL_READER *cr;

	cr = &crs[crID];
	cr->Status = 0;
	cr->Reliability = REL_COMMUNICATION_FAILURE;
	rtcGetDateTime(cr->ReliabilityTime);
	cr->CapturePurpose = CR_CAPTURE_IDENTIFY;
	memcpy(cr->TamperChangeOfStateTime, cr->ReliabilityTime, 6);
	memcpy(cr->TamperAlarmStateTime, cr->ReliabilityTime, 6);
	memcpy(cr->IntrusionChangeOfStateTime, cr->ReliabilityTime, 6);
	memcpy(cr->IntrusionAlarmStateTime, cr->ReliabilityTime, 6);
	cr->Update = cr->Client = 0;
}

int crGetReliability(int crID)
{
	int		state;

	state = crs[crID].Reliability;
	return state;
}

void crSetReliability(int crID, int Reliability)
{
	crs[crID].Reliability = (unsigned char)Reliability;
	rtcGetDateTime(crs[crID].ReliabilityTime);
}

void crLocalTamperPVChanged(int crID, int PV)
{
	CREDENTIAL_READER *cr;
	int		i;

	cr = &crs[crID];
	rtcGetDateTime(cr->TamperChangeOfStateTime);
	i = 8 + crID;
if(PV) cprintf("%ld ReaderTamper-%d: On\n", DS_TIMER, crID);
else   cprintf("%ld ReaderTamper-%d: Off\n", DS_TIMER, crID); 
	if(biTimeout[i]) biTimeout[i] = 0;
	else if(PV) {
		biTimer[i] = TICK_TIMER;
		biTimeout[i] = rtcSecond2Tick(1);
	} else {
		biTimer[i] = TICK_TIMER;
		biTimeout[i] = rtcSecond2Tick(1);
	}
//	if(PutInputCallBack && biID < 8) PutInputCallBack(ID);
}

void crProcessLocalTamper(int crID)
{
	int		i;
	
	i = 8 + crID;
	if(biTimeout[i] && (TICK_TIMER-biTimer[i]) > biTimeout[i]) {
		biTimeout[i] = 0;
		crTamperAlarmChanged(crID);
	}
}

void crRemoteTamperPVChanged(int crID)
{
	CREDENTIAL_READER *cr;

	cr = &crs[crID];
	rtcGetDateTime(cr->TamperChangeOfStateTime);
if(crGetTamperPresentValue(crID)) cprintf("%ld ReaderTamper-%d: On\n", DS_TIMER, crID);
else	cprintf("%ld ReaderTamper-%d: Off\n", DS_TIMER, crID);
	if(ServerNetState > 2) crNotifyStatus(crID);
//	if(PutInputCallBack) PutInputCallBack(biID);	
}

void crTamperAlarmChanged(int crID)
{
	CREDENTIAL_READER	*cr;
	unsigned char	ctm[8];

 	cr = &crs[crID];
	if(!cr->cfg->TamperAlarmInhibit) {
if(crGetTamperAlarmState(crID)) cprintf("%ld ReaderTamper-%d: Alarm\n", DS_TIMER, crID);
else cprintf("%ld ReaderTamper-%d: Normal\n", DS_TIMER, crID);
		rtcGetDateTime(ctm);
		memcpy(cr->TamperAlarmStateTime, ctm, 6);
		if(crGetTamperAlarmState(crID)) EventAdd(OT_CREDENTIAL_READER, crID, E_READER_TAMPER_ALARM, ctm, NULL);
		else	EventAdd(OT_CREDENTIAL_READER, crID, E_READER_TAMPER_NORMAL, ctm, NULL);
		if(ServerNetState > 2) crNotifyStatus(crID);
		//if(PutInputCallBack) PutInputCallBack(biID);
	}
}

void crIntrusionPVChanged(int crID)
{
	CREDENTIAL_READER *cr;

	cr = &crs[crID];
	if(cr->cfg->IntrusionMode) {
		rtcGetDateTime(cr->IntrusionChangeOfStateTime);
if(crGetIntrusionPresentValue(crID)) cprintf("%ld Intrusion-%d: Armed\n", DS_TIMER, crID);
else	cprintf("%ld Intrusion-%d: Diarmed\n", DS_TIMER, crID); 
		if(crGetIntrusionPresentValue(crID)) EventAdd(OT_CREDENTIAL_READER, crID, E_INTRUSION_ALARM+2, cr->IntrusionChangeOfStateTime, NULL);
		else	EventAdd(OT_CREDENTIAL_READER, crID, E_INTRUSION_NORMAL+2, cr->IntrusionChangeOfStateTime, NULL);
		if(ServerNetState > 2) crNotifyStatus(crID);
		//if(PutInputCallBack) PutInputCallBack(biID);	
	}
}

void crIntrusionAlarmChanged(int crID)
{
	CREDENTIAL_READER	*cr;

 	cr = &crs[crID];
	if(cr->cfg->IntrusionMode) {
		rtcGetDateTime(cr->IntrusionAlarmStateTime);
if(crGetIntrusionAlarmState(crID)) cprintf("%ld Intrusion-%d: Violated\n", DS_TIMER, crID);
else cprintf("%ld Intrusion-%d: Non Violated\n", DS_TIMER, crID);
		if(crGetIntrusionAlarmState(crID)) EventAdd(OT_CREDENTIAL_READER, crID, E_INTRUSION_ALARM, cr->IntrusionAlarmStateTime, NULL);
		else	EventAdd(OT_CREDENTIAL_READER, crID, E_INTRUSION_NORMAL, cr->IntrusionAlarmStateTime, NULL);
		if(ServerNetState > 2) crNotifyStatus(crID);
		//if(PutInputCallBack) PutInputCallBack(biID);
	}
}

int crEncodeStatus(int crID, unsigned char *Buffer)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*p;
	unsigned char	val;
	unsigned long	lVal;

	cr = &crs[crID];
	p = Buffer;
	*p++ = crGetReliability(crID);
	datetime2longtime(cr->ReliabilityTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	if(crGetTamperPresentValue(crID)) val = 0x01; else val = 0x00;
	if(crGetTamperAlarmState(crID)) val |= 0x02;
	if(crGetIntrusionPresentValue(crID)) val |= 0x04;
	if(crGetIntrusionAlarmState(crID)) val |= 0x08;
	*p++ = val;
//cprintf("crEncodeStatus: ID=%d Rel=%d Stat=%02x\n", crID, crGetReliability(crID), val);
	datetime2longtime(cr->TamperChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	datetime2longtime(cr->TamperAlarmStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	datetime2longtime(cr->IntrusionChangeOfStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	datetime2longtime(cr->IntrusionAlarmStateTime, &lVal); LONGtoBYTE(lVal, p); p += 4;	
	return p - Buffer;
}

#include "defs_obj.h"
#include "defs_pkt.h"
#include "net.h"

void _SendNet(NET *net);

void crNotifyStatus(int crID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_CREDENTIAL_READER;
	*p++ = crID;
	p += crEncodeStatus(crID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

void crNotifyObjectsStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[256];
	int		i, ObjID;

	net = (NET *)Buffer;
	p = net->Data;
	ObjID = unitGetAccessDoor(ID);
	*p++ = OT_ACCESS_DOOR;
	if(adIsEnable(ObjID)) {
		*p++ = ObjID; p += adEncodeStatus(ObjID, p);
	}
	net->DataLength = p - net->Data;
	if(net->DataLength > 1) {
		netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
		_SendNet(net);
	}
	p = net->Data;
	ObjID = unitGetBinaryInput(ID);
	*p++ = OT_BINARY_INPUT;
	for(i = 0;i < 4;i++, ObjID++)
		if(biIsEnable(ObjID) && biGetAssignedDoor(ObjID) < 0 && biGetAssignedFirePoint(ObjID) < 0) {
			*p++ = ObjID; p += biEncodeStatus(ObjID, p);
		}
	net->DataLength = p - net->Data;
	if(net->DataLength > 1) {
		netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
		_SendNet(net);
	}
	p = net->Data;
	ObjID = unitGetBinaryInput(ID);
	*p++ = OT_BINARY_OUTPUT;
	for(i = 0;i < 2;i++, ObjID++)
		if(boIsEnable(ObjID) && boGetAssignedDoor(ObjID) < 0 && !boIsAssignedAlarmAction(ObjID)) {
			*p++ = ObjID; p += boEncodeStatus(ObjID, p);
		}
	net->DataLength = p - net->Data;
	if(net->DataLength > 1) {
		netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
		_SendNet(net);
	}
}

int crsEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_CREDENTIAL_READER;
	for(i = 0;i < MAX_CR_SZ;i++) {
//if(crIsEnable(i)) cprintf("[%d] unitType=%d crType=%d\n", i, unitGetType(i), crGetType(i));
		if(crIsEnable(i) && (unitGetType(i) == 2 || crGetType(i))) {
			*p++ = i; p += crEncodeStatus(i, p);
		}
	}
	return p - Buffer;
}

int crCapture(CREDENTIAL_READER *cr)
{
	unsigned char	data[20];
	int		id, rval, enable;

	id = cr->ID;
	if(crIsEnable(id)) enable = 1;
	else	enable = 0;
	if(unitGetType(id) == 2 || crGetType(id)) {
		rval = 0;
		if(enable && !crGetCardReaderEnable(cr->cfg)) enable = 0;
		if(enable && cr->Captured && !cr->Result) {
			rval = cr->DataLength;
		}
		cr->Captured = 0;
	} else {	
		if(enable && !crGetWiegandReaderEnable(cr->cfg)) enable = 0;
		if(enable && !cr->Status) {
			wiegandOpen((int)cr->ID); cr->Status = 1;
#ifdef CARD_DEBUG
	cprintf("Open WiegandReader: %d\n", id);
#endif
		} else if(!enable && cr->Status) {
			wiegandClose((int)cr->ID); cr->Status = 0;
#ifdef CARD_DEBUG
	cprintf("Close WiegandReader: %d\n", id);
#endif
		}
		rval = 0;
		if(cr->Status) {
			rval = wiegandRead(id, data+1);
			if(rval >= 8) {
				data[0] = rval;
cprintf("WiegandRaw=%d [%02x", rval, data[1]); for(id = 1;id < ((rval+7)>>3);id++) cprintf("-%02x", (int)data[1+id]); cprintf("\n"); 
				rval = crReadWiegand(cr, data);
			} else	rval = 0;
		}
	}
	return rval;
}

void crClear(CREDENTIAL_READER *cr)
{
	if(cr->Status) wiegandRead((int)cr->ID, NULL);
}

// Return	0=Cancelled  1=Invalid credential  2=Valid credential
int crRead(CREDENTIAL_READER *cr, unsigned char *data, int size)
{
	unsigned char	*d, *s;
	int		rval, i, len;

	rval = 2;
	s = data; d = cr->Data;
	cr->CredentialType = *s++;
	len = size - 1;
	if(cr->CredentialType == CREDENTIAL_CARD && (len % 10) == 0) {
		for(i = 0;len >= 10;i++) {
			cr->readerFormat[i] = *s++;
			memcpy(d, s, 9); s += 9; d += 9; len -= 10;
		}
		cr->DataLength = d - cr->Data;
	} else {
		memcpy(d, s, len);
		if(cr->CredentialType == CREDENTIAL_FP && len < 400) {
			memset(d+len, 0, 400-len); len = 400;	
		}
		cr->DataLength = len;
		memset(cr->readerFormat, 0, 4);
	}
	return rval;
}

// Return	0=Cancelled  1=Invalid card  9=Valid card
int crReadWiegand(CREDENTIAL_READER *cr, unsigned char *Data)
{
	unsigned char	*p, temp;
	int		rval, BitLength;
#ifdef CARD_DEBUG
	int		i, len;
#endif

	BitLength = Data[0];
	p = Data + 1;
	switch(BitLength) {
	case 26:	rval = wiegandCheck26Data(p); break;
	case 32:	rval = 32; break;
	case 34:	rval = wiegandCheck34Data(p); break;
	case 35:	rval = wiegandCheck35Data(p); break;
//	case 37:	rval = wiegandCheck37Data(p); break;
	case 37:	rval = 37; break;
	case 130:	rval = wiegandCheck130Data(p); break;
	case 66:	rval = wiegandCheck66Data(p); break;
	case 64:	rval = 64; break;
	default:	rval = 0;
	}
#ifdef CARD_DEBUG
if(rval) cprintf("crReadWiegand(%d): L=%d OK [%02x", (int)cr->ID, rval, (int)p[0]);
else	 cprintf("crReadWiegand(%d): L=%d Fail [%02x", (int)cr->ID, BitLength, (int)p[0]);
if(BitLength == 37) len = 5; else len = rval >> 3;
for(i = 1;i < len;i++) cprintf("-%02x", (int)p[i]); cprintf("]\n");
#endif
	if(rval == 26) {
		if(cr->cfg->crWiegand.ByteOrder) {
			temp = p[2]; p[2] = p[0]; p[0] = temp;
		}
		if(cr->cfg->crWiegand.Format26Bit == 1) {
			bin2card(p+1, 2, cr->Data);
		} else if(cr->cfg->crWiegand.Format26Bit == 2) {
			wiegandReverse26Data(p);
			bin2card(p+1, 2, cr->Data);
		} else {
			bin2card(p, 3, cr->Data);
		}
		rval = 9;
	} else if(rval == 34 || rval == 32 || rval == 35) {
		if(cr->cfg->crWiegand.ByteOrder && rval != 5) {
			temp = p[3]; p[3] = p[0]; p[0] = temp;
			temp = p[2]; p[2] = p[1]; p[1] = temp;
		}
		bin2card(p, 4, cr->Data);
		rval = 9;
	} else if(rval == 37) {
		bin2card(p, 5, cr->Data);
		rval = 9;	
	} else if(rval == 130) {
		p[16] = 0;
		digits2card((char *)p, cr->Data); 
		rval = 9;
	} else if(rval == 66 || rval == 64) {
		bin2card(p, 8, cr->Data);
		rval = 9;
	} else {
		rval = 1;
	}
	if(rval > 1) {
		cr->CredentialType = CREDENTIAL_CARD;
		cr->DataLength = rval;
//		CardCaptureBeep();
	}
	return rval;
}

int crVerifyMatchCard(CREDENTIAL_READER *cr, unsigned char *Data)
{
	unsigned char	*p;
	int		size, index;

	index = 0;
	size = cr->DataLength;
	p = cr->Data;
	while(size >= 9) {
		if(!n_memcmp(Data, p, 9)) break;
		index++; p += 9; size -= 9;
	}
	if(size < 9) {
		cr->CardIndex = 0;
		index = 0;
	} else {
		cr->CardIndex = (unsigned char)index;
		cr->readerFormat[0] = cr->readerFormat[index];
		index = 1;
	} 
	return index;
}

#include "user.h"
#include "fsuser.h"

int crIdentifyMatchCard(CREDENTIAL_READER *cr, FP_USER *user)
{
	unsigned char	*p;
	int		rval, size, index;
	
	index = 0;
	size = cr->DataLength;
	p = cr->Data;
	rval = 0;
	while(size >= 9) {
		userfsSeek(0L);
		rval = userfsGetCardData(user, p);
		if(rval) break;
		index++; p += 9; size -= 9;
	}
	if(rval < 0) index = -1;
	else if(!rval) {
		cr->CardIndex = 0; index = 0;
	} else {
		cr->CardIndex = (unsigned char)index;
		cr->readerFormat[0] = cr->readerFormat[index];
		index = 1;
	}
	return index;
}

void bin2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x10 + len;
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, 0x00, len);
}

// len: nibble length
void bcd2card(unsigned char *data, int len, unsigned char *buf)
{
	unsigned char	*p;
	int		c, val;

	p = buf;
	*p++ = 0x20;
	val = len >> 1;
	memcpy(p, data, val); p += val;
	if(len & 1) {
		c = data[val]; c |= 0x0f;
		*p++ = c; val++;
	}
	val = buf + 9 - p;
	if(val > 0) memset(p, 0xff, val);
}

void digits2card(char *data, unsigned char *buf)
{
	unsigned char	*p;

	p = buf;
	*p++ = 0x20;
	string2bcd(data, 16, p);
}

void str2card(char *data, unsigned char *buf)
{
	unsigned char	*p;
	int		len;

	len = strlen(data);
	p = buf;
	*p++ = 0x30;	
	memcpy(p, data, len); p += len;
	len = buf + 9 - p;
	if(len > 0) memset(p, ' ', len);
}
