#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "msg.h"
#include "syscfg.h"
#include "unit.h"
#include "sche.h"
#include "evt.h"
#include "acap.h"
#include "acaz.h"


#include "acazcfg.c"


int azExist(long nID)
{
	int		enable;

	if(nID == 0) enable = 1;
	else if(nID > 0 && nID < MAX_AZ_SZ) enable = sys_cfg->AccessZones[nID-1].Enable;
	else	enable = 0;
	return enable;
}

int azGetEncoded(long nID, unsigned char *buf)
{
	ACCESS_ZONE_CFG	*az_cfg;
	int		rval;
	
	az_cfg = &sys_cfg->AccessZones[nID-1];;
	if(!az_cfg->Enable) rval = 0;
	else	rval = azEncode(az_cfg, buf);
	return rval;
}

int azAdd(long nID)
{
	ACCESS_ZONE_CFG	*az_cfg, _tmp_cfg;
	unsigned char	buf[32];
	int		rval, size;

	az_cfg = &sys_cfg->AccessZones[nID-1];;
	azSetDefault(az_cfg);
	az_cfg->Enable = 1;
	az_cfg = &sys_cfg->AccessZones[nID-1];;
	size = azEncode(az_cfg, buf);
	memcpy(&_tmp_cfg, az_cfg, sizeof(ACCESS_ZONE_CFG));
	rval = syscfgWrite(sys_cfg);
	if(!rval) rval = 1;
	else	memcpy(az_cfg, &_tmp_cfg, sizeof(ACCESS_ZONE_CFG)); 
	return rval;
}	

int azAddEncoded(long nID, unsigned char *buf)
{
	ACCESS_ZONE_CFG	*az_cfg, _tmp_cfg;
	unsigned char	tmpbuf[32];
	int		rval, size;

	az_cfg = &sys_cfg->AccessZones[nID-1];;
	size = azEncode(az_cfg, tmpbuf);
	buf[0] |= 0x01;		// Enable=1;
	if(!n_memcmp(buf, tmpbuf, size)) rval = 1;
	else {
		memcpy(&_tmp_cfg, az_cfg, sizeof(ACCESS_ZONE_CFG));
		azDecode(az_cfg, buf);
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(az_cfg, &_tmp_cfg, sizeof(ACCESS_ZONE_CFG)); 
	}
	return rval;
}

int azRemove(long nID)
{
	ACCESS_ZONE_CFG	*az_cfg, _tmp_cfg;
	int		rval;

	az_cfg = &sys_cfg->AccessZones[nID-1];
	if(!az_cfg->Enable) rval = 0;
	else {
		memcpy(&_tmp_cfg, az_cfg, sizeof(ACCESS_ZONE_CFG));
		azSetDefault(az_cfg);
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(az_cfg, &_tmp_cfg, sizeof(ACCESS_ZONE_CFG)); 
	}
	return rval;
}

int azRemoveAll(void)
{
	ACCESS_ZONE_CFG	_tmp_cfg[MAX_AZ_SZ-1];
	int		i, rval, size;

	size = sizeof(ACCESS_ZONE_CFG) * (MAX_AZ_SZ-1);
	memcpy(_tmp_cfg, sys_cfg->AccessZones, size);
	for(i = 1;i < MAX_AZ_SZ;i++)
		azSetDefault(&sys_cfg->AccessZones[i-1]);
	if(!n_memcmp(sys_cfg->AccessZones, _tmp_cfg, size)) rval = 1;
	else {
		rval = syscfgWrite(sys_cfg);
		if(!rval) rval = 1;
		else	memcpy(sys_cfg->AccessZones, _tmp_cfg, size);
	}
	return rval;
}
