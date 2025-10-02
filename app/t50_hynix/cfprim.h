#ifndef _CF_PRIM_H_
#define _CF_PRIM_H_


char *credTitle(void);
char *credName(int id);
char *credClassName(int classId);
void credGetDataName(unsigned char *data, char *name);
char *cardFieldTypeTitle(void);
char *cardFieldOffsetTitle(void);
char *cardFieldLengthTitle(void);
char *cardFieldTypeName(void *self);
void cardFieldGetOffsetBitName(void *self, char *name);
void cardFieldGetLengthBitName(void *self, char *name);
void cardFieldGetOffsetByteName(void *self, char *name);
void cardFieldGetLengthByteName(void *self, char *name);
char *cfCardTypeTitle(void);
char *cfCardAppTitle(void);
char *cfOutputWiegandTitle(void);
char *cfCodingTitle(void);
char *cfParityTitle(void);
char *cfPaddingTitle(void);
char *cfCardFieldsTitle(void);
char *cfReaderSettingTitle(void);
char *cfBlockNoTitle(void);
char *cfMifareKeyTitle(void);
char *cfSectorTitle(void);
char *cfBlockTitle(void);
char *cfAidTitle(void);
char *cfAddTitle(void);
char *cfSerialSpeedTitle(void);
char *cfSerialParityTitle(void);
char *cfSerialStopBitsTitle(void);
char *cfScUidCardType(int id);
char *cfScBlockCardType(int id);
char *cfScFileCardType(int id);
char *cfSerialCardType(int id);
char *cfScBlockCardAppName(int id);
char *cfScFileCardAppName(int id);
char *cfSerialCardAppName(int id);
char *cfCodingName(int id);
char *cfParityName(int id);
char *cfSerialSpeedName(int id);
char *cfSerialParityName(int id);
char *cfSerialStopBitsName(int id);

char *cfCardName(void *self);
char *cfCardCredentialName(void *self);
char *cfCardCardTypeName(void *self);
char *cfCardCardAppName(void *self);
void cfCardGetOutputWiegandName(void *self, char *name);
char *cfCardCodingName(void *self);
void cfCardGetMifareKeyName(void *self, char *name);
char *cfCardMifareKeyName(void *self);
void cfCardSetMifareKeyName(void *self, char *name);
void cfCardGetBlockNoName(void *self, char *name);
void cfCardGetAidName(void *self, char *name);
char *cfCardAidName(void *self);
void cfCardSetAidName(void *self, char *name);
char *cfCardSerialSpeedName(void *self);
char *cfCardSerialParityName(void *self);
char *cfCardSerialStopBitsName(void *self);
int  cfCardSetCredentialWithKeyValue(void *self, char *buf);
int  cfCardSetCardTypeWithKeyValue(void *self, char *buf);
int  cfCardSetCardAppWithKeyValue(void *self, char *buf);
int  cfCardSetOutputWiegandWithKeyValue(void *self, char *buf);
int  cfCardSetCodingWithKeyValue(void *self, char *buf);
int  cfCardSetBlockMifareKeyWithKeyValue(void *self, char *buf);
int  cfCardSetIso7816AidWithKeyValue(void *self, char *buf);
int  cfCardSetBlockNoWithKeyValue(void *self, char *buf);
int  cfCardSetSerialSpeedWithKeyValue(void *self, char *buf);
int  cfCardSetSerialParityWithKeyValue(void *self, char *buf);
int  cfCardSetSerialStopBitsWithKeyValue(void *self, char *buf);
void cfCardEncodeCredentialSpinner(void *self, int cfId, char *title, char *buf);
void cfCardEncodeUidCardTypeSpinner(void *self, char *title, char *buf);
void cfCardEncodeBlockCardTypeSpinner(void *self, char *title, char *buf);
void cfCardEncodeIso7816CardTypeSpinner(void *self, char *title, char *buf);
void cfCardEncodeSerialCardTypeSpinner(void *self, char *title, char *buf);
void cfCardEncodeBlockCardAppSpinner(void *self, char *title, char *buf);
void cfCardEncodeIso7816CardAppSpinner(void *self, char *title, char *buf);
void cfCardEncodeSerialCardAppSpinner(void *self, char *title, char *buf);
void cfCardEncodeOutputWiegandSpinner(void *self, char *title, char *buf);
void cfCardEncodeCodingSpinner(void *self, char *title, char *buf);
void cfCardEncodeBlockNoPicker(void *self, char *title, char *buf);
void cfCardEncodeSerialSpeedSpinner(void *self, char *title, char *buf);
void cfCardEncodeSerialParitySpinner(void *self, char *title, char *buf);
void cfCardEncodeSerialStopBitsSpinner(void *self, char *title, char *buf);

void cfWiegandGetName(void *self, char *name);
char *cfWiegandCredentialName(void *self);
void cfWiegandGetOutputWiegandName(void *self, char *name);
char *cfWiegandCodingName(void *self);
void cfWiegandGetPaddingName(void *self, char *name);
char *cfWiegandParityName(void *self);
int  cfWiegandSetCredentialWithKeyValue(void *self, char *buf);
int  cfWiegandSetParityWithKeyValue(void *self, char *buf);
int  cfWiegandSetCodingWithKeyValue(void *self, char *buf);
void cfWiegandEncodeCredentialSpinner(void *self, char *title, char *buf);
void cfWiegandEncodeParitySpinner(void *self, char *title, char *buf);
void cfWiegandEncodeCodingSpinner(void *self, char *title, char *buf);


#endif

