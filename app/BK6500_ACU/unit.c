#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "iodev.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "bio.h"
#include "cr.h"
#include "ta.h"
#include "sinport.h"
#include "slavenet.h"
#include "cli.h"
#include "event.h"
#include "user.h"

#include "unitprim.c"
#include "unitcfg.c"

extern unsigned char gLocalDateTimeVersion;

static UNIT		*Units;
static IOUNIT	*IOUnits;


void unitsResetSystem(void)
{
	unsigned char	buf[8];

	buf[0] = RESET_REQ;
	memset(buf+1, 0, 7);
	SlaveSioWriteBuffer(buf, 8);
	SlaveSinWriteBuffer(buf, 8);
cprintf("unitsResetSystem...\n");
}

static void _ResetUnit(UNIT *unit)
{
	unit->Reliability = REL_COMMUNICATION_FAILURE;
	rtcGetDateTime(unit->ReliabilityTime);
}

void unitsInit(UNIT *g_Units, int size)
{
	UNIT	*unit;
	int		i;

	Units = g_Units;
	for(i = 0, unit = Units;i < size;i++, unit++) {
		unit->ID	 = i;
		_ResetUnit(unit);
	}
}

int unitsEncodeStatusAll(void *buf)
{
	unsigned char	*p, val, mask;
	int		i;

	p = (unsigned char *)buf;
	val = 0; mask = 0x80;
	for(i = 2;i < MAX_UNIT_SZ;i += 2) {
		if(!unitGetReliability(i)) val |= mask;
		mask >>= 1;
		if(!mask) {
			*p++ = val; val = 0; mask = 0x80;
		}
	}
	*p++ = val;
	val = 0; mask = 0x80;
	for(i = 0;i < MAX_CR_SZ;i++) {
		if(!crGetReliability(i)) val |= mask;			
		mask >>= 1;
		if(!mask) {
			*p++ = val; val = 0; mask = 0x80;
		}
	}
	*p++ = val;
	val = 0; mask = 0x80;
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		if(!iounitGetReliability(i)) val |= mask;
		mask >>= 1;
		if(!mask) {
			*p++ = val; val = 0; mask = 0x80;
		}
	}
	*p++ = val;
	return p - (unsigned char *)buf;
}

UNIT *unitsGetAt(int UnitID)
{
	return &Units[UnitID];
}

int unitGetReliability(int ID)
{
	int		val;
	
	if(ID < 2) val = 0;
	else {
		if(ID & 1) ID--;
		val = Units[ID].Reliability;
	}
	return val;
}

void unitSetReliability(int ID, int Reliability)
{
	if(ID & 1) ID--;
	if(ID > 1) Units[ID].Reliability = (unsigned char)Reliability;
	rtcGetDateTime(Units[ID].ReliabilityTime);
	if(crGetType(ID)) unitSetCruDoor(ID);
	ID++;
	if(crGetType(ID)) unitSetCruDoor(ID);
}

static void _ResetIOUnit(IOUNIT *unit)
{
	unit->Reliability = REL_COMMUNICATION_FAILURE;
	rtcGetDateTime(unit->ReliabilityTime);
}

void iounitsInit(IOUNIT *g_Units, int size)
{
	IOUNIT	*unit;
	int		i;

	IOUnits = g_Units;
	for(i = 0, unit = IOUnits;i < size;i++, unit++) {
		unit->ID	= i;
		_ResetIOUnit(unit);
	}
}

IOUNIT *iounitsGetAt(int ID)
{
	return &IOUnits[ID];
}

int iounitGetReliability(int ID)
{
	int		reliability;

	reliability = IOUnits[ID].Reliability;
	return reliability;
}

void iounitSetReliability(int ID, int Reliability)
{
	IOUnits[ID].Reliability = (unsigned char)Reliability;
	rtcGetDateTime(IOUnits[ID].ReliabilityTime);
}

int _GetReqIdleMode(int ID)
{
	CR_CFG	*cr_cfg;
	int		val;

	if(crIsEnable(ID)) {
		cr_cfg = &sys_cfg->CredentialReaders[ID];
		if(crGetType(ID)) {
			if(crGetCardReaderEnable(cr_cfg)) val = 0x04; else val = 0x00;		// Card
			if(crGetPINReaderEnable(cr_cfg)) val |= 0x02;						// ID/PIN
			if(crGetFPReaderEnable(cr_cfg)) val |= 0x01;						// FP
		} else {
			if(crGetWiegandReaderEnable(cr_cfg)) val = 0x04; else val = 0x00;	// Card
			//if(crGetPINReaderEnable(cr_cfg)) val |= 0x02;						// ID/PIN
		}
	} else	val = 0x00;
	return val;
}

