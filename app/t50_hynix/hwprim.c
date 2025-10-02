#include <stdio.h>
#include <string.h>
#include "syscfg.h"
#include "msg.h"
#include "hw.h"
#include "hwprim.h"


char *devAddress(void *self)
{
	char	*p;
	
	p = gText;
	sprintf(p, "Dev-%d", (int)sys_cfg->devId);
	return gText;
}

char *unitAddress(void *self)
{
	UNIT	*unit = self;
	char	*p;
	
	p = gText;
	strcpy(p, "Unit"); p += strlen(p);
	*p++ = unit->id + '0';
	*p = 0;
	return gText;
}

char *unitModelName(void *self)
{
	UNIT	*unit = self;
	char	*p;
	int		model;

	p = gText;
	if(unitIsNonNull(unit)) {
		model = unitModel(unit);
		if(devLocalUnitModel(NULL) && unit->id == 0) model = 5;
		strcpy(p, xmenu_unit_model[model]);
	} else {
		p[0] = ' '; p[1] = 0;
	}
	return gText;
}

char *chAddress(int id)
{
	void	*unit;
	char	*p;

	p = gText;
	unit = unitsGet(CH2UNIT(id));
	strcpy(p, unitAddress(unit)); p += strlen(p);
	sprintf(p, ".%d", (id & 0x03) + 1);
	return gText;
}

char *crAddress(void *self)
{
	CREDENTIAL_READER	*cr = self;
	void	*unit;
	char	*p, temp[64];;
	
	p = temp;
	if(devLocalUnitModel(NULL) || cr->id) {
		unit = unitsGet(CH2UNIT(cr->id));
		strcpy(p, unitAddress(unit)); p += strlen(p); *p++ = '.';
	}
	strcpy(p, crAddressName(cr));
	strcpy(gText, temp);
	return gText;
}

char *crAddressName(void *self)
{
	CREDENTIAL_READER	*cr = self;
	char	*p;

	p = gText;
	if(devLocalUnitModel(NULL) || cr->id) {
		*p++ = (cr->id & 3) + '1'; *p++ = '-';
	}
	strcpy(p, "Reader"); 
	return gText;
}

char *crTypeName(void *self)
{
	CREDENTIAL_READER	*cr = self;
	char	*p;
	int		type;

	p = gText;
	type = crType(cr);
	if(type >= 0) {
printf("crTypeName: type=%d\n", type);
		strcpy(p, xmenu_reader_type[type]);
	} else {
		p[0] = ' '; p[1] = 0;
	}
	return gText;
}

char *subdevAddress(void *self)
{
	SUBDEVICE	*subdev = self;
	void	*unit;
	char	*p, temp[64];

	p = temp;
	unit = unitsGet(CH2UNIT(subdev->id));
	strcpy(p, unitAddress(unit)); p += strlen(p);
	*p++ = '.';
	strcpy(p, subdevAddressName(subdev));
	strcpy(gText, temp);
	return gText;
}

char *subdevAddressName(void *self)
{
	SUBDEVICE	*subdev = self;
	char	*p;

	p = gText;
	*p++ = (subdev->id & 3) + '1';
	strcpy(p, "-Device"); 
	return gText;
}

char *subdevModelName(void *self)
{
	SUBDEVICE	*subdev = self;
	char	*p;
	int		model;

	p = gText;
	if(subdevIsNonNull(subdev)) {
		model = subdevModel(subdev);
		strcpy(p, xmenu_dev_model[model+2]);
	} else {
		p[0] = ' '; p[1] = 0;
	}
	return gText;
}

char *biAddress(void *self)
{
	BINARY_INPUT	*bi = self;
	void	*subdev, *unit;
	char	*p, temp[64];;
	int		id;
	
	id = bi->id;
	p = temp;
	if(devLocalUnitModel(NULL) || bi->id > 3) {
		if(id < MAX_BI_SZ) {
			if(id < MAX_BI_HALF_SZ) {
				subdev = subdevsGet(SUBDEVBI2CH(id));
				strcpy(p, subdevAddress(subdev));
			} else {
				unit = unitsGet(UNITBI2UNIT(id));
				strcpy(p, unitAddress(unit));
			}
		} else {
			unit = unitsGet(XBI2UNIT(id));
			strcpy(p, unitAddress(unit)); 
		}
		p += strlen(p); *p++ = '.';
	}
	strcpy(p, biAddressName(bi));
	strcpy(gText, temp);
	return gText;
}

