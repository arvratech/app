#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "..\BootLoader\flash.h"
#include "..\BootMonitor\fs.h"
#include "iodev.h"
#include "msg.h"
#include "rtc.h"
#include "sin.h"
#include "sche.h"
#include "lang.h"
#include "sysdep.h"
#include "syscfg.h"
#include "sclib.h"
#include "ta.h"

SYS_CFG	*sys_cfg, _sys_cfg;
static unsigned char	MACAddr[8];
static FS_FILE	*fs[1];


static FS_FILE *_OpenSIB(int file)
{
	FS_FILE		*f;

	if(!fsExist(file)) {
		f = fsCreate(file);
		if(!f) {
#ifdef BK_DEBUG
			cprintf("fsCreate(%d) error\n", file);
#endif
			return (FS_FILE *)0;
		}
		fsClose(f);
	}
	f = fsOpen(file, FS_WRITE);	
	if(!f) {
#ifdef BK_DEBUG
		cprintf("fsOpen(%d) error\n", file);
#endif
		return (FS_FILE *)0;
	}
#ifdef BK_DEBUG
	cprintf("SIB(file=%d) is opened: size=%ld Mode=%d\n", file, f->position, (int)f->mode);
#endif
	return f;
}

int syscfgInitialize(void)
{	
	sys_cfg = &_sys_cfg;
	flashWriteEnable();
	fs[0] = _OpenSIB(0);
	flashWriteDisable();
	GetMACAddress(MACAddr);
	syscfgSetDefault(sys_cfg);
//syscfgReset(sys_cfg);
	if(!fs[0]) return -1;
	syscfgRead(sys_cfg);
	syscfgSetUnits(sys_cfg);
	return 0;
}

void syscfgReset(SYS_CFG *sys_cfg)
{
	flashWriteEnable();
	fsSeek(fs[0], 0L, SEEK_SET);
	fsShift(fs[0], NULL, -1);
	flashWriteDisable();
	syscfgSetDefault(sys_cfg);
}

void syscfgSetDefaultByLanguage(SYS_CFG *sys_cfg);

extern unsigned char Default_IPAddress[];
extern unsigned char Default_SubnetMask[];


void syscfgSetDefault(SYS_CFG *sys_cfg)
{
	CR_CFG	*cr_cfg;
	int		i;
	
	memset(sys_cfg, 0, sizeof(SYS_CFG));
	sys_cfg->DeviceID			= 1;
	strcpy(sys_cfg->AdminPIN, "1111");
	devSetDefault(&sys_cfg->Device);
	nwpSetDefault(&sys_cfg->NetworkPort);
	for(i = 0;i < MAX_UNIT_SZ;i++)	 unitSetDefault(&sys_cfg->Units[i], i);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) iounitSetDefault(&sys_cfg->IOUnits[i], i);
	for(i = 0;i < MAX_AD_SZ;i++)	 adSetDefault(&sys_cfg->AccessDoors[i]);
	for(i = 0;i < MAX_AP_SZ;i++)	 apSetDefault(&sys_cfg->AccessPoints[i]);
	for(i = 0;i < MAX_AZ_SZ-1;i++)	 azSetDefault(&sys_cfg->AccessZones[i]);
	for(i = 0;i < MAX_BI_SZ+MAX_XBI_SZ;i++) biSetDefault(&sys_cfg->BinaryInputs[i], i);
	for(i = 0;i < MAX_BO_SZ+MAX_XBO_SZ;i++) boSetDefault(&sys_cfg->BinaryOutputs[i]);
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = &sys_cfg->CredentialReaders[i];
		cr_cfg->Version	= 1;
		crSetDefaultAll(cr_cfg);
	}
//	for(i = 0;i < MAX_CR_SZ;i++) memset(sys_cfg->CRIPAddrs[i], 0, 4);	
	for(i = 0;i < MAX_UNIT_SZ;i++) actSetDefault(&sys_cfg->AlarmActions[i]);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) fzSetDefault(&sys_cfg->FireZones[i], i);
	for(i = 0;i < MAX_OC_SZ-2;i++) ocSetDefault(&sys_cfg->OutputCommands[i]);
	memcpy(sys_cfg->ServerIPAddress, Default_IPAddress, 4); sys_cfg->ServerIPAddress[3] = 100; 
	sys_cfg->ServerPort			= 8008;
	sys_cfg->Passback			= 0x00;
	sys_cfg->RepeatAccessDelay	= 10;
	memset(sys_cfg->PassbackResetTime, 0xff, 2);
	sys_cfg->FPIdentify			= 1;	// 	
	sys_cfg->FPSecurityLevel	= 6;	// 1-9 for Secugen  2012/2 5=>6
	sys_cfg->SCKeyVersion		= 1;
	sckeySetDefault(sys_cfg->SCPrimaryKey);
	camSetDefault(&sys_cfg->Camera);
	for(i = 0;i < 2;i++) memset(sys_cfg->InOutTimeSection[i], 0, 4);
	taSetOptionEx(2, 1);	// taSetOption(Keyoption=2(Fixed 1), DisplayOption=1(Disable))
	sys_cfg->FuncKeyTimer		= 0;
	taSetItemsDefault();
	sys_cfg->TAVersion			= 1;
	syscfgSetDefaultByLanguage(sys_cfg);
}