void _CruSetState(int ID)
{
	CR_CFG	*cr_cfg;
	unsigned char	c;
	int		val;

	cr_cfg = &sys_cfg->CredentialReaders[ID];
	c = Units[ID].CruReq[0];
	if(crGetCardReaderEnable(cr_cfg)) c |= 0x04; else c &= 0xfb;	// Card
	if(crGetPINReaderEnable(cr_cfg)) c |= 0x02; else c &= 0xfd;		// PIN
	if(crGetFPReaderEnable(cr_cfg)) c |= 0x01; else c &= 0xfe;		// FP
	val = taGetState();
	c &= 0xe7;
	if(val == 1) c |= 0x08;
	else if(val == 2) c |= 0x10; 	
	Units[ID].CruReq[0] = c;
}

void _CruSetDoor(int ID)
{
	int		PV, DoorStatus, DoorAlarmState;

	if(adIsEnable(ID)) {
		Units[ID].CruReq[1] = 0x80;
		//if(!adGetReliability(ID)) {
		if(adGetReliability(ID)) {		// Bug fix: 2015.7.27
			Units[ID].CruReq[1] |= 0x40;
		} else {
			PV = adGetPresentValue(ID);
			if(PV == 1) PV = 1;
			else if(PV & 0x01) PV = 1;
			else	PV = 0;
			DoorStatus = adGetDoorStatus(ID);
			DoorAlarmState = adEvaluateAlarmState(ID);
			if(DoorAlarmState) {
				DoorStatus = 2; DoorAlarmState--;
			}
			Units[ID].CruReq[1] |= (DoorStatus << 4) | (DoorAlarmState << 3) | (PV << 1);
		}			
	} else {
		Units[ID].CruReq[1] = 0x00;
	}
}

void _CruSetVersion(int ID)
{
	CR_CFG	*cr_cfg;

	cr_cfg = &sys_cfg->CredentialReaders[ID];
	Units[ID].CruReq[2] = cr_cfg->Version;
	Units[ID].CruReq[3] = sys_cfg->SCKeyVersion;
	Units[ID].CruReq[4] = sys_cfg->TAVersion;
	Units[ID].CruReq[5] = gLocalDateTimeVersion;
}

void unitInitRequest(int ID)
{
	if(crGetType(ID)) {
		Units[ID].IouReq[0] = 0x00;
		Units[ID].IouReq[1] = 0x00;
		Units[ID].CruReq[0] = 0x00;
		_CruSetState(ID);
		_CruSetDoor(ID);
		_CruSetVersion(ID);
//cprintf("%ld CRU-%d: Init Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[ID].CruReq[2], (int)Units[ID].CruReq[3]);
	} else {
		Units[ID].IouReq[0] = 0x00;
		Units[ID].IouReq[1] = 0x00;
		Units[ID].CruReq[0] = (unsigned char)_GetReqIdleMode(ID);
		Units[ID].CruReq[1] = 0x00;
		if(!(ID & 1)) unitEncodeIouConfig(ID);
//cprintf("%ld Unit-%d: Init Req=%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].Req[0], (int)Units[ID].Req[1]);
	}
}

void _ACUnitSetState(int ID)
{
	CR_CFG	*cr_cfg;
	unsigned char	val;

	cr_cfg = &sys_cfg->CredentialReaders[ID];
	val = 0x00;
	if(crGetCardReaderEnable(cr_cfg)) val |= 0x40;	// Card
	if(crGetPINReaderEnable(cr_cfg))  val |= 0x20;	// PIN
	if(crGetFPReaderEnable(cr_cfg))   val |= 0x10;	// FP
	val |= taGetState() << 2;
	val |= adAlarmState(ID);
	Units[ID].IouReq[0] = val;
}

void unitcrInitRequest(int ID)
{
	CREDENTIAL_READER	*cr;

	_ACUnitSetState(ID);
	Units[ID].IouReq[1] = 0x00;
	cr = crsGetAt(ID);
	cr->Update		= 0x0fc;	// AdminPIN-LocalDateTime-IOConfig-CRConfig-SCKey-T&A-0-Event-0
	cr->Client		= 0x00;	
	cr->XferState	= cr->XferRetry	= cr->CliRetry = 0;
//cprintf("%ld ACUnit%d: Init 02x-%02x\n", DS_TIMER, ID, (int)Units[ID].IouReq[0], (int)Units[ID].IouReq[1]);
}

void unitcrConnected(int ID, int reset)
{
	CREDENTIAL_READER	*cr;

	cr = crsGetAt(ID);
	if(reset) cr->Update = 0xfc;	// LocalDateTime-IOConfig-CRConfig-SCKey-T&A-AdminPIN
	else	  cr->Update |= 0x00;	// 0-0-0-0-0-0
	cr->Client = 0x00;
}

