#ifndef _FS_USER_H_
#define _FS_USER_H_

#include "..\BootLoader\flash.h"
#include "..\BootMonitor\fs.h"
#include "user.h"
#include "pno.h"


int  fsOpenFiles(int MaxUserFileSize, int MaxTmplFileSize, void *Buffer);
int  fsInitIndex(void);

int  userfsSeek(long position);
long userfsTell(void);
int  userfsEof(void);
int  userfsRead(FP_USER *user);
int  userfsGet(FP_USER *user);
int  userfsGetCardData(FP_USER *user, unsigned char *CardData);
int  userfsGetCountCardData(unsigned char *CardData);
int  userfsGetPINData(FP_USER *user, unsigned char *PINData);
int  userfsGetEncoded(long nID, unsigned char *data);
int  userfsAddEncoded(long nID, unsigned char *data);
int  userfsReadBulk(unsigned char *UserData, int Size);
int  userfsAddBulk(unsigned char *UserData, int Size);
int  userfsRemove(long nID);
int  userfsRemoveAll(void);
int  userfsExist(long nID);
int  userfsGetCount(void);

long userfsTellFPTemplate(void);
int  userfsEofFPTemplate(void);
int  userfsGetFPTemplate(FP_USER *user, unsigned char *FPTemplate);
int  userfsGetEncodedFPTemplate(unsigned long FPID, unsigned char *buf);
int  userfsAddEncodedFPTemplate(unsigned char *buf);
int  userfsAddFPTemplate(unsigned long FPID, unsigned char *buf);
int  userfsReadBulkFPTemplate(unsigned char *FPTemplate, int Size, long *NextPosition);
int  userfsAddBulkFPTemplate(unsigned char *FPTemplate, int Size);
int  userfsRemoveFPTemplate(FP_USER *user);
int  userfsGetFPTemplateCount(void);

int  userfsSeekEx(long position);
long userfsTellEx(void);
int  userfsEofEx(void);
int  userfsGetEx(FP_USER *user);
int  userfsAddEx(FP_USER *user);
int  userfsGetExEncoded(long nID, unsigned char *data);
int  userfsAddExEncoded(long nID, unsigned char *data);
int  userfsReadBulkEx(unsigned char *Data, int Size);
int  userfsAddBulkEx(unsigned char *Data, int Size);
int  userfsRemoveAllEx(void);
int  userfsGetExCount(void);

int  userfsSeekPhoto(long position);
long userfsTellPhoto(void);
int  userfsEofPhoto(void);
int  userfsGetPhoto(FP_USER *user);
int  userfsAddPhoto(FP_USER *user);
int  userfsGetPhotoEncoded(long nID, unsigned char *data);
int  userfsAddPhotoEncoded(long nID, unsigned char *data);
int  userfsReadBulkPhoto(unsigned char *Data, int Size);
int  userfsAddBulkPhoto(unsigned char *Data, int Size);
int  userfsRemoveAllPhoto(void);
int  userfsGetPhotoCount(void);

int  userfsSeekAccessRights(long position);
long userfsTellAccessRights(void);
int  userfsEofAccessRights(void);
int  userfsGetAccessRights(FP_USER *user);
int  userfsAddAccessRights(FP_USER *user);
int  userfsGetAccessRightsEncoded(long nID, unsigned char *data);
int  userfsAddAccessRightsEncoded(long nID, unsigned char *data);
int  userfsReadBulkAccessRights(unsigned char *Data, int Size);
int  userfsAddBulkAccessRights(unsigned char *Data, int Size);
int  userfsRemoveAllAccessRights(void);
int  userfsGetAccessRightsCount(void);

int  userfsSeekTransaction(long position);
int  userfsReadTransaction(FP_USER *user);
int  userfsGetTransaction(FP_USER *user);
int  userfsAddTransaction(FP_USER *user);
int  userfsRemoveTransaction(long nID);
int  userfsRemoveAllTransaction(void);

int  fsAddEvent(unsigned char *data, int size);
int  fsReadEvent(unsigned char *data);
int  fsReadNextEvent(unsigned char *data);
int  fsShiftEvent(int size);
int  fsGetEventSize(void);
int  fsRemoveAllEvent(void);

int  pnofsSeek(long position);
long pnofsTell(void);
int  pnofsEof(void);
int  pnofsRead(PHONE_NO *pno);
int  pnofsGet(PHONE_NO *pno);
int  pnofsGetPhoneNo(PHONE_NO *pno);
int  pnofsGetEncoded(long nID, unsigned char *data);
int  pnofsAddEncoded(long nID, unsigned char *data);
int  pnofsReadBulk(unsigned char *PhoneData, int Size);
int  pnofsAddBulk(unsigned char *PhoneData, int Size);
int  pnofsRemove(PHONE_NO *pno);
int  pnofsRemovePhoneNo(PHONE_NO *pno);
int  pnofsRemoveAll(void);
int  pnofsExistPhoneNo(char *PhoneNo);
int  pnofsGetCount(void);

int  fsAddCamImage(unsigned char *data, int size);
int  fsReadCamImage(unsigned char *data);
int  fsShiftCamImage(int size);
int  fsGetCamImageSize(void);
int  fsRemoveAllCamImage(void);


#endif	/* _FS_USER_H_ */