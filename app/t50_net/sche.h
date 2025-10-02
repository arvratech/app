#ifndef _SCHE_H_
#define _SCHE_H_


// Object Type definitions
#define SCHEDULE_TYPE		0x51
#define TIMEZONE_TYPE		0x52
#define HOLIDAY_TYPE		0x53


#define ALWAYS_VAL		99

int  holidayEncode(void *buf, unsigned char *holDate, int holTab);
int  holidayDecode(void *buf, unsigned char *holDate, int *holTab);
int  holidayValidate(unsigned char *holDate, int holTab, unsigned char *curDate);
int  tmsIsNull(unsigned char *tms);
int  tmsValidate(unsigned char *tms);
int  tmsBound(unsigned char *tms, unsigned char *curTime);
int  tmsValidateEx(unsigned char *tms);
int  tmsBoundEx(unsigned char *tms, unsigned char *curTime);
int  tmzEncode(void *buf, int tmzID, unsigned char *tmzTab);
int  tmzDecode(void *buf, int *tmzID, unsigned char *tmzTab);
int  tmzValidateID(int tmzID);
int  tmzValidateData(int tmzID, unsigned char *tmzTab);
int  tmzValidate(int tmzID);
int  tmzBound(int tmzID, unsigned char *curTime);
int  scheEncode(void *buf, int scheID, unsigned char *scheTab);
int  scheDecode(void *buf, int *scheID, unsigned char *scheTab);
int  scheValidateID(int scheID);
int  scheValidateData(int scheID, unsigned char *scheTab);
int  scheValidate(int scheID);
int  scheBound(int scheID, unsigned char *curDateTime);
int  IsHoliday(unsigned char *curDate, int HolTable);


#endif

