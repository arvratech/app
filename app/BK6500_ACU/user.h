#ifndef _USER_H_
#define _USER_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define USER_RECORD_SZ				32
#define USER_RECORD_SHIFT			5
#define USER_EX_RECORD_SZ			32
#define USER_PHOTO_RECORD_SZ		2048
#define USER_TXN_RECORD_SZ			16
#define USER_AR_RECORD_SZ			64
#define USER_AR_BODY_SZ				60
#define USER_AR_RECORD_SHIFT		6
#define EVENT_RECORD_SZ				16

// User Access Mode definitions
#define	USER_FP					0x01	// 0-bit
#define	USER_CARD				0x02	// 1-bit
#define	USER_PIN				0x04	// 2-bit
#define	USER_CARD_FP			0x08	// 3-bit
#define	USER_CARD_PIN			0x10	// 4-bit
#define	USER_CARD_FP_PIN		0x20	// 5-bit
#define	USER_FP_PIN				0x40	// 6-bit


typedef struct _FP_USER {
	long			ID;
	unsigned char	ActivateDate[3];
	unsigned char	ExpireDate[3];
	unsigned char	AccessMode;
	unsigned char	CardStatus;
	unsigned char	FPSecurityLevel;
	unsigned char	reserve1[2];
	unsigned char	DefaultTimeRange;
	long			AccessRights;
	unsigned char	PIN[4];
	unsigned char	CardData[9];
	unsigned char	FPFlag;
	unsigned char	AccessPoint;
	unsigned char	ZoneTo;
	unsigned char	Passback;
	unsigned char	FuncCode;
	unsigned char	AccessTime[6];
	char			UserName[22];
	char			UserExternalID[14];
	unsigned short	Width, Height;
	unsigned char	QuantityLevel;
	unsigned char	reserve3[1];
	short			JPEGSize;
	unsigned char	*JPEGBuffer;
	short			Count;
	unsigned char	AccessRules[27][2];
} FP_USER;

#define	FP_DATA_1_EXIST		1
#define	FP_DATA_2_EXIST		2

#define	FP_USER_HEAD_SZ		20


_EXTERN_C void userSetDefault(FP_USER *user);
_EXTERN_C int  userEncode(FP_USER *user, void *buf);
_EXTERN_C int  userDecode(FP_USER *user, void *buf);
_EXTERN_C int  userValidate(FP_USER *user);
_EXTERN_C void userEncodeID(long nID, void *buf);
_EXTERN_C void userEncodeFPID(unsigned long FPID, void *buf);
_EXTERN_C void userDecodeID(long *pID, void *buf);
_EXTERN_C void userDecodeFPID(unsigned long *pFPID, void *buf);
_EXTERN_C int  userValidateID(long nID);
_EXTERN_C void userID2FPID(long nID, unsigned long *pFPID);
_EXTERN_C void userFPID2ID(unsigned long FPID, long *pID);
_EXTERN_C void userSetFPData1(FP_USER *user);
_EXTERN_C void userSetFPData2(FP_USER *user);
_EXTERN_C int  userFPData1IsValid(FP_USER *user);
_EXTERN_C int  userFPData2IsValid(FP_USER *user);
_EXTERN_C int  userGetFPDataCount(FP_USER *user);
_EXTERN_C int  userEvaluateAccessRights(FP_USER *user, int apID, unsigned char *ctm);
_EXTERN_C int  userDelayPassback(FP_USER *user, int apID, unsigned char *ctm);
_EXTERN_C int  userEvaluatePassback(FP_USER *user, int apID, unsigned char *ctm);
_EXTERN_C int  userEvaluatePassbackOnExit(FP_USER *user, int apID, unsigned char *ctm);
_EXTERN_C void userClearEx(FP_USER *user);
_EXTERN_C int  userEncodeEx(FP_USER *user, void *buf);
_EXTERN_C int  userDecodeEx(FP_USER *user, void *buf);
_EXTERN_C int  userValidateEx(FP_USER *user);
_EXTERN_C int  userDecodeEx(FP_USER *user, void *buf);
_EXTERN_C int  userIsNullEx(FP_USER *user);
_EXTERN_C void userClearPhoto(FP_USER *user);
_EXTERN_C int  userEncodePhoto(FP_USER *user, void *buf);
_EXTERN_C int  userDecodePhoto(FP_USER *user, void *buf);
_EXTERN_C int  userValidatePhoto(FP_USER *user);
_EXTERN_C void userClearAccessRights(FP_USER *user);
_EXTERN_C int  userEncodeAccessRights(FP_USER *user, void *buf);
_EXTERN_C int  userDecodeAccessRights(FP_USER *user, void *buf);
_EXTERN_C int  userValidateAccessRights(FP_USER *user);
_EXTERN_C void userEncodeTransaction(FP_USER *user, void *buf);
_EXTERN_C void userDecodeTransaction(FP_USER *user, void *buf);
_EXTERN_C long userGetNewID(long StartID, long MinID, long MaxID);
_EXTERN_C int  userIsFP(FP_USER *user);
_EXTERN_C int  userIsCard(FP_USER *user);
_EXTERN_C int  userIsPIN(FP_USER *user);
_EXTERN_C void userCopy(FP_USER *userD, FP_USER *userS);
_EXTERN_C int  userCompare(FP_USER *userD, FP_USER *userS);
_EXTERN_C int  userBoundAccessRights(FP_USER *user, unsigned char *ctm);
_EXTERN_C void userCopyHead(FP_USER *user1, FP_USER *user2);
_EXTERN_C void userPrint(FP_USER *user);

_EXTERN_C int  userVerifyMatchPIN(FP_USER *user, unsigned char *Data);
_EXTERN_C int  userVerifyMatchCard(FP_USER *user, unsigned char *Data);
_EXTERN_C int  userIdentifyMatchCard(FP_USER *user, unsigned char *Data);
_EXTERN_C int  userVerifyMatchFP(FP_USER *user, unsigned char *FPTemplate);
_EXTERN_C int  userIdentifyMatchFP(FP_USER *user, unsigned char *FPTemplate);


#endif	/* _USER_H_ */
