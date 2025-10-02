#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include "dirent.h"
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "msg.h"
#include "rtc.h"
//#include "sche.h"
#include "lang.h"
#include "sclib.h"
#include "slvmlib.h"
#include "sysdep.h"
#include "syscfg.h"

SYS_CFG		*sys_cfg, _sys_cfg;

static char cfgDirName[32]	   = "cfg";
static char syscfgFileName[16] = "syscfg.dat";

// Ver1.0  2020.6.15
//    - A20-IOWM 보드에 포팅
// Ver1.1.1	2020.12.04
//    - POE 모니터링 추가
// Ver1.1.2	2020.12.28
//    - firmware name: t40a => t50s
// Ver1.2.1	2020.04.23
//    - support Hynix EMV card, sio BLE card 
// Ver1.3.1	2021.05.04
//    - support RS-485
// Ver1.4.11	2022.11.18
//    - support EMV Granted Success voice ment
//    - bug fix: clear Hynix Emv Reader setting at ACU connect

// Ver6.1, Ver1.4  2020.5.15
//	  - ESP32 slave 에 의한 버젼 분리
// Ver6.2, Ver1.5  2020.6.28
//	  - New EMV card 지원
// Ver7.0.1  2024.1.12
//	  - Key Foundary EMV card 지원 버젼

#ifdef _ESP32
static unsigned char bk_version[3] = { 6, 2, 31 };
//static unsigned char bk_version[3] = { 7, 0, 32 }; // key Foundary EMV card 
#else
static unsigned char bk_version[3] = { 1, 5, 31 };
//static unsigned char bk_version[3] = { 7, 0, 32 };   //key Fondary EMV card 
#endif


FILE *_OpenSIB(char *fileName)
{
	FILE	*fp;
	struct stat	_stat;
	char	filePath[128];

	sprintf(filePath, "%s/%s", cfgDirName, fileName); 
	fp = fopen(filePath, "r+b");
	if(!fp) fp = fopen(filePath, "w+");
	if(!fp) {
		printf("fopen(%s) error\n", filePath);
		return (FILE *)0;
	}
	stat(filePath, &_stat);
printf("SIB(file=%s) is opened: size=%ld\n", filePath, _stat.st_size);
	return fp;
}

void _DeleteSIB(char *fileName)
{
	char	filePath[128];

	sprintf(filePath, "%s/%s", cfgDirName, fileName); 
printf("remove: %s\n", filePath);
	remove(filePath);
}

int syscfgInitialize(void)
{	
	DIR 	*dir;
	int		i;

	dir = opendir(cfgDirName);
	if(!dir) mkdir(cfgDirName, 644);
	sys_cfg = &_sys_cfg;
	syscfgSetDefault(sys_cfg);
//syscfgReset(sys_cfg);
	syscfgRead(sys_cfg);
	return 0;
}

void syscfgReset(SYS_CFG *cfg)
{
	int		i;

	_DeleteSIB(syscfgFileName);
	sync();
	syscfgSetDefault(cfg);
}

void syscfgSetDefaultByLanguage(SYS_CFG *cfg);

static unsigned char IPAddr[4] =		{ 192, 168, 2, 2 };
static unsigned char SubnetMask[4] = 	{ 255, 255, 255, 0 };

