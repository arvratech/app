#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "syscfg.h"
#include "slvmlib.h"
#include "slvm.h"
#include "dev.h"
#include "bio.h"


extern BINARY_INPUT		_BinaryInputs[];
extern BINARY_OUTPUT	_BinaryOutputs[];


void bisInit(void)
{
	BINARY_INPUT	*bi;
	int		i, size, dlu;

	dlu = devLocalUnitModel(NULL);
	if(dlu) size = MAX_BI_SZ + MAX_XBI_SZ;
	else	size = MAX_BI_HALF_SZ + MAX_XBI_HALF_SZ;
	for(i = 0, bi = _BinaryInputs;i < size;i++, bi++) {
		bi->cls			= CLS_BINARY_INPUT;
		if(dlu || i < MAX_BI_QUARTER_SZ) bi->id = i;
		else if(i < MAX_BI_HALF_SZ) bi->id = i + MAX_BI_QUARTER_SZ;
		else	bi->id = i + MAX_BI_HALF_SZ;
		bi->cfg			= &sys_cfg->binaryInputs[i];
		bi->status		= 0;
		//rtcGetDateTime(bi->changeOfStateTime);
		memcpy(bi->alarmStateTime, bi->changeOfStateTime, 6);
	}
}

void *_BisGet(int id)
{
	if(!devLocalUnitModel(NULL)) {
		if(id >= MAX_BI_HALF_SZ && id < MAX_BI_SZ) id -= MAX_BI_QUARTER_SZ;
		else if(id >= MAX_BI_SZ) id -= MAX_BI_HALF_SZ;
	}
	return &_BinaryInputs[id];
}

void *bisGet(int id)
{
	void	*bi, *parent;
	int		 dlu, model;

	dlu = devLocalUnitModel(NULL);
	if(!dlu && id < 4) {
		if(!devHaveIo(NULL)) id = -1;
	} else if(id < MAX_BI_HALF_SZ) {
		parent = subdevsGet(SUBDEVBI2CH(id));
		if(!parent || !subdevIsNonNull(parent) || !subdevHaveIo(parent)) id = -1;
	} else if(id < MAX_BI_SZ) {
		parent = unitsGet(UNITBI2UNIT(id));
		if(!parent || !unitIsNonNull(parent)) id = -1;
		else {
			model = unitModel(parent);
			if(model != 1 && model != 2 || model == 2 && (BI2CH(id) & 0x03) > 1) id = -1;
		}
	} else {
		parent = unitsGet(XBI2UNIT(id));
		if(!parent || !unitIsNonNull(parent) || unitModel(parent) != 3) id = -1;
	}
	if(id < 0) bi = NULL;
	else	bi = _BisGet(id);
	return bi;
}

void bosInit(void)
{
	BINARY_OUTPUT	*bo;
	int		i, size, dlu;

	dlu = devLocalUnitModel(NULL);
	if(dlu) size = MAX_BO_SZ + MAX_XBO_SZ;
	else	size = MAX_BO_HALF_SZ + MAX_XBO_HALF_SZ;
	for(i = 0, bo = _BinaryOutputs;i < size;i++, bo++) {
		bo->cls			= CLS_BINARY_OUTPUT;
		if(dlu || i < MAX_BO_QUARTER_SZ) bo->id = i;
		else if(i < MAX_BO_HALF_SZ) bo->id = i + MAX_BO_QUARTER_SZ;
		else	bo->id = i + MAX_BO_HALF_SZ;
		bo->cfg			= &sys_cfg->binaryOutputs[i];
		bo->status		= 0;
		//rtcGetDateTime(bi->changeOfStateTime);
		//memcpy(bi->alarmStateTime, bi->changeOfStateTime, 6);
	}
}

void *_BosGet(int id)
{
	if(!devLocalUnitModel(NULL)) {
		if(id >= MAX_BO_HALF_SZ && id < MAX_BO_SZ) id -= MAX_BO_QUARTER_SZ;
		else if(id >= MAX_BO_SZ) id -= MAX_BO_HALF_SZ;
	}
	return &_BinaryOutputs[id];
}

