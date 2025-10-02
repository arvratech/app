#include "acadcfg.h"
#include "crcfg.h"
#include "almcfg.h"
#include "firecfg.h"
#include "unitcfg.h"

static UNIT_CFG			*UnitsCfg[MAX_UNIT_SZ];
static ACCESS_DOOR_CFG	*DoorsCfg[MAX_AD_SZ];
static CR_CFG			*ReadersCfg[MAX_CR_SZ];
static ALARM_ACTION_CFG	*AlarmActionsCfg[MAX_UNIT_SZ];
static IOUNIT_CFG		*IOUnitsCfg[MAX_IOUNIT_SZ];

#ifdef WIN32

#include <Windows.h>
static int	UnitsCriSecInited;
static CRITICAL_SECTION	UnitsCriSec;

void unitsEnter(UNITS_CFG *units_cfg)
{
	int		i;

	if(!UnitsCriSecInited) {
		InitializeCriticalSection(&UnitsCriSec);
		UnitsCriSecInited = 1;
	}
	EnterCriticalSection(&UnitsCriSec);	
	for(i = 0;i < MAX_UNIT_SZ;i++) UnitsCfg[i]   = &units_cfg->Units[i];
	for(i = 0;i < MAX_AD_SZ;i++)   DoorsCfg[i]   = (ACCESS_DOOR_CFG *)&units_cfg->Doors[i];
	for(i = 0;i < MAX_CR_SZ;i++)   ReadersCfg[i] = (CR_CFG *)&units_cfg->Readers[i];
	for(i = 0;i < MAX_UNIT_SZ;i++) AlarmActionsCfg[i] = (ALARM_ACTION_CFG *)&units_cfg->AlarmActions[i];
	for(i = 0;i < MAX_IOUNIT_SZ;i++) IOUnitsCfg[i] = &units_cfg->IOUnits[i];
}

void unitsLeave(void)
{
	LeaveCriticalSection(&UnitsCriSec);
}

#else

void unitscfgSet(UNITS_CFG *units_cfg)
{
	int		i;
	
	for(i = 0;i < MAX_UNIT_SZ;i++) UnitsCfg[i]   = &units_cfg->Units[i];
	for(i = 0;i < MAX_AD_SZ;i++)   DoorsCfg[i]   = (ACCESS_DOOR_CFG *)&units_cfg->Doors[i];
	for(i = 0;i < MAX_CR_SZ;i++)   ReadersCfg[i] = (CR_CFG *)&units_cfg->Readers[i];
	for(i = 0;i < MAX_UNIT_SZ;i++)  AlarmActionsCfg[i] = (ALARM_ACTION_CFG *)&units_cfg->AlarmActions[i];
	for(i = 0;i < MAX_IOUNIT_SZ;i++) IOUnitsCfg[i]   = &units_cfg->IOUnits[i];
}

void unitscfgGet(void **pUnits, void **pDoors, void **pReaders, void **pAlarmActions, void **pIOUnits)
{
	*pUnits			= UnitsCfg;
	*pDoors			= DoorsCfg;
	*pReaders		= ReadersCfg;
	*pAlarmActions	= AlarmActionsCfg;	
	*pIOUnits		= IOUnitsCfg;
}

#endif

void unitsSetDefault(void)
{
	ACCESS_DOOR_CFG 	*ad_cfg;
	CR_CFG				*cr_cfg;
	ALARM_ACTION_CFG	*act_cfg;
	int		i;

	for(i = 0;i < MAX_UNIT_SZ;i++) unitSetDefault(UnitsCfg[i], i);
	for(i = 0;i < MAX_AD_SZ;i++) {
		ad_cfg = DoorsCfg[i];
		ad_cfg->Enable	  	 		= 1;
		ad_cfg->RequestToExitEnable = 1;
		ad_cfg->DoorStatusEnable   	= 0;
		ad_cfg->AuxLockEnable		= 0;
	}
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = ReadersCfg[i];
		cr_cfg->Type				= 0;
	}
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		act_cfg = AlarmActionsCfg[i];
		act_cfg->AlarmOutput		= 0xff;
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) iounitSetDefault(IOUnitsCfg[i], i);
}

