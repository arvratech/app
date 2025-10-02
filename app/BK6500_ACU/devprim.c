#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "syscfg.h"
#include "unit.h"
#include "bio.h"
#include "acad.h"
#include "acap.h"
#include "cr.h"
#include "sinport.h"
#include "slavenet.h"
#include "wiegand.h"
#include "user.h"
#include "evt.h"
#include "event.h"
#include "gwnd.h"
#include "user.h"
#include "alm.h"
#include "slave.h"
#include "auth.h"
#include "cli.h"
#include "adm.h"
#include "desktop.h"
#include "devprim.h"

extern CREDENTIAL_READER _gCredentialReaders[];
extern FP_USER *gUser;


CREDENTIAL_READER *_GetCredentialReader(int apID)
{
	return &_gCredentialReaders[apID];
}

unsigned long	ConsoleTimer;
int		ConsoleState;

void ConsoleCatptureCardStart(void)
{
	CREDENTIAL_READER	*cr;
	int		ID;

	ID = 0;
	cr = &_gCredentialReaders[ID];
	cr->CapturePurpose = CR_CAPTURE_ENROLL;
	if(unitGetType(ID) == 2) {
		unitcrSetCaptureStart(ID, USER_CARD);
		cr->Captured = 0; cr->Result = 0;
	} else if(crGetType(ID)) {
		unitSetCruCaptureStart(ID, USER_CARD);
		cr->Captured = 0; cr->Result = 0;
	}
}

void ConsoleCatptureCardEnd(void)
{
	CREDENTIAL_READER	*cr;
	int		ID;
	
	ID = 0;
	cr = &_gCredentialReaders[ID];
	cr->CapturePurpose = CR_CAPTURE_IDENTIFY;
	if(unitGetType(ID) == 2) {
		unitcrSetCaptureEnd(ID);
		cr->Captured = 0; cr->Result = 0;
	} else if(crGetType(ID)) {
		unitSetCruCaptureEnd(ID);
		cr->Captured = 0; cr->Result = 0;
	}
}

int ConsoleCaptureCard(unsigned char *Data)
{
	CREDENTIAL_READER	*cr;
	int		rval;
	
	cr = &_gCredentialReaders[0];
	rval = crCapture(cr);
	if(rval > 1) memcpy(Data, cr->Data, 9);
	return rval;
}

void ConsoleClearCard(void)
{
	crClear(&_gCredentialReaders[0]);
}

unsigned char auth[2], buzz[2];
unsigned long authTmr[2], buzzTmr[2], InputTimer[10];
int		buzzTmout[2];

void InitLocalUnit(void)
{
	int		i;

	for(i = 0;i < 2;i++) {
		auth[i] = buzz[i] = 0;
		ioOffAuth(i); ioOffBuzzer(i);
	}
}

void ProcessLocalUnit(void)
{
	ACCESS_POINT	*ap;
	CREDENTIAL_READER *cr;
	int		i, rval;

	for(i = 0;i < 2;i++) {
		cr = _GetCredentialReader(i);
		if(!crGetType(i) && cr->CapturePurpose == CR_CAPTURE_IDENTIFY) {
			ap = apsGetAt(i);
			rval = crCapture(cr);
			if(rval > 1) {
				if(auth[i]) { ioOffAuth(i); auth[i] = 0; }
				if(buzz[i]) { ioOffBuzzer(i); buzz[i] = 0; }
				ApIdentifyCredential(ap, cr);
			}
		}
		if(auth[i] && (TICK_TIMER-authTmr[i]) >= 43) {	// 672ms
			ioOffAuth(i); auth[i] = 0;
		}
		if(buzz[i] && (TICK_TIMER-buzzTmr[i]) >= buzzTmout[i]) {
			buzz[i]++;
			if(buzz[i] & 1) ioOnBuzzer(i); else ioOffBuzzer(i);
			if(buzz[i] == 2 || buzz[i] > 9) buzz[i] = 0;
			else if(buzz[i] == 8) {
				buzzTmr[i] = TICK_TIMER; buzzTmout[i] = 12;	// 180ms
			} else {
				buzzTmr[i] = TICK_TIMER; buzzTmout[i] = 10;	// 150ms
			}
		}
	}
	ioProcessInputAlarm();
	if(adIsEnable(0)) adProcessLocal(0);
	if(adIsEnable(1)) adProcessLocal(1);
	for(i = 0;i < 8;i++) biProcessLocal(i);
	for(i = 0;i < 2;i++) crProcessLocalTamper(i);
//	for(i = 0;i < MAX_UNIT_SZ;i++) if(actIsEnable(i)) actProcess(i);	2014.9.2
}