void syscfgSetDefault(SYS_CFG *cfg)
{
	unsigned char	temp[4];
	int		i, j, val;
	
	memset(cfg, 0, sizeof(SYS_CFG));
	memcpy(cfg->firmwareVersion, syscfgFirmwareVersion(cfg), 3);
	strcpy(cfg->adminPassword, "1111");
	cfg->devId				= 1;
	cfg->devMode			= 0;			// 0:master  1:slave
	cfg->devOption			= ADMPWD_DIGITS_ONLY;
	cfg->language			= LANG_KOR;		// English
	cfg->dateNotation		= 0x00;
	cfg->lcdContrast		= 50;
	getKernelLocalVersion(temp);
	if(temp[0] == '2') cfg->lcdBrightness = 10;	// 90
	else    cfg->lcdBrightness  = 36;
	cfg->mediaOption		= 0xc0;
	cfg->userSoundVolume	= 70;
	cfg->mediaSoundVolume	= 80;
	cfg->callSoundVolume	= 80;
	cfg->systemSoundVolume	= 40;
	cfg->sleepMode			= 0;
	cfg->sleepWaitTime		= 600;
	cfg->authWaitTime		= 20;
	cfg->authResultTime		= 25;
	cfg->authResultOption	= 0x00;
	cfg->fpSecurityLevel	= 6;			// 1-9 for Secugen
	cfg->scRfoNormalLevel	= 14;
//	memset(cfg->reserve, 0, 2);
	cfg->networkOption		= USE_DHCP;
	memcpy(cfg->dhcpIpAddress, IPAddr, 4);
	memcpy(cfg->ipAddress, IPAddr, 4);
	memcpy(cfg->subnetMask, SubnetMask, 4);
	memcpy(cfg->gatewayIpAddress, IPAddr, 4); cfg->gatewayIpAddress[3] = 1; 
	memset(cfg->dnsIpAddress, 0, 4);
	memcpy(cfg->serverIpAddress, IPAddr, 4); cfg->serverIpAddress[3] = 100; 
	cfg->serverIpPort		= 8008;
	memcpy(cfg->masterIpAddress, IPAddr, 4); cfg->masterIpAddress[3] = 101; 
	cfg->masterIpPort		= 8008;
	camSetDefault(&sys_cfg->camera);
	bleSetDefault(&sys_cfg->ble);
	for(i = 0;i < MAX_WPA_NET_SZ;i++) wpanetSetDefault(&cfg->wpanets[i]);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) _UnitSetDefault(&cfg->units[i]);
	for(i = 0;i < MAX_CH_HALF_SZ-1;i++) _SubdevSetDefault(&cfg->subdevices[i]);
	for(i = 0;i < MAX_CH_HALF_SZ;i++) _CrSetDefault(&cfg->credentialReaders[i]);
	cfg->credentialReaders[0].type = CR_TYPE_PRIMARY;
	for(i = 0;i < MAX_BI_HALF_SZ+MAX_XBI_HALF_SZ;i++) _BiSetDefault(&cfg->binaryInputs[i]);
	cfg->binaryInputs[0].type = BI_TYPE_DOOR;
	for(i = 0;i < MAX_BO_HALF_SZ+MAX_XBO_HALF_SZ;i++) _BoSetDefault(&cfg->binaryOutputs[i]);
	cfg->binaryOutputs[0].type = BI_TYPE_DOOR;
	for(i = 0;i < MAX_CH_HALF_SZ;i++) _AdSetDefault(&cfg->accessDoors[i]);
	for(i = 0;i < MAX_AA_SZ;i++) _AaSetDefault(&cfg->alarmActions[i]); 
	cfg->alarmActions[0].enable = 1;
	_CfFpSetDefault(&cfg->fpFormat);
	_CfPinSetDefault(&cfg->pinFormat);
	for(i = 0;i < MAX_CF_CARD_SZ;i++) {
#ifdef _HYNIX
		if(i == 1) val = CRED_INT64;
		else	val = CRED_INT32;
#else
		val = CRED_INT32;
#endif
		_CfCardSetDefault(&cfg->cardFormats[i], i, val); 
	}
	for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) _CfWiegandSetDefault(&cfg->wiegandFormats[i], i, -1); 
	for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) _CfWiegandSetDefault(&cfg->wiegandOutputFormats[i], i, -1); 
	for(i = 0;i < MAX_CF_WIEGAND_PIN_SZ;i++) _CfWiegandPinSetDefault(&cfg->wiegandPinOutputFormats[i], i); 
	cfg->OperationMode		= 1;		// 1:근태 4:급식
	cfg->AcuModel			= 1;
	cfg->AcuCommID			= 1;
	for(i = 0;i < 2;i++) memset(cfg->InOutTimeSection[i], 0, 4);
	for(i = 0;i < 5;i++) memset(cfg->MealTimeSection[i], 0, 4);
	for(i = 0;i < 5;i++) cfg->MealMenu[i] = 1;
	cfg->MealOption			= 0x00;
	for(i = 0;i < 5;i++)
		for(j = 0;j < 4;j++) cfg->MealMenuPrice[i][j] = 0;
}

int EncodeOperation(SYS_CFG *cfg, void *buf, int LocalUse)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->OperationMode;
	*p++ = cfg->AcuModel;
	*p++ = cfg->AcuCommID;
	if(LocalUse) {
		for(i = 0;i < 2;i++, p += 4) bin2bcd(cfg->InOutTimeSection[i], 4, p);
		for(i = 0;i < 5;i++, p += 4) bin2bcd(cfg->MealTimeSection[i], 4, p);
	} else if(cfg->OperationMode == 2 || cfg->OperationMode == 3) {
		for(i = 0;i < 2;i++, p += 4) bin2bcd(cfg->InOutTimeSection[i], 4, p);
		for(i = 0;i < 3;i++, p += 4) memset(p, 0, 4);
	} else if(cfg->OperationMode == 4) {
		for(i = 0;i < 5;i++, p += 4) bin2bcd(cfg->MealTimeSection[i], 4, p);
	} else {
		for(i = 0;i < 5;i++, p += 4) memset(p, 0, 4);		
	}
	for(i = 0;i < 5;i++) *p++ = cfg->MealMenu[i];
	*p++ = cfg->MealOption;
	return p - (unsigned char *)buf;
}

