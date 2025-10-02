#ifndef _AC_AR_H_
#define _AC_AR_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define ACCESS_RIGHTS_RECORD_SZ		64
#define ACCESS_RIGHTS_BODY_SZ		60


typedef struct _ACCESS_RIGHTS {
	long		ID;
	unsigned char	Accompaniment[4];
	unsigned char	DefaultTimeRange;	
	unsigned char	reserve[1];
	short			Count;
	unsigned char	AccessRules[27][2];
} ACCESS_RIGHTS;


_EXTERN_C int  arEncodeID(long nID, void *buf);
_EXTERN_C int  arDecodeID(long *pID, void *buf);
_EXTERN_C int  arValidateID(long nID);
_EXTERN_C int  arEncode(ACCESS_RIGHTS *ar, void *buf);
_EXTERN_C int  arDecode(ACCESS_RIGHTS *ar, void *buf);
_EXTERN_C int  arValidate(ACCESS_RIGHTS *ar);
_EXTERN_C int  arGetCodedSize(void);
_EXTERN_C int  aruleValidate(unsigned char *arule);

int arEvaluate(long arID, int apID, unsigned char *ctm);


#endif	/* _AC_AR_H_ */