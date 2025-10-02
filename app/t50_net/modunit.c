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
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "wpanet.h"
#include "appact.h"
#include "mod.h"


static int _ModCodeInUnit(int unitId, unsigned char *buf)
{
	void	*bi;
	unsigned char	*p, val, msk;
	int		i, id;

	p = buf;
	id = UNIT2XBI(unitId);
	val = 0; msk = 0x80;
	for(i = 0;i < 8;i++, msk >>= 1) {
		bi = bisGet(id+i);
		if(biPolarity(bi)) val |= msk; 
	}
	*p++ = val;
	val = 0; msk = 0x80;
	for( ;i < 16;i++, msk >>= 1) {
		bi = bisGet(id+i);
		if(biPolarity(bi)) val |= msk; 
	}
	*p++ = val;
	val = 0; msk = 0x80;
	for( ;i < 24;i++, msk >>= 1) {
		bi = bisGet(id+i);
		if(biPolarity(bi)) val |= msk; 
	}
	*p++ = val;
	for(i = 0;i < 24;i++) {
		bi = bisGet(id+i);
		*p++ = biTimeDelay(bi); *p++ = biTimeDelayNormal(bi);
	}
	return p - buf;
}

static int _ModCodeOutUnit(int unitId, unsigned char *buf)
{
	void	*bo;
	unsigned char	*p, val, msk;
	int		i, id;

	p = buf;
	id = UNIT2XBO(unitId);
	val = 0; msk = 0x80;
	for(i = 0;i < 8;i++, msk >>= 1) {
		bo = bosGet(id+i);
		if(boPolarity(bo)) val |= msk; 
	}
	*p++ = val;
	val = 0; msk = 0x80;
	for( ;i < 12;i++) {
		bo = bosGet(id+i);
		if(boPolarity(bo)) val |= msk; 
	}
	*p++ = val;
	return p - buf;
}

int _ModCodeIoConfig(int ioId, unsigned char *buf);

static int _ModCodeUnit(int unitId, unsigned char *buf)
{
	void	*unit, *subdev;
	unsigned char	*p, val, msk;
	int		i, id, model;
	
	unit = unitsGet(unitId);
	p = buf;
	if(unitIsNonNull(unit)) {
		*p++ = model = unitModel(unit);
		if(model < 3) {
			if(!devLocalUnitModel(NULL) && unitId == 0) i = 1;
			else	i = 0;
			msk = 0x80 >> i;
			val = 0;
			id = UNIT2CH(unitId);
			for( ;i < 4;i++, msk >>= 1) {
				subdev = subdevsGet(id+i);
				if(subdevIsNonNull(subdev) && subdevModel(subdev) > 2) val |= msk;
			}
		}
		*p++ = val;
		if(model == 1 || model == 2) {
			id = UNIT2UNITIO(unitId);
			p += _ModCodeIoConfig(id, p); id++;
			p += _ModCodeIoConfig(id, p); id++;
			if(model == 1) {
				p += _ModCodeIoConfig(id, p); id++;
				p += _ModCodeIoConfig(id, p);
			}
		} else if(model == 3) {
			p += _ModCodeInUnit(unitId, p);
		} else if(model == 4) {
			p += _ModCodeOutUnit(unitId, p);
		}
	} else {
		*p++ = 0xff;
	}
	return p - buf;
}

static int _ModCodeUnitSlave(unsigned char *buf)
{
	unsigned char	*p, val;
	
	p = buf;
	*p++ = 0;		// unitModel=0:Virtual
	if(syscfgNetworkType(NULL)) val = 0x80;
	else	val = 0x00;
	*p++ = val;
	return p - buf;
}

int modUnitStatusSize(int id)
{
	void	*unit;
	int		model, size;

	size = 0;
	unit = unitsGet(id);
	if(unit && unitIsNonNull(unit)) {
		model = unitModel(unit);
		switch(model) {
		case 0:	size =  2; break;	// Virtual
		case 1:	size = 14; break;	// K200-4/Local(K100-4)
		case 2:	size =  8; break;	// K200-2/Local(K100-2)
		case 3:	size =  7; break;	// K500-I/24
		case 4:	size =  9; break;	// K700-R/12
		}
	}
	return size;
}

void _BiUnitStatus(int unitId, unsigned char *buf)
{
	BINARY_INPUT	*bi;
	unsigned char	*p, *s, val, msk;
	int		i, j, id;

	s = buf;
	p = unitStats[unitId];
	id = UNIT2XBI(unitId);
	bi = bisGet(id);
	for(i = 0;i < 3;i++, p++, s++) {
		val = p[0] ^ s[0];
		if(val) {
			p[0] = s[0];
			msk = 0x80;
			for(i = 0;j < 8;i++, bi++) {
				if(val & msk) _BiPvChanged(bi, p[0] & msk);
				msk >>= 1;
			}
		}
	}
	bi = bisGet(id);
	for(i = 0;i < 3;i++, p++, s++) {
		val = p[0] ^ s[0];
		if(val) {
			p[0] = s[0];
			msk = 0x80;
			for(j = 0;j < 8;j++, bi++) {
				if(val & msk) _BiAlarmChanged(bi, p[0] & msk);
				msk >>= 1;
			}
		}
	}
}

void _BoUnitStatus(int unitId, unsigned char *buf)
{
	BINARY_OUTPUT	*bo;
	unsigned char	*p, *s, val, msk;
	int		i, j, id;

	s = buf;
	p = unitStats[unitId];
	id = UNIT2XBO(unitId);
	bo = bosGet(id);
	val = p[0] ^ s[0];
	if(val) {
		p[0] = s[0];
		msk = 0x80;
		for(i = 0;i < 8;i++, bo++) {
			if(val & msk) _BoPvChanged(bo, p[0] & msk);
			msk >>= 1;
		}
	}
	p++; s++;
	val = p[0] ^ s[0];
	if(val) {
		p[0] = s[0];
		msk = 0x80;
		for(i = 0;i < 4;i++, bo++) {
			if(val & msk) _BoPvChanged(bo, p[0] & msk);
			msk >>= 1;
		}
	}
}

void modUnitStatus(int unitId, unsigned char *buf)
{
	void	*unit;
	unsigned char	*p;
	int		i, id, val, model;

	unit = unitsGet(unitId);
	model = unitModel(unit);
	p = buf;
	val = *p++;
	if(val) {
		printf("Unit: %d ss=%d\n", unitId, val);
		return;
	}
	switch(model) {
	case 0:		// Virtual
		val = *p++;
		break;
	case 1:		// K200-4/Local(K100-4)
		val = *p++;
		id = UNIT2UNITIO(unitId);
		for(i = 0;i < 4;i++) {
			modIoStatus(id+i, p);
			p += 3;
		}
		break;
	case 2:		// K200-2/Local(K100-2)
		val = *p++;
		id = UNIT2UNITIO(unitId);
		for(i = 0;i < 2;i++) {
			modIoStatus(id+i, p);
			p += 3;
		}
		break;
	case 3:		// K500-I/24
		_BiUnitStatus(unitId, p);
		break;
	case 4:		// K700-R/12
		_BoUnitStatus(unitId, p);
		break;
	}
}