void iounitInitRequest(int ID)
{
	IOUnits[ID].Req[0] = IOUnits[ID].Req[1] = 0x00;
	iounitEncodeConfig(ID);		
//cprintf("%ld IOUnit-%d: Init Req\n", DS_TIMER, ID);
}

void unitEncodeIouConfig(int UnitID)
{
	ACCESS_DOOR_CFG		*ad_cfg;
	BINARY_INPUT_CFG	*bi_cfg;
	BINARY_OUTPUT_CFG	*bo_cfg;
	unsigned char	buf[40], *p, *p0, val, mask;
	int		i, j, ID, len;

	if(unitGetType(UnitID) == 2) CliIOConfigChanged(UnitID);
	else if(UnitID > 1) {
		if(UnitID & 1) UnitID--;
		p = buf;
		ad_cfg = &sys_cfg->AccessDoors[UnitID];
		for(i = 0;i < 2;i++, ad_cfg++) {
			if(adIsEnable(UnitID)) val = 0x01; else val = 0x00;
			if(ad_cfg->RequestToExitEnable) val |= 0x02;
			if(ad_cfg->DoorStatusEnable) val |= 0x04;
			if(ad_cfg->AuxLockEnable) val |= 0x08;
			val |= ad_cfg->LockType << 4;
			*p++ = val;
			*p++ = ad_cfg->DoorPulseTime;
			*p++ = ad_cfg->DoorPulseTime;
			*p++ = ad_cfg->DoorOpenTooLongTime;
			*p++ = ad_cfg->TimeDelay;
			*p++ = ad_cfg->TimeDelayNormal;
		}
		p0 = p;
		p += 2;
		ID = UNIT2BI(UnitID);
		bi_cfg = &sys_cfg->BinaryInputs[ID];
		ID = UNIT2BO(UnitID);
		bo_cfg = &sys_cfg->BinaryOutputs[ID];
		for(i = 0;i < 2;i++) {
			val = 0;
			mask = 0x01;
			for(j = 0;j < 4;j++, bi_cfg++, mask <<= 1) {
				*p++ = bi_cfg->TimeDelay;
				*p++ = bi_cfg->TimeDelayNormal;
				if(bi_cfg->Polarity) val |= mask;
			}
			for(j = 0;j < 2;j++, bo_cfg++, mask <<= 1)
				if(bo_cfg->Polarity) val |= mask;
			*p0++ = val;
		}
		len = p - buf;
		p = p0 = SlaveNetGetUnitCfgData(UnitID);
		if(!p[2] || n_memcmp(buf, p+4, len)) {
			*p++ = 1;		 // lock
			p++;			 // ClientVersion
			val = *p; val++; // Version
			if(!val) val = 1;
			*p++ = val;
			*p++ = len;
			memcpy(p, buf, len);
//cprintf("IOU%d CfgData: CliVer=%d Ver=%d Len=%d\n", UnitID, (int)p0[1], (int)p0[2], (int)p0[3]);
			*p0 = 0;		// unlock
		}
	}
}

void iounitEncodeConfig(int UnitID)
{
	BINARY_INPUT_CFG	*bi_cfg;
	BINARY_OUTPUT_CFG	*bo_cfg;
	unsigned char	buf[60], *p, val, mask;
	int		i, ID, len;

	p = buf;
	*p++ = 0x20;	// Command
	*p++ = 0x20;	// ObjectType
	if(iounitGetType(UnitID)) {
		ID = iounitGetBinaryOutput(UnitID);
		bo_cfg = &sys_cfg->BinaryOutputs[ID];
		mask = 0x80; val = 0;
		for(i = 0;i < MAX_IOUNIT_BO_SZ;i++, bo_cfg++) {
			if(bo_cfg->Polarity) val |= mask;
			mask >>= 1;
			if(!mask) {
				*p++ = val; val = 0; mask = 0x80;
			}
		}
		*p++ = val;
	} else {
		ID = iounitGetBinaryInput(UnitID);
		bi_cfg = &sys_cfg->BinaryInputs[ID];
		mask = 0x80; val = 0;
		for(i = 0;i < MAX_IOUNIT_BI_SZ;i++, bi_cfg++) {
			if(bi_cfg->Polarity) val |= mask;
			mask >>= 1;
			if(!mask) {
				*p++ = val; val = 0; mask = 0x80;
			}
		}
		bi_cfg = &sys_cfg->BinaryInputs[ID];
		for(i = 0;i < MAX_IOUNIT_BI_SZ;i++, bi_cfg++) {
			*p++ = bi_cfg->TimeDelay;
			*p++ = bi_cfg->TimeDelayNormal;
		}
	}
	len = p - buf;	// len=53
	p = SlaveNetGetIOUnitCfgData(UnitID);
	if(n_memcmp(buf, p+3, len)) {
		p[0] = 1;		 // lock
		p[1] |= 0x01; p[2] = len; memcpy(p+3, buf, len);
//cprintf("IOUnit%d CfgData: Len=%d\n", UnitID, len);
		p[0] = 0;		// unlock
	}
}

