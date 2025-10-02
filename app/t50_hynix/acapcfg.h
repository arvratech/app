#ifndef _ACAP_CFG_H_
#define _ACAP_CFG_H_

#include "cf.h"

#define MAX_AP_SZ		1

// supoortedCardFormats index
#define SC_READER			0
#define EM_READER			1
#define WIEGAND_READER		2
#define SERIAL_READER		3
#define IP_READER			4


typedef struct _ACCESS_POINT_CFG {
	unsigned char	authorizationMode;
	unsigned char	supportedFpFormat;
	unsigned char	supportedPinFormat;
	unsigned char	supportedCardFormats[MAX_CF_SZ-2];
	unsigned char	unregisteredAlarm;
	unsigned char	expiredAlarm;
	unsigned char	deniedAlarm;
	unsigned char	serverAuth;
	char			duressDigits[7];
	unsigned char	duressAlarm;
	unsigned char	serve[2];
} ACCESS_POINT_CFG;


void apSetDefault(ACCESS_POINT_CFG *cfg);
int  apEncode(ACCESS_POINT_CFG *cfg, void *buf);
int  apDecode(ACCESS_POINT_CFG *cfg, void *buf);
int  apValidate(ACCESS_POINT_CFG *cfg);


#endif
