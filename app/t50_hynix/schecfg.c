#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include "dirent.h"
#include "defs.h"
#include "prim.h"
#include "msg.h"
#include "rtc.h"
#include "schecfg.h"

SCHE_CFG	*sche_cfg, _sche_cfg;

static char schecfgFileName[16] = "schecfg.dat";


int schecfgInitialize(void)
{	
	DIR 	*dir;
	int		i;

	sche_cfg = &_sche_cfg;
	schecfgSetDefault(sche_cfg);
	schecfgRead(sche_cfg);
	return 0;
}

void schecfgSetDefault(SCHE_CFG *cfg)
{
	unsigned char	temp[4];

	memset(cfg, 0, sizeof(SCHE_CFG));
	rtcGetDate(temp);
	schecfgClearHoliday(cfg, NULL, temp);
	schecfgClearTimeZone(cfg, 0);
	schecfgClearSchedule(cfg, 0);
}

void schecfgGetHoliday(SCHE_CFG *cfg, unsigned char *holDate, int *holTab)
{
	HOLIDAY	*h;
	
	if(cfg->holidays[0].year == holDate[0]) h = &cfg->holidays[0];
	else	h = &cfg->holidays[1];
	*holTab = 0;
	if(is_holiday(h->holTabs[0], (int)holDate[0], (int)holDate[1], (int)holDate[2])) *holTab |= 0x01;
	if(is_holiday(h->holTabs[1], (int)holDate[0], (int)holDate[1], (int)holDate[2])) *holTab |= 0x02;
	if(is_holiday(h->holTabs[2], (int)holDate[0], (int)holDate[1], (int)holDate[2])) *holTab |= 0x04;
	if(is_holiday(h->holTabs[3], (int)holDate[0], (int)holDate[1], (int)holDate[2])) *holTab |= 0x08;
}