void unitSetDoorPV(int ID, int PresentValue)
{	
	int		n;

	if(Units[ID].IouReq[0] & 0x01) n = 0; else n = 1;
	Units[ID].IouReq[0] &= 0x18;
	if(PresentValue & 0x01) {
		if(PresentValue == 1) Units[ID].IouReq[0] |= 0x04 + n;
		else	Units[ID].IouReq[0] |= 0x03;
cprintf("%ld IOU=%d: PV=%d Unlock Req=%02x-%02x\n", DS_TIMER, ID, PresentValue, (int)Units[ID].IouReq[0], (int)Units[ID].IouReq[1]);
	} else {
		if(PresentValue) Units[ID].IouReq[0] |= 0x01;
cprintf("%ld IOU=%d: PV=%d Lock Req=%02x-%02x\n", DS_TIMER, ID, PresentValue, (int)Units[ID].IouReq[0], (int)Units[ID].IouReq[1]);
	}
	if(ID > 1) unitIouDataRequest(ID);
}

void unitcrSetDoorPV(int ID, int PresentValue)
{	
	unsigned short	val, PV, Pri;

	PV = adGetPresentValue(ID);
	if(PV == 0) {
		PV = 0; Pri = 0;
	} else if(PV == 1) {
		PV = 2;
		Pri = (Units[ID].IouReq[1] >> 3) & 0x07; Pri++; if(Pri > 7) Pri = 0; 
	} else {
		if(PV & 0x01) PV = 1; else PV = 0;
		Pri = 1;
	}
	val = Units[ID].IouReq[1] & 0x07;
	val |= (PV << 6) | (Pri << 3);
	Units[ID].IouReq[1] = val;
//cprintf("%ld ACUnit=%d: DoorPV=%d FA=%d\n", DS_TIMER, ID, adGetPresentValue(ID), faGetPresentValue(ID));
//cprintf("%ld ACUnit=%d: PV=%d Pri=%d Req=%02x-%02x\n", DS_TIMER, ID, (int)PV, (int)Pri, (int)Units[ID].IouReq[0], (int)Units[ID].IouReq[1]);
	unitcrDataRequest(ID);
}

void unitSetOutputPV(int boID, int PresentValue)
{
	unsigned char	val;
	int		ID;

	ID = boGetUnit(boID);
	val = Units[ID].IouReq[0];
	if(boID & 1) {
		if(PresentValue) val |= 0x10; else val &= 0xef;
	} else {
		if(PresentValue) val |= 0x08; else val &= 0xf7;
	}
	Units[ID].IouReq[0] = val;
	if(ID > 1) unitIouDataRequest(ID);
}

void unitcrSetOutputPV(int boID, int PresentValue)
{	
	unsigned char	val;
	int		ID;

	ID = boGetUnit(boID);
	val = Units[ID].IouReq[1];
	if(boID & 1) {
		if(PresentValue) val |= 0x02; else val &= 0xfd;
	} else {
		if(PresentValue) val |= 0x04; else val &= 0xfb;
	}
	Units[ID].IouReq[1] = val;
	unitcrDataRequest(ID);
}

void unitcrSetState(int ID)
{
	_ACUnitSetState(ID);
	unitcrDataRequest(ID);	
}

void unitSetCruState(int ID)
{
	if(crGetType(ID)) {
		_CruSetState(ID);	
//cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[ID].CruReq[2], (int)Units[ID].CruReq[3]);
		unitCruDataRequest(ID);
	}
}

void unitSetCruDoor(int ID)
{
	if(crGetType(ID)) {
		_CruSetDoor(ID);	
//cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[ID].CruReq[2], (int)Units[ID].CruReq[3]);
		unitCruDataRequest(ID);
	}
}

void unitSetCruVersion(int ID)
{
	if(crGetType(ID)) {
		_CruSetVersion(ID);	
cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[ID].CruReq[2], (int)Units[ID].CruReq[3], (int)Units[ID].CruReq[4], (int)Units[ID].CruReq[5]);
		unitCruDataRequest(ID);
	}
}

void unitSetCruCaptureStart(int ID, int Mode)
{
	int		c, val;

	Units[ID].CruReq[1] |= 0x01;	
	if(Mode == USER_CARD) val = 0x04;
	else if(Mode == USER_PIN) val = 0x02;
	else if(Mode == USER_FP) val = 0x01;
	else	val = 0;
	c = Units[ID].CruReq[0] & 0xf8;
	c |= val;
	Units[ID].CruReq[0] = c;
//cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[ID].CruReq[2], (int)Units[ID].CruReq[3]);
	unitCruDataRequest(ID);
}

