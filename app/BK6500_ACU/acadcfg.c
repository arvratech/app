#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "acadcfg.h"


void adSetDefault(ACCESS_DOOR_CFG *cfg)
{
	cfg->Enable						= 1;
	cfg->RequestToExitEnable 		= 1;
	cfg->DoorStatusEnable   		= 0;
	cfg->AuxLockEnable				= 0;
	cfg->LockType					= 0;  
	cfg->DoorPulseTime				= 5;
	cfg->TimeDelay					= 1;
	cfg->TimeDelayNormal			= 1;
	cfg->UnlockSchedule				= 0xff;
	cfg->LockSchedule				= 0xff;
	cfg->DoorOpenTooLongTime		= 10;
	cfg->DoorOpenTooLongEnable		= 0;
	cfg->DoorForcedOpenEnable		= 0;
}

int adEncode(ACCESS_DOOR_CFG *cfg, void *buf)
{
	unsigned char	*p, val;

	p = (unsigned char *)buf;
	if(cfg->Enable) val = 0x01; else val = 0x00;
	if(cfg->RequestToExitEnable) val |= 0x02;
	if(cfg->DoorStatusEnable) val |= 0x04;
	if(cfg->AuxLockEnable) val |= 0x08;
	*p++ = val;
	if(cfg->LockType) val = 0x01; else val = 0x00;
	*p++ = val;
	*p++ = cfg->DoorPulseTime;
	*p++ = cfg->TimeDelay;
	*p++ = cfg->TimeDelayNormal;
	*p++ = cfg->UnlockSchedule;
	*p++ = cfg->LockSchedule;
	*p++ = cfg->DoorOpenTooLongTime;
	val = 0x00;
	if(cfg->DoorOpenTooLongEnable) val |= 0x01;
	if(cfg->DoorForcedOpenEnable)  val |= 0x04;
	*p++ = val;
 	return p - (unsigned char *)buf;
}

int adDecode(ACCESS_DOOR_CFG *cfg, void *buf)
{
	unsigned char	*p, val;
	
	p = (unsigned char *)buf;
	val = *p++;
	cfg->Enable						= val & 0x01;
	cfg->RequestToExitEnable		= (val >> 1) & 0x01;
	cfg->DoorStatusEnable			= (val >> 2) & 0x01;
	cfg->AuxLockEnable				= (val >> 3) & 0x01;
	val = *p++;
	if(val & 0x01) cfg->LockType = 1; else cfg->LockType = 0;
	cfg->DoorPulseTime				= *p++;
	cfg->TimeDelay					= *p++;
	cfg->TimeDelayNormal			= *p++;
	cfg->UnlockSchedule				= *p++;
	cfg->LockSchedule				= *p++;
	cfg->DoorOpenTooLongTime		= *p++;
	val = *p++;
	if(val & 0x01) cfg->DoorOpenTooLongEnable = 1; else cfg->DoorOpenTooLongEnable = 0;
	if(val & 0x04) cfg->DoorForcedOpenEnable  = 1; else cfg->DoorForcedOpenEnable  = 0;
 	return p - (unsigned char *)buf;
}

int adDecode2(ACCESS_DOOR_CFG *cfg, void *buf)
{
	unsigned char	*p, val;
	
	p = (unsigned char *)buf;
	val = *p++;
	cfg->Enable						= val & 0x01;
	cfg->RequestToExitEnable		= (val >> 1) & 0x01;
	cfg->DoorStatusEnable			= (val >> 2) & 0x01;
	cfg->AuxLockEnable				= (val >> 3) & 0x01;
	val = *p++;
	if(val & 0x01) cfg->LockType = 1; else cfg->LockType = 0;
	cfg->DoorPulseTime				= *p++;
	cfg->TimeDelay					= *p++;
	cfg->TimeDelayNormal			= *p++;
 	return p - (unsigned char *)buf;
}

int adValidate(ACCESS_DOOR_CFG *afg)
{
	return 1;
}

int adGetCodedSize(void)
{
	return 15;
}

