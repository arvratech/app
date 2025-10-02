#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "acapcfg.h"


void apSetDefault(ACCESS_POINT_CFG *cfg)
{
	cfg->AuthorizationMode	= 1;
	cfg->AlarmAction		= 0x00;
	cfg->ZoneFrom			= 0xff;
	cfg->ZoneTo				= 0xff;
	cfg->PassbackTimeout	= 0;	// Disable
}

int apEncode(ACCESS_POINT_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = cfg->AuthorizationMode;
	*p++ = cfg->AlarmAction;
	*p++ = cfg->ZoneFrom;
	*p++ = cfg->ZoneTo;
	*p++ = cfg->PassbackTimeout;
	return p - (unsigned char *)buf;
}

int apDecode(ACCESS_POINT_CFG *cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	cfg->AuthorizationMode	= *p++;
	cfg->AlarmAction		= *p++;
	cfg->ZoneFrom			= *p++;
	cfg->ZoneTo				= *p++;
	cfg->PassbackTimeout	= *p++;
	return p - (unsigned char *)buf;
}

int apValidate(ACCESS_POINT_CFG *cfg)
{
	return 1;
}

int apValidateZone(ACCESS_POINT_CFG *cfg)
{
	if(cfg->ZoneFrom == 0xff || cfg->ZoneTo == 0xff || cfg->ZoneFrom == cfg->ZoneTo) return 0;
	else	return 1;	
}

int apGetCodedSize(void)
{
	return 5;
}