void unitSetCruCaptureEnd(int ID)
{
	Units[ID].CruReq[1] &= 0xfe;
	_CruSetState(ID);
//cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[UnitID].CruReq[2], (int)Units[UnitID].CruReq[3]);
	unitCruDataRequest(ID);
}

void unitcrSetCaptureStart(int ID, int Mode)
{
	unsigned char	val;

	if(Mode == USER_CARD) val = 0x40;
	else if(Mode == USER_PIN) val = 0x20;
	else if(Mode == USER_FP) val = 0x10;
	else	val = 0;
	val |= 0x80;
	Units[ID].IouReq[0] = (Units[ID].IouReq[0] & 0x0f) | val;
//cprintf("%ld AVUnit-%d: %02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].Req[0], (int)Units[ID].Req[1], (int)Units[UnitID].CruReq[2], (int)Units[UnitID].CruReq[3]);
	unitcrDataRequest(ID);
}

void unitcrSetCaptureEnd(int ID)
{
	_ACUnitSetState(ID);
//cprintf("%ld CRU-%d: Req=%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)Units[ID].CruReq[0], (int)Units[ID].CruReq[1], (int)Units[UnitID].CruReq[2], (int)Units[UnitID].CruReq[3]);
	unitcrDataRequest(ID);
}

void unitAuthResult(int UnitID, int Result)
{
	unsigned char	val;

	val = Units[UnitID].CruReq[1] = Result;
//cprintf("%ld CRU-%d: unitAuthResult=%d Req=%02x-%02x\n", DS_TIMER, UnitID, Result, (int)Units[UnitID].CruReq[0], (int)Units[UnitID].CruReq[1]);
	if(UnitID > 1) unitCruDataRequest(UnitID);
}

void unitAdd(int UnitID, int Control)
{
	unsigned char	buf[8];

	buf[0] = CONTROL_REQ;
	buf[1] = UnitID;
	if(Control & 0x02) buf[2] = 1; else buf[2] = 0;
	if(Control & 0x01) buf[3] = 1; else buf[3] = 0;
cprintf("Add Unit-%d %02x-%02x\n", UnitID, (int)buf[2], (int)buf[3]);
	SlaveSioWriteBuffer(buf, 8);
}

void unitcrAdd(int UnitID, int Control)
{
	unsigned char	buf[8];

	buf[0] = CONTROL_REQ;
	buf[1] = 0x60 + UnitID;
	buf[2] = 0;
	if(Control & 0x01) buf[3] = 1; else buf[3] = 0;
	if(Control & 0x02) {
cprintf("Add Sin ACUnit-%d %02x-%02x\n", UnitID, (int)buf[2], (int)buf[3]);
		SlaveSinWriteBuffer(buf, 8);
	} else {
cprintf("Add Sio ACUnit-%d %02x-%02x\n", UnitID, (int)buf[2], (int)buf[3]);
		SlaveSioWriteBuffer(buf, 8);
	}
}

void iounitAdd(int UnitID, int Control)
{
	unsigned char	buf[8];

	buf[0] = CONTROL_REQ;
	buf[1] = 0x40 + UnitID;
	if(Control & 0x02) buf[2] = 1; else buf[2] = 0;
	if(Control & 0x01) buf[3] = 1; else buf[3] = 0;
cprintf("Add IOUnit-%d %02x-%02x\n", UnitID, (int)buf[2], (int)buf[3]);
	SlaveSinWriteBuffer(buf, 8);	// ?
}

void iounitSetPresentValue(int UnitID, int boID, int PresentValue)
{
	unsigned char	c, msk;
	int		val;
	
	val = iounitGetBinaryOutput(UnitID);
	val = boID - val;
	msk = 0x80 >> (val & 0x07);
	val >>= 3;
cprintf("UnitID=%d Idx=%d Msk=%d\n", UnitID, val, (int)msk);
	c = IOUnits[UnitID].Req[val];
	if(PresentValue) c |= msk;
	else	 c &= ~msk;
	IOUnits[UnitID].Req[val] = c;
	iounitDataRequest(UnitID);
}

void unitIouDataRequest(int UnitID)
{
	unsigned char	buf[8];

	buf[0] = DATA_REQ;
	buf[1] = UnitID;
	buf[2] = Units[UnitID].IouReq[0];
	buf[3] = Units[UnitID].IouReq[1];
	memset(buf+4, 0, 4);
//cprintf("%ld IOU-%d: Req=%02x-%02x\n", DS_TIMER, UnitID, (int)buf[2], (int)buf[3]);
	SlaveSioWriteBuffer(buf, 8);
}

