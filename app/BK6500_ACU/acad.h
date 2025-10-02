#ifndef _AC_AD_H_
#define _AC_AD_H_

#include "acadcfg.h"


extern unsigned long adPulseTimer[], adTimer[];
extern int			 adPulseTimeout[], adTimeout[];
extern unsigned char adState[];


typedef struct _ACCESS_DOOR {
	unsigned char	ID;
	unsigned char	PresentValue;
	unsigned char	PrevPresentValue;
	unsigned char	alarmState;
	unsigned char	ChangeOfStateTime[6];
	unsigned char	DoorAlarmStateTime[6];
	ACCESS_DOOR_CFG	*cfg;
} ACCESS_DOOR;


void adsInit(ACCESS_DOOR *g_ads, int size);
void adsSet(ACCESS_DOOR *g_ads);
ACCESS_DOOR *adsGetAt(int adID);
void adsSetCallBack(void (*StatusCallBack)(), void (*PVCallBack)());
void adsInitPresentValue(void);
void adReset(int adID);
int  adGetLockType(int adID);
void adSetLockType(int adID, int Type);
int  adGetOpenTooLong(int adID);
void adSetOpenTooLong(int adID, int Enable);
int  adGetForcedOpen(int adID);
void adSetForcedOpen(int adID, int Enable);
int  adGetPresentValue(int adID);
void adSetPresentValue(int adId, int PresentValue);
int  adAlarmState(int adID);
void adInitPresentValue(int adID);
void adRequestToExit(int adID);
void adDoorOpened(int adID);
void adDoorClosed(int adID);
void adProcessLocal(int adID);
void adRemoteDoorStatusChanged(int adID);
void adRemotePVChanged(int adID);
void adAlarmChanged(int adID);
int  adGetDoorStatus(int adID);
int  adCheckLockSchedule(int adID, unsigned char *ctm);
void adProcessLockSchedule(int adID, unsigned char *ctm);
int  adEncodeShortStatus(int adID, void *buf);
int  adShortStatus(int apID);
int  adEncodeStatus(int adID, unsigned char *Buffer);
void adNotifyStatus(int adID);
int  adsEncodeStatus(unsigned char *Buffer);


#endif	/* _AC_AD_H_ */