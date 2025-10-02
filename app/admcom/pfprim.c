#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"
#include "wpaprim.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "cf.h"
#include "pfprim.h"



char *cfPinEntryIntervalTitle(void)
{
	return xmenu_pin_format[0];
}

char *cfPinEntryStopSentinelTitle(void)
{
	return xmenu_pin_format[1];
}

char *cfPinMaximumLengthTitle(void)
{
	return xmenu_pin_format[2];
}

char *cfPinFacilityCodeTitle(void)
{
	return xmenu_pin_format[3];
}

char *cfPinNumberTitle(void)
{
	return xmenu_pin_format[4];
}

char *cfPinName(void *self)
{
	return xmenu_cred_format[2];
}

// Credential Format PIN Object

char *cfPinOutputWiegandName(void *self)
{
	CF_PIN	*pf = self;
	int		wfId;

	wfId = pf->cfg->outputWiegand;
	return cfWgPinName(cfWgPinsGet(wfId));
}

int cfPinSetOutputWiegandWithKeyValue(void *self, char *buf)
{
	CF_PIN	*pf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
printf("%d [%s] %d\n", (int)pf->cfg->outputWiegand, key, val);
	if(pf->cfg->outputWiegand == val) val = 0;
	else {
		pf->cfg->outputWiegand = val; val = 1;
	}
	return val;
}

void cfPinEncodeOutputWiegandSpinner(void *self, char *title, char *buf)
{
	CF_PIN	*pf = self;
	char	*p;
	char	temp[64];
	int		i, count;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)pf->cfg->outputWiegand); p += strlen(p);
	for(i = 0;i < 4;i++) {
		sprintf(p, "%s,", cfWgPinName(cfWgPinsGet(i))); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 4;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

void cfPinGetEntryIntervalName(void *self, char *name)
{
	CF_PIN	*cf = self;

	sprintf(name, "%d %s", (int)cf->cfg->entryInterval, admSecondUnitTitle());
}

void cfPinGetEntryStopSentinelName(void *self, char *name)
{
	CF_PIN	*cf = self;
	int		val;

	val = cf->cfg->entryStopSentinel;
	if(val) strcpy(name, "#");
	else	strcpy(name, xmenu_enable[1]);
}

void cfPinGetMaximumLengthName(void *self, char *name)
{
	CF_PIN	*cf = self;

	sprintf(name, "%d %s", (int)cf->cfg->maxLength, admDigitUnitTitle());
}

// Credential Format PIN Wiegand Object

char *cfWgPinName(void *self)
{
	CF_WIEGAND_PIN	*cf = self;

	return xmenu_wiegand_pin[cf->id];
}

char *cfWgPinParityName(void *self)
{
	CF_WIEGAND_PIN	*cf = self;
	int		val;

	if(cf->id == 1) val = 0;
	else if(cf->id == 2) val = 1;
	else	val = cf->cfg->parity;
	return xmenu_wiegand_parity[val];
}

char *cfWgPinCodingName(void *self)
{
	CF_WIEGAND_PIN	*cf = self;
	int		val;

	if(cf->id == 2) val = 0; else val = 1;
	return xmenu_field_coding[val];
}

void cfWgPinGetFacilityCodeName(void *self, char *name)
{
	CF_WIEGAND_PIN	*cf = self;

	sprintf(name, "%d", cf->cfg->data[0]);
}

char *cfWgPinFacilityCodeName(void *self)
{
	CF_WIEGAND_PIN	*cf = self;

	sprintf(gText, "%d", cf->cfg->data[0]);
	return gText;
}

void cfWgPinSetFacilityCodeName(void *self, char *name)
{
	CF_WIEGAND_PIN	*cf = self;
	int		val;

	val = n_atoi(name);
	cf->cfg->data[0] = val;
}

void cfWgPinGetPinNumberName(void *self, char *name)
{
	CF_WIEGAND_PIN	*cf = self;
	CF_PIN		*pf;
	int		val;

	switch(cf->id) {
	case 0:	sprintf(name, "4 %s", admBitUnitTitle()); break;
	case 1:	sprintf(name, "8 %s", admBitUnitTitle()); break;
	case 2:	sprintf(name, "16 %s", admBitUnitTitle()); break;
	case 3:
		pf = cfPinGet();		
		val = cfPinMaximumLength(pf);
		sprintf(name, "%d %s(%d %s)", val*4, admBitUnitTitle(), val, admDigitUnitTitle()); break;
	}
}

int cfWgPinSetParityWithKeyValue(void *self, char *buf)
{
	CF_WIEGAND_PIN	*cf = self;
	char	key[64], value[64];
	int		val;

	get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(cf->cfg->parity == val) val = 0;
	else {
		cf->cfg->parity = val; val = 1;
	}
	return val;
}

int cfWgPinSetFacilityCodeWithKeyValue(void *self, char *buf)
{
	CF_WIEGAND_PIN	*cf = self;
	char	*p, key[64], value[64];
	int		val;

	p = get_keyvalue(buf, key, value);
	val = n_atoi(value);
	if(val == cf->cfg->data[0]) val = 0;
	else {
		cf->cfg->data[0] = val; val = 1;
	}
	return val;
}

void cfWgPinEncodeParitySpinner(void *self, char *title, char *buf)
{
	CF_WIEGAND_PIN	*cf = self;
	char	*p;
	int		i;

	p = buf;
	sprintf(p, "%s=%d\n", title, (int)cf->cfg->parity); p += strlen(p);
	for(i = 0;i < 2;i++) {
		sprintf(p, "%s,", cfWiegandParityName(i)); p += strlen(p);
	}
	*(p-1) = '\n';
	for(i = 0;i < 2;i++) {
		sprintf(p, "%d,", i); p += strlen(p);
	}
	*(p-1) = '\n'; *p = 0;
}