void unitCruDataRequest(int UnitID)
{
	unsigned char	buf[8];

	buf[0] = DATA_REQ;
	buf[1] = 0x20 | UnitID;
	buf[2] = Units[UnitID].CruReq[0];
	buf[3] = Units[UnitID].CruReq[1];
	buf[4] = Units[UnitID].CruReq[2];
	buf[5] = Units[UnitID].CruReq[3];
	buf[6] = Units[UnitID].CruReq[4];
	buf[7] = Units[UnitID].CruReq[5];
//cprintf("%ld CRU-%d: CruReq=%02x-%02x-%02x-%02x-%02x-%02x\n", DS_TIMER, UnitID, (int)buf[2], (int)buf[3], (int)buf[4], (int)buf[5], (int)buf[6], (int)buf[7]);
	SlaveSioWriteBuffer(buf, 8);
}

void unitcrDataRequest(int UnitID)
{
	unsigned char	buf[8];

	buf[0] = DATA_REQ;
	buf[1] = 0x60 | UnitID;
	buf[2] = Units[UnitID].IouReq[0];
	buf[3] = Units[UnitID].IouReq[1];
	memset(buf+4, 0, 4);
//cprintf("%ld ACUnit-%d: Req=%02x-%02x\n", DS_TIMER, UnitID, (int)buf[2], (int)buf[3]);
	if(crGetType(UnitID)) SlaveSinWriteBuffer(buf, 8);
	else	SlaveSinWriteBuffer(buf, 8);	// ?
}

void iounitDataRequest(int UnitID)
{
	unsigned char	buf[8];

	buf[0] = DATA_REQ;
	buf[1] = 0x40 | UnitID;
	buf[2] = IOUnits[UnitID].Req[0];
	buf[3] = IOUnits[UnitID].Req[1];
	memset(buf+4, 0, 4);
cprintf("%ld IOUnit%d: Req=%02x-%02x\n", DS_TIMER, UnitID, (int)buf[2], (int)buf[3]);
	SlaveSioWriteBuffer(buf, 8);
}

void unitcrProcessReq(int ID, unsigned char *buf)
{
	UNIT	*unit;
	int		i, val0, val1, msk, max;

 	unit = unitsGetAt(ID);
	val0 = adGetPresentValue(ID);
	if(val0 == 0) val1 = 0;
	else if(val0 == 1) val1 = 2;
	else if(val0 & 0x01) val1 = 1;
	else	val1 = 0;
 	unit->IouRsp[0] = (unit->IouRsp[0] & 0x3f) | (val1 << 6);
	val0 = buf[0] ^ unit->IouRsp[0]; val1 = buf[1] ^ unit->IouRsp[1];
 	unit->IouRsp[0] = buf[0]; unit->IouRsp[1] = buf[1];
 	if(val0) {
   		if(val0 & 0xc0) adRemotePVChanged(ID);
	 	if(val0 & 0x30) adAlarmChanged(ID);
  		i = unitGetBinaryInput(ID); max = i + 2;
 		for(msk = 0x08;i < max;i++) {
 			if(val0 & msk) biRemotePVChanged(i);
 			msk >>= 1;
 			if(val0 & msk) biAlarmChanged(i);
			msk >>= 1;
 		}
 	}
 	if(val1) {
  		i = unitGetBinaryInput(ID) + 2; max = i + 2;
 		for(msk = 0x80;i < max;i++) {
 			if(val1 & msk) biRemotePVChanged(i);
 			msk >>= 1;
 			if(val1 & msk) biAlarmChanged(i);
			msk >>= 1;	
 		}
  		if(val1 & 0x08) crRemoteTamperPVChanged(ID);
  		if(val1 & 0x04) crTamperAlarmChanged(ID); 		
  		if(val1 & 0x02) crIntrusionPVChanged(ID);
  		if(val1 & 0x01) crIntrusionAlarmChanged(ID); 		
 	}
}

UNIT *unitsGetUnits(void)
{
	return Units;
}