int EncodeExtraOperation(SYS_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i, j;

	p = (unsigned char *)buf;
	for(i = 0;i < 5;i++)
		for(j = 0;j < 4;j++) { SHORTtoBYTE(cfg->MealMenuPrice[i][j], p); p += 2; }
	return p - (unsigned char *)buf;
}

int DecodeOperation(SYS_CFG *cfg, void *buf, int LocalUse)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cfg->OperationMode = *p++;
	if(LocalUse || cfg->OperationMode == 1 || cfg->OperationMode == 2) {
		cfg->AcuModel	= *p++;
		cfg->AcuCommID	= *p++;
	} else	p += 2;
	if(LocalUse) {
		for(i = 0;i < 2;i++, p += 4) bcd2bin(p, 4, cfg->InOutTimeSection[i]);
		for(i = 0;i < 5;i++, p += 4) bcd2bin(p, 4, cfg->MealTimeSection[i]);
		for(i = 0;i < 5;i++) cfg->MealMenu[i] = *p++;
		cfg->MealOption = *p++;
	} else if(cfg->OperationMode == 2 || cfg->OperationMode == 3) {
		for(i = 0;i < 2;i++, p += 4) bcd2bin(p, 4, cfg->InOutTimeSection[i]);
		p += 18;
	} else if(cfg->OperationMode == 4) {
		for(i = 0;i < 5;i++, p += 4) bcd2bin(p, 4, cfg->MealTimeSection[i]);
		for(i = 0;i < 5;i++) cfg->MealMenu[i] = *p++;
		cfg->MealOption = *p++;
	} else {
		p += 26;
	}
	return p - (unsigned char *)buf;
}

int DecodeExtraOperation(SYS_CFG *cfg, void *buf, int LocalUse)
{
	unsigned char	*p;
	int		i, j;

	p = (unsigned char *)buf;
	if(LocalUse) {
		for(i = 0;i < 5;i++)
			for(j = 0;j < 4;j++) { BYTEtoSHORT(p, &cfg->MealMenuPrice[i][j]); p += 2; }
	} else if(cfg->OperationMode == 2 || cfg->OperationMode == 3) {
		p += 40;
	} else if(cfg->OperationMode == 4) {
		for(i = 0;i < 5;i++)
			for(j = 0;j < 4;j++) { BYTEtoSHORT(p, &cfg->MealMenuPrice[i][j]); p += 2; }
	} else {
		p += 40;
	}
	return p - (unsigned char *)buf;
}

int ValidateOperation(SYS_CFG *cfg)
{
	int		i;

	if(!cfg->OperationMode || cfg->OperationMode > 4) return 0;
	if((cfg->OperationMode == 1 || cfg->OperationMode == 2) && (!cfg->AcuCommID || cfg->AcuCommID > 6)) return 0;
	if(cfg->OperationMode == 2 || cfg->OperationMode == 3) {
		for(i = 0;i < 2;i++)
			if(!tmsIsNull(cfg->InOutTimeSection[i]) && !tmsValidateEx(cfg->InOutTimeSection[i])) return 0;
	} else if(cfg->OperationMode == 4) {
		for(i = 0;i < 5;i++)
			if(!tmsIsNull(cfg->MealTimeSection[i]) && !tmsValidateEx(cfg->MealTimeSection[i])) return 0;
		for(i = 0;i < 5;i++)
			if(!cfg->MealMenu[i] || cfg->MealMenu[i] > 4) return 0;
	}
	return 1;
}

