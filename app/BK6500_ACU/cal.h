#ifndef _CALENDAR_H_
#define _CALENDAR_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif

#define CALENDAR_RECORD_SZ	128
#define CALENDAR_BODY_SZ	126

/*
// DateRange Structure
typedef struct _DATE_RANGE {
	unsigned char	StartDate[2];
	unsigned char	EndDate[2];	
} DATE_RANGE;

// TimeRange Structure
typedef struct _TIME_RANGE {
	unsigned char	StartTime[2];
	unsigned char	EndTime[2];	
} TIME_RANGE;

// Calendar data structure
typedef struct _CALENDAR {
	long	ObjectID;
	unsigned char	t;
	unsigned char	reserve[3];
	union {
		unsigned char	Date[3];
		DATE_RANGE		DateRange;
		unsigned char	WeekNDay[3];
    } u;
} CALENDAR;
*/

// Calendar data structure
typedef struct _CALENDAR {
	long	ID;
	int		Count;
	unsigned char	DateList[18][7];
} CALENDAR;

_EXTERN_C int  calEncodeID(long nID, void *buf);
_EXTERN_C int  calDecodeID(long *pID, void *buf);
_EXTERN_C int  calValidateID(long nID);
_EXTERN_C int  ceValidate(unsigned char *ce);
_EXTERN_C int  cePresentValue(unsigned char *ce, unsigned char *cdate);
_EXTERN_C int  dateValidate(unsigned char *date);
_EXTERN_C int  dateCompare(unsigned char *date, unsigned char *cdate);
_EXTERN_C int  weekndayValidate(unsigned char *wnd);
_EXTERN_C int  weekndayCompare(unsigned char *wnd, unsigned char *cdate);
_EXTERN_C int  dtrValidate(unsigned char *dtr);
_EXTERN_C int  dtrCompare(unsigned char *dtr, unsigned char *date);

int  calEncode(CALENDAR *cal, void *buf);
int  calDecode(CALENDAR *cal, void *buf);
int  calValidate(CALENDAR *cal);
int  calPresentValue(long calID, unsigned char *cdate);


#endif	/* _CALENDAR_H_ */