void ProcessRemoteIOU(int ID, unsigned char *buf)
{
	UNIT	*unit;
	int		i, val1, val2, mask, max;

 	unit = unitsGetAt(ID);
 	val1 = buf[2] ^ unit->IouRsp[0]; val2 = buf[3] ^ unit->IouRsp[1];
 	unit->IouRsp[0] = buf[2]; unit->IouRsp[1] = buf[3];
 	if(val1) {
  		i = unitGetBinaryInput(ID); max = i + 4;
 		for(mask = 0x02;i < max;i++, mask <<=1)
 			if(val1 & mask) biRemotePVChanged(i);
   		if(val1 & 0x20) adRemotePVChanged(ID);
	 	adAlarmChanged(ID);
 	}
 	if(val2) {
 		i = unitGetBinaryInput(ID); max = i + 4;
 		for(mask = 0x02;i < max;i++, mask <<= 1)
			if(val2 & mask) biAlarmChanged(i);
 	}
}

void ProcessRemoteCRU(int ID, unsigned char *buf)
{
	UNIT			*unit;
	ACCESS_POINT	*ap;
	CREDENTIAL_READER *cr;
	unsigned char	*p;
	int		rval, val, cmd, size;

	if(buf[0] == REQ_IND) {
		ap = apsGetAt(ID);
		cr = _GetCredentialReader(ID);
		p = SlaveNetGetRxReqData(ID); size = SlaveNetGetRxReqDataSize(ID);
		if(crGetType(ID)) {
			cmd = *p;
			switch(cmd) {
			case 0x30:
				cr->FuncKey = p[1];
				rval = crRead(cr, p+2, size-2);
cprintf("%lu ID=%d Identify Type=%d Data=%d\n", DS_TIMER, ID, (int)cr->CredentialType, cr->DataLength);
				if(rval > 1) ApIdentifyCredential(ap, cr);
				break;
			case 0x31:
				cr->SessionNo = p[1];
				rval = crRead(cr, p+2, size-2);
cprintf("%lu ID=%d Verify: Session=%d Type=%d Data=%d\n", DS_TIMER, ID, (int)cr->SessionNo, (int)cr->CredentialType, cr->DataLength);
				if(rval > 1) ApVerifyCredential(ap, cr);
				break;
			case 0x32:
				cr->Result = p[1];
				if(!cr->Result) rval = crRead(cr, p+2, size-2);
				else	cr->DataLength = 0;
cprintf("%lu ID=%d Capture: Type=%d Data=%d\n", DS_TIMER, ID, (int)cr->CredentialType, cr->DataLength);
				ApCaptureCredential(cr);
				break;
			default:
				SlaveProcessRequest((int)cr->ID);	
				break;
			}
		} else {
//cprintf("[%02x", (int)p[0]); for(val = 1;val < size;val++) cprintf("-%02x", (int)p[val]); cprintf("]\n");
			rval = crReadWiegand(cr, p+1);
cprintf("%ld ID=%d Identify(IOU): Rval=%d Data=%d\n", DS_TIMER, ID, rval, (int)p[1]);
			if(rval > 1) ApIdentifyCredential(ap, cr);
			else	unitAuthResult((int)ap->ID, 1);
			//rval = crRead(cr);
			//if(rval > 1) rval = MenuIdentifyCard(ap, user);
 		}
 	}
 	unit = unitsGetAt(ID);
 	val = buf[2] ^ unit->CruRsp[0];
 	unit->CruRsp[0] = buf[2];
 	if(val) {
		if(val & 0x01) crRemoteTamperPVChanged(ID);
  		if(val & 0x02) crTamperAlarmChanged(ID); 		
 	}
}

void ProcessRemoteIU(int ID, unsigned char *buf)
{
	IOUNIT	*unit;
	unsigned char	cval[8];
	int		i, j, biID, val, mask;

 	unit = iounitsGetAt(ID);
 	for(i = 0;i < 6;i++) {
 		cval[i] = buf[2+i] ^ unit->Rsp[i]; unit->Rsp[i] = buf[2+i];
 	}
  	biID = iounitGetBinaryInput(ID);
  	for(i = 0;i < 3;i++) {
  		val = cval[i];
		if(val) {
 			for(j = 0, mask = 0x80;j < 8;j++, mask >>= 1)
 				if(val & mask) biRemotePVChanged(biID+j);
  		}
  		biID += 8;
  	}
  	biID = iounitGetBinaryInput(ID);
  	for(i = 3;i < 6;i++) {
  		val = cval[i];  		
		if(val) {
 			for(j = 0, mask = 0x80;j < 8;j++, mask >>= 1)
 				if(val & mask) biAlarmChanged(biID+j);
  		}
  		biID += 8;
  	}
}

