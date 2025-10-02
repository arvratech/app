#ifndef _SYS_CFG_H_
#define _SYS_CFG_H_

#include "hw.h"
#include "cfcfg.h"
#include "aa.h"
#include "wpanetcfg.h"
#include "cam.h"
#include "ble.h"


typedef struct _sys_cfg {
	unsigned char	firmwareVersion[3];
	char			adminPassword[13];
	unsigned short	devId;
	unsigned char	devMode;
	unsigned char	devOption;
	unsigned char	language;
	unsigned char	dateNotation;
	unsigned char	lcdContrast;
	unsigned char	lcdBrightness;
	unsigned char	mediaOption;
	unsigned char	userSoundVolume;
	unsigned char	mediaSoundVolume;
	unsigned char	callSoundVolume;
	unsigned char	systemSoundVolume;
	unsigned char	sleepMode;
	unsigned short	sleepWaitTime;
	unsigned char	authWaitTime;
	unsigned char	authResultTime;
	unsigned char	authResultOption;
	unsigned char	fpSecurityLevel;
	unsigned char	scRfoNormalLevel;
	unsigned char	networkOption;
	unsigned char	reserved[2];
	unsigned char	dhcpIpAddress[4];
	unsigned char	ipAddress[4];
	unsigned char	subnetMask[4];
	unsigned char	gatewayIpAddress[4];
	unsigned char	dnsIpAddress[4];
	unsigned char	serverIpAddress[4];
	unsigned char	masterIpAddress[4];
	unsigned short	serverIpPort;
	unsigned short	masterIpPort;
	CAM_CFG			camera;
	BLE_CFG			ble;
	WPA_NET_CFG		wpanets[MAX_WPA_NET_SZ];
	UNIT_CFG		units[MAX_UNIT_HALF_SZ];
	SUBDEVICE_CFG		subdevices[MAX_CH_HALF_SZ-1];
	CREDENTIAL_READER_CFG	credentialReaders[MAX_CH_HALF_SZ];
	BINARY_INPUT_CFG	binaryInputs[MAX_BI_HALF_SZ + MAX_XBI_HALF_SZ];
	BINARY_OUTPUT_CFG	binaryOutputs[MAX_BO_HALF_SZ+MAX_XBO_HALF_SZ];
	ACCESS_DOOR_CFG		accessDoors[MAX_CH_HALF_SZ];
	ALARM_ACTION_CFG	alarmActions[MAX_AA_SZ];
	CF_FP_CFG			fpFormat;
	CF_PIN_CFG			pinFormat;
	CF_CARD_CFG			cardFormats[MAX_CF_CARD_SZ];
	CF_WIEGAND_CFG		wiegandFormats[MAX_CF_WIEGAND_SZ];
	CF_WIEGAND_CFG		wiegandOutputFormats[MAX_CF_WIEGAND_SZ];
	CF_WIEGAND_PIN_CFG	wiegandPinOutputFormats[MAX_CF_WIEGAND_PIN_SZ];
	unsigned char	eventOption;
	unsigned char	eventWiegand;
	unsigned char	eventOutput;
	unsigned char	otherOption;
	unsigned char	OperationMode;
	unsigned char	AcuModel;
	unsigned char	AcuCommID;
	unsigned char	InOutTimeSection[2][4];
	unsigned char	MealTimeSection[5][4];
	unsigned short	MealMenuPrice[5][4];
	unsigned char	MealMenu[5];
	unsigned char	MealOption;	
} SYS_CFG;

extern SYS_CFG	*sys_cfg;
extern char		gText[];


int  syscfgInitialize(void);
void syscfgReset(SYS_CFG *cfg);
void syscfgSetDefault(SYS_CFG *cfg);
int  syscfgCompare(SYS_CFG *cfg, SYS_CFG *tcfg);
void syscfgCopy(SYS_CFG *cfg, SYS_CFG *tcfg);
int  EncodeOperation(SYS_CFG *cfg, void *buf, int LocalUse);
int  EncodeExtraOperation(SYS_CFG *cfg, void *buf);
int  DecodeOperation(SYS_CFG *cfg, void *buf, int LocalUse);
int  DecodeExtraOperation(SYS_CFG *cfg, void *buf, int LocalUse);
int  ValidateOperation(SYS_CFG *cfg);
int  syscfgRead(SYS_CFG *cfg);
int  syscfgWrite(SYS_CFG *cfg);
int  syscfgBackup(void *buf);
int  syscfgRecovery(void *buf, int size);
void syscfgPrint(SYS_CFG *cfg);
void SetDefaultIPAddress(unsigned char *ipAddr);

