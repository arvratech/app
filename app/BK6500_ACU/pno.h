#ifndef _PHONE_NO_H_
#define _PHONE_NO_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define	MAX_PHONENO_SZ			8
#define MAX_PHONENO_HALF_SZ		4

#define PHONE_NO_RECORD_SZ		16

// Phone No data structure
typedef struct _PHONE_NO {
	long	ObjectID;
	char	PhoneNo[12];
	unsigned char	IPAddress[4];
} PHONE_NO;


_EXTERN_C void pnoSetDefault(PHONE_NO *pno);
_EXTERN_C int  pnoEncode(PHONE_NO *pno, void *buf);
_EXTERN_C int  pnoDecode(PHONE_NO *pno, void *buf);
_EXTERN_C int  pnoValidate(PHONE_NO *pno);
_EXTERN_C void pnoEncodeID(long nID, void *buf);
_EXTERN_C void pnoDecodeID(long *pID, void *buf);
_EXTERN_C int  pnoValidateID(long nID);
_EXTERN_C int  pnoEncodePhoneNo(char *PhoneNo, void *buf);
_EXTERN_C int  pnoDecodePhoneNo(char *PhoneNo, void *buf);
_EXTERN_C int  pnoValidatePhoneNo(char *PhoneNo);
_EXTERN_C void pnoCopy(PHONE_NO *pnoD, PHONE_NO *pnoS);
_EXTERN_C int  pnoCompare(PHONE_NO *pno1, PHONE_NO *pno2);
_EXTERN_C long pnoGetNewID(long StartID, long MinID, long MaxID);



#endif	/* _PHONE_NO_H_ */