int unitsEncode(void *buf)
{
	ACCESS_DOOR_CFG 	*ad_cfg;
	CR_CFG				*cr_cfg;
	ALARM_ACTION_CFG	*act_cfg;
	unsigned char	*p, val;
	int		i;	

	p = (unsigned char *)buf;
	for(i = 0;i < MAX_UNIT_SZ;i++) p += unitEncode(UnitsCfg[i], p);
	for(i = 0;i < MAX_AD_SZ;i++) {
		ad_cfg = DoorsCfg[i];
		if(ad_cfg->Enable) val = 0x01; else val = 0x00;
		if(ad_cfg->RequestToExitEnable) val |= 0x02;
		if(ad_cfg->DoorStatusEnable) val |= 0x04;
		if(ad_cfg->AuxLockEnable) val |= 0x08;
		*p++ = val;
	}
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = ReadersCfg[i];
		*p++ = cr_cfg->Type;
	}
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		act_cfg = AlarmActionsCfg[i];
		*p++ = act_cfg->AlarmOutput;
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitEncode(IOUnitsCfg[i], p);
	return p - (unsigned char *)buf;
}

int unitsDecode(void *buf, int size)
{
	ACCESS_DOOR_CFG 	*ad_cfg;
	CR_CFG				*cr_cfg;
	ALARM_ACTION_CFG	*act_cfg;
	unsigned char	*p, val;
	int		i;	

	p = (unsigned char *)buf;
	for(i = 0;i < MAX_UNIT_SZ;i++) p += unitDecode(UnitsCfg[i], p);
	for(i = 0;i < MAX_AD_SZ;i++) {
		ad_cfg = DoorsCfg[i];
		val = *p++;
		ad_cfg->Enable				= val & 0x01;
		ad_cfg->RequestToExitEnable	= (val >> 1) & 0x01;
		ad_cfg->DoorStatusEnable	= (val >> 2) & 0x01;
		ad_cfg->AuxLockEnable		= (val >> 3) & 0x01;
	}
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = ReadersCfg[i];
		cr_cfg->Type				= *p++;
	}
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		act_cfg = AlarmActionsCfg[i];
		act_cfg->AlarmOutput		= *p++;
	}
	val = p - (unsigned char *)buf;
	if(val >= MAX_IOUNIT_SZ) {
		for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitDecode(IOUnitsCfg[i], p);
	}
	return p - (unsigned char *)buf;
}	

int unitsValidate(void)
{
	return 1;
}

void unitSetDefault(UNIT_CFG *unit_cfg, int ID)
{
	if(ID < 2) unit_cfg->Enable = 1;	// Local Unit
	else	   unit_cfg->Enable = 0;	// Remote Unit	
	unit_cfg->Type = 0;		// K-200
}

int unitEncode(UNIT_CFG *unit_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	if(unit_cfg->Enable) val = 0x01; else val = 0x00;
	val |= unit_cfg->Type << 1;
	*p = val;
	return 1;
}

int unitDecode(UNIT_CFG *unit_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	val = *p;
	unit_cfg->Enable = val & 0x01;
	unit_cfg->Type	 = val >> 1;
	return 1;
}

int unitValidate(UNIT_CFG *unit_cfg)
{
	return 1;
}

int unitGetEnable(int UnitID)
{
	int		enable;

	if(UnitID >= 0 && UnitID < MAX_UNIT_SZ) enable = UnitsCfg[UnitID]->Enable;
	else	enable = 0;
	return enable;
}

void unitSetEnable(int UnitID, int Enable)
{
	if(UnitID >= 0 && UnitID < MAX_UNIT_SZ) UnitsCfg[UnitID]->Enable = (unsigned char)Enable;
}

// Return	-1:Null 0:K-200 1:K-220 2:K-300
int unitGetType(int UnitID)
{
	int		type;

	if(UnitID >= 0 && UnitID < MAX_UNIT_SZ) {
		if(UnitID & 1) UnitID--;
		type = UnitsCfg[UnitID]->Type;
		if(UnitID < 2 && type != 2) type = 0;
	} else	type = -1; 
	return type;
}

void unitSetType(int UnitID, int Type)
{
	if(UnitID >= 0 && UnitID < MAX_UNIT_SZ) {		
		UnitsCfg[UnitID]->Type = (unsigned char)Type;
		if(UnitID & 1) UnitID--; else UnitID++;
		UnitsCfg[UnitID]->Type = (unsigned char)Type;
	}
}

void iounitSetDefault(IOUNIT_CFG *unit_cfg, int ID)
{
	unit_cfg->Enable = 0;
	if(iounitCanTypeInput(ID)) unit_cfg->Type = 0;	// K-500(Input)
	else	 unit_cfg->Type = 1;					// K-700(Output)
}