void schecfgSetHoliday(SCHE_CFG *cfg, unsigned char *holDate, int holTab)
{
	HOLIDAY	*h;
	
	if(cfg->holidays[0].year == holDate[0]) h = &cfg->holidays[0];
	else	h = &cfg->holidays[1];
	if(holTab & 0x01) set_holiday(h->holTabs[0], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	else			clear_holiday(h->holTabs[0], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	if(holTab & 0x02) set_holiday(h->holTabs[1], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	else			clear_holiday(h->holTabs[1], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	if(holTab & 0x04) set_holiday(h->holTabs[2], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	else			clear_holiday(h->holTabs[2], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	if(holTab & 0x08) set_holiday(h->holTabs[3], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	else			clear_holiday(h->holTabs[3], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
}

void schecfgClearHoliday(SCHE_CFG *cfg, unsigned char *holDate, unsigned char *curDate)
{
	HOLIDAY		*h;
	
	if(!holDate || !holDate[0] && !holDate[1] && !holDate[2] && !holDate[3]) {
		h = &cfg->holidays[0];
		h->year = curDate[0];
		memset(h->holTabs, 0, 184);
		h = &cfg->holidays[1];
		h->year = curDate[0] + 1;
		memset(h->holTabs, 0, 184);
	} else {	
		if(cfg->holidays[0].year == holDate[0]) h = &cfg->holidays[0];
		else	h = &cfg->holidays[1];
		clear_holiday(h->holTabs[0], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
		clear_holiday(h->holTabs[1], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
		clear_holiday(h->holTabs[2], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
		clear_holiday(h->holTabs[3], (int)holDate[0], (int)holDate[1], (int)holDate[2]);
	}
}

void schecfgGetTimeZone(SCHE_CFG *cfg, int tmzID, unsigned char *tmzTab)
{
	memcpy(tmzTab, cfg->timeZones[tmzID-1], 16);
}

void schecfgSetTimeZone(SCHE_CFG *cfg, int tmzID, unsigned char *tmzTab)
{
	memcpy(cfg->timeZones[tmzID-1], tmzTab, 16); 
}

// tmzID=0 clear all Timezone
void schecfgClearTimeZone(SCHE_CFG *cfg, int tmzID)
{
	if(tmzID <= 0) memset(cfg->timeZones, 0xff, SCHECFG_TMZ_SZ);
	else	memset(cfg->timeZones[tmzID-1], 0xff, 16); 
}

void schecfgGetSchedule(SCHE_CFG *cfg, int scheID, unsigned char *scheTab)
{
	memcpy(scheTab, cfg->schedules[scheID-1], 11);
}

void schecfgSetSchedule(SCHE_CFG *cfg, int scheID, unsigned char *scheTab)
{
	memcpy(cfg->schedules[scheID-1], scheTab, 11); 
}

// scheID=0 clear all Schedule
void schecfgClearSchedule(SCHE_CFG *cfg, int scheID)
{
	if(scheID <= 0) memset(cfg->schedules, 0xff, SCHECFG_SCHE_SZ);
	memset(cfg->schedules[scheID-1], 0xff, 11); 
}

int schecfgEncode(SCHE_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	char2bcd(cfg->holidays[0].year, p); p++;
	memcpy(p, cfg->holidays[0].holTabs, 184); p += 184;
	char2bcd(cfg->holidays[1].year, p); p++;
	memcpy(p, cfg->holidays[1].holTabs, 184); p += 184;
 	bin2bcd((unsigned char *)cfg->timeZones, SCHECFG_TMZ_SZ, p); p += SCHECFG_TMZ_SZ;
 	memcpy(p, cfg->schedules, SCHECFG_SCHE_SZ); p += SCHECFG_SCHE_SZ;
 	return p - (unsigned char *)buf;
}

int schecfgDecode(SCHE_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		c, i;

	p = (unsigned char *)buf;
	bcd2char(p, &cfg->holidays[0].year); p++;
	memcpy(cfg->holidays[0].holTabs, p, 184); p += 184;
	bcd2char(p, &cfg->holidays[1].year); p++;
	memcpy(cfg->holidays[1].holTabs, p, 184); p += 184;
 	bcd2bin(p, SCHECFG_TMZ_SZ, (unsigned char *)cfg->timeZones); p += SCHECFG_TMZ_SZ; 
 	memcpy(cfg->schedules, p, SCHECFG_SCHE_SZ); p += SCHECFG_SCHE_SZ;
 	return p - (unsigned char *)buf;
}

int schecfgCompare(SCHE_CFG *cfg, SCHE_CFG *tcfg)
{
	if(!memcmp(cfg, tcfg, sizeof(SCHE_CFG))) return 0;
	else	return -1;
}

void schecfgCopy(SCHE_CFG *cfg, SCHE_CFG *tcfg)
{
	memcpy(cfg, tcfg, sizeof(SCHE_CFG));
}

FILE *_OpenSIB(char *fileName);
void _DeleteSIB(char *fileName);

int schecfgRead(SCHE_CFG *cfg)
{
	FILE		*fp;
	unsigned char	buf[1024];
	int		rval;

	fp = _OpenSIB(schecfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fread(buf, 1, 1024, fp);
//printf("fsRead(%d)=%d fsErrno=%d\n", (int)f->file, rval, fsErrno);
	if(rval >= SCHECFG_HOL_SZ+SCHECFG_TMZ_SZ+SCHECFG_SCHE_SZ) {
		rval = schecfgDecode(cfg, buf);
printf("schecfgDecode=%d\n", rval);
	}
	fclose(fp);
}

int schecfgWrite(SCHE_CFG *cfg)
{
	FILE		*fp;
	unsigned char	buf[1024];
	int		rval, size;

	size = schecfgEncode(cfg, buf);
printf("schecfgEncode=%d\n", size);
	fp = _OpenSIB(schecfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fwrite(buf, 1, size, fp);
//printf("fsWrite(%d)=%d\n",  size, rval);
	if(rval != size) {
#ifdef BK_DEBUG
		printf("fsWrite(%d) error\n", (int)f[section-1]->file);
#endif
		rval = -1;
	} else	rval = 0;
	fclose(fp);
	sync();
	return rval;
}

