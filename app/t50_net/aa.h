#ifndef _AC_H_
#define _AC_H_


// Alarm Command index definitions
#define AC_INDEX_BI1				0
#define AC_INDEX_BI2				1
#define AC_INDEX_BI3				2
#define AC_INDEX_BI4				3
#define AC_INDEX_FIRE_ALARM			4
#define AC_INDEX_DOOR_OPEN_TOO_LONG	5
#define AC_INDEX_DOOR_FORCED_OPEN	6
#define AC_INDEX_UNREGISTERED		7
#define AC_INDEX_EXPIRED			8
#define AC_INDEX_PASSBACK_DETECTED	9
#define AC_INDEX_ACCESS_DENIED		10
#define AC_INDEX_RESERVE			11


#define MAX_AA_SZ				8

#define MAX_ACTION_COMMAND_SZ	12

typedef struct _ACTION_COMMAND {
	unsigned char	enable;
	unsigned char	onTime;
	unsigned char	offTime;
	unsigned char	repeatCount;
} ACTION_COMMAND;

// Alarm Action Configuration structure
typedef struct _ALARM_ACTION_CFG {
	unsigned char	enable;
	unsigned char	reserve[3];
	ACTION_COMMAND	actionCommands[MAX_ACTION_COMMAND_SZ];
} ALARM_ACTION_CFG;

// Alarm Action structure
typedef struct _ALARM_ACTION {
	unsigned char	id;
	unsigned char	reserve[3];
	ALARM_ACTION_CFG	*cfg;
} ALARM_ACTION;


void *aasGet(int id);
void *aasAt(int index);
int  aasIndexFor(int id);
void *aasAdd(void *self);
void aasRemove(int id);
void aasRemoveAt(int index);
int  aasGetList(void *objs[]);
int  aasCount(void);

void _AaSetDefault(ALARM_ACTION_CFG *cfg);
int  _AaEncode(ALARM_ACTION_CFG *cfg, void *buf);
int  _AaDecode(ALARM_ACTION_CFG *cfg, void *buf);
int  _AaValidate(ALARM_ACTION_CFG *cfg);

int  aaId(void *self);
void aaSetDefault(void *self);
void aaCopy(void *d, void *s);
void *aaActionCommandAtIndex(void *self, int index);
int  aaEnableAtIndex(void *self, int index);
void aaSetEnableAtIndex(void *self, int index, int enable);
int  aaOnTimeAtIndex(void *self, int index);
void aaSetOnTimeAtIndex(void *self, int index, int onTime);
int  aaOffTimeAtIndex(void *self, int index);
void aaSetOffTimeAtIndex(void *self, int index, int offTime);
int  aaRepeatCountAtIndex(void *self, int index);
void aaSetRepeatCountAtIndex(void *self, int index, int repeatCount);
int  aaacEnable(void *self);
void aaacSetEnable(void *self, int enable);
int  aaacOnTime(void *self);
void aaacSetOnTime(void *self, int onTime);
int  aaacOffTime(void *self);
void aaacSetOffTime(void *self, int offTime);
int  aaacRepeatCount(void *self);
void aaacSetRepeatCount(void *self, int repeatCount);


#endif

