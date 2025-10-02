#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "msg.h"
#include "slvmlib.h"
#include "lcdc.h"
#include "slvm.h"
#include "dev.h"
#include "unit.h"
#include "subdev.h"
#include "bio.h"
#include "ad.h"
#include "intrusion.h"
#include "admtitle.h"

static char *bk_maker		= "Arvratech";
#ifdef _ESP32
static char bk_model[16]	= "T40B-Slave";
//static char bk_model[16] = "T40B-K-Slave";
char    _firmwareName[8]	= "t60s";
#else
static char bk_model[16]	= "T40A-Slave";
//static char bk_model[16]	= "T40A-K-Slave";
char    _firmwareName[8]	= "t40a";	// t40a
#endif
char	_kernelName[8]		= "uImage";
char	_bgscreenName[16]	= "bgscreen.bmp";
char	_bgvideoName[16]	= "screen.mp4";
char	usbAppDir[24]		= "/media/usb0/app";
char	usbBootDir[24]		= "/media/usb0/boot";
char	tgtAppDir[16]		= "/root/app";
char	tgtBootDir[16]		= "/boot";


unsigned char	_devSystemStatus, _devStatus, _lastRestartReason, _timeOfDeviceRestart[6];
unsigned char	_devReliability, _devReliabilityTime[6];

UNIT		_Units[MAX_UNIT_HALF_SZ];
SUBDEVICE	_Subdevices[MAX_CH_HALF_SZ-1];
CREDENTIAL_READER	_CredentialReaders[MAX_CH_HALF_SZ];
BINARY_INPUT	_BinaryInputs[MAX_BI_HALF_SZ + MAX_XBI_HALF_SZ];
BINARY_OUTPUT	_BinaryOutputs[MAX_BO_HALF_SZ+MAX_XBO_HALF_SZ];
ACCESS_DOOR		_AccessDoors[MAX_CH_HALF_SZ];


void devInit(void)
{
	_devSystemStatus    = SS_IDLE;
	_devStatus          = 0;
	_lastRestartReason  = 0;
	_devReliability     = 0;
	rtcGetDateTime(_timeOfDeviceRestart);
	rtcGetDateTime(_devReliabilityTime);
}

void devSetRestartReason(int reason)
{
	if(_lastRestartReason != (unsigned char)reason) {
		_lastRestartReason = (unsigned char)reason;
	}
}

int devReliability(void *self)
{
	return (int)_devReliability;
}

unsigned char *devReliabilityTime(void *self)
{
	return _devReliabilityTime;
}


void *devGet(void)
{
	return (void *)sys_cfg; 
}

int devDisplayModel(void *self)
{
	if(lcdPixelWidth() < 400) return 0;
	else	return 1;
}

// 0:None  1:Local(K100-4)  2:Local(K100-2)
int devLocalUnitModel(void *self)
{
	return 0;
}

// devLocalUnitModel=0		0:Wiegand Reader  1:Wiegand+Keypad  2:Cadenatio  3:T35S  4:T35S-NonIo
// devLocalUnitModel=1,2	0:K100-4/K100-2
int devModel(void *self)
{
	return 3;
}

int devMode(void *self)
{
	return 1;
}

void devSetMode(void *self, int devMode)
{
	sys_cfg->devMode = devMode;
}

BOOL devHaveIo(void *self)
{
	return TRUE;
}

int devId(void *self)
{
	return  (int)sys_cfg->devId;
}

void devSetId(void *self, int devId)
{
	sys_cfg->devId = devId;
}

void devGetSlaveAddress(void *self, int *values)
{
	int		val;

	val = sys_cfg->devId;
	values[0] = val >> 1;
	values[1] = val & 0x01;
}

void devSetSlaveAddress(void *self, int *values)
{
	unsigned short		val;

	val = (values[0] << 1) + values[1];
	sys_cfg->devId = val;
}

void devGetDeviceName(char *name)
{
	char	*p;
    int     id, n;

	id = devId(NULL);
	p = name;
	if(id < 2) {
		strcpy(p, devLocalTitle()); p += strlen(p);
	} else {
		n = (id >> 1) - 1;
		strcpy(p, admUnitTitle()); p += strlen(p);
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}	
		*p++ = n + '0';
	}
	*p++ = '-'; *p++ = (id & 1) + '1';
	*p = 0;
}

void devGetDeviceFullName(char *name)
{
    char    *p;

    p = name;
	strcpy_chr(p, xmenu_mst_slv[1], ' ');
	p += strlen(p);
	*p++ = ' ';
	devGetDeviceName(p);
}

char *devLocalTitle(void)
{
	return xmenu_unit_model[3];
}

char *devSlaveAddressTitle(void)
{
    return xmenu_hardware[1];
}

char *devUnitTitle(void)
{
    return xmenu_hardware[10];
}

char *devChannelTitle(void)
{
    return xmenu_hardware[6];
}

