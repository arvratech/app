#ifndef _CF_CFG_H_
#define _CF_CFG_H_

#include "NSEnum.h"

// Credential definition
#define CRED_INT32				0
#define CRED_INT64				1
#define CRED_FC32_CN32			2
#define CRED_BCD_DIGITS			3
#define CRED_CHARACTER_STRING	4
#define CRED_WIEGAND			9
#define CRED_PIN				10
#define CRED_ID					11

// Card format ID definition
#define CFCARD_SC_UID			0
#define CFCARD_SC_BLOCK			1
#define CFCARD_SC_FILE			2
#define CFCARD_EM				3
#define CFCARD_SERIAL			4
#define CFCARD_IP				5
#define CFCARD_WIEGAND			6

// Field coding definition
#define CF_CODING_BINARY		0
#define CF_CODING_BCD			1
#define CF_CODING_ASCII			2
#define CF_CODING_REVERSE_BINARY 3

// Wiegand parity definition
#define CF_NONE_PARITY			0
#define CF_EVEN_ODD_PARITY		1
#define CF_OTHER_PARITY			2

#define MAX_CREDENTIAL_SZ		5
#define MAX_CARD_FIELD_SZ		4
#define MAX_CF_CARD_SZ			6
#define MAX_CF_WIEGAND_SZ		8
#define MAX_CF_WIEGAND_PIN_SZ	4	


// FP Format structure
typedef struct _CF_FP_CFG {
	unsigned char	exposure;
	unsigned char	captureMode;	
	unsigned char	reserve[2];
} CF_FP_CFG;

// PIN Format structure
typedef struct _CF_PIN_CFG {
	unsigned char	outputWiegand;
	unsigned char	entryInterval;
	unsigned char	entryStopSentinel;	// 0:Unused  1:'#'
	unsigned char	maxLength;
} CF_PIN_CFG;

// Card Field structure
typedef struct _CARD_FIELD {
	unsigned char	fieldType;
	unsigned char	offset;
	unsigned char	length;
	unsigned char	reserve[1];
} CARD_FIELD;

// Card Format structure
typedef struct _CF_CARD_CFG {
	unsigned char	credential;
	unsigned char	cardType;
	unsigned char	cardApp;
	unsigned char	outputWiegand;
	unsigned char	coding;
	unsigned char	reserve[3];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
	unsigned char	data[12];
} CF_CARD_CFG;

// Wiegand Format structure
typedef struct _CF_WIEGAND_CFG {
	unsigned char	credential;
	unsigned char	outputWiegand;
	unsigned char	coding;
	unsigned char	parity;			// 0:None 1:Normal even/odd parity
	unsigned char	padding;
	unsigned char	reserve[3];
	CARD_FIELD		cardFields[MAX_CARD_FIELD_SZ];
} CF_WIEGAND_CFG;

// Wiegand PIN Format structure
typedef struct _CF_WIEGAND_PIN_CFG {
	unsigned char	parity;			// 0:None 1:Normal even/odd parity
	unsigned char	data[3];
} CF_WIEGAND_PIN_CFG;

void _CfFpSetDefault(CF_FP_CFG *cfg);
int  _CfFpEncode(CF_FP_CFG *cfg, void *buf);
int  _CfFpDecode(CF_FP_CFG *cfg, void *buf);
void _CfPinSetDefault(CF_PIN_CFG *cfg);
int  _CfPinEncode(CF_PIN_CFG *cfg, void *buf);
int  _CfPinDecode(CF_PIN_CFG *cfg, void *buf);
void _CfCardSetDefault(CF_CARD_CFG *cfg, int id, int cred);
int  _CfCardEncode(CF_CARD_CFG *cfg, int id, void *buf);
int  _CfCardDecode(CF_CARD_CFG *cfg, int id, void *buf);
void _CfWiegandSetDefault(CF_WIEGAND_CFG *cfg, int id, int cred);
int  _CfWiegandEncode(CF_WIEGAND_CFG *cfg, void *buf);
int  _CfWiegandDecode(CF_WIEGAND_CFG *cfg, void *buf);
void _CfWiegandPinSetDefault(CF_WIEGAND_PIN_CFG *cfg, int id);
int  _CfWiegandPinEncode(CF_WIEGAND_PIN_CFG *cfg, void *buf);
int  _CfWiegandPinDecode(CF_WIEGAND_PIN_CFG *cfg, void *buf);
void _CfCardSetDefaultSerialBarCode(CF_CARD_CFG *cfg);


#endif

