#ifndef _BIO_H_
#define _BIO_H_


// Binary Input type definitions
#define BI_TYPE_GENERAL				0
#define BI_TYPE_DOOR				1		// Request To Exit / Door Swicth
#define BI_TYPE_ELEVATOR			2
#define BI_TYPE_INTRUSION_ZONE		3

// Binary Output type definitions
#define BO_TYPE_GENERAL				0
#define BO_TYPE_DOOR				1		// Door Lock / Alarm Action
#define BO_TYPE_ELEVATOR_BUTTON		2


typedef struct _BINARY_INPUT_CFG {
	unsigned char	type;
	unsigned char	polarity;
	unsigned char	eventDetectionEnable;
	unsigned char	timeDelay;
	unsigned char	timeDelayNormal;
	unsigned char	alarmInhibit;
	unsigned char	alarmInhibitScheID;
	unsigned char	alarmOC;
} BINARY_INPUT_CFG;

typedef struct _BINARY_OUTPUT_CFG {
	unsigned char	type;
	unsigned char	polarity;
	unsigned char	reserve[2];
} BINARY_OUTPUT_CFG;

typedef struct _BINARY_INPUT {
	unsigned char	cls;
	unsigned char	status;
	unsigned short	id;
	unsigned char	changeOfStateTime[6];
	unsigned char	alarmStateTime[6];
	BINARY_INPUT_CFG *cfg;
} BINARY_INPUT;

typedef struct _BINARY_OUTPUT {
	unsigned char	cls;
	unsigned char	status;
	unsigned short	id;
	unsigned char	changeOfStateTime[6];
	BINARY_OUTPUT_CFG *cfg;
} BINARY_OUTPUT;


void bisInit(void);
void *bisGet(int id);
void bosInit(void);
void *bosGet(int id);

void _BiSetDefault(BINARY_INPUT_CFG *cfg);
int  _BiEncode(BINARY_INPUT_CFG *cfg, void *buf);
int  _BiDecode(BINARY_INPUT_CFG *cfg, void *buf);
int  _BiValidate(BINARY_INPUT_CFG *cfg);
int  _BiGetCodedSize(void);
void _BbSetDefault(BINARY_OUTPUT_CFG *cfg);
int  _BbEncode(BINARY_OUTPUT_CFG *cfg, void *buf);
int  _BbDecode(BINARY_OUTPUT_CFG *cfg, void *buf);
int  _BbValidate(BINARY_OUTPUT_CFG *cfg);
int  _BbGetCodedSize(void);

int  biId(void *self);
BOOL biIsNonNull(void *self);
int  biType(void *self);
BOOL biIsCreatable(void *self);
BOOL biIsCreatableDoor(void *self);
BOOL biIsDoorType(void *self);
BOOL biIsRequestToExit(void *self);
BOOL biIsDoorSwitch(void *self);
void biCreate(void *self, int type);
void biDelete(void *self);
void *biAssignedDoor(void *self);
int  biPolarity(void *self);
void biSetPolarity(void *self, int polarity);
int  biEventDetectionEnable(void *self);
void biSetEventDetectionEnable(void *self, int enable);
int  biTimeDelay(void *self);
void biSetTimeDelay(void *self, int timeDelay);
int  biTimeDelayNormal(void *self);
void biSetTimeDelayNormal(void *self, int timeDelayNormal);
int  biAlarmInhibit(void *self);
void biSetAlarmInhibit(void *self, int inhibit);
int  biPresentValue(void *self);
int  biRawAlarm(void *self);
int  biAlarm(void *self);

int  boId(void *self);
BOOL boIsNonNull(void *self);
int  boType(void *self);
BOOL boIsCreatable(void *self);
BOOL boIsCreatableDoor(void *self);
BOOL boIsDoorType(void *self);
BOOL boIsAlarmAction(void *self);
void boCreate(void *self, int type);
void boDelete(void *self);
void *boAssignedDoor(void *self);
int  boPolarity(void *self);
void boSetPolarity(void *self, int polarity);
int  boPresentValue(void *self);


#endif