void *bosGet(int id)
{
	void	*bo, *parent;
	int		 dlu, model;

	dlu = devLocalUnitModel(NULL);
	if(!dlu && id < 2) {
		if(!devHaveIo(NULL)) id = -1;
	} else if(id < MAX_BO_HALF_SZ) {
		parent = subdevsGet(SUBDEVBO2CH(id));
		if(!parent || !subdevIsNonNull(parent) || !subdevHaveIo(parent)) id = -1;
	} else if(id < MAX_BO_SZ) {
		parent = unitsGet(UNITBO2UNIT(id));
		if(!parent || !unitIsNonNull(parent)) id = -1;
		else {
			model = unitModel(parent);
			if(model != 1 && (model != 2 || (BO2CH(id) & 0x03) > 1)) id = -1;
		}
	} else {
		parent = unitsGet(XBO2UNIT(id));
		if(!parent || !unitIsNonNull(parent) || unitModel(parent) != 4) id = -1;
	}
	if(id < 0) bo = NULL;
	else	bo = _BosGet(id);
	return bo;
}

void _BiSetDefault(BINARY_INPUT_CFG *cfg)
{
	cfg->type					= 0xff;
	cfg->polarity				= 0;
	cfg->eventDetectionEnable	= 0;
	cfg->timeDelay				= 1;
	cfg->timeDelayNormal		= 1;
	cfg->alarmInhibit			= 1;
	cfg->alarmInhibitScheID		= 0;
	cfg->alarmOC				= 0xff;
}

int _BiEncode(BINARY_INPUT_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	*p++ = cfg->type;
	if(cfg->polarity) val = 0x01; else val = 0x00;
	if(cfg->eventDetectionEnable) val |= 0x02;
	if(cfg->alarmInhibit) val |= 0x04;
	*p++ = val;
	*p++ = cfg->timeDelay;
	*p++ = cfg->timeDelayNormal;
	*p++ = cfg->alarmInhibitScheID;	
	*p++ = cfg->alarmOC;
	return p - (unsigned char *)buf;
}

int _BiDecode(BINARY_INPUT_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	cfg->type				= *p++;
	val = *p++;
	if(val & 0x01) cfg->polarity = 1; else cfg->polarity = 0;
	if(val & 0x02) cfg->eventDetectionEnable = 1; else cfg->eventDetectionEnable = 0;
	if(val & 0x04) cfg->alarmInhibit = 1; else cfg->alarmInhibit = 0;
	cfg->timeDelay			= *p++;
	cfg->timeDelayNormal	= *p++;
	cfg->alarmInhibitScheID	= *p++;
	cfg->alarmOC			= *p++;
	return p - (unsigned char *)buf;
}

int _BiValidate(BINARY_INPUT_CFG *bi_cfg)
{
	return 1;
}

void _BoSetDefault(BINARY_OUTPUT_CFG *cfg)
{
	cfg->type			= 0xff;
	cfg->polarity		= 0;
}

int _BoEncode(BINARY_OUTPUT_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	*p++	= cfg->type;
	if(cfg->polarity) val = 0x01; else val = 0x00;
	*p++	= val;
	return p - (unsigned char *)buf;
}

int _BoDecode(BINARY_OUTPUT_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	cfg->type	= *p++;
	val = *p++;
	if(val & 0x01) cfg->polarity = 1; else cfg->polarity = 0;
	return p - (unsigned char *)buf;
}

int _BoValidate(BINARY_OUTPUT_CFG *cfg)
{
	return 1;
}

int biId(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->id;
}

BOOL biIsNonNull(void *self)
{
	BINARY_INPUT	*bi = self;
	BOOL	bVal;

	if(bi->cfg->type == 0xff) bVal = FALSE;
	else	bVal = TRUE;
	return bVal;
}

// 0:General  1:Door(RequestToExit/DoorSwitch)  2:Elevator  3:IntrusionZone  4..254:Undefined
int biType(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->type;
}

