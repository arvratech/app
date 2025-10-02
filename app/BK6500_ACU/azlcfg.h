#ifndef _AZL_CFG_H_
#define _AZL_CFG_H_

typedef struct _ACCESS_ZONE_LOCAL_CFG {
	unsigned char	AuthorizationMode;
	unsigned char	LocalReader;
	unsigned char	WiegandReader;	
	unsigned char	FPReader;
	unsigned char	UnregisteredAlarm;
	unsigned char	ExpiredAlarm;
	unsigned short	reserve1;
} ACCESS_ZONE_LOCAL_CFG;

void azlfgSetDefault(ACCESS_ZONE_LOCAL_CFG *azl_cfg);
int  azlcfgEncode(ACCESS_ZONE_LOCAL_CFG *azl_cfg, void *buf);
int  azlcfgDecode(ACCESS_ZONE_LOCAL_CFG *azl_cfg, void *buf);
int  azlcfgValidate(ACCESS_ZONE_LOCAL_CFG *azl_cfg);

#endif	/* _AZL_CFG_H_ */