int syscfgEncode(SYS_CFG *sys_cfg, void *buf)
{
	CR_CFG			*cr_cfg;
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = 'D'; *p++ = 9 + '0';		// TypeIdentifier
	int2bcd((int)sys_cfg->DeviceID, 4, p); p += 2;
	string2bcd(sys_cfg->AdminPIN, 8, p); p += 4;
	p += devEncode(&sys_cfg->Device, p, 0);
	p += nwpEncode(&sys_cfg->NetworkPort, p);
	for(i = 0;i < MAX_UNIT_SZ;i++)	 p += unitEncode(&sys_cfg->Units[i], p);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitEncode(&sys_cfg->IOUnits[i], p);
	for(i = 0;i < MAX_AD_SZ;i++)	 p += adEncode(&sys_cfg->AccessDoors[i], p);
	for(i = 0;i < MAX_AP_SZ;i++)	 p += apEncode(&sys_cfg->AccessPoints[i], p);
	for(i = 0;i < MAX_AZ_SZ-1;i++)	 p += azEncode(&sys_cfg->AccessZones[i], p);
	for(i = 0;i < MAX_BI_SZ+MAX_XBI_SZ;i++) p += biEncode(&sys_cfg->BinaryInputs[i], p);
	for(i = 0;i < MAX_BO_SZ+MAX_XBO_SZ;i++) p += boEncode(&sys_cfg->BinaryOutputs[i], p);
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = &sys_cfg->CredentialReaders[i];
		*p++ = cr_cfg->Version;
		p += crEncodeAll(cr_cfg, p);
//cr_cfg->Type = 0;	// Cheongju
	}
//	for(i = 0;i < MAX_CR_SZ;i++) { memcpy(p, sys_cfg->CRIPAddrs[i], 4); p += 4; }
	for(i = 0;i < MAX_UNIT_SZ;i++) p += actEncode(&sys_cfg->AlarmActions[i], p);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) p += fzEncode(&sys_cfg->FireZones[i], p);
	for(i = 0;i < MAX_OC_SZ-2;i++) p += ocEncode(&sys_cfg->OutputCommands[i], p);
	memcpy(p, sys_cfg->ServerIPAddress, 4); p += 4;
	SHORTtoBYTE(sys_cfg->ServerPort, p); p += 2;
	*p++ = sys_cfg->Passback;
	*p++ = sys_cfg->RepeatAccessDelay;
	bin2bcd(sys_cfg->PassbackResetTime, 2, p); p += 2;
	*p++ = sys_cfg->FPIdentify;
	*p++ = sys_cfg->FPSecurityLevel;
	*p++ = sys_cfg->SCKeyVersion;
	p += sckeyEncode(sys_cfg->SCPrimaryKey, p);
	memset(p, 0xff, 25); p += 25;	// reserve;
	p += camEncode(&sys_cfg->Camera, p);
	for(i = 0;i < 2;i++, p += 4) bin2bcd(sys_cfg->InOutTimeSection[i], 4, p);
	*p++ = sys_cfg->TAOption;
	*p++ = sys_cfg->FuncKeyTimer;
	p += taEncodeItems(sys_cfg, p);	
	*p++ = sys_cfg->TAVersion;
	p += taEncodeTexts(sys_cfg, p);
 	return p - (unsigned char *)buf;
}

