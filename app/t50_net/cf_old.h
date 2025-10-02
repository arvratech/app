#ifndef _CF_H_
#define _CF_H_


#define MAX_CF_SZ		7

#define CF_SC_READER_UID		0
#define CF_SC_READER_BLOCK		1
#define CF_SC_READER_FILE		2
#define CF_EM_READER			3
#define CF_WIEGAND_READER		4
#define CF_SERIAL_READER		5
#define CF_IP_READER			6

// AuthenticationFactor formatType definition
#define FORMAT_TYPE_BINARY		0
#define FORMAT_TYPE_BCD			1
#define FORMAT_TYPE_ASCII		2

// encoding definition
#define CF_CODING_BINARY		0
#define CF_CODING_BCD			1
#define CF_CODING_ASCII			2

#define MAX_CF_WIEGAND_SZ		8
#define MAX_CARD_FIELD_SZ		4


// Card Field structure
typedef struct _CARD_FIELD {
	unsigned char	fieldType;
	unsigned char	offset;
	unsigned char	length;
	unsigned char	coding;
} CARD_FIELD;

// Credential Format FP structure
typedef struct _CF_FP {
	unsigned char	exposure;
	unsigned char	captureMode;	
	unsigned char	reserve[2];
} CF_FP;

// Credential Format PIN structure
typedef struct _CF_PIN {
	unsigned char	length;
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[2];
} CF_PIN;

// Credential Format SC UID structure
typedef struct _CF_SC_UID {
	unsigned char	scType;
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[2];
} CF_SC_UID;

// Credential Format SC Block structure
typedef struct _CF_SC_BLOCK {
	unsigned char	credential;
	unsigned char	scType;
	unsigned char	scApp;
	unsigned char	mifareKey[6];
	unsigned char	blockNumber;
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[1];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_SC_BLOCK;

// Credential Format SC ISO7816-4 structure
typedef struct _CF_SC_ISO7816 {
	unsigned char	credential;
	unsigned char	scType;
	unsigned char	scApp;
	unsigned char	aidLength;
	unsigned char	aid[10];
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[1];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_SC_ISO7816;

// Credential Format SM structure
typedef struct _CF_EM {
	unsigned char	credential;
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[2];
} CF_EM;

// Credential Format Wiegand structure
typedef struct _CF_WIEGAND {
	unsigned char	credential;
	unsigned char	parityMethod;
	unsigned char	reserve[2];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_WIEGAND;

// Credential Format Serial structure
typedef struct _CF_SERIAL {
	unsigned char	credential;
	unsigned char	cardApp;
	unsigned char	readerSetting;
	unsigned char	cfWiegandOutput;
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_SERIAL;

// Credential Format IP structure
typedef struct _CF_IP {
	unsigned char	credential;
	unsigned char	cardApp;
	unsigned char	readerIPAddress[4];
	unsigned short	readerPort;
	unsigned char	cfWiegandOutput;
	unsigned char	reserve[3];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_IP;


// Credential Formats structure
typedef struct _CFS_CFG {
	CF_FP			cfFp;
	CF_PIN			cfPin;
	CF_SC_UID		cfScUid;
	CF_SC_BLOCK		cfScBlock;
	CF_SC_ISO7816	cfScIso7816;
	CF_EM			cfEm;
	CF_WIEGAND		cfWiegands[MAX_CF_WIEGAND_SZ];
	CF_SERIAL		cfSerial;
	CF_IP			cfIp;
} CFS_CFG;


void cfsSetDefault(CFS_CFG *cfg);
int  cfsEncode(CFS_CFG *cfg, void *buf);
int  cfsDecode(CFS_CFG *cfg, void *buf);

int  cardFieldsGetFieldType(CARD_FIELD *cardFields, int id, int fieldType);
void cardFieldsSetFieldType(CARD_FIELD *cardFields, int id, int fieldType);
int  cardFieldsGetOffset(CARD_FIELD *cardFields, int id);
void cardFieldsSetOffset(CARD_FIELD *cardFields, int id, int offset);
int  cardFieldsGetLength(CARD_FIELD *cardFields, int id);
void cardFieldsSetLength(CARD_FIELD *cardFields, int id, int length);
int  cardFieldsGetCoding(CARD_FIELD *cardFields, int id);
void cardFieldsSetCoding(CARD_FIELD *cardFields, int id, int coding);
int  cfScUidGetScType(void);
void cfScUidSetScType(int scType);
int  cfScUidGetCfWiegandOutput(void);
void cfScUidSetCfWiegandOutput(int cfWiegandOutput);
int  cfScBlockGetCredential(void);
void cfScBlockSetCredential(int credential);
int  cfScBlockGetScType(void);
void cfScBlockSetScType(int scType);
int  cfScBlockGetScApp(void);
void cfScBlockSetScApp(int scApp);
unsigned char *cfScBlockGetMifareKey(void);
void cfScBlockSetMifareKey(unsigned char *mifareKey);
int  cfScBlockGetBlockNumber(void);
void cfScBlockSetBlockNumber(int blockNumber);
int  cfScBlockGetCfWiegandOutput(void);
void cfScBlockSetCfWiegandOutput(int cfWiegandOutput);
CARD_FIELD *cfScBlockGetCardFields(void);
int  cfScIso7816GetCredential(void);
void cfScIso7816SetCredential(int credential);
int  cfScIso7816GetScType(void);
void cfScIso7816SetScType(int scType);
int  cfScIso7816GetScApp(void);
void cfScIso7816SetScApp(int scApp);
int  cfScIso7816GetAidlength(void);
void cfScIso7816SetAidlength(int aidLength);
unsigned char *cfScIso7816GetAid(void);
void cfScIso7816SetAid(unsigned char *aid);
int  cfScIso7816GetCfWiegandOutput(void);
void cfScIso7816SetCfWiegandOutput(int cfWiegandOutput);
CARD_FIELD *cfScIso7816GetCardFields(void);
int  cfEmGetCredential(void);
void cfEmSetCredential(int credential);
int  cfEmGetCfWiegandOutput(void);
void cfEmSetCfWiegandOutput(int cfWiegandOutput);
int  cfWiegandGetCredential(int id);
void cfWiegandSetCredential(int id, int credential);
int  cfWiegandGetparityMethod(int id);
void cWiegandSetParityMethod(int id, int parityMethod);
CARD_FIELD *cfScWiegandGetCardFields(int id);
int  cfSerialGetCredential(void);
void cfSerialSetCredential(int credential);
int  cfSerialGetCardApp(void);
void cfSerialSetCardApp(int cardApp);
int  cfSerialGetReaderSpeed(void);
void cfSerialSetReaderSpeed(int speed);
int  cfSerialGetReaderParity(void);
void cfSerialSetReaderParity(int parity);
int  cfSerialGetReaderStopBits(void);
void cfSerialSetReaderStopBits(int stopBits);
int  cfSerialGetCfWiegandOutput(void);
void cfSerialSetCfWiegandOutput(int cfWiegandOutput);
CARD_FIELD *cfScSerialGetCardFields(void);
int  cfIpGetCredential(void);
void cfIpSetCredential(int credential);
int  cfIpGetCardApp(void);
void cfIpSetCardApp(int cardApp);
unsigned char *cfIpGetReaderIPAddress(void);
void cfIpSetReaderIPAddress(unsigned char *ipAddr);
int  cfIpGetReaderPort(void);
void cfIpSetReaderPort(int port);
int  cfIpGetCfWiegandOutput(void);
void cfIpSetCfWiegandOutput(int cfWiegandOutput);
CARD_FIELD *cfIpGetCardFields(void);


#endif
