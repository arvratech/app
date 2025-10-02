#ifndef _FIRE_H_
#define _FIRE_H_

#include "firecfg.h"


typedef struct _FIRE_POINT {
	unsigned char	ID;
	unsigned char	PresentValue;
	unsigned char	TrackingValue;
	unsigned char	reserve[1];
	unsigned char	ChangeOfStateTime[6];
} FIRE_POINT;

typedef struct _FIRE_ZONE {
	unsigned char	ID;
	unsigned char	PresentValue;
	unsigned char	Silenced;	
	unsigned char	reserve[3];
	unsigned char	ChangeOfStateTime[6];
	FIRE_ZONE_CFG	*cfg;
} FIRE_ZONE;


void fpsInit(FIRE_POINT *g_fps, int size);
void fpsSet(FIRE_POINT *g_fps);
FIRE_POINT *fpsGetAt(int ID);
void fpReset(int ID);
int  fpIsEnable(int ID);
int  fpEncodeStatus(int ID, void *buf);
int  fpGetPresentValue(int ID);
void fpSetPresentValue(int ID, int PresentValue, unsigned char *ctm);
int  fpGetTrackingValue(int ID);
void fpSetTrackingValue(int ID, int TrackingValue);
void fzsInit(FIRE_ZONE *g_fzs, int size);
void fzsSet(FIRE_ZONE *g_fzs);
FIRE_ZONE *fzsGetAt(int ID);
void fzReset(int ID);
int  fzGetEnable(int fzID);
void fzSetEnable(int fzID, int Enable);
int  fzGetFirePoint(int ID, int fpID);
void fzSetFirePoint(int ID, int fpID, int Enable);
int  fzGetAccessDoor(int ID, int adID);
void fzSetAccessDoor(int ID, int adID, int Enable);
int  fzGetAlarmAction(int ID, int actID);
void fzSetAlarmAction(int ID, int actID, int Enable);
int  fzGetPresentValue(int ID);
void fzSetPresentValue(int ID, int PresentValue, unsigned char *ctm);
int  fzGetTrackingValue(int ID);
int  fzGetSilenced(int fzID);
void fzSetSilenced(int ID, int Silenced, unsigned char *ctm);
int  fzEncodeStatus(int ID, void *buf);
void fzNotifyStatus(int ID);
int  fzsEncodeStatus(unsigned char *Buffer);
int  fpGetFireZonePresentValue(int fpID);
int  adGetFireZoneUnsilenced(int adID);
int  actGetFireZoneUnsilenced(int actID);


#endif	/* _FIRE_H_ */