#ifndef _BIO_CFG_H_
#define _BIO_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


typedef struct _BINARY_INPUT_CFG {
	unsigned char	Polarity;
	unsigned char	TimeDelay;
	unsigned char	TimeDelayNormal;
	unsigned char	AlarmInhibit;
	unsigned char	AlarmInhibitScheID;
	unsigned char	reserve[3];
} BINARY_INPUT_CFG;

typedef struct _BINARY_OUTPUT_CFG {
	unsigned char	Polarity;
	unsigned char	reserve1;
	unsigned short	reserve2;
} BINARY_OUTPUT_CFG;


_EXTERN_C void biSetDefault(BINARY_INPUT_CFG *bi_cfg, int biID);
_EXTERN_C int  biEncode(BINARY_INPUT_CFG *bi_cfg, void *buf);
_EXTERN_C int  biDecode(BINARY_INPUT_CFG *bi_cfg, void *buf);
_EXTERN_C int  biValidate(BINARY_INPUT_CFG *bi_cfg);
_EXTERN_C int  biGetCodedSize(void);
_EXTERN_C void boSetDefault(BINARY_OUTPUT_CFG *bo_cfg);
_EXTERN_C int  boEncode(BINARY_OUTPUT_CFG *bo_cfg, void *buf);
_EXTERN_C int  boDecode(BINARY_OUTPUT_CFG *bo_cfg, void *buf);
_EXTERN_C int  boValidate(BINARY_OUTPUT_CFG *bo_cfg);
_EXTERN_C int  boGetCodedSize(void);


#endif	/* _BIO_CFG_H_ */