void ProcessAuthResult(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user)
{
	unsigned char	*p, *p0;
	unsigned long	lVal;
	int		ID, result;

	ID = cr->ID;
	if(unitGetType(ID) == 2 || crGetType(ID)) {
		p = p0 = SlaveNetGetTxRspData(ID);
		if(!cr->AuthType) {	// Identify
			*p++ = cr->Result;
			*p++ = cr->AccessEvent;
			*p++ = cr->CardIndex;
			if(cr->AccessEvent) *p++ = adShortStatus(ID);
			else	*p++ = cr->CaptureMode;
			*p++ = ap->SessionNo;
			if(user) {
				userEncodeID(user->ID, p); p += 3;
				if(user->UserName[0] || user->UserExternalID[0]) {
					memcpy_pad(p, user->UserName, 20); p += 20;
					string2bcd(user->UserExternalID, 14, p); p += 7;
				}
			}
			//if(ap->AccessEvent && unitGetType(ID) == 2 && crGetType(ID)) {
			if(ap->AccessEvent && unitGetType(ID) == 2) {	// modified 2019.3.22  unitType=2 경우 crType=0:RS-285 1:IP
				if(!user) {
					userEncodeID(0, p); p += 3;
				}
				datetime2longtime(ap->AccessTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
			}
if(user) cprintf("%ld ID=%d Identify: Rst=%d Evt=%d ID=%d Data=%d\n", DS_TIMER, ID, (int)cr->Result, (int)cr->AccessEvent, user->ID, p - p0);
else	cprintf("%ld ID=%d Identify: Rst=%d Evt=%d Data=%d\n", DS_TIMER, ID, (int)cr->Result, (int)cr->AccessEvent, p - p0);
		} else {
			*p++ = cr->Result;
			*p++ = cr->AccessEvent;
			*p++ = cr->CardIndex;
			if(cr->AccessEvent) *p++ = adShortStatus(ID);
			else	*p++ = cr->CaptureMode;
			//if(ap->AccessEvent && unitGetType(ID) == 2 && crGetType(ID)) {
			if(ap->AccessEvent && unitGetType(ID) == 2) {	// modified 2019.3.22  nuitType=2 경우 crType=0:RS-285 1:IP
				datetime2longtime(ap->AccessTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
			}
if(user) cprintf("%ld ID=%d Verify: Rst=%d Evt=%d ID=%d Data=%d\n", DS_TIMER, ID, (int)cr->Result, (int)cr->AccessEvent, user->ID, p - p0);
else	cprintf("%ld ID=%d Verify: Rst=%d Evt=%d Data=%d\n", DS_TIMER, ID, (int)cr->Result, (int)cr->AccessEvent, p - p0);
		}	
		SlaveNetSetTxRspDataSize(ID, p - p0, 0);
	} else if(cr->AccessEvent) {
		if(evtIsGranted(cr->AccessEvent)) result = 3;
		else	result = 2;
		if(ID < 2) {
			if(result == 3) {
				ioOnAuth(ID); auth[ID] = 1; authTmr[ID] = TICK_TIMER;	// 84 168
				ioOnBuzzer(ID); buzz[ID] = 1; buzzTmr[ID] = TICK_TIMER; buzzTmout[ID] = 12;	// 180ms
			} else {
				ioOnBuzzer(ID); buzz[ID] = 7; buzzTmr[ID] = TICK_TIMER; buzzTmout[ID] = 10;	// 150ms
			}
		} else {
cprintf("%ld ID=%d Identify(Wiegand): Result=%d Event=%d\n", DS_TIMER, ID, result, (int)cr->AccessEvent);
			unitAuthResult(ID, result);
		}
	}
	UpdateReaderEvent(ID);
}

void ProcessDoors(void)
{
	int		i;

	for(i = 0;i < MAX_AD_SZ;i++) {
		if(adIsEnable(i)) {
			adAlarmChanged(i);
		}
	}
}

void ProcessDoorsLockSchedule(void)
{
	unsigned char	ctm[8];
	int		i;
	
	rtcGetDateTime(ctm);
	for(i = 0;i < MAX_AD_SZ;i++) {
		if(adIsEnable(i)) adProcessLockSchedule(i, ctm);
	}
}

extern int	CliID;

void ProcessAlarmAction(void)
{
	CREDENTIAL_READER	*cr;
	int		i;

	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++) {
		if(actIsEnable(i)) actProcess(i);
		if(unitGetType(i) == 2 && crIsEnable(i) && (cr->Client || CliID == i || !crGetReliability(i))) CliDownload(i);
	}
	ClisTimeoutXferCompleted();
}

int IsBlackWhiteMode(void)
{
	if(sys_cfg->Device.DeviceOption & BW_MODE) return 1;
	else	return 0;
}

int GetSvrSioPort(void)
{
	return (int)1;
}
