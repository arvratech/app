#ifndef _SCHE_CFG_H_
#define _SCHE_CFG_H_


typedef struct _HOLIDAY {
	unsigned char	year;
	char			reserve1;
	short			reserve2;
	unsigned char 	holTabs[4][46];
} HOLIDAY;

typedef struct _SCHE_CFG {
	HOLIDAY			holidays[2];
	unsigned char	timeZones[32][16];
	unsigned char	schedules[32][11];
} SCHE_CFG;

#define SCHECFG_HOL_SZ		370
#define SCHECFG_TMZ_SZ		512
#define SCHECFG_SCHE_SZ		352

extern SCHE_CFG	*sche_cfg;

int  schecfgInitialize(void);
void schecfgSetDefault(SCHE_CFG *cfg);
void schecfgGetHoliday(SCHE_CFG *cfg, unsigned char *holDate, int *holTab);
void schecfgSetHoliday(SCHE_CFG *cfg, unsigned char *HolDate, int holTab);
void schecfgClearHoliday(SCHE_CFG *cfg, unsigned char *holDate, unsigned char *curDate);
void schecfgGetTimeZone(SCHE_CFG *cfg, int tmzID, unsigned char *tmzTab);
void schecfgSetTimeZone(SCHE_CFG *cfg, int tmzID, unsigned char *tmzTab);
void schecfgClearTimeZone(SCHE_CFG *cfg, int tmzID);
void schecfgGetSchedule(SCHE_CFG *cfg, int scheID, unsigned char *scheTab);
void schecfgSetSchedule(SCHE_CFG *cfg, int scheID, unsigned char *scheTab);
void schecfgClearSchedule(SCHE_CFG *cfg, int scheID);
int  schecfgEncode(SCHE_CFG *cfg, void *buf);
int  schecfgDecode(SCHE_CFG *cfg, void *buf);
int  schecfgCompare(SCHE_CFG *cfg, SCHE_CFG *tcfg);
void schecfgCopy(SCHE_CFG *cfg, SCHE_CFG *tcfg);
int  schecfgRead(SCHE_CFG *cfg);
int  schecfgWrite(SCHE_CFG *cfg);


#endif