BOOL biIsCreatable(void *self)
{
	BINARY_INPUT	*bi = self;
	BOOL	bVal;

	if(!biIsNonNull(bi) || biType(bi) == BI_TYPE_GENERAL) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL biIsCreatableDoor(void *self)
{
	BINARY_INPUT	*bi = self;
	BOOL	bVal;

	if(!biIsNonNull(bi) || biType(bi) == BI_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL biIsDoorType(void *self)
{
	BINARY_INPUT	*bi = self;
	BOOL	bVal;

	if(biIsNonNull(bi) && biType(bi) == BI_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL biIsRequestToExit(void *self)
{
	BINARY_INPUT	*bi = self;
	int		no;
	BOOL	bVal;

	no = biId(bi) & 0x03;
	if(biIsNonNull(bi) && no == 0 && biType(bi) == BI_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL biIsDoorSwitch(void *self)
{
	BINARY_INPUT	*bi = self;
	int		no;
	BOOL	bVal;

	no = biId(bi) & 0x03;
	if(biIsNonNull(bi) && no == 1 && biType(bi) == BI_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void _BiDelete(int id)
{
	BINARY_INPUT	*bi;

	bi = _BisGet(id);
	bi->cfg->type = 0xff;
}

void biCreate(void *self, int type)
{
	BINARY_INPUT	*bi = self;

	_BiDelete((int)bi->id);
	_BiSetDefault(bi->cfg);
	bi->cfg->type = (unsigned char)type;
}

void biDelete(void *self)
{
	BINARY_INPUT	*bi = self;

	_BiDelete((int)bi->id);
}

void *biAssignedDoor(void *self)
{
	BINARY_INPUT	*bi = self;
	void	*ad;
	int		id, no, type;

	id = biId(bi);
	type = biType(bi);
	no = id & 0x03;
	if((no == 0 || no == 1) && type == BI_TYPE_DOOR) ad = adsGet(BI2CH(id));
	else	ad = NULL;
	return ad;
}

int biPolarity(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->polarity;
}

void biSetPolarity(void *self, int polarity)
{
	BINARY_INPUT	*bi = self;

	bi->cfg->polarity = polarity;
}

int biEventDetectionEnable(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->eventDetectionEnable;
}

void biSetEventDetectionEnable(void *self, int enable)
{
	BINARY_INPUT	*bi = self;

	bi->cfg->eventDetectionEnable = enable;
}

int biTimeDelay(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->timeDelay;
}

void biSetTimeDelay(void *self, int timeDelay)
{
	BINARY_INPUT	*bi = self;

	bi->cfg->timeDelay = timeDelay;
}

int biTimeDelayNormal(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->timeDelayNormal;
}

void biSetTimeDelayNormal(void *self, int timeDelayNormal)
{
	BINARY_INPUT	*bi = self;

	bi->cfg->timeDelayNormal = timeDelayNormal;
}

int biAlarmInhibit(void *self)
{
	BINARY_INPUT	*bi = self;

	return (int)bi->cfg->alarmInhibit;
}

void biSetAlarmInhibit(void *self, int inhibit)
{
	BINARY_INPUT	*bi = self;

	bi->cfg->alarmInhibit = inhibit;
}

extern unsigned char    commStats[];

int biPresentValue(void *self)
{
	BINARY_INPUT	*bi = self;
	int		 id, pv, dluModel;

	id = biId(bi);
	dluModel = devLocalUnitModel(NULL);
	if(!dluModel && id < 4) {
		pv = commStats[2] >> (7-id) & 0x1;
	} else if(id < MAX_BI_HALF_SZ) {
		bi = _BisGet(id);
	} else if(id < MAX_BI_SZ) {
		pv = unitStats[UNITBI2UNIT(id)][1+(id&3)*3] >> (7-(id&3)) & 0x1;
	} else {
		pv = unitStats[XBI2UNIT(id)][(id+23)/24] >> (7-(id&7)) & 0x1;
	}
	return pv;
}

int biRawAlarm(void *self)
{
	BINARY_INPUT	*bi = self;
	int		 id, alarm, dlu;

	id = biId(bi);
	dlu = devLocalUnitModel(NULL);
	if(!dlu && id < 4) {
		alarm = commStats[2] >> (3-id) & 0x1;
	} else if(id < MAX_BI_HALF_SZ) {
		bi = _BisGet(id);
	} else if(id < MAX_BI_SZ) {
		alarm = unitStats[UNITBI2UNIT(id)][1+(id&3)*3] >> (3-(id&3)) & 0x1;
	} else {
		alarm = unitStats[XBI2UNIT(id)][(id+23)/24] >> (3-(id&7)) & 0x1;
	}
	return alarm;
}

int biAlarm(void *self)
{
	BINARY_INPUT	*bi = self;
	int		 id, alarm, dlu;

	id = biId(bi);
	dlu = devLocalUnitModel(NULL);
	if(!dlu && id < 4) {
		alarm = commStats[2] >> (3-id) & 0x1;
	} else if(id < MAX_BI_HALF_SZ) {
		bi = _BisGet(id);
	} else if(id < MAX_BI_SZ) {
		alarm = unitStats[UNITBI2UNIT(id)][1+(id&3)*3] >> (3-(id&3)) & 0x1;
	} else {
		alarm = unitStats[XBI2UNIT(id)][(id+23)/24] >> (3-(id&7)) & 0x1;
	}
	if(!biAlarmInhibit(bi) && alarm) alarm = 1; else alarm = 0;
	return alarm;
}

int boId(void *self)
{
	BINARY_OUTPUT	*bo = self;

	return (int)bo->id;
}

BOOL boIsNonNull(void *self)
{
	BINARY_OUTPUT	*bo = self;
	BOOL	bVal;

	if(bo->cfg->type == 0xff) bVal = FALSE;
	else	bVal = TRUE;
	return bVal;
}

// 0:General  1:DoorLock  2:AlarmAction  3:ElevatorButton  4..254:Undefined
int boType(void *self)
{
	BINARY_OUTPUT	*bo = self;

	return (int)bo->cfg->type;
}

BOOL boIsCreatable(void *self)
{
	BINARY_OUTPUT	*bo = self;
	BOOL	bVal;

	if(!boIsNonNull(bo) || boType(bo) == BO_TYPE_GENERAL) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL boIsCreatableDoor(void *self)
{
	BINARY_OUTPUT	*bo = self;
	BOOL	bVal;

	if(!boIsNonNull(bo) || boType(bo) == BO_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL boIsDoorType(void *self)
{
	BINARY_OUTPUT	*bo = self;
	BOOL	bVal;

	if(boIsNonNull(bo) && boType(bo) == BI_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

BOOL boIsAlarmAction(void *self)
{
	BINARY_OUTPUT	*bo = self;
	int		no;
	BOOL	bVal;

	no = boId(bo) & 0x01;
	if(boIsNonNull(bo) && no == 1 && boType(bo) == BO_TYPE_DOOR) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void _BoDelete(int id)
{
	BINARY_OUTPUT	*bo;

	bo = _BosGet(id);
	bo->cfg->type = 0xff;
}

void boCreate(void *self, int type)
{
	BINARY_OUTPUT	*bo = self;

	_BoDelete((int)bo->id);
	_BoSetDefault(bo->cfg);
	bo->cfg->type = (unsigned char)type;
}

void boDelete(void *self)
{
	BINARY_OUTPUT	*bo = self;

	_BoDelete((int)bo->id);
}

void *boAssignedDoor(void *self)
{
	BINARY_OUTPUT	*bo = self;
	void	*ad;
	int		id, type;

	id = boId(bo);
	type = boType(bo);
	if(type == BO_TYPE_DOOR) ad = adsGet(BO2CH(id));
	else	ad = NULL;
	return ad;
}

int boPolarity(void *self)
{
	BINARY_OUTPUT	*bo = self;

	return (int)bo->cfg->polarity;
}

void boSetPolarity(void *self, int polarity)
{
	BINARY_OUTPUT	*bo = self;

	bo->cfg->polarity = polarity;
}

int boPresentValue(void *self)
{
	BINARY_OUTPUT	*bo = self;
	int		 id, pv, dluModel;

	id = boId(bo);
	dluModel = devLocalUnitModel(NULL);
	if(!dluModel && id < 2) {
		pv = commStats[3] >> (7-(id)) & 0x1;
	} else if(id < MAX_BO_HALF_SZ) {
		bo = _BosGet(id);
	} else if(id < MAX_BO_SZ) {
		pv = unitStats[UNITBO2UNIT(id)][2+(id&1)*3] >> (7-(id&1)) & 0x1;
	} else {
		pv = unitStats[XBO2UNIT(id)][(id+11)/12] >> (7-(id&7)) & 0x1;
	}
	return pv;
}