int acunitEncodeStatus(int ID, unsigned char *Buffer)
{
	UNIT	*unit;
	unsigned char	*p;
	unsigned long	lVal;
	
 	unit = unitsGetAt(ID);
	p = Buffer;
	*p++ = unitGetReliability(ID);
	datetime2longtime(unit->ReliabilityTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - Buffer;
}

#include "net.h"

void _SendNet(NET *net);

void acunitNotifyStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_AC_UNIT;
	*p++ = ID;
	p += acunitEncodeStatus(ID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

void acunitNotifyObjectsStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[256];
	int		i, ObjID;

	net = (NET *)Buffer;
	p = net->Data;
	ObjID = unitGetAccessDoor(ID);
	*p++ = OT_ACCESS_DOOR;
	for(i = 0;i < 2;i++, ObjID++)
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
	for(i = 0;i < 8;i++, ObjID++)
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
	for(i = 0;i < 4;i++, ObjID++)
		if(boIsEnable(ObjID) && boGetAssignedDoor(ObjID) < 0 && !boIsAssignedAlarmAction(ObjID)) {
			*p++ = ObjID; p += boEncodeStatus(ObjID, p);
		}
	net->DataLength = p - net->Data;
	if(net->DataLength > 1) {
		netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
		_SendNet(net);
	}
}

int acunitsEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_AC_UNIT;
	for(i = 0;i < MAX_UNIT_SZ;i += 2)
		if(unitGetEnable(i) && unitGetType(i) != 2) {
			*p++ = i; p += acunitEncodeStatus(i, p);
		}
	return p - Buffer;
}

int iounitEncodeStatus(int ID, unsigned char *Buffer)
{
	IOUNIT	*unit;
	unsigned char	*p;
	unsigned long	lVal;
	
 	unit = iounitsGetAt(ID);
	p = Buffer;
	*p++ = iounitGetReliability(ID);
	datetime2longtime(unit->ReliabilityTime, &lVal); LONGtoBYTE(lVal, p); p += 4;
	return p - Buffer;
}

void iounitNotifyStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[64];
	
	net = (NET *)Buffer;
	p = net->Data;
	*p++ = OT_IO_UNIT;
	*p++ = ID;
	p += iounitEncodeStatus(ID, p);
	net->DataLength = p - net->Data;	
	netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
	_SendNet(net);
}

void iounitNotifyObjectsStatus(int ID)
{
	NET		*net;
	unsigned char	*p, Buffer[400];
	int		i, ObjID, type;

	net = (NET *)Buffer;
	p = net->Data;
	type = iounitGetType(ID);
	if(type == 0) {
		ObjID = iounitGetBinaryInput(ID);
		*p++ = OT_BINARY_INPUT;
		for(i = 0;i < MAX_IOUNIT_BI_SZ;i++, ObjID++) {
			*p++ = ObjID; p += biEncodeStatus(ObjID, p);
		}
	} else {
		ObjID = iounitGetBinaryInput(ID);
		*p++ = OT_BINARY_OUTPUT;
		for(i = 0;i < MAX_IOUNIT_BO_SZ;i++, ObjID++) {
			*p++ = ObjID; p += boEncodeStatus(ObjID, p);
		}
	}
	net->DataLength = p - net->Data;
	if(net->DataLength > 1) {
		netCodeUnconfRequest(net, P_STATUS_NOTIFICATION);
		_SendNet(net);
	}
}

int iounitsEncodeStatus(unsigned char *Buffer)
{
	unsigned char*p;
	int		i;

	p = Buffer;
	*p++ = OT_IO_UNIT;
	for(i = 0;i < MAX_IOUNIT_SZ;i += 2)
		if(iounitGetEnable(i)) {
			*p++ = i; p += iounitEncodeStatus(i, p);
		}
	return p - Buffer;
}

int adGetReliability(int adID)
{
	int		val;

	if(unitGetType(adID) == 2) val = crGetReliability(adID);
//	else if(adID < 2) val = unitGetReliability(adID);
//	else	val = 0;
	else	val = unitGetReliability(adID);	// Bug fix: 2015.7.27
	return val;
}

int adEvaluateAlarmState(int adID)
{
	ACCESS_DOOR		*ad;
	int		val, c;
	
	val = 0;
	if(unitGetType(adID) == 2) {
		ad = adsGetAt(adID);
		c = (Units[adID].IouRsp[0] >> 4) & 0x03;
		if(c == 1 && ad->cfg->DoorOpenTooLongEnable ) val = 1;		// OpenTooLong
		else if(c == 2 && ad->cfg->DoorForcedOpenEnable) val = 2;	// ForcedOpen
	} else if(adID < 2) {
		c = adState[adID];
		if(c >= 4) val = c - 3;	// 1:OpenTooLong 2:ForcedOpen
	} else if(Units[adID].IouRsp[0] & 0x80) {
		ad = adsGetAt(adID);
		c = Units[adID].IouRsp[0] & 0x40;
		if(c && ad->cfg->DoorOpenTooLongEnable) val = 1;		// OpenTooLong
		else if(!c && ad->cfg->DoorForcedOpenEnable) val = 2;	// ForcedOpen
	}
	return val;
}

int adGetDoorPV(int adID)
{
	if(unitGetType(adID) == 2) {
		if((Units[adID].IouRsp[0] & 0xc0) >= 0x80) return 1;
		else	return 0;
	} else {
		if(Units[adID].IouRsp[0] & 0x20) return 1;
		else	return 0;
	}
}

