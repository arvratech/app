#ifndef _ACCT_H_
#define _ACCT_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define ACCT_RECORD_SZ			128


typedef struct _ACCT {
	long		id;
	char		url[66];
	char		acctName[34];
} ACCT;


_EXTERN_C void acctSetDefault(ACCT *acct);
_EXTERN_C int  acctEncode(ACCT *acct, void *buf);
_EXTERN_C int  acctDecode(ACCT *acct, void *buf);
_EXTERN_C int  acctValidate(ACCT *acct);
_EXTERN_C int  acctEncodeID(long id, void *buf);
_EXTERN_C int  acctDecodeID(long *pId, void *buf);
_EXTERN_C int  acctValidateID(long nID);
_EXTERN_C void acctCopy(ACCT *acctD, ACCT *acctS);
_EXTERN_C int  acctCompare(ACCT *acct1, ACCT *acct2);


#endif

