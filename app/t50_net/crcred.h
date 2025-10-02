#ifndef _CR_CRED_H_
#define _CR_CRED_H_


void cardPrint(unsigned char *buf);
void bin2card(unsigned char *data, int len, unsigned char *buf);
void bcd2card(unsigned char *data, int len, unsigned char *buf);
void digits2card(char *data, unsigned char *buf);
void digitslen2card(char *data, int len, unsigned char *buf);
void str2card(char *data, unsigned char *buf);
void strlen2card(char *data, int len, unsigned char *buf);
int  cardisdigits(unsigned char *data);

void crCapturedUserId(CREDENTIAL_READER *cr, long userId);
void crCapturedPin(CREDENTIAL_READER *cr, char *pin);
int  crCapturedCard(CREDENTIAL_READER *cr, unsigned char *data, int size);
void crClearCred(void *self);
void cardClear(CREDENTIAL_READER *cr);
int  cardPeek(CREDENTIAL_READER *cr);
int  crVerifyMatchCard(CREDENTIAL_READER *cr, unsigned char *cardData);
int  crVerifyMatchPin(CREDENTIAL_READER *cr, unsigned char *pin);
int  crIdentifyMatchCard(CREDENTIAL_READER *cr, void *user);


#endif