int syscfgEncode(SYS_CFG *cfg, void *buf)
{
	unsigned char	*p, val;
	int		i;

	p = (unsigned char *)buf;
	*p++ = 'B'; *p++ = 5;		// TypeIdentifier
	memcpy(p, syscfgFirmwareVersion(cfg), 3); p += 3;
	memcpy_pad(p, cfg->adminPassword, 12); p += 12;
	int2bcd((int)cfg->devId, 4, p); p += 2;
	*p++ = cfg->devMode;
	*p++ = cfg->devOption;
	*p++ = cfg->language;
	*p++ = cfg->dateNotation;
	*p++ = cfg->lcdContrast;
	*p++ = cfg->lcdBrightness;
	*p++ = cfg->mediaOption;
	*p++ = cfg->userSoundVolume;
	*p++ = cfg->mediaSoundVolume;
	*p++ = cfg->callSoundVolume;
	*p++ = cfg->systemSoundVolume;
	*p++ = cfg->sleepMode;
	SHORTtoBYTE(cfg->sleepWaitTime, p); p += 2;
	*p++ = cfg->authWaitTime;
	*p++ = cfg->authResultTime;
	*p++ = cfg->authResultOption;
	*p++ = cfg->fpSecurityLevel;
	*p++ = cfg->scRfoNormalLevel;
//	memcpy(p, cfg->reserve, 3); p += 3;
	*p++ = cfg->networkOption;
	memcpy(p, cfg->dhcpIpAddress, 4); p += 4;
	memcpy(p, cfg->ipAddress, 4); p += 4;
	memcpy(p, cfg->subnetMask, 4); p += 4;
	memcpy(p, cfg->gatewayIpAddress, 4); p += 4;
	memcpy(p, cfg->dnsIpAddress, 4); p += 4;
	memcpy(p, cfg->serverIpAddress, 4); p += 4;
	SHORTtoBYTE(cfg->serverIpPort, p); p += 2;
	memcpy(p, cfg->masterIpAddress, 4); p += 4;
	SHORTtoBYTE(cfg->masterIpPort, p); p += 2;
	p += camEncode(&cfg->camera, p);
	p += bleEncode(&cfg->ble, p);
	for(i = 0;i < MAX_WPA_NET_SZ;i++) p += wpanetEncode(&cfg->wpanets[i], p);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) p += _UnitEncode(&cfg->units[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ-1;i++) p += _SubdevEncode(&cfg->subdevices[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ;i++) p += _CrEncode(&cfg->credentialReaders[i], p);
	for(i = 0;i < MAX_BI_HALF_SZ+MAX_XBI_HALF_SZ;i++) p += _BiEncode(&cfg->binaryInputs[i], p);
	for(i = 0;i < MAX_BO_HALF_SZ+MAX_XBO_HALF_SZ;i++) p += _BoEncode(&cfg->binaryOutputs[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ;i++) p += _AdEncode(&cfg->accessDoors[i], p);
	for(i = 0;i < MAX_AA_SZ;i++) p += _AaEncode(&cfg->alarmActions[i], p); 
	p += _CfFpEncode(&cfg->fpFormat, p);
	p += _CfPinEncode(&cfg->pinFormat, p);
	for(i = 0;i < MAX_CF_CARD_SZ;i++) p += _CfCardEncode(&cfg->cardFormats[i], i, p); 
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) p += _CfWiegandEncode(&cfg->wiegandFormats[i], p); 
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) p += _CfWiegandEncode(&cfg->wiegandOutputFormats[i], p); 
	for(i = 0;i < MAX_CF_WIEGAND_PIN_SZ;i++) p += _CfWiegandPinEncode(&cfg->wiegandPinOutputFormats[i], p); 
	p += EncodeOperation(cfg, p, 1);
	p += EncodeExtraOperation(cfg, p);
 	return p - (unsigned char *)buf;
}

