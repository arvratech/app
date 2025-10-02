#ifndef _AC_AD_CFG_H_
#define _AC_AD_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


typedef struct _ACCESS_DOOR_CFG {
	unsigned char	Enable;					// 0:Disable 1:Enable
	unsigned char	RequestToExitEnable;	// 0:Disable 1:Enable
	unsigned char	DoorStatusEnable;		// 0:Disable 1:Enable
	unsigned char	AuxLockEnable;			// 0:Disable 1:Enable
	unsigned char	LockType;  
	unsigned char	DoorPulseTime;
	unsigned char	TimeDelay;
	unsigned char	TimeDelayNormal;
	unsigned char	UnlockSchedule;
	unsigned char	LockSchedule;
	unsigned char	DoorOpenTooLongTime;
	unsigned char	DoorOpenTooLongEnable;	// 0:Disable 1:Enable
	unsigned char	DoorForcedOpenEnable;	// 0:Disable 1:Enable
	unsigned char	reserve[3];
} ACCESS_DOOR_CFG;


_EXTERN_C void adSetDefault(ACCESS_DOOR_CFG *cfg);
_EXTERN_C int  adEncode(ACCESS_DOOR_CFG *cfg, void *buf);
_EXTERN_C int  adDecode(ACCESS_DOOR_CFG *cfg, void *buf);
_EXTERN_C int  adDecode2(ACCESS_DOOR_CFG *cfg, void *buf);
_EXTERN_C int  adValidate(ACCESS_DOOR_CFG *cfg);
_EXTERN_C int  adGetCodedSize(void);
/*
_EXTERN_C int  adGetRequestToExitEnable(int adID);
_EXTERN_C void adSetRequestToExitEnable(int adID, int Enable);
_EXTERN_C int  adGetDoorStatusEnable(int adID);
_EXTERN_C void adSetDoorStatusEnable(int adID, int Enable);
_EXTERN_C int  adGetAuxLockEnable(int adID);
_EXTERN_C void adSetAuxLockEnable(int adID, int Enable);
_EXTERN_C int  adGetLockType(int adID);
_EXTERN_C void adSetLockType(int adID, int Type);
_EXTERN_C int  adGetOpenTooLong(int adID);
_EXTERN_C void adSetOpenTooLong(int adID, int Enable);
_EXTERN_C int  adGetForcedOpen(int adID);
_EXTERN_C void adSetForcedOpen(int adID, int Enable);
_EXTERN_C int  adGetOpenTooLongOutput(int adID);
_EXTERN_C int  adGetForcedOpenOutput(int adID);
*/


#endif	/* _AC_AD_CFG_H_ */