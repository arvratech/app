#ifndef _AC_AZ_H_
#define _AC_AZ_H_

#include "acazcfg.h"


typedef struct _ACCESS_ZONE {
	unsigned char	ID;
	unsigned char	reserve[3];
	ACCESS_ZONE_CFG *cfg;
} ACCESS_ZONE;


void azsInit(ACCESS_ZONE *g_azs, int size);
void azsSet(ACCESS_ZONE *g_azs);
ACCESS_ZONE *azsGetAt(int azID);
void azReset(int apID);
int  azIsEntryPoint(int azID, int apID);

int  azExist(long nID);
int  azAdd(long nID);
int  azGetEncoded(long nID, unsigned char *buf);
int  azAddEncoded(long nID, unsigned char *buf);
int  azRemove(long nID);
int  azRemoveAll(void);


#endif	/* _AC_AZ_H_ */