unsigned char *syscfgFirmwareVersion(void *self);
char *syscfgFirmwareVersionName(void *self);
char *syscfgModel(void *self);
unsigned char *syscfgMACAddress(void *self);
int  syscfgDeviceId(void *self);
void syscfgSetDeviceId(void *self, int deviceId);
char *syscfgDeviceIdName(void *self);
void syscfgSetDeviceIdName(void *self, char *name);
BOOL syscfgDisplayDateTime(void *self);
void syscfgSetDisplayDatetime(void *self, BOOL bVal);
int  syscfgScRfoNormalLevel(void *self);
void syscfgSetScRfoNormalLevel(void *self, int val);
int  syscfgAuthWaitTime(void *self);
void syscfgSetAuthWaitTime(void *self, int val);
int  syscfgAuthResultTime(void *self);
void syscfgSetAuthResultTime(void *self, int val);
BOOL syscfgResultInhibitAuth(void *self);
void syscfgSetResultInhibitAuth(void *self, BOOL bVal);
int  syscfgSleepMode(void *self);
void syscfgSetSleepMode(void *self, int val);
int  syscfgSleepWaitTime(void *self);
void syscfgSetSleepWaitTime(void *self, int val);
char *syscfgAdminPwd(void *self);
void syscfgSetAdminPwd(void *self, char *pwd);
BOOL syscfgAdminPwdDigitsOnly(void *self);
void syscfgSetAdminPwdDigitsOnly(void *self, BOOL bVal);
int  syscfgLanguage(void *self);
void syscfgSetLanguage(void *self, int val);
int  syscfgBacklightTime(void *self);
void syscfgSetBacklightTime(void *self, int val);
int  syscfgTimeNotation(void *self);
void syscfgSetTimeNotation(void *self, int val);
int  syscfgLcdContrast(void *self);
void syscfgSetLcdContrast(void *self, int val);
int  syscfgLcdBrightness(void *self);
void syscfgSetLcdBrightness(void *self, int val);
int  syscfgUserSoundVolume(void *self);
void syscfgSetUserSoundVolume(void *self, int val);
int  syscfgMediaSoundVolume(void *self);
void syscfgSetMediaSoundVolume(void *self, int val);
int  syscfgCallSoundVolume(void *self);
void syscfgSetCallSoundVolume(void *self, int val);
int  syscfgSystemSoundVolume(void *self);
void syscfgSetSystemSoundVolume(void *self, int val);
BOOL syscfgKeypadTone(void *self);
void syscfgSetKeypadTone(void *self, BOOL bVal);
BOOL syscfgTouchSounds(void *self);
void syscfgSetTouchSounds(void *self, BOOL bVal);
int  syscfgNetworkType(void *self);
void syscfgSetNetworkType(void *self, int type);
BOOL syscfgUseDhcp(void *self);
void syscfgSetUseDhcp(void *self, BOOL bVal);
unsigned char *syscfgDhcpIpAddress(void *self);
void syscfgSetDhcpIpAddress(void *self, unsigned char *ipAddress);
unsigned char *syscfgIpAddress(void *self);
void syscfgSetIpAddress(void *self, unsigned char *ipAddress);
char *syscfgIpAddressName(void *self);
void syscfgSetIpAddressName(void *self, char *name);
unsigned char *syscfgSubnetMask(void *self);
void syscfgSetSubnetMask(void *self, unsigned char *subnetMask);
char *syscfgSubnetMaskName(void *self);
void syscfgSetSubnetMaskName(void *self, char *name);
unsigned char *syscfgGatewayIpAddress(void *self);
void syscfgSetGatewayIpAddress(void *self, unsigned char *ipAddress);
char *syscfgGatewayIpAddressName(void *self);
void syscfgSetGatewayIpAddressName(void *self, char *name);
unsigned char *syscfgDnsIpAddress(void *self);
void syscfgSetDnsIpAddress(void *self, unsigned char *ipAddress);
char *syscfgDnsIpAddressName(void *self);
void syscfgSetDnsIpAddressName(void *self, char *name);
unsigned char *syscfgServerIpAddress(void *self);
void syscfgSetServerIpAddress(void *self, unsigned char *ipAddress);
char *syscfgServerIpAddressName(void *self);
void syscfgSetServerIpAddressName(void *self, char *name);
int  syscfgServerIpPort(void *self);
void syscfgSetServerIpPort(void *self, int ipPort);
char *syscfgServerIpPortName(void *self);
void syscfgSetServerIpPortName(void *self, char *name);
unsigned char *syscfgMasterIpAddress(void *self);
void syscfgSetMasterIpAddress(void *self, unsigned char *ipAddress);
char *syscfgMasterIpAddressName(void *self);
void syscfgSetMasterIpAddressName(void *self, char *name);
int  syscfgMasterIpPort(void *self);
void syscfgSetMasterIpPort(void *self, int ipPort);
char *syscfgMasterIpPortName(void *self);
void syscfgSetMasterIpPortName(void *self, char *name);


#endif

