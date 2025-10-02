#ifndef _AC_AP_CFG_H_
#define _AC_AP_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


typedef struct _ACCESS_POINT_CFG {
	unsigned char	AuthorizationMode;
	unsigned char	AlarmAction;
	unsigned char	ZoneFrom;
	unsigned char	ZoneTo;
	unsigned char	PassbackTimeout;
	unsigned char	reserve[3];	
} ACCESS_POINT_CFG;


_EXTERN_C void apSetDefault(ACCESS_POINT_CFG *cfg);
_EXTERN_C int  apEncode(ACCESS_POINT_CFG *cfg, void *buf);
_EXTERN_C int  apDecode(ACCESS_POINT_CFG *cfg, void *buf);
_EXTERN_C int  apValidate(ACCESS_POINT_CFG *cfg);
_EXTERN_C int  apValidateZone(ACCESS_POINT_CFG *cfg);
_EXTERN_C int  apGetCodedSize(void);


#endif	/* _AC_AP_CFG_H_ */