int syscfgDecode(SYS_CFG *cfg, void *buf, int size)
{
	unsigned char	*p;
	int		c, i, ver;

	p = (unsigned char *)buf;
printf("syscfgDecode: [%c%c]\n", (int)p[0], (int)p[1]);
	if(p[0] != 'B' || p[1] != 5) {
printf("syscfg new....\n");
		syscfgReset(cfg);
		return 0;
	}
	ver = p[1]; 
	p += 2;
	memcpy(cfg->firmwareVersion, p, 3); p += 3;
	memcpy_chop(cfg->adminPassword, p, 12); p += 12;
	cfg->devId = bcd2int(p, 4); p += 2;
	cfg->devMode			= *p++;
	cfg->devOption			= *p++;
	cfg->language			= *p++;
//printf("%d.%d.%d [%s] [%s] [%s] %d %d\n", (int)gfg->mwareVersion[0], (int)cfg->firmwareVersion[1], (int)cfg->firmwareVersion[2], cfg->model, cfg->maker, cfg->adminPassword, (int)cfg->fpSecurityLevel, (int)cfg->language);
	cfg->dateNotation		= *p++;
	cfg->lcdContrast		= *p++;
	cfg->lcdBrightness		= *p++;
	cfg->mediaOption		= *p++;
	cfg->userSoundVolume	= *p++;
	cfg->mediaSoundVolume	= *p++;
	cfg->callSoundVolume	= *p++;
	cfg->systemSoundVolume	= *p++;
	cfg->sleepMode			= *p++;
	BYTEtoSHORT(p, &cfg->sleepWaitTime); p += 2; if(!cfg->sleepWaitTime) cfg->sleepWaitTime = 600;
	cfg->authWaitTime		= *p++;
	cfg->authResultTime		= *p++;
	cfg->authResultOption	= *p++;
	cfg->fpSecurityLevel	= *p++;
	cfg->scRfoNormalLevel	= *p++;
//	memcpy(cfg->reserve, p, 3); p += 3;
	cfg->networkOption		= *p++;
	memcpy(cfg->dhcpIpAddress, p, 4); p += 4;
	memcpy(cfg->ipAddress, p, 4); p += 4;
	memcpy(cfg->subnetMask, p, 4); p += 4;
	memcpy(cfg->gatewayIpAddress, p, 4); p += 4;
	memcpy(cfg->dnsIpAddress, p, 4); p += 4;
	memcpy(cfg->serverIpAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &cfg->serverIpPort); p += 2;
	memcpy(cfg->masterIpAddress, p, 4); p += 4;
	BYTEtoSHORT(p, &cfg->masterIpPort); p += 2;
	p += camDecode(&cfg->camera, p);
	p += bleDecode(&cfg->ble, p);
	for(i = 0;i < MAX_WPA_NET_SZ;i++) p += wpanetDecode(&cfg->wpanets[i], p);
	for(i = 0;i < MAX_UNIT_HALF_SZ;i++) p += _UnitDecode(&cfg->units[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ-1;i++) p += _SubdevDecode(&cfg->subdevices[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ;i++) p += _CrDecode(&cfg->credentialReaders[i], p);
	for(i = 0;i < MAX_BI_HALF_SZ+MAX_XBI_HALF_SZ;i++) p += _BiDecode(&cfg->binaryInputs[i], p);
	for(i = 0;i < MAX_BO_HALF_SZ+MAX_XBO_HALF_SZ;i++) p += _BoDecode(&cfg->binaryOutputs[i], p);
	for(i = 0;i < MAX_CH_HALF_SZ;i++) p += _AdDecode(&cfg->accessDoors[i], p);
	for(i = 0;i < MAX_AA_SZ;i++) p += _AaDecode(&cfg->alarmActions[i], p); 
	p += _CfFpDecode(&cfg->fpFormat, p);
	p += _CfPinDecode(&cfg->pinFormat, p);
	for(i = 0;i < MAX_CF_CARD_SZ;i++) p += _CfCardDecode(&cfg->cardFormats[i], i, p); 
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) p += _CfWiegandDecode(&cfg->wiegandFormats[i], p); 
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) p += _CfWiegandDecode(&cfg->wiegandOutputFormats[i], p); 
	for(i = 0;i < MAX_CF_WIEGAND_PIN_SZ;i++) p += _CfWiegandPinDecode(&cfg->wiegandPinOutputFormats[i], p); 
	p += DecodeOperation(cfg, p, 1);
	p += DecodeExtraOperation(cfg, p, 1);
 	return p - (unsigned char *)buf;
}

int syscfgCompare(SYS_CFG *cfg, SYS_CFG *tcfg)
{
	unsigned char	*s1, *s2, buf[8000];
	int		size, i;
	
	size = 0;
	s1 = buf; s2 = buf + 4000;
	size = syscfgEncode(cfg, s1);
	size = syscfgEncode(tcfg, s2);
	if(size > 0 && !memcmp(s1, s2, size)) return 0;
	else	return -1;
}

void syscfgCopy(SYS_CFG *cfg, SYS_CFG *tcfg)
{
	memcpy(cfg, tcfg, sizeof(SYS_CFG));
}

