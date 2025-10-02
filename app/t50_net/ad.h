#ifndef _AD_H_
#define _AD_H_


// Door PV definitions
#define PV_LOCK				0
#define PV_UNLOCK			1
#define PV_PULSE_UNLOCK		2
#define PV_EX_PULSE_UNLOCK	3
#define PV_NULL				-1


typedef struct _ACCESS_DOOR_CFG {
	unsigned char	doorPulseTime;
	unsigned char	doorExPulseTime;
	unsigned char	lockType;
	unsigned char	eventDetectionEnable;
	unsigned char	timeDelay;
	unsigned char	timeDelayNormal;
	unsigned char	doorOpenTooLongEnable;
	unsigned char	doorOpenTooLongTime;
	unsigned char	forcedOpenEnable;
	unsigned char	alarmAction;
	unsigned char	unlockScheID;
	unsigned char	lockScheID;
	unsigned char	rtePriority;
	unsigned char	reserve[3];
} ACCESS_DOOR_CFG;

typedef struct _ACCESS_DOOR {
	unsigned char	cls;
	unsigned char	id;
	unsigned char	alarmState;
	unsigned char	pv;
	unsigned char	priority;
	unsigned char	priorityMsk;
	unsigned char	priorityVal;
	unsigned char	hymo;
	unsigned char	auxLock;
	unsigned char	reserve[3];
	ACCESS_DOOR_CFG	*cfg;
} ACCESS_DOOR;


void adsInit(void);
void *adsGet(int id);
int  adsMaxSize(void);

void _AdSetDefault(ACCESS_DOOR_CFG *cfg);
int  _AdEncode(ACCESS_DOOR_CFG *cfg, void *buf);
int  _AdDecode(ACCESS_DOOR_CFG *cfg, void *buf);
int  _AdValidate(ACCESS_DOOR_CFG *cfg);

int  adId(void *self);
BOOL adIsNonNull(void *self);
BOOL adIsCreatable(void *self);
void adCreate(void *self);
void adDelete(void *self);
void *adPrimaryReader(void *self);
void *adSecondaryReader(void *self);
int  adGetCreatableIoChs(void *self, int *ioChIds);
void adChangeIoCh(void *self, int ioChId);
int  adIoSet(void *self);

int  adDoorPulseTime(void *self);
void adSetDoorPulseTime(void *self, int doorPulseTime);
int  adDoorExtendedPulseTime(void *self);
void adSetDoorExtendedPulseTime(void *self, int doorExendedPulseTime);
int  adLockType(void *self);
void adSetLockType(void *self, int lockType);
int  adEventDetectionEnable(void *self);
void adSetEventDetectionEnable(void *self, int enable);
int  adTimeDelay(void *self);
void adSetTimeDelay(void *self, int timeDelay);
int  adTimeDelayNormal(void *self);
void adSetTimeDelayNormal(void *self, int timeDelayNormal);
int  adDoorOpenTooLongEnable(void *self);
void adSetDoorOpenTooLongEnable(void *self, int enable);
int  adDoorOpenTooLong(void *self);
int  adDoorOpenTooLongTime(void *self);
void adSetDoorOpenTooLongTime(void *self, int doorOpenTooLongTime);
int  adForcedOpenEnable(void *self);
void adSetForcedOpenEnable(void *self, int enable);
int  adForcedOpen(void *self);
int  adAlarmAction(void *self);
void adSetAlarmAction(void *self, int aaId);
int  adUnlockScheID(void *self);
void adSetUnlockScheID(void *self, int unlockScheID);
int  adLockScheID(void *self);
void adSetLockScheID(void *self, int lockScheID);

int  adRtePriority(void *self);
void adSetRtePriority(void *self, int rtePriority);
void adGetPriorityArray(void *self, int *priorityArray);
void adSetPriorityArray(void *self, int *priorityArray);
int  adPresentValue(void *self);
int  adPriority(void *self);
int  adPresentValueAtPriority(void *self, int priority);
int  adSetPresentValue(void *self, int presentValue, int priority);
int  adNullifyPresentValue(void *self, int priority);
int  adModifyPresentValue(void *self, int pvId);
int  adAlarmState(void *self);
void adSetAlarmState(void *self, int alarm);
void adProcessAlarmCommand(void *self, int pv);
void adStopAlarmCommand(void *self);
int  adDoorStatus(void *self);
int  adCheckLockSchedule(void *self, unsigned char *ctm);
void adProcessLockSchedule(void *self, unsigned char *ctm);
int  adEncodeStatus(void *self, void *buf);

int  AdSetPv(void *ad, int pv, int priority);
void AdModifyPv(void *ad, int pvId);


#endif

