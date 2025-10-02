#ifndef _FIRE_CFG_H_
#define _FIRE_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


// FireZone data structure
typedef struct _FIRE_ZONE_CFG {
	unsigned char	Enable;
	unsigned char	FirePointBits[2];
	unsigned char	AccessDoorBits[3];
	unsigned char	AlarmActionBits[3];
	unsigned char	reserve[3];
} FIRE_ZONE_CFG;


_EXTERN_C void fzSetDefault(FIRE_ZONE_CFG *cfg, int ID);
_EXTERN_C int  fzEncode(FIRE_ZONE_CFG *cfg, void *buf);
_EXTERN_C int  fzDecode(FIRE_ZONE_CFG *cfg, void *buf);
_EXTERN_C int  fzValidate(FIRE_ZONE_CFG *cfg);


#endif	/* _FIRE_CFG_H_ */