int biGetPresentValue(int biID)
{
	BINARY_INPUT	*bi;
	int		val, msk, ID;

	if(biID < MAX_BI_SZ) {
		ID = biGetUnit(biID);
		if(unitGetType(ID) == 2) {
			val = biID & 0x03;
			val <<= 1;
			if(val < 4) {
				msk = 0x08 >> val;
				if(Units[ID].IouRsp[0] & msk) val = 1;
				else	val = 0;	
			} else {
				msk = 0x80 >> (val-4);
//cprintf("val=%d msk=%d Rsp[1]=%02x\n", (int)val, (int)msk, (int)Units[ID].IouRsp[1]);
				if(Units[ID].IouRsp[1] & msk) val = 1;
				else	val = 0;				
			}
		} else if(biID < 8) {
			bi = bisGetAt(biID);
			val = biPV[biID] ^ bi->cfg->Polarity;
		} else {
			msk = 0x02 << (biID & 0x03);
			if(Units[ID].IouRsp[0] & msk) val = 1;
			else	val = 0;
		}
	} else {
		ID = biGetIOUnit(biID);
	  	val = iounitGetBinaryInput(ID);
		val = biID - val;
		msk = 0x80 >> (val & 0x07);
		if(IOUnits[ID].Rsp[val>>3] & msk) val = 1; else val = 0; 
	}
	return val;
}

int biGetAlarmState(int biID)
{
	BINARY_INPUT	*bi;
	int		val, msk, ID;

	if(biGetAlarmInhibit(biID)) val = 0;
	else if(biID < MAX_BI_SZ) {
		ID = biGetUnit(biID);
		val = biID & 0x03;
		if(unitGetType(ID) == 2) {
			val <<= 1;
			if(val < 4) {
				msk = 0x04 >> val;
				if(Units[ID].IouRsp[0] & msk) val = 1;
				else	val = 0;	
			} else {
				msk = 0x40 >> (val-4);
				if(Units[ID].IouRsp[1] & msk) val = 1;
				else	val = 0;				
			}
		} else if(biID < 8) {
			bi = bisGetAt(biID);
			val = biPV[biID] ^ bi->cfg->Polarity;
			if(val && !biTimeout[biID] || !val && biTimeout[biID]) val = 1;
			else	val = 0;
		} else {
			msk = 0x02 << val;
			if(Units[ID].IouRsp[1] & msk) val = 1;
			else	val = 0;
		}
	} else {
		ID = biGetIOUnit(biID);
	  	val = iounitGetBinaryInput(ID);
		val = biID - val;
		msk = 0x80 >> (val & 0x07);
		if(IOUnits[ID].Rsp[3+(val>>3)] & msk) val = 1; else val = 0; 
	}
	return val;
}

int biGetReliability(int biID)
{
	int		ID, val;

	ID = BI2UNIT(biID);
	if(unitGetType(ID) == 2) val = crGetReliability(ID);
	else if(ID < 2) val = 0;
	else	val = unitGetReliability(ID);
	return val;
}

int crGetTamperPresentValue(int crID)
{
	int		val;

	if(unitGetType(crID) == 2) {
		if(Units[crID].IouRsp[1] & 0x08) val = 1;
		else	val = 0;
	} else if(crID < 2 && !crGetType(crID)) {
		val = biPV[8 + (crID&1)];
	} else {
		if(Units[crID].CruRsp[0] & 0x01) val = 1;
		else	val = 0;
	}
	return val;
}

int crGetTamperAlarmState(int crID)
{
	CREDENTIAL_READER	*cr;
	int		i, val;

	cr = crsGetAt(crID);
	if(cr->cfg->TamperAlarmInhibit) val = 0;
	else {
		if(unitGetType(crID) == 2) {
			if(Units[crID].IouRsp[1] & 0x04) val = 1;
			else	val = 0;
		} else if(crID < 2 && !crGetType(crID)) {
			i = 8 + (crID & 1);
			if(biPV[i] && !biTimeout[i] || !biPV[i] && biTimeout[i]) val = 1;
			else	val = 0;
		} else {
			if(Units[crID].CruRsp[0] & 0x02) val = 1;
			else	val = 0;			
		}
	}
	return val;
}

int crGetIntrusionPresentValue(int crID)
{
	int		val;

	if(unitGetType(crID) == 2 && Units[crID].IouRsp[1] & 0x02) val = 1;
	else	val = 0;
	return val;
}

int crGetIntrusionAlarmState(int crID)
{
	int		val;

	if(unitGetType(crID) == 2 && Units[crID].IouRsp[1] & 0x01) val = 1;
	else	val = 0;
	return val;
}

