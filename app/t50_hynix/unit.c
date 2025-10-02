#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "syscfg.h"
#include "dev.h"
#include "subdev.h"
#include "bio.h"
#include "unit.h"


extern UNIT		_Units[];


void unitsInit(void)
{
	UNIT	*unit;
	int		i, size;

	if(devLocalUnitModel(NULL)) size = MAX_UNIT_SZ;
	else	size = MAX_UNIT_HALF_SZ;
	for(i = 0, unit = _Units;i < size;i++, unit++) {
		unit->cls		= CLS_UNIT;
		unit->id		= i;
		unit->cfg		= &sys_cfg->units[i];
	}
}

void *_UnitsGet(int id)
{
	return &_Units[id];
}

void *unitsGet(int id)
{
	UNIT	*unit;
	int		i, size;

	if(devLocalUnitModel(NULL)) size = MAX_UNIT_SZ;
	else	size = MAX_UNIT_HALF_SZ;
	if(id >= 0 && id < size) unit = &_Units[id];
	else	unit = NULL;
	return unit; 
}

int unitsMaxSize(void)
{
	int		size;

	if(devLocalUnitModel(NULL)) size = MAX_UNIT_SZ;
	else	size = MAX_UNIT_HALF_SZ;
	return size;
}

void _UnitSetDefault(UNIT_CFG *cfg)
{
	cfg->model = 0xff;	
}

int _UnitEncode(UNIT_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->model;
	return p - (unsigned char *)buf;
}

int _UnitDecode(UNIT_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->model = *p++;
	return p - (unsigned char *)buf;
}

int _UnitValidate(UNIT_CFG *cfg)
{
	return 1;
}

int unitId(void *self)
{
	UNIT	*unit = self;

	return (int)unit->id;
}

BOOL unitIsNonNull(void *self)
{
	UNIT	*unit = self;
	BOOL	bVal;

	if(devLocalUnitModel(NULL)) {
		if(unit->id == 0 || unit->cfg->model != 0xff) bVal = TRUE;
		else	bVal = FALSE;
	} else {
		if(unit->cfg->model != 0xff) bVal = TRUE;
		else	bVal = FALSE;
	}
	return bVal;
}

// 0:Virtual  1:K200-4/Local(K100-4)  2:K200-2/Local(K100-2)  3:K500-I/24  4:K700-R/12
int unitModel(void *self)
{
	UNIT	*unit = self;
	int		model;

	model = devLocalUnitModel(NULL);
	if(!model || unit->id > 0) {
		model = unit->cfg->model;
		if(model > 4) model = 0;
	}
	return model;
}

void _SubdevDelete(int id);
void _BiDelete(int id);
void _BoDelete(int id);

void _UnitDelete(int id)
{
	UNIT	*unit;
	int		i, cid;

	if(!devLocalUnitModel(NULL) || id > 0) {
		cid = UNIT2CH(id);
		if(cid) i = 0;
		else	i = 1;
printf("cid=%d i=%d\n", cid, i);
		for( ;i < 4;i++) _SubdevDelete(cid+i);
		cid = UNIT2UNITBI(id);
		for(i = 0;i < 16;i++) _BiDelete(cid+i);
		cid = UNIT2UNITBO(id);
		for(i = 0;i < 8;i++) _BoDelete(cid+i);
		if(id) {
			cid = UNIT2XBI(id);
			for(i = 0;i < 24;i++) _BiDelete(cid+i);
			cid = UNIT2XBO(id);
			for(i = 0;i < 12;i++) _BoDelete(cid+i);
		}
		unit = _UnitsGet(id);
		unit->cfg->model = 0xff;
	}
}

void unitCreate(void *self, int model)
{
	UNIT	*unit = self;

	_UnitDelete((int)unit->id);
	_UnitSetDefault(unit->cfg);
	unit->cfg->model = (unsigned char)model;
}

void unitDelete(void *self)
{
	UNIT	*unit = self;

	_UnitDelete((int)unit->id);
}

int unitReliability(void *self)
{
	UNIT	*unit = self;

	return (int)unit->reliability;
}

int unitGetAvailableModels(void *self, int *models)
{
	UNIT	*unit = self;
	int		i;
	
	if(unit->id == 0) {
		if(devLocalUnitModel(NULL)) i = 0;
		else {
			for(i = 0;i < 3;i++) models[i] = i;
		}
	} else {
		for(i = 0;i < 5;i++) models[i] = i;
	}
	return i;
}

int unitHaveInputsSize(void *self)
{
	UNIT	*unit = self;
	int		size, model;

	model = unitModel(unit);
	if(model == 0 || model == 1) size = 16;
	else if(model == 2) size = 8;
	else if(model == 3) size = 24;
	else	size = 0;
	return size;
}

int unitMaxOutputsSize(void *self)
{
	UNIT	*unit = self;
	int		size, model;

	model = unitModel(unit);
	if(model == 0 || model == 1) size = 8;
	else if(model == 2) size = 4;
	else if(model == 4) size = 12;
	else	size = 0;
	return size;
}

