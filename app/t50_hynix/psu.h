#ifndef _PSU_H_
#define _PSU_H_


void psuInit(void (*onAcAlarmStateChanged)(void), void (*onBatAlarmStateChanged)(void), void (*onBatStatusChanged)(void), void (*onPoeStatusChanged)(void));
int  psuPoeStatus(void *self);
void PsuOnPoeStatusChanged(void *self, int status);
char *psuPoeStatusName(void *self);
int  psuAcVoltageNow(void *self);
void PsuOnAcVoltageNowChanged(void *self, int volt);
int  psuAcAlarmState(void *self);
int  psuBatVoltageNow(void *self);
void PsuOnBatVoltageNowChanged(void *self, int volt);
int  psuBatAlarmState(void *self);
int  psuBatStatus(void *self);
int  psuBatCapacity(void *self);
char *psuBatStatusName(void *self);
int  psuBatPower(void);
int  PsuOnDataChanged(void);


#endif

