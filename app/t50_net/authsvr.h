#ifndef _AUTH_SVR_H_
#define _AUTH_SVR_H_


void AuthIdentifyCredential(CREDENTIAL_READER *cr, FP_USER *user);
void AuthVerifyCredential(CREDENTIAL_READER *cr, FP_USER *user);
void AuthWiegand(CREDENTIAL_READER *cr);
void PinCharWiegand(int ch);
void PinStrWiegand(char *str);


#endif

