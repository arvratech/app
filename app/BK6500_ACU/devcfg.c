#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "devcfg.h"

static char *bk_version	= "BK6500_v3.9.4";
static char *bk_model	= "BK6500";
static char *bk_maker	= "Arvratech";


void devSetDefault(DEVICE_CFG *dev_cfg)
{
	strcpy(dev_cfg->FirmwareVersion, bk_version);
	strcpy(dev_cfg->Maker, bk_maker);
	strcpy(dev_cfg->Model, bk_model);
	dev_cfg->Language			= 1;		// LANG_ENG:English
	dev_cfg->DeviceOption		= 0x00;		// TAMPER disable
	dev_cfg->DateNotation		= 0;
	dev_cfg->TimeNotation		= 0;
	dev_cfg->EventOption		= 0x00;
	dev_cfg->EventOutput		= 0;
	dev_cfg->EventWiegand		= 1;
	dev_cfg->BackLightTime		= 10;
	dev_cfg->LCDContrast		= 32;
	dev_cfg->LCDBrightness		= 12;
	dev_cfg->UserSoundVolume	= 10;
	dev_cfg->AdminSoundVolume	= 8;
	dev_cfg->KeySoundVolume		= 6;
	memset(dev_cfg->Reserve, 0, 4);
}

int devEncode(DEVICE_CFG *dev_cfg, void *buf, int ReadOnly)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	if(ReadOnly) {
		memset(p, 0xff, 40); p += 40;
	} else {
		memcpy_pad(p, dev_cfg->FirmwareVersion, 16); p += 16;
		memcpy_pad(p, dev_cfg->Maker, 16); p += 16;		
		memcpy_pad(p, dev_cfg->Model,  8); p += 8;
	}
	*p++ = dev_cfg->Language;
	*p++ = dev_cfg->DeviceOption;
	*p++ = dev_cfg->DateNotation;
	*p++ = dev_cfg->TimeNotation;
	*p++ = dev_cfg->EventOption;
	*p++ = dev_cfg->EventOutput;
	*p++ = dev_cfg->EventWiegand;
	*p++ = dev_cfg->BackLightTime;
	*p++ = dev_cfg->LCDContrast;
	*p++ = dev_cfg->LCDBrightness;
	*p++ = dev_cfg->UserSoundVolume;
	*p++ = dev_cfg->AdminSoundVolume;
	*p++ = dev_cfg->KeySoundVolume;
	memcpy(p, dev_cfg->Reserve, 4); p += 4;
 	return p - (unsigned char *)buf;
}

int devDecode(DEVICE_CFG *dev_cfg, void *buf, int ReadOnly)
{
	unsigned char	*p;
	
	p = (unsigned char *)buf;
	if(ReadOnly) p += 40;
	else {
		memcpy_chop(dev_cfg->FirmwareVersion, p, 16); p += 16;
		memcpy_chop(dev_cfg->Maker, p, 16); p += 16;
		memcpy_chop(dev_cfg->Model, p,  8); p += 8;
	}
	dev_cfg->Language			= *p++;
	dev_cfg->DeviceOption		= *p++;
	dev_cfg->DateNotation		= *p++;
	dev_cfg->TimeNotation		= *p++;
	dev_cfg->EventOption		= *p++;
	dev_cfg->EventOutput		= *p++;
	dev_cfg->EventWiegand		= *p++;
	dev_cfg->BackLightTime		= *p++;
	dev_cfg->LCDContrast		= *p++;
	dev_cfg->LCDBrightness		= *p++;
	dev_cfg->UserSoundVolume	= *p++;
	dev_cfg->AdminSoundVolume	= *p++;
	dev_cfg->KeySoundVolume		= *p++;
	memcpy(dev_cfg->Reserve, p, 4); p += 4;
 	return p - (unsigned char *)buf;
}

int devValidate(DEVICE_CFG *dev_cfg)
{
//	if((dev_cfg->DeviceOption & 0xe0) || !(dev_cfg->AccessModule & 0x07) || dev_cfg->AccessModule & 0xf8) return 0;
//	if(dev_cfg->FPSecurityLevel < 1 || dev_cfg->FPSecurityLevel > 5) return 0;
//	if(dev_cfg->UnlockTime < 1 || !scheValidateID((int)dev_cfg->UnlockScheID) || !scheValidateID((int)dev_cfg->LockScheID)) return 0;
	return 1;
}

char *syscfgGetFirmwareVersion(void)
{
	return bk_version;
}