char *devModelName(void *self)
{
	//if(lcdPixelWidth() > 320) return bk_model2;
	//else	return bk_model1;
	return bk_model;
}

char *devMakerName(void *self)
{
	return bk_maker;
}

char *devFirmwareName(void *self)
{
	return _firmwareName;
}

int hwObjectClass(void *self)
{
	unsigned char	*p;

	p = self;
	return (int)*p;
}

BOOL devEnableTamper(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & CR_TAMPER) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void devSetEnableTamper(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= CR_TAMPER;
	else	 sys_cfg->devOption &= ~CR_TAMPER;
}

BOOL devEnableBattery(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & BATTERY) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void devSetEnableBattery(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= BATTERY;
	else	 sys_cfg->devOption &= ~BATTERY;
}

BOOL devEnableBatteryLock(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & BATTERY_LOCK) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void devSetEnableBatteryLock(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= BATTERY_LOCK;
	else	 sys_cfg->devOption &= ~BATTERY_LOCK;
}

BOOL devEnableIntrusion(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & INTRUSION_ALARM) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void devSetEnableIntrusion(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= INTRUSION_ALARM;
	else	 sys_cfg->devOption &= ~INTRUSION_ALARM;
}

BOOL devEnableIpPhone(void *self)
{
	BOOL	bVal;

	if(sys_cfg->devOption & IP_PHONE) bVal = TRUE;
	else	bVal = FALSE;
	return bVal;
}

void devSetEnableIpPhone(void *self, BOOL bVal)
{
	if(bVal) sys_cfg->devOption |= IP_PHONE;
	else	 sys_cfg->devOption &= ~IP_PHONE;
}

int devStatus(void *self)
{
	if(_devSystemStatus || _devStatus) return 1;
	else	return 0;
}

int devSystemStatus(void *self)
{
	return (int)_devSystemStatus;
}

void devSetSystemStatus(void *self, int ss)
{
	_devSystemStatus = ss;
}

int devShutdownReason(void *self)
{
	int		val;

	if(_devSystemStatus == SS_SHUTDOWN) val = _devStatus & G_SHUTDOWN_REASON_MASK;
	else	val = -1;
	return val;
}

void devSetShutdown(void *self, int reason)
{
	_devStatus &= G_SHUTDOWN_REASON_MASK;
	_devStatus |= reason;
	_devSystemStatus = SS_SHUTDOWN;
}

int devTestMode(void *self)
{
	int		val;

	if(_devStatus & G_TEST_MODE) val = 1; else val = 0;
	return val;
}

void devSetTestMode(void *self, int val)
{
	if(val) _devStatus |= G_TEST_MODE;
	else	_devStatus &= ~G_TEST_MODE;
printf("### TestMode=%d ###\n", devTestMode(NULL));
}

extern unsigned char    commStats[];

int devMasterRestartReason(void *self)
{
	int		val;

	val = commStats[0] >> 6 & 0x03;
	return val;
}

int devAdminPass(void *self)
{
	int		val;

	val = commStats[0] >> 4 & 0x01;
	return val;
}

int devOccupancySensor(void *self)
{
	int		val;

	val = commStats[0] >> 3 & 0x01;
	return val;
}

int devScReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 7 & 0x01;
	return val;
}

int devEmReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 6 & 0x01;
	return val;
}

int devWiegandReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 5 & 0x01;
	return val;
}

int devSerialReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 4 & 0x01;
	return val;
}

int devFpReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 3 & 0x01;
	return val;
}

int devSamReaderOpened(void *self)
{
	int		val;

	val = commStats[1] >> 2 & 0x01;
	return val;
}

int EncodeDevStatus(void *buf)
{
	unsigned char	*p, c;
	
	p = (unsigned char *)buf;
    if(!tamperIsEnable(NULL)) c = 0x00;
    else if(tamperPresentValue(NULL)) c = 0x02;
	else	c = 0x01;
	*p++ = c;
	return p - (unsigned char *)buf;
} 

int EncodeStatus(void *buf)
{
	void	*ad;
	unsigned char	*p;

	p = (unsigned char *)buf;
	p += EncodeDevStatus(p);
	ad = adsGet(0);
	p += adEncodeStatus(ad, p);
	*p++ = 0;		// ad++; p += adEncodeStatus(ad, p);
	return p - (unsigned char *)buf;
}

void GetSlaveFullName(int id, char *name)
{
	char	*p;
	int		n;
	
	p = name;
	if(id < 2) {
		strcpy(p, devLocalTitle()); p += strlen(p);
	} else {
		n = (id >> 1) - 1;
		strcpy(p, devUnitTitle()); p += strlen(p);
		if(n > 9) {
			*p++ = (n / 10) + '0';
			n %= 10;
		}	
		*p++ = n + '0';
	}
	*p++ = '-'; *p++ = (id & 1) + '1';
	*p = 0;
}

void GetTargetFirmwareFileName(char *path)
{
    sprintf(path, "%s/%s.out", tgtAppDir, _firmwareName);
}

