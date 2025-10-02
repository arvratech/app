#ifndef _USER_H_
#define _USER_H_


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
	long			id;
	unsigned char	activateDate[3];
	unsigned char	expireDate[3];
	unsigned char	accessMode;
	unsigned char	cardStatus;
	unsigned char	fpSecurityLevel;
	unsigned char	reserve1[2];
	unsigned char	defaultTimeRange;
	long			accessRights;
	unsigned char	pin[4];
	unsigned char	cardData[9];
	unsigned char	fpFlag;
	unsigned char	accessPoint;
	unsigned char	zoneTo;
	unsigned char	passback;
	unsigned char	funcCode;
	unsigned char	accessTime[6];
	char			userName[22];
	char			userExternalId[14];
	unsigned short	width, height;
	unsigned char	quantityLevel;
	unsigned char	reserve3[1];
	short			jpegSize;
	unsigned char	*jpegBuffer;
	short			count;
	unsigned char	accessRules[27][2];
} FP_USER;

#define	FP_DATA_1_EXIST		1
#define	FP_DATA_2_EXIST		2

#define	FP_USER_HEAD_SZ		20

void userSetDefault(FP_USER *user);
int  userEncode(FP_USER *user, void *buf);
int  userDecode(FP_USER *user, void *buf);
int  userValidate(FP_USER *user);
long userId(void *self);
void userSetId(void *self, long id);
unsigned char *userCardData(void *self);
void userSetCardData(void *self, unsigned char *cardData);
unsigned char *userPin(void *self);
void userSetPin(void *self, unsigned char *pin);
void userEncodeId(long id, void *buf);
void userDecodeId(long *pId, void *buf);
void userEncodeFpid(unsigned long fpId, void *buf);
void userDecodeFpid(unsigned long *pFpId, void *buf);
int  userValidateId(long id);
void userId2Fpid(long id, unsigned long *pFpId);
void userFpid2Id(unsigned long fpId, long *pId);
void userSetFPData1(FP_USER *user);
void userSetFPData2(FP_USER *user);
int  userFPData1IsValid(FP_USER *user);
int  userFPData2IsValid(FP_USER *user);
int  userGetFPDataCount(FP_USER *user);
void userClearEx(FP_USER *user);
int  userEncodeEx(FP_USER *user, void *buf);
int  userDecodeEx(FP_USER *user, void *buf);
int  userValidateEx(FP_USER *user);
int  userDecodeEx(FP_USER *user, void *buf);
int  userIsNullEx(FP_USER *user);
void userClearPhoto(FP_USER *user);
int  userEncodePhoto(FP_USER *user, void *buf);
int  userDecodePhoto(FP_USER *user, void *buf);
int  userValidatePhoto(FP_USER *user);
void userClearAccessRights(FP_USER *user);
int  userEncodeAccessRights(FP_USER *user, void *buf);
int  userDecodeAccessRights(FP_USER *user, void *buf);
int  userValidateAccessRights(FP_USER *user);
void userEncodeTransaction(FP_USER *user, void *buf);
void userDecodeTransaction(FP_USER *user, void *buf);
long userGetNewID(long startID, long minID, long maxID);
int  userIsFP(FP_USER *user);
int  userIsCard(FP_USER *user);
int  userIsPIN(FP_USER *user);
void userCopy(FP_USER *userD, FP_USER *userS);
int  userCompare(FP_USER *userD, FP_USER *userS);
int  userBoundAccessRights(FP_USER *user, unsigned char *ctm);
void userCopyHead(FP_USER *user1, FP_USER *user2);
void userPrint(FP_USER *user);

int  userDelayPassback(FP_USER *user, void *cr, unsigned char *ctm);
int  userEvaluatePassback(FP_USER *user, void *cr, unsigned char *ctm);
int  userEvaluatePassbackOnExit(FP_USER *user, void *cr, unsigned char *ctm);
int  userVerifyMatchPin(FP_USER *user, unsigned char *data);
int  userVerifyMatchCard(FP_USER *user, unsigned char *data);
int  userIdentifyMatchCard(FP_USER *user, unsigned char *data);
int  userVerifyMatchFp(FP_USER *user, unsigned char *fpTemplate);
int  userIdentifyMatchFp(FP_USER *user, unsigned char *fpTemplate);


#endif

