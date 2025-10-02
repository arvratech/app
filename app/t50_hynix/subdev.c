#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "syscfg.h"
#include "dev.h"
#include "unit.h"
#include "bio.h"
#include "subdev.h"


extern SUBDEVICE	_Subdevices[];


void subdevsInit(void)
{
	SUBDEVICE	*subdev;
	NET_SVC		*svc;
	int		i, size, dlu;

	dlu = devLocalUnitModel(NULL);
	if(dlu) size = MAX_CH_SZ;
	else	size = MAX_CH_HALF_SZ - 1;
	for(i = 0, subdev = _Subdevices;i < size;i++, subdev++) {
		subdev->cls		= CLS_SUBDEVICE;
		if(dlu) subdev->id = i;
		else	subdev->id = i + 1;
		subdev->cfg = &sys_cfg->subdevices[i];
//		netsvcInit(&subdev->confSvc, 1, subdev->confBuf);
	}
}

void *_SubdevsGet(int id)
{
	int		idx;

	idx = id;
	if(!devLocalUnitModel(NULL)) idx--;
	return &_Subdevices[idx];
}

void *subdevsGet(int id)
{
	void	*subdev, *unit;

	if(!devLocalUnitModel(NULL) && id == 0) subdev = NULL;
	else {
		unit = unitsGet(CH2UNIT(id));
		if(unit && unitIsNonNull(unit) && unitModel(unit) < 3) subdev = _SubdevsGet(id);
		else	subdev = NULL;
	}
	return subdev;
}

int subdevsMaxSize(void)
{
	int		size;

	if(devLocalUnitModel(NULL)) size = MAX_CH_SZ;
	else	size = MAX_CH_HALF_SZ;
	return size;
}

void _SubdevSetDefault(SUBDEVICE_CFG *cfg)
{
	cfg->model			= 0xff;
	strcpy(cfg->venderName, "");
	strcpy(cfg->modelName, "");
//	memcpy(cfg->FirmwareVersion, bk_version, 3);
	cfg->language		= 1;		// LANG_ENG:English
	cfg->deviceOption	= 0x00;
}

int _SubdevEncode(SUBDEVICE_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->model;
	memcpy_pad(p, cfg->venderName, 16); p += 16;		
	memcpy_pad(p, cfg->modelName, 16); p += 16;
	memcpy(p, cfg->firmwareVersion, 3); p += 3;
	*p++ = cfg->language;
	*p++ = cfg->deviceOption;
 	return p - (unsigned char *)buf;
}

int _SubdevDecode(SUBDEVICE_CFG *cfg, void *buf)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	cfg->model			= *p++;
	memcpy_chop(cfg->venderName, p, 16); p += 16;
	memcpy_chop(cfg->modelName, p, 16); p += 16;
	memcpy(cfg->firmwareVersion, p, 3); p += 3;
	cfg->language		= *p++;
	cfg->deviceOption	= *p++;
 	return p - (unsigned char *)buf;
}

int _SubdevValidate(SUBDEVICE_CFG *cfg)
{
	return 1;
}

int subdevId(void *self)
{
	SUBDEVICE	*subdev = self;

	return (int)subdev->id;
}

BOOL subdevIsNonNull(void *self)
{
	SUBDEVICE	*subdev = self;
	BOOL	bVal;

	if(subdev->cfg->model == 0xff) bVal = FALSE;
	else	bVal = TRUE;
	return bVal;
}

// 0:Wiegand Reader  1:Wiegand+Keypad  2:Cadenatio  3:T35S  4:T35S-NonIo
int subdevModel(void *self)
{
	SUBDEVICE	*subdev = self;
	int		model;
	
	model = subdev->cfg->model;
	return model;
}

BOOL subdevHaveIo(void *self)
{
	SUBDEVICE	*subdev = self;
	int		model;
	BOOL	bVal;

	model = subdevModel(subdev);
	if(model == 0 || model == 1 || model == 4) bVal = FALSE;
	else	bVal = TRUE;
	return bVal;
}

void _CrDelete(int id);
void _BiDelete(int id);
void _BoDelete(int id);

void _SubdevDelete(int id)
{
	SUBDEVICE	*subdev;
	int		i, cid;

	_CrDelete(id);
	cid = CH2SUBDEVBI(id);
	for(i = 0;i < 4;i++) _BiDelete(cid+i);
	cid = CH2SUBDEVBO(id);
	for(i = 0;i < 2;i++) _BoDelete(cid+i);
	subdev = _SubdevsGet(id);
	subdev->cfg->model = 0xff;
}

void subdevCreate(void *self, int model)
{
	SUBDEVICE	*subdev = self;

	_SubdevDelete((int)subdev->id);
	_SubdevSetDefault(subdev->cfg);
	subdev->cfg->model = (unsigned char)model;
}

void subdevDelete(void *self)
{
	SUBDEVICE	*subdev = self;

	_SubdevDelete((int)subdev->id);
}

// 0:Wiegand Reader  1:Wiegand+Keypad  2:Cadenatio  3:T35S  4:T35S-NonIo
int subdevGetAvailableModels(void *self, int *models)
{
	SUBDEVICE	*subdev = self;
	void	*unit;
	int		i, cnt, id, model;
	
	id = subdevId(subdev);
	unit = unitsGet(CH2UNIT(id));
	model = unitModel(unit);
	if(model == 0 || model == 1 && CH2UNITCH(id) > 1) i = 2;
	else	i = 0;
	for(cnt = 0;i < 5;i++, cnt++) models[cnt] = i;
	return cnt;
}

NET_SVC *subdevConfSvc(void *self)
{
	SUBDEVICE	*subdev = self;

	return &subdev->confSvc; 
}

int subdevReliability(void *self)
{
	SUBDEVICE	*subdev = self;

	return (int)subdev->reliability;
}