int syscfgDecode(SYS_CFG *sys_cfg, void *buf, int size)
{
	CR_CFG			*cr_cfg;
	unsigned char	*p;
	int		c, i, ver;

	p = (unsigned char *)buf;
	ver = p[1] - '0';
cprintf("syscfgDecode: [%c-%d]\n", p[0], ver);
	if(p[0] != 'D' || (ver != 8 && ver != 9)) { 
cprintf("syscfg new....\n");
		flashWriteEnable();
		fsSeek(fs[0], 0L, SEEK_SET);
		fsShift(fs[0], NULL, -1);
		flashWriteDisable();
		return 0;
	}
	p += 2;
	c = bcd2int(p, 4); p += 2; sys_cfg->DeviceID = c;
	bcd2string(p, 8, sys_cfg->AdminPIN); p += 4;
	p += devDecode(&sys_cfg->Device, p, 0);
	p += nwpDecode(&sys_cfg->NetworkPort, p);
	for(i = 0;i < MAX_UNIT_SZ;i++) p += unitDecode(&sys_cfg->Units[i], p);
	if(ver >= 8) {
		for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitDecode(&sys_cfg->IOUnits[i], p);
	}
	for(i = 0;i < MAX_AD_SZ;i++) p += adDecode(&sys_cfg->AccessDoors[i], p);
	for(i = 0;i < MAX_AP_SZ;i++) p += apDecode(&sys_cfg->AccessPoints[i], p);
	for(i = 0;i < MAX_AZ_SZ-1;i++) p += azDecode(&sys_cfg->AccessZones[i], p);
	for(i = 0;i < MAX_BI_SZ;i++) p += biDecode(&sys_cfg->BinaryInputs[i], p);
	if(ver >= 8) {
		for( ;i < MAX_BI_SZ+MAX_XBI_SZ;i++) p += biDecode(&sys_cfg->BinaryInputs[i], p);
	}	
	for(i = 0;i < MAX_BO_SZ;i++) p += boDecode(&sys_cfg->BinaryOutputs[i], p);
	if(ver >= 8) {
		for( ;i < MAX_BO_SZ+MAX_XBO_SZ;i++) p += boDecode(&sys_cfg->BinaryOutputs[i], p);
	}	
	for(i = 0;i < MAX_CR_SZ;i++) {
		cr_cfg = &sys_cfg->CredentialReaders[i];
		cr_cfg->Version = *p++;
		p += crDecodeAll(cr_cfg, p);
	}
//	if(ver >= 10) for(i = 0;i < MAX_CR_SZ;i++) { memcpy(sys_cfg->CRIPAddrs[i], p, 4); p += 4; }
	for(i = 0;i < MAX_UNIT_SZ;i++) p += actDecode(&sys_cfg->AlarmActions[i], p);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) {
		if(ver >= 9) p += fzDecode(&sys_cfg->FireZones[i], p);
		else	p += 4;
	}
	for(i = 0;i < MAX_OC_SZ-2;i++) p += ocDecode(&sys_cfg->OutputCommands[i], p);
	memcpy(sys_cfg->ServerIPAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &sys_cfg->ServerPort); p += 2;
	sys_cfg->Passback			= *p++;
	sys_cfg->RepeatAccessDelay	= *p++;
	bcd2bin(p, 2, sys_cfg->PassbackResetTime); p += 2;
	sys_cfg->FPIdentify			= *p++;
	sys_cfg->FPSecurityLevel	= *p++;
	sys_cfg->SCKeyVersion		= *p++;
	p += sckeyDecode(sys_cfg->SCPrimaryKey, p);
	c = size - (p - (unsigned char *)buf);
	if(c > 204) {
		p = p + c - 208;
		p += camDecode(&sys_cfg->Camera, p);
	} else {
		sys_cfg->Camera.Mode		= *p++;
		sys_cfg->Camera.RunState	= *p++;
		sys_cfg->Camera.Channel		= *p++;
		sys_cfg->Camera.Option		= *p++;
	}
	for(i = 0;i < 2;i++, p += 4) bcd2bin(p, 4, sys_cfg->InOutTimeSection[i]);
	sys_cfg->TAOption		= *p++;
	sys_cfg->FuncKeyTimer	= *p++;
	p += taDecodeItems(sys_cfg, p);
	sys_cfg->TAVersion		= *p++;
	p += taDecodeTexts(sys_cfg, p);
cprintf("size=%d cam=%02x %02x tna=%02x %02x %02x\n", c, (int)sys_cfg->Camera.Mode, (int)sys_cfg->Camera.RunState, (int)sys_cfg->TAOption, (int)sys_cfg->FuncKeyTimer, (int)sys_cfg->TAVersion);
cprintf("size=%d %d serverIPAddress=%d.%d.%d.%d port=%d\n", size, p-(unsigned char *)buf, (int)sys_cfg->ServerIPAddress[0], (int)sys_cfg->ServerIPAddress[1], (int)sys_cfg->ServerIPAddress[2], (int)sys_cfg->ServerIPAddress[3], (int)sys_cfg->ServerPort);
 	return p - (unsigned char *)buf;
}

