#ifndef _CR_H_
#define _CR_H_


// Credential Reader Type definitions
#define CR_TYPE_GENERAL				0
#define CR_TYPE_PRIMARY				1
#define CR_TYPE_SECONDARY			2
#define CR_TYPE_ELEVATOR			3

// Type definitions
#define CR_TYPE_CARD				1

// CardType definitions
#define CR_CARDTYPE_NONE			0
#define CR_CARDTYPE_SPI				1
#define CR_CARDTYPE_TCPIP			2

// MifareFormat definitions
#define CR_MIFAREFORMAT_SERIAL_NO	1	
#define CR_MIFAREFORMAT_SPECIAL		2	
#define CR_MIFAREFORMAT_FPONCARD	3	

// Credential Type definitions
#define CREDENTIAL_NONE				0
#define CREDENTIAL_ID				1
#define CREDENTIAL_PIN				2
#define CREDENTIAL_FP				3
#define CREDENTIAL_CARD				4
#define CREDENTIAL_AUTOKEY			5


// Credential Reader Configuration structure
typedef struct _CREDENTIAL_READER_CFG {
	unsigned char	type;
	unsigned char	readers;
	unsigned char	scFormat;
	unsigned char	wiegandFormats;
	unsigned char	authMode;
	unsigned char	ledBeep;
	unsigned char	option;
	unsigned char	reserve[1];
	unsigned short	ipReaderPort;
	unsigned char	ipReaderIpAddress[4];
	unsigned char	authorizationMode;
	unsigned char	duressAlarm;
	unsigned char	duressDigits[4];
} CREDENTIAL_READER_CFG;


// Card Reader structure
typedef struct _CREDENTIAL_READER {
	unsigned char	cls;
	unsigned char	id;
	unsigned char	status;
	unsigned char	credType;
	unsigned char	cardType;
	unsigned char	cardFormat;			// 1=SerialNo 2=SpecialCard 3=FPonCard
	unsigned char	accessEvent;
	unsigned char	accessTime[6];
	unsigned char	result;
	unsigned char	funcKey;
	unsigned char	authMode;
	unsigned char	authDone;
	unsigned char	captureMode;
	unsigned char	capturedMode;
	unsigned char	cardIndex;
	unsigned char	retryCount;
	unsigned char	sessionNo;
	unsigned char	outputWiegand;
	unsigned char   fpFlag;
	unsigned char   readerFormat;
	unsigned char   reserve[3];
	long			userId;
	char			userName[20];
	char			userExternalId[14];
	unsigned char	pin[4];
	unsigned char	cardData[10];
	int				dataLength;
	unsigned char	data[80];
	unsigned char	evtbuf[16];
	CREDENTIAL_READER_CFG	*cfg;
} CREDENTIAL_READER;


void crsInit(void);
void *crsGet(int id);
int  crsMaxSize(void);;

void _CrSetDefault(CREDENTIAL_READER_CFG *cfg);
int  _CrEncode(CREDENTIAL_READER_CFG *cfg, void *buf);
int  _CrDecode(CREDENTIAL_READER_CFG *cfg, void *buf);

int  crId(void *self);
BOOL crIsNonNull(void *self);
int  crType(void *self);
void crSetType(void *self, int type);
BOOL crIsCreatable(void *self);
BOOL crIsCreatablePrimary(void *self);
BOOL crIsCreatableSecondary(void *self);
BOOL crIsPrimary(void *self);
BOOL crIsSecondary(void *self);
void crCreate(void *self, int type);
void crDelete(void *self);
void *crAssignedDoor(void *self);

BOOL crIsEnableCardReader(void *self);
BOOL crIsEnableScReader(void *cr);
void crSetEnableScReader(void *cr, BOOL enable);
BOOL crIsEnableEmReader(void *cr);
void crSetEnableEmReader(void *cr, BOOL enable);
BOOL crIsEnableWiegandReader(void *cr);
void crSetEnableWiegandReader(void *cr, BOOL enable);
BOOL crIsEnableSerialReader(void *cr);
void crSetEnableSerialReader(void *cr, BOOL enable);
BOOL crIsEnableIpReader(void *cr);
void crSetEnableIpReader(void *cr, BOOL enable);
BOOL crIsEnablePinReader(void *cr);
void crSetEnablePinReader(void *cr, BOOL enable);
BOOL crIsEnableFpReader(void *cr);
void crSetEnableFpReader(void *cr, BOOL enable);
BOOL crIsEnableHynixEmvReader(void *self);
void crSetEnableHynixEmvReader(void *self, BOOL enable);
int  crScFormat(void *cr);
void crSetScFormat(void *cr, int scf);
BOOL crIsEnableWiegandFormat(void *cr, int wfId);
void crSetEnableWiegandFormat(void *cr, int wfId, BOOL enable);
BOOL crIsEnableCardReaders(void *cr);
int  crMainKeypad(void *cr);
int  crAuthMode(void *self);
void crSetAuthMode(void *self, int authMode);
int  crLedBeep(void *cr);
void crSetLedBeep(void *cr, int ledBeep);
void *crMatchWiegandFormat(CREDENTIAL_READER *cr, int bitLength);
int  crCheckWiegandOverlap(CREDENTIAL_READER *cr);
void crAccessTimeName(CREDENTIAL_READER *cr, char *name);
char *crName(void *self, int type);
int  crAuthorizationMode(void *self);
void crSetAuthorizationMode(void *self, int authorizationMode);
int  crDuressAlarm(void *self);
void crSetDuressAlarm(void *self, int duressAarm);
unsigned char *crDuressDigits(void *self);
void crSetDuressDigits(void *self, unsigned char *duressDigits);
int  crEncodeStatus(void *self, void *buf);
int  crResult(void *self);
void crSetResult(void *self, int result);
int  crAcessEvent(void *self);
void crSetAcessEvent(void *self, int acessEvent);
int  crCaptureMode(void *self);
void crSetCaptureMode(void *self, int captureMode);
int  crCapturedMode(void *self);
void crSetCapturedMode(void *self, int capturedMode);


#endif