int iounitEncode(IOUNIT_CFG *unit_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	if(unit_cfg->Enable) val = 0x01; else val = 0x00;
	val |= unit_cfg->Type << 1;
	*p = val;
	return 1;
}

int iounitDecode(IOUNIT_CFG *unit_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	val = *p;
	unit_cfg->Enable = val & 0x01;
	unit_cfg->Type	 = val >> 1;
	return 1;
}

int iounitValidate(IOUNIT_CFG *unit_cfg)
{
	return 1;
}

int iounitGetEnable(int ID)
{
	int		enable;

	if(ID >= 0 && ID < MAX_IOUNIT_SZ) enable = IOUnitsCfg[ID]->Enable;
	else	enable = 0;
	return enable;
}

void iounitSetEnable(int ID, int Enable)
{
	if(ID >= 0 && ID < MAX_IOUNIT_SZ) IOUnitsCfg[ID]->Enable = (unsigned char)Enable;
}

// Return	-1:Null 0:K-500 1:K-700
int iounitGetType(int ID)
{
	int		type;

	if(ID >= 0 && ID < MAX_IOUNIT_SZ-4) type = IOUnitsCfg[ID]->Type;
	else if(ID >= MAX_IOUNIT_SZ-4 && ID < MAX_IOUNIT_SZ) type = 1;
	else	type = -1;
	return type;
}

void iounitSetType(int ID, int Type)
{
	if(!Type && ID >= 0 && ID < MAX_IOUNIT_SZ-4 || Type && ID >= 0 && ID < MAX_IOUNIT_SZ) IOUnitsCfg[ID]->Type = (unsigned char)Type;
}

int adGetEnable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ) enable = DoorsCfg[adID]->Enable;
	else	enable = 0;
	return enable;
}

void adSetEnable(int adID, int Enable)
{
	if(adID >= 0 && adID < MAX_AD_SZ) DoorsCfg[adID]->Enable = (unsigned char)Enable;
}

int adGetRequestToExitEnable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ) enable = DoorsCfg[adID]->RequestToExitEnable;
	else	enable = 0;
	return enable;
}

void adSetRequestToExitEnable(int adID, int Enable)
{
	if(adID >= 0 && adID < MAX_AD_SZ) DoorsCfg[adID]->RequestToExitEnable = (unsigned char)Enable;
}

int adGetDoorStatusEnable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ) enable = DoorsCfg[adID]->DoorStatusEnable;
	else	enable = 0;
	return enable;
}

void adSetDoorStatusEnable(int adID, int Enable)
{
	if(adID >= 0 && adID < MAX_AD_SZ) DoorsCfg[adID]->DoorStatusEnable = (unsigned char)Enable;
}

int adGetAuxLockEnable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ) enable = DoorsCfg[adID]->AuxLockEnable;
	else	enable = 0;
	return enable;
}

void adSetAuxLockEnable(int adID, int Enable)
{
	if(adID >= 0 && adID < MAX_AD_SZ) DoorsCfg[adID]->AuxLockEnable = (unsigned char)Enable;
}

int adIsUsable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ && UnitsCfg[adID]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

