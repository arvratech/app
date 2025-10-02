#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "unitcfg.h"
#include "biocfg.h"


void biSetDefault(BINARY_INPUT_CFG *bi_cfg, int biID)
{
	int		ID;

	bi_cfg->Polarity		= 0;
	bi_cfg->TimeDelay		= 1;
	bi_cfg->TimeDelayNormal	= 1;
	ID = biGetUnit(biID);
	if(ID >= 0 && (biID & 0x07) == 3) bi_cfg->AlarmInhibit = 0;
	else	bi_cfg->AlarmInhibit = 1;
	bi_cfg->AlarmInhibitScheID = 0;
}

int biEncode(BINARY_INPUT_CFG *bi_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	if(bi_cfg->Polarity) val = 0x01; else val = 0x00;
	if(bi_cfg->AlarmInhibit) val |= 0x02;
	*p++ = val;
	*p++ = bi_cfg->TimeDelay;
	*p++ = bi_cfg->AlarmInhibitScheID;	
	return p - (unsigned char *)buf;
}

int biDecode(BINARY_INPUT_CFG *bi_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	val = *p++;
	if(val & 0x01) bi_cfg->Polarity = 1; else bi_cfg->Polarity = 0;
 	if(val & 0x02) bi_cfg->AlarmInhibit = 1; else bi_cfg->AlarmInhibit = 0;
 	bi_cfg->TimeDelay 			= *p++;
	bi_cfg->AlarmInhibitScheID	= *p++;
 	return p - (unsigned char *)buf;
}

int biValidate(BINARY_INPUT_CFG *bi_cfg)
{
	return 1;
}

int biGetCodedSize(void)
{
	return 3;
}

void boSetDefault(BINARY_OUTPUT_CFG *bo_cfg)
{
	bo_cfg->Polarity	= 0;
}

int boEncode(BINARY_OUTPUT_CFG *bo_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	if(bo_cfg->Polarity) *p = 0x01; else *p = 0x00;
	p++;
	return p - (unsigned char *)buf;
}

int boDecode(BINARY_OUTPUT_CFG *bo_cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	val = *p++;
	if(val & 0x01) bo_cfg->Polarity = 1; else bo_cfg->Polarity = 0;
 	return p - (unsigned char *)buf;
}

int boValidate(BINARY_OUTPUT_CFG *bo_cfg)
{
	return 1;
}

int boGetCodedSize(void)
{
	return 1;
}