int syscfgRead(SYS_CFG *cfg)
{
	FILE		*fp;
	unsigned char	buf[4000];
	int		i, rval, size[6];

	fp = _OpenSIB(syscfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fread(buf, 1, 4000, fp);
//printf("fsRead(4000)=%d fsErrno=%d\n", rval, fsErrno);
	if(rval >= 200) {
		rval = syscfgDecode(cfg, buf, rval);
printf("syscfgDecode=%d\n", rval);
	}
	fclose(fp);
	if(memcmp(cfg->firmwareVersion, syscfgFirmwareVersion(cfg), 3)) {
//printf("%s => %s\n", cfg->firmwareVersion, syscfgGetFirmwareVersion());
		memcpy(cfg->firmwareVersion, syscfgFirmwareVersion(cfg), 3);
		syscfgWrite(cfg);
	}
	return rval;
}

int syscfgWrite(SYS_CFG *cfg)
{
	FILE	*fp;
	unsigned char	buf[4000];
	int		i, rval, size;

	size = syscfgEncode(cfg, buf);
printf("syscfgEncode=%d\n", size);
	fp = _OpenSIB(syscfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fwrite(buf, 1, size, fp);
printf("fsWrite(%d)=%d\n", size, rval);
	if(rval != size) {
#ifdef BK_DEBUG
		printf("fsWrite error\n");
#endif
		rval = -1;
	} else	rval = 0;
	fclose(fp);
	sync();
	return rval;
}

int syscfgBackup(void *buf)
{
	FILE	*fp;
	int		rval;

	fp = _OpenSIB(syscfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fread(buf, 1, 2200, fp);
	fclose(fp);
	return rval;
}

int syscfgRecovery(void *buf, int size)
{
	_DeleteSIB(syscfgFileName);
	return 0;
}

void syscfgPrint(SYS_CFG *cfg)
{
#ifdef BK_DEBUG
#endif
}

void SetDefaultIPAddress(unsigned char *ipAddr)
{
	memcpy(ipAddr, IPAddr, 4);
}

char	gText[64];

unsigned char *syscfgFirmwareVersion(void *self)
{
	return bk_version;
}

char *syscfgFirmwareVersionName(void *self)
{
	unsigned char	*p;

	p = syscfgFirmwareVersion(self);
	sprintf(gText, "%d.%d.%d", (int)p[0], (int)p[1], (int)p[2]);
	return gText;
}

char *syscfgDeviceIdName(void *self)
{
	int		id;

	id = devId(self);
	n_utoa((unsigned long)id, gText);
	return gText;
}

void syscfgSetDeviceIdName(void *self, char *name)
{
	int		id;

	id = n_atoi(name);
	devSetId(self, id);
}

BOOL syscfgDisplayDatetime(void *self)
{
	if(sys_cfg->dateNotation & DISPLAY_DATETIME) return  TRUE;
	else	return FALSE;
}

void syscfgSetDisplayDatetime(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->dateNotation |= DISPLAY_DATETIME;
	else	 sys_cfg->dateNotation &= ~DISPLAY_DATETIME;
}

int syscfgScRfoNormalLevel(void *self)
{
	return (int)sys_cfg->scRfoNormalLevel;
}

void syscfgSetScRfoNormalLevel(void *self, int val)
{
	sys_cfg->scRfoNormalLevel = val;
}

int syscfgAuthWaitTime(void *self)
{
	return (int)sys_cfg->authWaitTime;
}

void syscfgSetAuthWaitTime(void *self, int val)
{
	sys_cfg->authWaitTime = val;
}

int syscfgAuthResultTime(void *self)
{
	return (int)sys_cfg->authResultTime;
}

void syscfgSetAuthResultTime(void *self, int val)
{
	sys_cfg->authResultTime = val;
}

BOOL syscfgResultInhibitAuth(void *self)
{
	if(sys_cfg->authResultOption & 0x01) return TRUE;
	else	return FALSE;
}

void syscfgSetResultInhibitAuth(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->authResultOption |= 0x01;
	else	 sys_cfg->authResultOption &= 0xfe;
}

int syscfgSleepMode(void *self)
{
	return (int)sys_cfg->sleepMode;
}

void syscfgSetSleepMode(void *self, int sleepMode)
{
	sys_cfg->sleepMode = sleepMode;
}

int syscfgSleepWaitTime(void *self)
{
	return (int)sys_cfg->sleepWaitTime;
}

void syscfgSetSleepWaitTime(void *self, int val)
{
	sys_cfg->sleepWaitTime = val;
}

int syscfgLanguage(void *self)
{
	return (int)sys_cfg->language;
}

void syscfgSetLanguage(void *self, int val)
{
	int		lang;

	lang = sys_cfg->language;
	sys_cfg->language = val;
	if(lang != val) ChangeLanguage(val);
}

char *syscfgAdminPwd(void *self)
{
	return sys_cfg->adminPassword;
}

void syscfgSetAdminPwd(void *self, char *pwd)
{
	strcpy(sys_cfg->adminPassword, pwd);
}

BOOL syscfgAdminPwdDigitsOnly(void *self)
{
	if(sys_cfg->devOption & ADMPWD_DIGITS_ONLY) return TRUE;
	else	return FALSE;
}

void syscfgSetAdminPwdDigitsOnly(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= ADMPWD_DIGITS_ONLY;
	else	 sys_cfg->devOption &= ~ADMPWD_DIGITS_ONLY;
}

int syscfgTimeNotation(void *self)
{
	int		val;

	if(sys_cfg->dateNotation & TIME_NOTATION) val = 1;
	else	val = 0;
	return val;
}

void syscfgSetTimeNotation(void *self, int val)
{
	if(val) sys_cfg->dateNotation |= TIME_NOTATION;
	else	sys_cfg->dateNotation &= ~TIME_NOTATION;
}

int syscfgLcdContrast(void *self)
{
	return (int)sys_cfg->lcdContrast;
}

void syscfgSetLcdContrast(void *self, int val)
{
	sys_cfg->lcdContrast = val;
}

int syscfgLcdBrightness(void *self)
{
	return (int)sys_cfg->lcdBrightness;
}

void syscfgSetLcdBrightness(void *self, int val)
{
	sys_cfg->lcdBrightness = val;
}

int syscfgUserSoundVolume(void *self)
{
	return (int)sys_cfg->userSoundVolume;
}

void syscfgSetUserSoundVolume(void *self, int val)
{
	sys_cfg->userSoundVolume = val;
}

int syscfgMediaSoundVolume(void *self)
{
	return (int)sys_cfg->mediaSoundVolume;
}

void syscfgSetMediaSoundVolume(void *self, int val)
{
	sys_cfg->mediaSoundVolume = val;
}

int syscfgCallSoundVolume(void *self)
{
	return (int)sys_cfg->callSoundVolume;
}

void syscfgSetCallSoundVolume(void *self, int val)
{
	sys_cfg->callSoundVolume = val;
}

int syscfgSystemSoundVolume(void *self)
{
	return (int)sys_cfg->systemSoundVolume;
}

void syscfgSetSystemSoundVolume(void *self, int val)
{
	int		vol;

	vol = sys_cfg->systemSoundVolume;
	sys_cfg->systemSoundVolume = val;
	if(vol != val) appSetSoundSetting((int)sys_cfg->systemSoundVolume, (int)sys_cfg->mediaOption);
}

BOOL syscfgKeypadTone(void *self)
{
	if(sys_cfg->mediaOption & MEDIA_KEYPAD_TONE) return TRUE;
	else	return FALSE;
}

void syscfgSetKeypadTone(void *self, BOOL bVal)
{
	unsigned char	option;

	option = sys_cfg->mediaOption;
	if(bVal) sys_cfg->mediaOption |= MEDIA_KEYPAD_TONE;
	else	 sys_cfg->mediaOption &= ~MEDIA_KEYPAD_TONE;
	if(option != sys_cfg->mediaOption) appSetSoundSetting((int)sys_cfg->systemSoundVolume, (int)sys_cfg->mediaOption);
}

BOOL syscfgTouchSounds(void *self)
{
	if(sys_cfg->mediaOption & MEDIA_TOUCH_SOUNDS) return TRUE;
	else	return FALSE;
}

void syscfgSetTouchSounds(void *self, BOOL bVal)
{
	unsigned char	option;

	option = sys_cfg->mediaOption;
	if(bVal) sys_cfg->mediaOption |= MEDIA_TOUCH_SOUNDS;
	else	 sys_cfg->mediaOption &= ~MEDIA_TOUCH_SOUNDS;
	if(option != sys_cfg->mediaOption) appSetSoundSetting((int)sys_cfg->systemSoundVolume, (int)sys_cfg->mediaOption);
}

int syscfgNetworkType(void *self)
{
	int		val;

	if(sys_cfg->networkOption & USE_WIFI) val = 1; else val = 0;
	return val;
}

void syscfgSetNetworkType(void *self, int type)
{
	if(type) sys_cfg->networkOption |= USE_WIFI;
	else	 sys_cfg->networkOption &= ~USE_WIFI;
}

BOOL syscfgUseDhcp(void *self)
{
	if(sys_cfg->networkOption & USE_DHCP) return TRUE;
	else	return FALSE;
}

void syscfgSetUseDhcp(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->networkOption |= USE_DHCP;
	else	 sys_cfg->networkOption &= ~USE_DHCP;
}

unsigned char *syscfgDhcpIpAddress(void *self)
{
	return sys_cfg->dhcpIpAddress;
}

void syscfgSetDhcpIpAddress(void *self, unsigned char *ipAddress)
{
	memcpy(sys_cfg->dhcpIpAddress, ipAddress, 4);
}

unsigned char *syscfgIpAddress(void *self)
{
	return sys_cfg->ipAddress;
}

void syscfgSetIpAddress(void *self, unsigned char *ipAddress)
{
	memcpy(sys_cfg->ipAddress, ipAddress, 4);
}

char *syscfgIpAddressName(void *self)
{
	return inet2addr(syscfgIpAddress(self));
}

void syscfgSetIpAddressName(void *self, char *name)
{
	addr2inet(name, sys_cfg->ipAddress);
}

unsigned char *syscfgSubnetMask(void *self)
{
	return sys_cfg->subnetMask;
}

void syscfgSetSubnetMask(void *self, unsigned char *subnetMask)
{
	memcpy(sys_cfg->subnetMask, subnetMask, 4);
}

char *syscfgSubnetMaskName(void *self)
{
	return inet2addr(syscfgSubnetMask(self));
}

void syscfgSetSubnetMaskName(void *self, char *name)
{
	addr2inet(name, sys_cfg->subnetMask);
}

unsigned char *syscfgGatewayIpAddress(void *self)
{
	return sys_cfg->gatewayIpAddress;
}

void syscfgSetGatewayIpAddress(void *self, unsigned char *ipAddress)
{
	memcpy(sys_cfg->gatewayIpAddress, ipAddress, 4);
}

char *syscfgGatewayIpAddressName(void *self)
{
	return inet2addr(syscfgGatewayIpAddress(self));
}

void syscfgSetGatewayIpAddressName(void *self, char *name)
{
	addr2inet(name, sys_cfg->gatewayIpAddress);
}

unsigned char *syscfgDnsIpAddress(void *self)
{
	return sys_cfg->dnsIpAddress;
}

void syscfgSetDnsIpAddress(void *self, unsigned char *ipAddress)
{
	memcpy(sys_cfg->dnsIpAddress, ipAddress, 4);
}

char *syscfgDnsIpAddressName(void *self)
{
	return inet2addr(syscfgDnsIpAddress(self));
}

void syscfgSetDnsIpAddressName(void *self, char *name)
{
	addr2inet(name, sys_cfg->dnsIpAddress);
}

unsigned char *syscfgServerIpAddress(void *self)
{
	return sys_cfg->serverIpAddress;
}

void syscfgSetServerIpAddress(void *self, unsigned char *ipAddress)
{
	memcpy(sys_cfg->serverIpAddress, ipAddress, 4);
}

char *syscfgServerIpAddressName(void *self)
{
	return inet2addr(syscfgServerIpAddress(self));
}

void syscfgSetServerIpAddressName(void *self, char *name)
{
	addr2inet(name, sys_cfg->serverIpAddress);
}

int syscfgServerIpPort(void *self)
{
	return (int)sys_cfg->serverIpPort;
}

void syscfgSetServerIpPort(void *self, int ipPort)
{
	sys_cfg->serverIpPort = ipPort;
}

char *syscfgServerIpPortName(void *self)
{
	int		port;

	port = syscfgServerIpPort(self);
	n_utoa((unsigned long)port, gText);
	return gText;
}

void syscfgSetServerIpPortName(void *self, char *name)
{
	int		port;

	port = n_atoi(name);
	syscfgSetServerIpPort(self, port);
}

unsigned char *syscfgMasterIpAddress(void *self)
{
	return sys_cfg->masterIpAddress;
}

void syscfgSetMasterIpAddress(void *self, unsigned char *ipAddress)
{
	unsigned char	ipAddr[4];

	memcpy(ipAddr, sys_cfg->masterIpAddress, 4);
	memcpy(sys_cfg->masterIpAddress, ipAddr, 4);
}

char *syscfgMasterIpAddressName(void *self)
{
	return inet2addr(syscfgMasterIpAddress(self));
}

void syscfgSetMasterIpAddressName(void *self, char *name)
{
	unsigned char	ipAddr[4];

	memcpy(ipAddr, sys_cfg->masterIpAddress, 4);
	addr2inet(name, sys_cfg->masterIpAddress);
}

int syscfgMasterIpPort(void *self)
{
	return (int)sys_cfg->masterIpPort;
}

void syscfgSetMasterIpPort(void *self, int ipPort)
{
	unsigned short	port;

	port = sys_cfg->masterIpPort;
	sys_cfg->masterIpPort = ipPort;
}

char *syscfgMasterPortName(void *self)
{
	int		port;

	port = syscfgMasterIpPort(self);
	n_utoa((unsigned long)port, gText);
	return gText;
}

void syscfgSetMasterIpPortName(void *self, char *name)
{
	int		port;

	port = n_atoi(name);
	syscfgSetMasterIpPort(self, port);
}

