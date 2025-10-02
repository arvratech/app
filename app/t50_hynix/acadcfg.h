#ifndef _ACAD_CFG_H_
#define _ACAD_CFG_H_


#define MAX_AD_SZ		1


typedef struct _ACCESS_DOOR_CFG {
	unsigned char	doorPulseTime;
	unsigned char	doorExPulseTime;
	unsigned char	lockType;
	unsigned char	eventDetectionEnable;
	unsigned char	timeDelay;
	unsigned char	timeDelayNormal;
	unsigned char	doorOpenTooLongEnable;
	unsigned char	doorOpenTooLongOC;
	unsigned char	doorOpenTooLongTime;
	unsigned char	forcedOpenEnable;
	unsigned char	forcedOpenOC;
	unsigned char	unlockScheID;
	unsigned char	lockScheID;
	unsigned char	reserve[3];
} ACCESS_DOOR_CFG;


void adSetDefault(ACCESS_DOOR_CFG *cfg);
int  adEncode(ACCESS_DOOR_CFG *cfg, void *buf);
int  adDecode(ACCESS_DOOR_CFG *cfg, void *buf);
int  adValidate(ACCESS_DOOR_CFG *cfg);


#endif
