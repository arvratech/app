#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "hw.h"
#include "hwprim.h"
#include "cf.h"
#include "cfprim.h"
#include "pfprim.h"
#include "slvmlib.h"
#include "slvm.h"
#include "dlg.h"
#include "pref.h"
#include "psmem.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actalert.h"
#include "pref.h"
#include "appact.h"


#include "adm_hwnwp.c"
#include "adm_hwcr.c"
#include "adm_hwsubdev.c"
#include "adm_hwbi.c"
#include "adm_hwbo.c"
#include "adm_hwad.c"
#include "adm_hwch.c"
#include "adm_hwunit.c"
#include "adm_hwdev.c"


void AdmHwUnitSubdevListStart(void *pss);
void AdmHwAdListStart(void *pss);
void AdmHwCrListStart(void *pss);
void AdmHwBiListStart(void *pss);
void AdmHwBoListStart(void *pss);


void AdmHwStart(void *pss)
{
	void	*ps, *unit;
	int		i, size;

	size = unitsMaxSize();
	for(i = 0;i < size;i++) {
		unit = unitsGet(i);
		if(unit && unitIsNonNull(unit)) {
			ps = PSaddScreen(pss, unitAddress(unit), AdmHwUnitSubdevListStart);
			PSsetContext(ps, unit);
			PSsetButton(ps, 1, AdmHwUnitStart);
		}
	}
	ps = PSaddScreen(pss, xmenu_hardware[0], AdmHwAdListStart);
	ps = PSaddScreen(pss, xmenu_hardware[1], AdmHwCrListStart);
	ps = PSaddScreen(pss, xmenu_hardware[2], AdmHwBiListStart);
	ps = PSaddScreen(pss, xmenu_hardware[3], AdmHwBoListStart);
}

void AdmHwUnitSubdevListStart(void *pss)
{
	void	*ps, *unit, *subdev;
	int		i, id;

	unit = PScontext(pss);
	id = UNIT2CH(unitId(unit));
	for(i = 0;i < 4;i++) {
		subdev = subdevsGet(id+i);
		if(subdev && subdevIsNonNull(subdev)) {
			ps = PSaddScreen(pss, subdevAddress(subdev), AdmHwSubdevStart);
			PSsetContext(ps, subdev);
		}
	}
}

void _AdmHwAdListReload(void *pss)
{
	void	*ad;
	int		i, size;

	size = adsMaxSize();
	for(i = 0;i < size;i++) {
		ad = adsGet(i);
		if(ad && adIsCreatable(ad)) _AdmAddAdSwitch(pss, i);
	}
}

void _AdmHwAdListPrefChanged(void *pss, int index, void *act)
{
	void	*ad;
	int		i, count, size, resultCode;

	resultCode = PSresultCode(pss);
printf("HwAdListPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode >= RESULT_OK) {
		size = adsMaxSize();
		for(i = count = 0;i < size;i++) {
			ad = adsGet(i);
			if(ad && adIsCreatable(ad)) count++;
		}
		if(PSchildrenCount(pss) != count) {
			PSremoveChildren(pss);
			_AdmHwAdListReload(pss);
			PrefReloadData(act);
		}
	}
}

void AdmHwAdListStart(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmHwAdListPrefChanged);
	_AdmHwAdListReload(pss);
}

void AdmHwCrListStart(void *pss)
{
	void	*cr;
	int		i, size;

	size = crsMaxSize();
	for(i = 0;i < size;i++) {
		cr = crsGet(i);
		if(cr && crIsCreatable(cr)) _AdmAddCrSwitch(pss, i);
	}
}

void AdmHwUnitBiListStart(void *pss);

void AdmHwBiListStart(void *pss)
{
	void	*ps, *unit, *bi;
	int		i, size, model;

	if(!devLocalUnitModel(NULL) && devHaveIo(NULL)) {
		for(i = 0;i < 4;i++) {
			bi = bisGet(i);
			if(bi && biIsCreatable(bi)) _AdmAddBiSwitch(pss, i);
		}
	}
}

void AdmHwUnitBoListStart(void *pss);

void AdmHwBoListStart(void *pss)
{
	void	*ps, *unit, *bo;
	int		i, size, model;

	if(!devLocalUnitModel(NULL) && devHaveIo(NULL)) {
		for(i = 0;i < 2;i++) {
			bo = bosGet(i);
			if(bo && boIsCreatable(bo)) _AdmAddBoSwitch(pss, i);
		}
	}
}

void AdmHwUnitBiListStart(void *pss)
{
	void	*ps, *unit, *bi;
	int		i, uid, id, size, model;

	unit = PScontext(pss);
	uid = unitId(unit);
	model = unitModel(unit);
	if(model < 3) {
		if(!devLocalUnitModel(NULL) && uid == 0) i = 4;
		else	i = 0;
		id = UNIT2SUBDEVBI(uid);
		for( ;i < 16;i++) {
			bi = bisGet(id+i);
			if(bi && biIsCreatable(bi)) _AdmAddBiSwitch(pss, id+i);
		}
	}
	if(model < 3) {
		id = UNIT2UNITBI(uid);
		size = 16;
	} else {
		id = UNIT2XBI(uid);
		size = 24;
	}
	for(i = 0;i < size;i++) {
		bi = bisGet(id+i);
		if(bi && biIsCreatable(bi)) _AdmAddBiSwitch(pss, id+i);
	}
}

void AdmHwUnitBoListStart(void *pss)
{
	void	*ps, *unit, *bo;
	int		i, uid, id, size, model;

	unit = PScontext(pss);
	uid = unitId(unit);
	model = unitModel(unit);
	if(model < 3) {
		if(!devLocalUnitModel(NULL) && uid == 0) i = 2;
		else	i = 0;
		id = UNIT2SUBDEVBO(uid);
		for( ;i < 8;i++) {
			bo = bosGet(id+i);
			if(bo && boIsCreatable(bo)) _AdmAddBoSwitch(pss, id+i);
		}
	}
	if(model < 3) {
		id = UNIT2UNITBO(uid);
		size = 8;
	} else {
		id = UNIT2XBO(uid);
		size = 12;
	}
	for(i = 0;i < size;i++) {
		bo = bosGet(id+i);
		if(bo && boIsCreatable(bo)) _AdmAddBoSwitch(pss, id+i);
	}
}

