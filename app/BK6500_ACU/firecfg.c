#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "unitcfg.h"
#include "firecfg.h"


void fzSetDefault(FIRE_ZONE_CFG *cfg, int ID)
{
	int		i;

	if(ID == 0) {
		cfg->Enable = 1;
		for(i = 0;i < 2;i++) cfg->FirePointBits[i]   = 0xff;
		for(i = 0;i < 3;i++) cfg->AccessDoorBits[i]  = 0xff;
		for(i = 0;i < 3;i++) cfg->AlarmActionBits[i] = 0xff;
	} else {
		cfg->Enable = 0;
		for(i = 0;i < 2;i++) cfg->FirePointBits[i]   = 0x00;
		for(i = 0;i < 3;i++) cfg->AccessDoorBits[i]	 = 0x00;
		for(i = 0;i < 3;i++) cfg->AlarmActionBits[i] = 0x00;
	}
}

int fzEncode(FIRE_ZONE_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;
	
	p = (unsigned char *)buf;
	*p++ = cfg->Enable;
	for(i = 0;i < 2;i++) *p++ = cfg->FirePointBits[i];
	for(i = 0;i < 3;i++) *p++ = cfg->AccessDoorBits[i];
	for(i = 0;i < 3;i++) *p++ = cfg->AlarmActionBits[i];
//cprintf("faEncode: %02x-%02x-%02x\n", (int)cfg->UnlockDoors[0], (int)cfg->UnlockDoors[1] , (int)cfg->UnlockDoors[2]);  
	return p - (unsigned char *)buf;
}

int fzDecode(FIRE_ZONE_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->Enable	= *p++;
	for(i = 0;i < 2;i++) cfg->FirePointBits[i]   = *p++;
	for(i = 0;i < 3;i++) cfg->AccessDoorBits[i]  = *p++;
	for(i = 0;i < 3;i++) cfg->AlarmActionBits[i] = *p++;
//LOG(0, "faDecode: %02x-%02x-%02x", (int)cfg->UnlockDoors[0], (int)cfg->UnlockDoors[1] , (int)cfg->UnlockDoors[2]);  
	return p - (unsigned char *)buf;
}

int fzValidate(FIRE_ZONE_CFG *cfg)
{
	return 1;
}

int fzGetCodedSize(void)
{
	return 6;
}