int syscfgCompare(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg)
{
	unsigned char	*s1, *s2, buf[6000];
	int		size;

	size = 0;
	s1 = buf; s2 = buf + 3000;
	syscfgEncode(sys_cfg, s1);
	size = syscfgEncode(tmp_cfg, s2);
	return n_memcmp(s1, s2, size);
}

void syscfgCopy(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg)
{
	memcpy(sys_cfg, tmp_cfg, sizeof(SYS_CFG));
}

int syscfgRead(SYS_CFG *sys_cfg)
{
	FS_FILE		*f;
	unsigned char	buf[3000];
	int		rval;

	f = fs[0];
	fsSeek(f, 0L, SEEK_SET);
	rval = fsRead(f, buf, 3000);
cprintf("fsRead(%d)=%d fsErrno=%d\n", (int)f->file, rval, fsErrno);
	if(rval >= 400) {
		rval = syscfgDecode(sys_cfg, buf, rval);
cprintf("Decode=%d\n", rval);
	}
	if(strcmp(sys_cfg->Device.FirmwareVersion, syscfgGetFirmwareVersion())) {
//cprintf("%s => %s\n", sys_cfg->Device.FirmwareVersion, syscfgGetFirmwareVersion());
		strcpy(sys_cfg->Device.FirmwareVersion, syscfgGetFirmwareVersion());
		syscfgWrite(sys_cfg);
	}
	return rval;
}

int syscfgWrite(SYS_CFG *sys_cfg)
{
	FS_FILE		*f;
	unsigned char	buf[3000];
	int		rval, size;

	flashWriteEnable();
	fsClean();
	size = syscfgEncode(sys_cfg, buf);
	f = fs[0];
	fsSeek(f, 0L, SEEK_SET);
	rval = fsWrite(f, buf, size);
cprintf("fsWrite(%d,%d)=%d\n",  (int)f->file, size, rval);
	if(rval != size) {
#ifdef BK_DEBUG
		cprintf("fsWrite error\n", (int)f->file);
#endif
		rval = -1;
	} else	rval = 0;
	flashWriteDisable();
	return rval;
}

int syscfgBackup(void *buf)
{
	FS_FILE	*f;
	int		rval;

	f = fs[0];
	fsSeek(f, 0L, SEEK_SET);
	rval = fsRead(f, buf, 3000);
	return rval;
}

int syscfgRecovery(void *buf, int size)
{
	FS_FILE	*f;
	int		rval;

	flashWriteEnable();
//	size = syscfgEncode(sys_cfg, buf);
	f = fs[0];
	fsSeek(f, 0L, SEEK_SET);
	fsShift(f, NULL, -1);
	rval = fsWrite(f, buf, size);
	flashWriteDisable();
	if(rval != size) rval = -1;
	return rval;
}

void syscfgSetUnits(SYS_CFG *sys_cfg)
{
	UNIT_CFG			**unit;
	ACCESS_DOOR_CFG		**ad;
	CR_CFG				**cr;
	ALARM_ACTION_CFG	**act;
	IOUNIT_CFG			**iounit;
	int		i;

	unitscfgGet((void **)&unit, (void **)&ad, (void **)&cr, (void **)&act, (void *)&iounit);
	for(i = 0;i < MAX_UNIT_SZ;i++) unit[i] = &sys_cfg->Units[i];
	for(i = 0;i < MAX_AD_SZ;i++)   ad[i] = &sys_cfg->AccessDoors[i];
	for(i = 0;i < MAX_CR_SZ;i++)   cr[i] = &sys_cfg->CredentialReaders[i];
	for(i = 0;i < MAX_UNIT_SZ;i++) act[i] = &sys_cfg->AlarmActions[i];
	for(i = 0;i < MAX_IOUNIT_SZ;i++) iounit[i] = &sys_cfg->IOUnits[i];
}

void syscfgCopyToUnits(SYS_CFG *sys_cfg)
{
	UNIT_CFG			**unit;
	ACCESS_DOOR_CFG		**ad;
	CR_CFG				**cr;
	ALARM_ACTION_CFG	**act;
	IOUNIT_CFG			**iounit;
	int		i;

	unitscfgGet((void **)&unit, (void **)&ad, (void **)&cr, (void **)&act, (void *)&iounit);
	for(i = 0;i < MAX_UNIT_SZ;i++) memcpy(unit[i], &sys_cfg->Units[i], 4);
	for(i = 0;i < MAX_AD_SZ;i++)   memcpy(ad[i], &sys_cfg->AccessDoors[i], 4);
	for(i = 0;i < MAX_CR_SZ;i++)   memcpy(cr[i], &sys_cfg->CredentialReaders[i], 4);
	for(i = 0;i < MAX_UNIT_SZ;i++) memcpy(act[i], &sys_cfg->AlarmActions[i], 4);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) memcpy(iounit[i], &sys_cfg->IOUnits[i], 4);
}

