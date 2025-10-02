#ifndef _AC_AZ_CFG_H_
#define _AC_AZ_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


typedef struct _ACCESS_ZONE_CFG {
	unsigned char	Enable;	
	unsigned char	PassbackMode;	
	unsigned char	PassbackOption;	
	unsigned char	HardPassbackSchedule;
	unsigned char	SoftPassbackSchedule;
	unsigned char	PassbackTimeout;
	unsigned char	reserve[2];	
} ACCESS_ZONE_CFG;


_EXTERN_C int  azValidateID(long nID);
_EXTERN_C void azSetDefault(ACCESS_ZONE_CFG *cfg);
_EXTERN_C int  azEncode(ACCESS_ZONE_CFG *cfg, void *buf);
_EXTERN_C int  azDecode(ACCESS_ZONE_CFG *cfg, void *buf);
_EXTERN_C int  azValidate(ACCESS_ZONE_CFG *cfg);
_EXTERN_C int  azGetCodedSize(void);


#endif