char *biAddressName(void *self)
{
	BINARY_INPUT	*bi = self;
	char	*p;
	int		id, n;
	
	id = bi->id;
	p = gText;
	if(id < MAX_BI_SZ) {
		if(id >= MAX_BI_HALF_SZ) {
			*p++ = (UNITBI2CH(id) & 0x03) + '1'; *p++ = '-';
		}
		n = id & 0x03;
		switch(n) {
		case 0:	strcpy(p, "exit"); break;
		case 1:	strcpy(p, "in1"); break;
		case 2:	strcpy(p, "in2"); break;
		case 3: strcpy(p, "fire"); break;
		}
	} else {
		strcpy(p, "input"); p += strlen(p);
		n = ((id-MAX_BI_SZ) % 24) + 1;
		if(n < 10) *p++ = n + '0';
		else {
			*p++ = n / 10 + '0'; *p++ = n % 10 + '0';
		}
		*p = 0;
	}
	return gText;
}

char *biTypeName(void *self)
{
	BINARY_INPUT	*bi = self;
	char	*p;
	int		type;

	p = gText;
	if(biIsNonNull(bi)) {
		type = biType(bi);
		if(type > 1 || type == 1 && (bi->id & 0x03) == 1) type++;
		strcpy(p, xmenu_input_type[type]);
	} else {
		p[0] = ' '; p[1] = 0;
	}
	return gText;
}

char *boAddress(void *self)
{
	BINARY_OUTPUT	*bo = self;
	void	*subdev, *unit;
	char	*p, temp[64];;
	int		id;
	
	id = bo->id;
	p = temp;
	if(devLocalUnitModel(NULL) || bo->id > 1) {
		if(id < MAX_BO_SZ) {
			if(id < MAX_BO_HALF_SZ) {
				subdev = subdevsGet(SUBDEVBO2CH(id));
				strcpy(p, subdevAddress(subdev));
			} else {
				unit = unitsGet(UNITBO2UNIT(id));
				strcpy(p, unitAddress(unit));
			}
		} else {
			unit = unitsGet(XBO2UNIT(id));
			strcpy(p, unitAddress(unit));
		}
		p += strlen(p); *p++ = '.';
	}
	strcpy(p, boAddressName(bo));
	strcpy(gText, temp);
	return gText;
}

char *boAddressName(void *self)
{
	BINARY_OUTPUT	*bo = self;
	char	*p;
	int		id, n;
	
	id = bo->id;
	p = gText;
	if(id < MAX_BO_SZ) {
		if(id >= MAX_BO_HALF_SZ) {
			*p++ = (UNITBO2CH(id) & 0x03) + '1'; *p++ = '-';
		}
		n = id & 0x01;
		switch(n) {
		case 0:	strcpy(p, "lock"); break;
		case 1:	strcpy(p, "relay"); break;
		}
	} else {
		strcpy(p, "relay"); p += strlen(p);
		n = ((id-MAX_BO_SZ) % 12) + 1;
		if(n < 10) *p++ = n + '0';
		else {
			*p++ = n / 10 + '0'; *p++ = n % 10 + '0';
		}
		*p = 0;
	}
	return gText;
}

char *boTypeName(void *self)
{
	BINARY_OUTPUT	*bo = self;
	char	*p;
	int		type;

	p = gText;
	if(boIsNonNull(bo)) {
		type = boType(bo);
		if(type > 1 || type == 1 && (bo->id & 0x01) == 1) type++;
		strcpy(p, xmenu_output_type[type]);
	} else {
		p[0] = ' '; p[1] = 0;
	}
	return gText;
}

char *adAddress(void *self)
{
	ACCESS_DOOR	*ad = self;
	void	*unit;
	char	*p, temp[64];

	p = temp;
	if(devLocalUnitModel(NULL) || ad->id) {
		unit = unitsGet(CH2UNIT(ad->id));
		strcpy(p, unitAddress(unit)); p += strlen(p); *p++ = '.';
	}
	strcpy(p, adAddressName(ad));
	strcpy(gText, temp);
	return gText;
}

char *adAddressName(void *self)
{
	ACCESS_DOOR	*ad = self;
	char	*p;

	p = gText;
	if(devLocalUnitModel(NULL) || ad->id) {
		*p++ = (ad->id & 3) + '1'; *p++ = '-';
	}
	strcpy(p, "Door");
	return gText;
}

