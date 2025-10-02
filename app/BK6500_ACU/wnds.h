#ifndef _WIND_H
#define _WIND_H

#include "gwnd.h"
#include "glabel.h"
#include "gedit.h"
#include "gradio.h"
#include "gmenu.h"
#include "gmenuc.h"
#include "gdate.h"
#include "gipaddr.h"
#include "gipstring.h"


void WndsInitialize(void);
void WizardWndInitialize(GWND *wnd, char *Title);
int  WndSaveConfig(GWND *ParentWnd, char *Title, int Section);
int  WndSaveByte(GWND *ParentWnd, char *Title, unsigned char *pVal, int NewVal, int Section);
int  WndSaveInteger(GWND *ParentWnd, char *Title, void *pVal, int Length, int NewVal, int Section);
int  WndSaveByteData(GWND *ParentWnd, char *Title, unsigned char *pVal, int Length, unsigned char *NewData, int Section);
int  WndSaveString(GWND *ParentWnd, char *Title, char *Data, char *NewData, int Section);

int  WndEnterID(GWND *ParentWnd, char *Title, long *pID, int NewUser, int TryCount);
int  WndEnterAutoNewID(GWND *ParentWnd, char *Title, long *pID);
int  WndEnterPhoneNo(GWND *ParentWnd, char *Title, char *PhoneNo, int NewPhone, int TryCount);
int  WndEnterWildPhoneNo(GWND *ParentWnd, char *Title, char *PhoneNo, int NewPhone, int TryCount);

void MakeGuideNumberNormal(char *GuideMsg, int Min, int Max);
void MakeGuideNumberMinute(char *GuideMsg, int Min, int Max);
void MakeGuideNumberSecond(char *GuideMsg, int Min, int Max);
void MakeGuideNumberMinuteAlways(char *GuideMsg, int Max);
void MakeGuideNumberSecondAlways(char *GuideMsg, int Max);
void MakeGuideNumberMinuteUnlimited(char *GuideMsg, int Max);
void MakeGuideNumberSecondPulse(char *GuideMsg, int Max);
void MakeGuideNumberDeciSecond(char *GuideMsg, int Min, int Max);
void MakeGuideNumberCentiSecond(char *GuideMsg, int Min, int Max);
void MakeGuideNumberSchedule(char *GuideMsg);
void DrawInputDigitDesp(int x, int y);
void _DrawInputDigitDesp(GWND *wnd);
int  WndInputNumber(GWND *ParentWnd, char *Title, char *GuideMsg, void *pVal, int Length, int Min, int Max, int InvalidMsg, int Section);
int  WndInputDigitString(GWND *ParentWnd, char *Title, char *Data, int MaxLength, int Section);
int  WndInputDigitStringEx(GWND *ParentWnd, char *Title, char *Data, int MaxLength, int Stage, int End);
int  WndInputIPString(GWND *ParentWnd, char *Title, char *Data, int Section);
int  WndInputIPStringEx(GWND *ParentWnd, char *Title, char *Data, int Stage, int End);
int  WndInputAnalog(GWND *ParentWnd, char *Title, unsigned char *pVal, int Min, int Max, void (*CallBack)(), int Section);
void TimeSection2Str(unsigned char *tms, char *buf);
void DrawInputDateTimeDesp(int x, int y);
int  WndInputTimeSection(GWND *ParentWnd, char *Title, unsigned char *tms, int Section);
int  WndInputDate(GWND *ParentWnd, char *Title, unsigned char *cdate);
int  WndInputTime(GWND *ParentWnd, char *Title, unsigned char *ctime);
int  WndInputIPAddress(GWND *ParentWnd, char *Title, unsigned char *IPAddr, int SubnetMask, int Section);
void WndViewIPAddress(GWND *ParentWnd, char *Title, unsigned char *IPAddr);
int  WndCapture(GWND *ParentWnd, char *Title, CREDENTIAL_READER *cr);


#endif	/* _WNDS_H */

