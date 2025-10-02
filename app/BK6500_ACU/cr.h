#ifndef _CR_H_
#define _CR_H_

#include "crcfg.h"


// CardType definitions
#define CR_CARDTYPE_NONE			0
#define CR_CARDTYPE_MIFARE			1
#define CR_CARDTYPE_EM				2
#define CR_CARDTYPE_WIEGAND			3
#define CR_CARDTYPE_SERIAL			4
#define CR_CARDTYPE_IP				5

// MifareFormat definitions
#define CR_MIFAREFORMAT_SERIAL_NO	1	
#define CR_MIFAREFORMAT_SPECIAL		2	
#define CR_MIFAREFORMAT_FPONCARD	3	

// Capture Purpose definitions
#define CR_CAPTURE_IDENTIFY			0
#define CR_CAPTURE_VERIFY			1
#define CR_CAPTURE_ENROLL			2

// Credential Type definitions
#define CREDENTIAL_NONE				0
#define CREDENTIAL_ID				1
#define CREDENTIAL_PIN				2
#define CREDENTIAL_FP				3
#define CREDENTIAL_CARD				4
#define CREDENTIAL_AUTOKEY			5


// Credential Reader Structure
typedef struct _CREDENTIAL_READER {
	unsigned char	ID;
	unsigned char	Type;
	unsigned char	Status;
	unsigned char	Reliability;
	unsigned char	ReliabilityTime[6];
	unsigned char	CapturePurpose;
	unsigned char	AuthType;
	unsigned char	Result;
	unsigned char	AccessEvent;
	unsigned char	CardIndex;
	unsigned char	CaptureMode;
	unsigned char	SessionNo;
	unsigned char	RetryCount;
	unsigned char	FuncKey;
	unsigned char	Captured;
	unsigned char	XferRetry;
	unsigned char	XferState;
	unsigned char	CliRetry;
	unsigned char	reserve1[1];
	unsigned short	Update;
	unsigned short	Client;	
	unsigned long	Timer;
	void			*nettsm;
	unsigned char	DoneMode;
	unsigned char	Retry;
	unsigned char	reserve2[1];
	unsigned char	CredentialType;
	unsigned char	readerFormat[4];
	unsigned char	Data[400];
	int				DataLength;
	unsigned char	IPAddress[4];
	unsigned char	TamperChangeOfStateTime[6];
	unsigned char	TamperAlarmStateTime[6];
	unsigned char	IntrusionChangeOfStateTime[6];
	unsigned char	IntrusionAlarmStateTime[6];
	CR_CFG			*cfg;
} CREDENTIAL_READER;


void crsInit(CREDENTIAL_READER *g_crs, int size);
void crsSet(CREDENTIAL_READER *g_crs);
CREDENTIAL_READER *crsGetAt(int crID);
void crReset(int crID);
int  crGetReliability(int crID);
void crSetReliability(int crID, int Reliability);
void crLocalTamperPVChanged(int crID, int PV);
void crProcessLocalTamper(int crID);
void crRemoteTamperPVChanged(int crID);
void crTamperAlarmChanged(int crID);
void crIntrusionPVChanged(int crID);
void crIntrusionAlarmChanged(int crID);
int  crEncodeStatus(int crID, unsigned char *Buffer);
void crNotifyStatus(int crID);
void crNotifyObjectsStatus(int ID);
int  crsEncodeStatus(unsigned char *Buffer);
int  crCapture(CREDENTIAL_READER *cr);
void crClear(CREDENTIAL_READER *cr);
int  crRead(CREDENTIAL_READER *cr, unsigned char *Data, int Size);
int  crReadWiegand(CREDENTIAL_READER *cr, unsigned char *Data);
int  crVerifyMatchCard(CREDENTIAL_READER *cr, unsigned char *Data);

void bin2card(unsigned char *data, int len, unsigned char *buf);
void bcd2card(unsigned char *data, int len, unsigned char *buf);
void digits2card(char *data, unsigned char *buf);
void str2card(char *data, unsigned char *buf);


#endif	/* _CR_H_ */