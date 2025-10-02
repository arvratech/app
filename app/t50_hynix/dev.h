#ifndef _DEV_H_
#define _DEV_H_

#include "NSEnum.h"

// Object Cls definitions
#define CLS_DEVICE				0
#define CLS_SUBDEVICE			1
#define CLS_UNIT				2
#define CLS_CREDENTIAL_READER	3
#define CLS_BINARY_INPUT		4
#define CLS_BINARY_OUTPUT		5
#define CLS_ACCESS_DOOR			6

// Hardware Configuration
#define MAX_UNIT_SZ				8		// Unit
#define MAX_UNIT_HALF_SZ		4		// Unit
#define MAX_CH_SZ				32
#define MAX_CH_HALF_SZ			16
#define MAX_BI_SZ				256		// 16 x 4 x 4
#define MAX_BI_HALF_SZ			128		//  8 x 4 x 4
#define MAX_BI_QUARTER_SZ		64		//  4 x 4 x 4
#define MAX_XBI_SZ				168		//  7 x 24
#define MAX_XBI_HALF_SZ			72		//  3 x 24
#define MAX_BO_SZ				128		// 16 x 4 x 2
#define MAX_BO_HALF_SZ			64		//  8 x 4 x 2
#define MAX_BO_QUARTER_SZ		32		//  4 x 4 x 2
#define MAX_XBO_SZ				84		//  7 x 12
#define MAX_XBO_HALF_SZ			36		//  3 x 12
#define MAX_AZ_SZ				23		//
#define MAX_OC_SZ				12		//

#define MAX_UNIT_XBI_SZ			24
#define MAX_UNIT_XBO_SZ			12

// ID conversion macro
#define CH2UNIT(id)			((id)>>2)
#define UNIT2CH(id)			((id)<<2)
#define IO2CH(id)			((id)&31)
#define IO2UNIT(id)			(((id)&31)>>2)
#define UNIT2SUBDEVIO(id)	((id)<<2)
#define UNIT2UNITIO(id)		(((id)<<2)+32)

#define SUBDEVBI2UNIT(id)	((id)>>4)
#define UNIT2SUBDEVBI(id)	((id)<<4)
#define UNITBI2UNIT(id)		(((id)-MAX_BI_HALF_SZ)>>4)
#define UNIT2UNITBI(id)		(((id)<<4)+MAX_BI_HALF_SZ)
#define XBI2UNIT(id)		(((id)-MAX_BI_SZ)/24+1)
#define UNIT2XBI(id)		(((id)-1)*24+MAX_BI_SZ)
#define SUBDEVBI2CH(id)		((id)>>2)
#define CH2SUBDEVBI(id)		((id)<<2)
#define UNITBI2CH(id)		(((id)-MAX_BI_HALF_SZ)>>2)
#define CH2UNITBI(id)		(((id)<<2)+MAX_BI_HALF_SZ)
#define BI2CH(id)			((id)>>2&31)
#define BI2IO(id)			((id)>>2)
#define IO2BI(id)			((id)<<2)

#define SUBDEVBO2UNIT(id)	((id)>>3)
#define UNIT2SUBDEVBO(id)	((id)<<3)
#define UNITBO2UNIT(id)		(((id)-MAX_BO_HALF_SZ)>>3)
#define UNIT2UNITBO(id)		(((id)<<3)+MAX_BO_HALF_SZ)
#define XBO2UNIT(id)		(((id)-MAX_BO_SZ)/12+1)
#define UNIT2XBO(id)		(((id)-1)*12+MAX_BO_SZ)
#define SUBDEVBO2CH(id)		((id)>>1)
#define CH2SUBDEVBO(id)		((id)<<1)
#define UNITBO2CH(id)		(((id)-MAX_BO_HALF_SZ)>>1)
#define CH2UNITBO(id)		(((id)<<1)+MAX_BO_HALF_SZ)
#define BO2CH(id)			((id)>>1&31)
#define BO2IO(id)			((id)>>1)
#define IO2BO(id)			((id)<<1)
#define CH2UNITCH(id)		((id)&3)

// systemStatus definitions
#define SS_OPERATIONAL			0
#define SS_NON_OPERATIONAL		1
#define SS_SHUTDOWN				1
#define SS_IDLE					2
#define SS_DOWNLOADING			3

// devStatus status definitions
#define G_SHUTDOWN_REASON_MASK	0x7		
#define G_WARM_RESTART			0
#define G_COLD_RESTART			1
#define G_POWER_FAIL			2
#define G_POWER_OFF				3
#define G_USER_SHUTDOWN			4
#define G_TEST_MODE				0x0010
#define G_TAMPER_PV				0x0020
#define G_TAMPER_ALARM			0x0040
#define G_FIRE_ALARM			0x0080


extern char		_firmwareName[];
extern char		_kernelName[];
extern char		_bgscreenName[];
extern char		_bgvideoName[];
extern char		usbAppDir[];
extern char		usbBootDir[];
extern char		tgtAppDir[];
extern char		tgtBootDir[];


void devInit(void);
void devSetRestartReason(int reason);
int  devReliability(void *self);
unsigned char *devReliabilityTime(void *self);
void *devGet(void);
int  devDisplayModel(void *self);
int  devModel(void *self);
int  devLocalUnitModel(void *self);
int  devMode(void *self);
void devSetMode(void *self, int devMode);
BOOL devHaveIo(void *self);
int  devId(void *self);
void devSetId(void *self, int devId);
void devGetSlaveAddress(void *self, int *values);
void devSetSlaveAddress(void *self, int *values);
void devGetDeviceName(char *name);
void devGetDeviceFullName(char *name);
char *devLocalTitle(void);
char *devSlaveAddressTitle(void);
char *devUnitTitle(void);
char *devChannelTitle(void);
char *devModelName(void *self);
char *devMakerName(void *self);
char *devFirmwareName(void *self);
int  hwObjectClass(void *self);
BOOL devEnableTamper(void *self);
void devSetEnableTamper(void *self, BOOL bVal);
BOOL devEnableBattery(void *self);
void devSetEnableBattery(void *self, BOOL bVal);
BOOL devEnableBatteryLock(void *self);
void devSetEnableBatteryLock(void *self, BOOL bVal);
BOOL devEnableIntrusion(void *self);
void devSetEnableIntrusion(void *self, BOOL bVal);
BOOL devEnableIpPhone(void *self);
void devSetEnableIpPhone(void *self, BOOL bVal);

int  devStatus(void *self);
int  devSystemStatus(void *self);
void devSetSystemStatus(void *self, int ss);
int  devShutdownReason(void *self);
void devSetShutdown(void *self, int reason);
int  devPowerLow(void *self);
void devSetPowerLow(void *self, int val);
int  devTestMode(void *self);
void devSetTestMode(void *self, int val);
int  devPowerPending(void *self);

int  devMasterRestartReason(void *self);
int  devTamperPresentValue(void *self);
int  devTamperAlarmState(void *self);
void devSetTamperAlarmState(void *self, int alarmState);
int  devAdminPass(void *self);
int  devOccupancySensor(void *self);
int  devScReaderOpened(void *self);
int  devEmReaderOpened(void *self);
int  devWiegandReaderOpened(void *self);
int  devSerialReaderOpened(void *self);
int  devFpReaderOpened(void *self);
int  devSamReaderOpened(void *self);

int  EncodeDevStatus(void *buf);
int  EncodeStatus(void *buf);
void GetSlaveFullName(int unitID, char *name);
void GetTargetFirmwareFileName(char *path);


#endif

