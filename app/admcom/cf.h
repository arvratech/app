#ifndef _CF_H_
#define _CF_H_

#include "cfcfg.h" 


// Credential Format FP structure
typedef struct _CF_FP {
	unsigned char	id;
	unsigned char	reserve[3];
	CF_FP_CFG		*cfg;
} CF_FP;

// Credential Format PIN structure
typedef struct _CF_PIN {
	unsigned char	id;
	unsigned char	reserve[3];
	CF_PIN_CFG		*cfg;
} CF_PIN;

// Credential Format Card structure
typedef struct _CF_CARD {
	unsigned char	id;
	unsigned char	reserve[3];
	void			*ctx;
	CF_CARD_CFG		*cfg;
} CF_CARD;

// Credential Format Wiegand structure
typedef struct _CF_WIEGAND {
	unsigned char	id;
	unsigned char	type;
	unsigned char	reserve[2];
	void			*ctx;
	CF_WIEGAND_CFG	*cfg;
} CF_WIEGAND;

// Credential Format Wiegand PIN structure
typedef struct _CF_WIEGAND_PIN {
	unsigned char	id;
	unsigned char	reserve[3];
	void			*ctx;
	CF_WIEGAND_PIN_CFG *cfg;
} CF_WIEGAND_PIN;


void cfsInit(void);
void *cfFpGet(void);
void cfFpCopy(void *d, void *s);
void *cfPinGet(void);
void cfPinCopy(void *d, void *s);
int  cfPinOutputWiegand(void *self);
void cfPinSetOutputWiegand(void *self, int outputWiegand);
int  cfPinEntryInterval(void *self);
void cfPinSetEntryInterval(void *self, int interval);
int  cfPinEntryStopSentinel(void *self);
void cfPinSetEntryStopSentinel(void *self, int c);
int  cfPinMaximumLength(void *self);
void cfPinSetMaximumLength(void *self, int length);
int  cfId(void *self);
int  cfPinEncodeData(void *self, unsigned char *credData, unsigned char *wgData);
void cfPinValidate(void *self);
int  cfPinValidateOutputWiegandFormat(void *self);
void *cfCardsGet(int id);
void *cfCardsAt(int index);
void *cfWiegandsGet(int id, int type);
void *cfWiegandsAt(int index, int type);
int  cfWiegandsIndexFor(int id, int type);
void *cfWiegandsAdd(void *self, int type);
void cfWiegandsRemove(int id, int type);
void cfWiegandsRemoveAt(int index, int type);
int  cfWiegandsCount(int type);
void *cfWgPinsGet(int id);

void cfCardSetDefault(void *self, int cred);
void cfCardSetDefaultSeialBarCode(void *self);
void cfCardCopy(void *d, void *s);
int  cfCardId(void *self);
int  cfCardCredential(void *self);
void cfCardSetCredential(void *self, int credential);
int  cfCardGetAvailableCredentials(void *self, int *arr);
int  cfCardCardType(void *self);
void cfCardSetCardType(void *self, int cardType);
int  cfCardCardApp(void *self);
void cfCardSetCardApp(void *self, int cardType);
int  cfCardOutputWiegand(void *self);
void cfCardSetOutputWiegand(void *self, int wfId);
int  cfCardGetAvailableOutputWiegands(void *self, int *arr);
int  cfCardCoding(void *self);
void cfCardSetCoding(void *self, int coding);
int  cfCardGetAvailableCodings(void *self, int *arr);
void *cfCardCardFieldAtIndex(void *self, int index);
int  cfCardMaximumCardFieldOffset(void *self, int fldId);
int  cfCardMinimumCardFieldLength(void *self, int fldId);
int  cfCardMaximumCardFieldLength(void *self, int fldId);
int  cfCardSerialSpeed(void *self);
void cfCardSetSerialSpeed(void *self, int speed);
int  cfCardSerialParity(void *self);
void cfCardSetSerialParity(void *self, int parity);
int  cfCardSerialStopBits(void *self);
void cfCardSetSerialStopBits(void *self, int stopBits);
unsigned char *cfCardMifareKey(void *self);
void cfCardSetMifareKey(void *self, unsigned char *mifareKey);
void cfCardGetBlockNo(void *self, int *values);
void cfCardSetBlockNo(void *self, int *values);
int  cfCardBlockNoSingle(void *self);
void cfCardSetBlockNoSingle(void *self, int value);
unsigned char *cfCardGetAid(void *self, int *aidLength);
void cfCardSetAid(void *self, unsigned char *aid, int aidLength);
void cfCardValidate(void *self);
int  cfCardValidateOutputWiegandFormat(void *self);
int  cfCardDecodeData(void *self, unsigned char *data, int dataLen, unsigned char *credData);
int  cfCardEncodedLength(void *self);

void cfWiegandSetDefault(void *self, int cred);
void cfWiegandCopy(void *d, void *s);
BOOL cfWiegandIsEnableReader(void *self);
void cfWiegandSetEnableReader(void *self, BOOL enable);
int  cfWiegandId(void *self);
void cfWiegandSetId(void *self, int id);
int  cfWiegandType(void *self);
int  cfWiegandCredential(void *self);
void cfWiegandSetCredential(void *self, int credential);
int  cfWiegandOutputWiegand(void *self);
void cfWiegandSetOutputWiegand(void *self, int wfId);
int  cfWiegandGetAvailableOutputWiegands(void *self, int *arr);
int  cfWiegandCoding(void *self);
void cfWiegandSetCoding(void *self, int coding);
int  cfWiegandGetAvailableCodings(void *self, int *arr);
int  cfWiegandParity(void *self);
void cfWiegandSetParity(void *self, int parity);
int  cfWiegandPadding(void *self);
void cfWiegandSetPadding(void *self, int padding);
void *cfWiegandCardFieldAtIndex(void *self, int index);
int  cfWiegandMaximumPadding(void *self);
int  cfWiegandMaximumCardFieldOffset(void *self, int fldId);
int  cfWiegandMinimumCardFieldLength(void *self, int fldId);
int  cfWiegandMaximumCardFieldLength(void *self, int fldId);
void cfWiegandValidate(void *self);
int  cfWiegandEncodeData(void *self, unsigned char *credData, unsigned char *wgData);
int  cfWiegandEncodedLength(void *self);

int  cardFieldType(void *self);
void cardFieldSetType(void *self, int fieldType);
int  cardFieldOffset(void *self);
void cardFieldSetOffset(void *self, int offset);
int  cardFieldLength(void *self);
void cardFieldSetLength(void *self, int length);

void cfWgPinCopy(void *d, void *s);
int  cfWgPinId(void *self);
int  cfWgPinParity(void *self);
void cfWgPinSetParity(void *self, int parity);
int  cfWgPinFacilityCode(void *self);
void cfWgPinSetFacilityCode(void *self, int facilityCode);

int  cfCardDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData);
int  cfWiegandDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData);


#endif

