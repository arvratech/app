#ifndef _SCHE_H_
#define _SCHE_H_


// Object Type definitions
#define SCHEDULE_TYPE		0x51
#define TIMEZONE_TYPE		0x52
#define HOLIDAY_TYPE		0x53


#define ALWAYS_VAL		99

int  holidayEncode(void *buf, unsigned char *HolDate, int HolTab);
int  holidayDecode(void *buf, unsigned char *HolDate, int *HolTab);
int  holidayValidate(unsigned char *HolDate, int HolTab, unsigned char *CurDate);
int  tmsIsNull(unsigned char *tms);
int  tmsValidate(unsigned char *tms);
int  tmsBound(unsigned char *tms, unsigned char *CurTime);
int  tmsValidateEx(unsigned char *tms);
int  tmsBoundEx(unsigned char *tms, unsigned char *CurTime);
int  tmzEncode(void *buf, int TmzID, unsigned char *TmzTab);
int  tmzDecode(void *buf, int *TmzID, unsigned char *TmzTab);
int  tmzValidateID(int TmzID);
int  tmzValidateData(int TmzID, unsigned char *TmzTab);
int  tmzValidate(int TmzID);
int  tmzBound(int TmzID, unsigned char *CurTime);
int  scheEncode(void *buf, int ScheID, unsigned char *ScheTab);
int  scheDecode(void *buf, int *ScheID, unsigned char *ScheTab);
int  scheValidateID(int ScheID);
int  scheValidateData(int ScheID, unsigned char *ScheTab);
int  scheValidate(int ScheID);
int  scheBound(int ScheID, unsigned char *CurDateTime);
int  IsHoliday(unsigned char *CurDate, int HolTable);

#endif
