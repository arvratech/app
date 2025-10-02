#ifndef _AC_AP_H_
#define _AC_AP_H_

#include "acapcfg.h"


typedef struct _ACCESS_POINT {
	unsigned char	ID;
	unsigned char	Output;
	unsigned char	State;
	unsigned char	AccessEvent;
	unsigned char	AccessTime[8];
	unsigned char	Duress;
	unsigned char	SessionNo;
	unsigned char	AuthMode;
	unsigned char	AuthDone;
	long			UserID;
	ACCESS_POINT_CFG *cfg;
	void			*cr;
	unsigned long	MsgTimer;
} ACCESS_POINT;


void apsInit(ACCESS_POINT *g_aps, int size);
void apsSet(ACCESS_POINT *g_aps);
ACCESS_POINT *apsGetAt(int apID);
void apReset(int apID);
int  apGetAuthorizationMode(int apID);
int  apGetAccessEvent(int apID);
void apSetAccessEvent(int apID, int AccessEvent, unsigned char *ctm);
int  apGetState(int apID);
int  apEncodeStatus(int apID, void *buf);


#endif	/* _AC_AP_H_ */