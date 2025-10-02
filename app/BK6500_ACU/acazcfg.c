#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "unitcfg.h"
#include "acazcfg.h"


int azValidateID(long nID)
{
	if(nID >= 0 && nID < MAX_AZ_SZ) return 1;
	else	return 0;
}

void azSetDefault(ACCESS_ZONE_CFG *cfg)
{
	cfg->Enable					= 0;
	cfg->PassbackMode			= 0;
	cfg->PassbackOption			= 0;
	cfg->HardPassbackSchedule	= 0xff;
	cfg->SoftPassbackSchedule	= 0xff;
	cfg->PassbackTimeout		= 0;	// Unlimited
}

int azEncode(ACCESS_ZONE_CFG *cfg, void *buf)
{
	unsigned char	*p, c;

	p = (unsigned char *)buf;
	if(cfg->Enable) c = 0x01; else c = 0x00;
	c |= (cfg->PassbackMode & 0x03) << 1;
	c |= cfg->PassbackOption << 3;
	*p++ = c;
	*p++ = cfg->HardPassbackSchedule;
	*p++ = cfg->SoftPassbackSchedule;
	*p++ = cfg->PassbackTimeout;
	return p - (unsigned char *)buf;
}

int azDecode(ACCESS_ZONE_CFG *cfg, void *buf)
{
	unsigned char	*p, c;

	p = (unsigned char *)buf;
	c = *p++;
	if(c & 0x01) cfg->Enable = 1; else cfg->Enable = 0;
  	cfg->PassbackMode			= (c >> 1) & 0x03;
 	cfg->PassbackOption			= c >> 3;
	cfg->HardPassbackSchedule	= *p++;
	cfg->SoftPassbackSchedule	= *p++;
	cfg->PassbackTimeout		= *p++;
	return p - (unsigned char *)buf;
}

int azValidate(ACCESS_ZONE_CFG *cfg)
{
	return 1;
}

#ifndef WIN32

int azGetCodedSize(void)
{
	return 4;
}

#endif