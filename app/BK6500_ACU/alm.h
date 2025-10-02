#ifndef _ALM_H_
#define _ALM_H_

#include "almcfg.h"


typedef struct _ALARM_ACTION {
	unsigned char	ID;
	unsigned char	PresentValue;
	unsigned char	Count;
	unsigned char	Loop;
	unsigned char	OutputCommand;
	unsigned char	reserve[3];
	unsigned long	OutputTimer;
	int				OutputTimeout;
	ALARM_ACTION_CFG *cfg;
} ALARM_ACTION;


void actsInit(ALARM_ACTION *g_acts, int size);
void actsSet(ALARM_ACTION *g_acts);
ALARM_ACTION *actsGetAt(int ID);
void actReset(int ID);
void actSetPresentValue(int ID, int PresentValue);
int  actGetOutputCommand(int ID, int Index);
void actProcess(int ID);

int  ocExist(long nID);
int  ocGetEncoded(long nID, unsigned char *buf); 
int  ocAddEncoded(long nID, unsigned char *buf); 
int  ocRemove(long nID);
int  ocRemoveAll(void);


#endif	/* _ALM_H_ */