void syscfgCopyFromUnits(SYS_CFG *sys_cfg)
{
	UNIT_CFG			**unit;
	ACCESS_DOOR_CFG		**ad;
	CR_CFG				**cr;
	ALARM_ACTION_CFG	**act;
	IOUNIT_CFG			**iounit;
	int		i;

	unitscfgGet((void **)&unit, (void **)&ad, (void **)&cr, (void **)&act, (void *)&iounit);
	for(i = 0;i < MAX_UNIT_SZ;i++) memcpy(&sys_cfg->Units[i], unit[i], 4);
	for(i = 0;i < MAX_AD_SZ;i++)   memcpy(&sys_cfg->AccessDoors[i], ad[i], 4);
	for(i = 0;i < MAX_CR_SZ;i++)   memcpy(&sys_cfg->CredentialReaders[i], cr[i], 4);
	for(i = 0;i < MAX_UNIT_SZ;i++) memcpy(&sys_cfg->AlarmActions[i], act[i], 4);
	for(i = 0;i < MAX_IOUNIT_SZ;i++) memcpy(&sys_cfg->IOUnits[i], iounit[i], 4);
}

int syscfgEncodeUnits(SYS_CFG *sys_cfg, void *buf)
{
	unsigned char	*p, val;
	ACCESS_DOOR_CFG		*ad_cfg;
	CR_CFG				*cr_cfg;
	ALARM_ACTION_CFG	*act_cfg;
	int		i;

	p = (unsigned char *)buf;
	for(i = 0;i < MAX_UNIT_SZ;i++) p += unitEncode(&sys_cfg->Units[i], p);
	for(i = 0, ad_cfg = sys_cfg->AccessDoors;i < MAX_AD_SZ;i++, ad_cfg++) {
		if(ad_cfg->Enable) val = 0x01; else val = 0x00;
		if(ad_cfg->RequestToExitEnable) val |= 0x02;
		if(ad_cfg->DoorStatusEnable) val |= 0x04;
		if(ad_cfg->AuxLockEnable) val |= 0x08;
		*p++ = val;
	}
	for(i = 0, cr_cfg = sys_cfg->CredentialReaders;i < MAX_CR_SZ;i++, cr_cfg++) {
		*p++ = cr_cfg->Type;
	}
	for(i = 0, act_cfg = sys_cfg->AlarmActions;i < MAX_UNIT_SZ;i++, act_cfg++) {
		*p++ = act_cfg->AlarmOutput;
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitEncode(&sys_cfg->IOUnits[i], p);
 	return p - (unsigned char *)buf;
}

int syscfgDecodeUnits(SYS_CFG *sys_cfg, void *buf)
{
	unsigned char	*p, val;
	ACCESS_DOOR_CFG		*ad_cfg;
	CR_CFG				*cr_cfg;
	ALARM_ACTION_CFG	*act_cfg;
	int		i;

	p = (unsigned char *)buf;
	for(i = 0;i < MAX_UNIT_SZ;i++) p += unitDecode(&sys_cfg->Units[i], p);
	for(i = 0, ad_cfg = sys_cfg->AccessDoors;i < MAX_AD_SZ;i++, ad_cfg++) {
		val = *p++;
		ad_cfg->Enable				= val & 0x01;
		ad_cfg->RequestToExitEnable	= (val >> 1) & 0x01;
		ad_cfg->DoorStatusEnable	= (val >> 2) & 0x01;
		ad_cfg->AuxLockEnable		= (val >> 3) & 0x01;
	}
	for(i = 0, cr_cfg = sys_cfg->CredentialReaders;i < MAX_CR_SZ;i++, cr_cfg++) {
		cr_cfg->Type			= *p++;
	}
	for(i = 0, act_cfg = sys_cfg->AlarmActions;i < MAX_UNIT_SZ;i++, act_cfg++) {
		act_cfg->AlarmOutput	= *p++;
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) p += iounitDecode(&sys_cfg->IOUnits[i], p);
 	return p - (unsigned char *)buf;
}

int syscfgValidateUnits(SYS_CFG *sys_cfg)
{
 	return 1;
}

void syscfgPrint(SYS_CFG *sys_cfg)
{
#ifdef BK_DEBUG
#endif
}

unsigned char *syscfgGetMACAddress(void)
{
	return MACAddr;
}

#include "devcfg.c"
#include "nwpcfg.c"
#include "camcfg.c"
