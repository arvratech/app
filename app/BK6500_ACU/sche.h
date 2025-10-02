#ifndef _SCHE_H_
#define _SCHE_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define MAX_SCHEDULE_SZ		64
#define MAX_CALENDAR_SZ		4

#define ALWAYS_VAL		99

#define SCHEDULE_RECORD_SZ		256
#define SCHEDULE_BODY_SZ		254

// Schedule data structure
typedef struct _SCHEDULE {
	long	ID;
	unsigned char	EffectivePeriod[6];
	unsigned char	reserve[2];
	unsigned char	WeeklySchedule[7][16];
	int		Count;
	unsigned char	ExceptionSchedule[5][25];
} SCHEDULE;


_EXTERN_C int  scheEncodeID(long nID, void *buf);
_EXTERN_C int  scheDecodeID(long *pID, void *buf);
_EXTERN_C int  scheValidateID(long nID);
_EXTERN_C int  seValidate(unsigned char *se);
_EXTERN_C int  tmrIsNull(unsigned char *tmr);
_EXTERN_C int  tmrValidate(unsigned char *tmr);
_EXTERN_C int  tmrCompare(unsigned char *tmr, unsigned char *time);

int  scheEncode(SCHEDULE *sche, void *buf);
int  scheDecode(SCHEDULE *sche, void *buf);
int  scheValidate(SCHEDULE *sche);
int  schePresentValue(long scheID, unsigned char *ctm);
int  sePeriodPresentValue(unsigned char *se, unsigned char *cdate);

int tmsIsNull(unsigned char *tms);
int tmsValidateEx(unsigned char *tms);
int tmsBoundEx(unsigned char *tms, unsigned char *CurTime);


#endif	/* _SCHE_H_ */