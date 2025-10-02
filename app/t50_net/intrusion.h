#ifndef _INTRUSION_H_
#define _INTRUSION_H_


void tamperInit(void);
unsigned char *tamperTrackValueTime(void *self);
unsigned char *tamperChangeOfStateTime(void *self);
void tamperSetOnPresentValueChanged(void *self, void (*onPresentValueChanged)(void *));
BOOL tamperIsEnable(void *self);
void tamperSetEnable(void *self, BOOL bVal);
int  tamperTrackValue(void *self);
int  tamperPresentValue(void *self);
void TamperOnTrackValueChanged(void *self, int trackValue);
int  tamperClearPresentValue(void *self);

void intrusionInit(void);
unsigned char *intrusionTrackValueTime(void *self);
unsigned char *intrusionChangeOfStateTime(void *self);
unsigned char *intrusionChangeOfModeTime(void *self);
void intrusionSetOnValueChanged(void *self, void (*onValueChanged)(void *, int, int));
BOOL intrusionIsEnable(void *self);
void intrusionSetEnable(void *self, BOOL bVal);
int  intrusionTrackValue(void *self);
int  intrusionPresentValue(void *self);
int  intrusionMode(void *self);
void IntrusionBiAlarmStateChanged(void *self, int alarm);
int  intrusionClearPresentValue(void *self);
void intrusionDisarm(void *self);


#endif

