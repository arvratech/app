#ifndef _SYS_CFG_H_
#define _SYS_CFG_H_

#include "defs.h"
#include "devcfg.h"
#include "unitcfg.h"
#include "nwpcfg.h"
#include "acadcfg.h"
#include "acapcfg.h"
#include "acazcfg.h"
#include "biocfg.h"
#include "crcfg.h"
#include "almcfg.h"
#include "firecfg.h"
#include "camcfg.h"
#include "tacfg.h"


typedef struct _sys_cfg {
	unsigned short	DeviceID;
	char			AdminPIN[10];
	DEVICE_CFG		  Device;
	NETWORK_PORT_CFG  NetworkPort;
	UNIT_CFG		  Units[MAX_UNIT_SZ];
	IOUNIT_CFG		  IOUnits[MAX_IOUNIT_SZ];
	ACCESS_DOOR_CFG	  AccessDoors[MAX_AD_SZ];
	ACCESS_POINT_CFG  AccessPoints[MAX_AP_SZ];
	ACCESS_ZONE_CFG	  AccessZones[MAX_AZ_SZ-1];
	BINARY_INPUT_CFG  BinaryInputs[MAX_BI_SZ+MAX_XBI_SZ];
	BINARY_OUTPUT_CFG BinaryOutputs[MAX_BO_SZ+MAX_XBO_SZ];
	CR_CFG			  CredentialReaders[MAX_CR_SZ];
//	unsigned char	  CRIPAddrs[MAX_CR_SZ][4];
	ALARM_ACTION_CFG  AlarmActions[MAX_UNIT_SZ];
	FIRE_ZONE_CFG	  FireZones[MAX_UNIT_HALF_SZ];
	OUTPUT_COMMAND_CFG OutputCommands[MAX_OC_SZ-2];
	unsigned char	ServerIPAddress[4];
	unsigned short	ServerPort;
	unsigned char	Passback;
	unsigned char	RepeatAccessDelay;
	unsigned char	PassbackResetTime[2];	
	unsigned char	FPIdentify;
	unsigned char	FPSecurityLevel;
	unsigned char	SCKeyVersion;
	unsigned char	SCPrimaryKey[6];
	unsigned char	SCSecondaryKey[6];
	unsigned char	SCKeyOption;
	unsigned char	reserve[2];
	CAM_CFG			Camera;
	unsigned char	InOutTimeSection[2][4];
	unsigned char	TAOption;
	unsigned char	FuncKeyTimer;
	unsigned char	TAItems[TA_ITEM_SIZE];
	unsigned char	TAVersion;
	char			TATexts[TA_TEXT_SIZE][24];
} SYS_CFG;

extern SYS_CFG	*sys_cfg;


int  syscfgInitialize(void);
void syscfgReset(SYS_CFG *sys_cfg);
void syscfgSetDefault(SYS_CFG *sys_cfg);
void _SetDefaultIPAddress(unsigned char *IPAddress);
int  syscfgEncode(SYS_CFG *sys_cfg, void *buf);
int  syscfgDecode(SYS_CFG *sys_cfg, void *buf, int size);
int  syscfgCompare(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg);
void syscfgCopy(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg);
int  syscfgRead(SYS_CFG *sys_cfg);
int  syscfgWrite(SYS_CFG *sys_cfg);
int  syscfgBackup(void *buf);
int  syscfgRecovery(void *buf, int size);
void syscfgSetUnits(SYS_CFG *sys_cfg);
void syscfgCopyToUnits(SYS_CFG *sys_cfg);
void syscfgCopyFromUnits(SYS_CFG *sys_cfg);
int  syscfgEncodeUnits(SYS_CFG *sys_cfg, void *buf);
int  syscfgDecodeUnits(SYS_CFG *sys_cfg, void *buf);
int  syscfgValidateUnits(SYS_CFG *sys_cfg);
void syscfgPrint(SYS_CFG *sys_cfg);
unsigned char *syscfgGetMACAddress(void);


#endif	/* _SYS_CFG_H_ */