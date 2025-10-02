#ifndef _PF_PRIM_H_
#define _PF_PRIM_H_


char *cfPinNumberTitle(void);
char *cfPinEntryIntervalTitle(void);
char *cfPinEntryStopSentinelTitle(void);
char *cfPinMaximumLengthTitle(void);
char *cfPinFacilityCodeTitle(void);
char *cfPinNumberTitle(void);

char *cfPinName(void *self);
char *cfPinOutputWiegandName(void *self);
int  cfPinSetOutputWiegandWithKeyValue(void *self, char *buf);
void cfPinEncodeOutputWiegandSpinner(void *self, char *title, char *buf);
void cfPinGetEntryIntervalName(void *self, char *name);
void cfPinGetEntryStopSentinelName(void *self, char *name);
void cfPinGetMaximumLengthName(void *self, char *name);

char *cfWgPinName(void *self);
char *cfWgPinParityName(void *self);
char *cfWgPinCodingName(void *self);
void cfWgPinGetFacilityCodeName(void *self, char *name);
char *cfWgPinFacilityCodeName(void *self);
void cfWgPinSetFacilityCodeName(void *self, char *name);
void cfWgPinGetPinNumberName(void *self, char *name);
int  cfWgPinSetParityWithKeyValue(void *self, char *buf);
int  cfWgPinSetFacilityCodeWithKeyValue(void *self, char *buf);
void cfWgPinEncodeParitySpinner(void *self, char *title, char *buf);


#endif