int adIsEnable(int adID)
{
	int		enable;

	if(adID >= 0 && adID < MAX_AD_SZ && UnitsCfg[adID]->Enable && DoorsCfg[adID]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

// Condition: adIsEnable=TRUE
// -1:No >=0:ExitPointID
int adGetExitPoint(int adID)
{
	int		apID;
	
	if(adID & 1) apID = adID - 1;
	else		 apID = adID + 1;
	if(!UnitsCfg[apID]->Enable && (UnitsCfg[apID]->Type == 2 || !ReadersCfg[apID]->Type) || UnitsCfg[apID]->Enable && DoorsCfg[apID]->Enable) apID = -1;
	return apID;
}

int apIsUsable(int apID)
{
	int		enable;

	if(apID >= 0 && apID < MAX_AP_SZ && (UnitsCfg[apID]->Enable || UnitsCfg[apID]->Type != 2 && ReadersCfg[apID]->Type)) enable = 1;
	else	enable = 0;
	return enable;
}

int apIsEnable(int apID)
{
	int		enable;

	if(apID >= 0 && apID < MAX_AP_SZ && (UnitsCfg[apID]->Enable || UnitsCfg[apID]->Type != 2 && ReadersCfg[apID]->Type)) enable = 1;
	else	enable = 0;
	return enable;
}

// Condition: apIsEnable=TRUE
// -1:No >=0:DoorID
int apGetAssignedDoor(int apID)
{
	int		adID;
	
	if(adIsEnable(apID)) adID = apID;
	else {
		if(apID & 1) adID = apID - 1;
		else		 adID = apID + 1;
		if(!UnitsCfg[adID]->Enable || !DoorsCfg[adID]->Enable) adID = -1;
	}
	return adID;
}

int biIsUsable(int biID)
{
	int		enable;

	if(biID >= 0 && biID < MAX_BI_SZ && UnitsCfg[BI2UNIT(biID)]->Enable
			|| biID >= MAX_BI_SZ && biID < MAX_BI_SZ+MAX_XBI_SZ && !IOUnitsCfg[XBI2UNIT(biID)]->Type && IOUnitsCfg[XBI2UNIT(biID)]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

int biIsEnable(int biID)
{
	int		enable;

	if(biID >= 0 && biID < MAX_BI_SZ && UnitsCfg[BI2UNIT(biID)]->Enable
		|| biID >= MAX_BI_SZ && biID < MAX_BI_SZ+MAX_XBI_SZ && !IOUnitsCfg[XBI2UNIT(biID)]->Type && IOUnitsCfg[XBI2UNIT(biID)]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

// -1:Null 0:Exit 1:Door 2:Input 3:Fire 4:Tamper
int biGetType(int biID)
{
	int		type;

	if(biID >= 0 && biID < MAX_BI_SZ) {
		type = biID & 0x07;
		if(type == 7) type = 4;
		else	type &= 0x03; 
	} else if(biID >= MAX_BI_SZ && biID < MAX_BI_SZ+MAX_XBI_SZ && !IOUnitsCfg[XBI2UNIT(biID)]->Type) {
		type = 2;
	} else	type = -1;
	return type;
}

// Condition: biIsEnable=TRUE
// -1:No >=0:DoorID
int biGetAssignedDoor(int biID)
{
	int		ID, type;

	ID = BI2UNIT(biID);
	type = biID & 0x03;
	if(type > 1 || !DoorsCfg[ID]->Enable || (!type && !DoorsCfg[ID]->RequestToExitEnable || type && !DoorsCfg[ID]->DoorStatusEnable)) ID = -1;
	return ID;
}

int boIsUsable(int boID)
{
	int		enable, ID;

	ID = BO2UNIT(boID);
	if(boID >= 0 && boID < MAX_BO_SZ && (UnitsCfg[ID]->Type || !(boID & 1)) && UnitsCfg[ID]->Enable
		|| boID >= MAX_BO_SZ && boID < MAX_BO_SZ+MAX_XBO_SZ && IOUnitsCfg[XBO2UNIT(boID)]->Type && IOUnitsCfg[XBO2UNIT(boID)]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

int boIsEnable(int boID)
{
	int		enable, ID;

	ID = BO2UNIT(boID);
	if(boID >= 0 && boID < MAX_BO_SZ && (UnitsCfg[ID]->Type || !(boID & 1)) && UnitsCfg[ID]->Enable
		|| boID >= MAX_BO_SZ && boID < MAX_BO_SZ+MAX_XBO_SZ && IOUnitsCfg[XBO2UNIT(boID)]->Type && IOUnitsCfg[XBO2UNIT(boID)]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

// -1:Null 0:Lock 1:Relay
int boGetType(int boID)
{
	int		type;

	if(boID >= 0 && boID < MAX_BO_SZ && (UnitsCfg[BO2UNIT(boID)]->Type || !(boID & 1))) type = boID & 1;
	else if(boID >= MAX_BO_SZ && boID < MAX_BO_SZ+MAX_XBO_SZ && IOUnitsCfg[XBO2UNIT(boID)]->Type) type = 1;
	else	type = -1;
	return type;
}

// Condition: boIsEnable=TRUE
// -1:No >=0:DoorID
int boGetAssignedDoor(int boID)
{
	int		ID, type;
	
	ID = BO2UNIT(boID);
	if(boID & 1) type = 1; else type = 0;
	if(!type && !DoorsCfg[ID]->Enable || type && (!DoorsCfg[ID]->Enable || !DoorsCfg[ID]->AuxLockEnable)) ID = -1;
	return ID;
}

// Condition: boIsEnable=TRUE
// 0:Non-Assigned 1:Assigned
int boIsAssignedAlarmAction(int boID)
{
	int		i, ID;
	
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		ID = AlarmActionsCfg[i]->AlarmOutput;
		if(ID == boID && boGetAssignedDoor(ID) < 0) break;
	}
	if(i < MAX_UNIT_SZ) return 1;
	else	return 0;
}

int crIsUsable(int crID)
{
	int		enable;

	if(crID >= 0 && crID < MAX_CR_SZ && (UnitsCfg[crID]->Enable || ReadersCfg[crID]->Type)) enable = 1;
	else	enable = 0;
	return enable;
}

int crIsEnable(int crID)
{
	int		enable;

	if(crID >= 0 && crID < MAX_CR_SZ && (UnitsCfg[crID]->Enable || UnitsCfg[crID]->Type != 2 && ReadersCfg[crID]->Type)) enable = 1;
	else	enable = 0;
	return enable;
}

// -1:Null 0:Wiegand 1:RS-485 2:IP
int crGetType(int crID)
{
	int		type;

	if(crID >= 0 && crID < MAX_CR_SZ) {
//		if(UnitsCfg[crID]->Type == 2) type = 0;
//		else	type = ReadersCfg[crID]->Type;
		type = ReadersCfg[crID]->Type;
	} else	type = -1;
	return type;
}

void crSetType(int crID, int Type)
{
	if(crID >= 0 && crID < MAX_CR_SZ) {
//		if(UnitsCfg[crID]->Type != 2) ReadersCfg[crID]->Type = (unsigned char)Type;
		ReadersCfg[crID]->Type = (unsigned char)Type;
	}
}

int fpIsUsable(int fpID)
{
	int		enable;

	if(fpID >= 0 && fpID < MAX_UNIT_HALF_SZ && UnitsCfg[fpID<<1]->Enable) enable = 1;
	else	enable = 0;
	return enable;
}

int actGetAlarmOutput(int actID)
{
	int		output;

	if(actID >= 0 && actID < MAX_UNIT_SZ) {
		output = AlarmActionsCfg[actID]->AlarmOutput;
		if(output == 0xff) output = -1;
	} else	output = -1;
	return output;
}

void actSetAlarmOutput(int actID, int boID)
{
	if(boID < 0) boID = 0xff;
	if(actID >= 0 && actID < MAX_UNIT_SZ) AlarmActionsCfg[actID]->AlarmOutput = (unsigned char)boID;
}

int actAlarmOutputIsEnable(int actID)
{
	int		enable, boID, ID;

	enable = 0;
	if(actID >= 0 && actID < MAX_UNIT_SZ) {
		boID = AlarmActionsCfg[actID]->AlarmOutput;
		ID = UNIT2BO(actID) & 0xfc;
		if(boIsEnable(boID) && boGetAssignedDoor(boID) < 0 && boID >= ID && boID < ID+4) enable = 1;	
	}
	return enable;
}

int actIsUsable(int actID)
{
	int		i, enable, boID;

	enable = 0;
	if(apIsUsable(actID)) {
		boID = UNIT2BO(actID) & 0xfc;
		for(i = 0;i < 4;i++, boID++)
			if(boIsEnable(boID) && boGetAssignedDoor(boID) < 0) break;
		if(i < 4) enable = 1;
	}
	return enable;
}

int actIsEnable(int actID)
{
	int		enable, boID, ID;

	enable = 0;
	if(apIsUsable(actID)) {
		boID = AlarmActionsCfg[actID]->AlarmOutput;
		ID = UNIT2BO(actID) & 0xfc;
		if(boIsEnable(boID) && boGetAssignedDoor(boID) < 0 && boID >= ID && boID < ID+4) enable = 1;	
	}
	return enable;
}

// Condition: actIsUsable=TRUE
int actGetUsableAlarmOutputs(int actID, unsigned char *Outputs)
{
	int		i, boID, Item;
	
	boID = UNIT2BO(actID);
	for(Item = i = 0;i < 2;i++, boID++)
		if(boIsEnable(boID) && boGetAssignedDoor(boID) < 0) {
			Outputs[Item] = boID; Item++; 
		}
	i = unitGetPair(actID);
	boID = UNIT2BO(i);
	for(i = 0;i < 2;i++, boID++)
		if(boIsEnable(boID) && boGetAssignedDoor(boID) < 0) {
			Outputs[Item] = boID; Item++; 
		}
	for(i = Item;i < 4;i++) Outputs[i] = 0xff;
	return Item;
}
