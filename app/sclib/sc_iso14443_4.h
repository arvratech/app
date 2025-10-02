#ifndef _SC_ISO_14443_4_H
#define _SC_ISO_14443_4_H


int  iso14443_4Deselect(int cid);
int  iso14443_4AnswerToSelect(int cid);
int  iso14443_4PPS(int cid);

int  iso14443_4Deselect(int cid);
int  iso14443_4AnswerToSelect(int cid);
int  iso14443_4SelectMF(unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4SelectDF(unsigned char *aid, int len, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4SelectFile(unsigned short fileId, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4GetData(unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4GetData2(unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4GetChallenge(unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4CreateSession(int keyNum, unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4CreateSession2(int keyNum, unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4InternalAuthenticate(unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4ExternalAuthenticate(unsigned char *auth, int len, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4ReadBinary(int SFI, int offset, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4ReadBinary2(int SFI, int offset, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4SecureReadBinary(int offset, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4ReadRecord(int sfi, int recordNumber, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso14443_4ReadRecord2(int sfi, int recordNumber, unsigned char *rxBuf, int maxLen, int *rxLen);


#endif
