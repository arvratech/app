#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "lang.h"
#include "msg.h"
#include "user.h"
#include "syscfg.h"
#include "viewprim.h"
#include "audio.h"
#include "cr.h"
#include "topprim.h"

extern int		gIOStatus;


void KeyBeep(void)
{
	unsigned char	msg[4];

	audioFlush();
	msg[0] = 124; msg[1] = sys_cfg->systemSoundVolume; msg[2] = 0;
	audioPost(msg);
}

void CardCaptureBeep(void)
{
	unsigned char	msg[4];

	audioFlush();
	msg[0] = 122; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
	audioPost(msg);
}

void FPCaptureBeep(void)
{
	unsigned char	msg[4];

	audioFlush();
	msg[0] = 123; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
	audioPost(msg);
}

void MentTamperAlarm(void)
{
	unsigned char	msg[4];

	audioFlush();
	audioSetAlarm();
	msg[0] = 125; msg[1] = 99; msg[2] = 1;
	audioPost(msg);
}

void MentOpenTooLongAlarm(void)
{
	unsigned char	msg[4];
	int		i;

	audioFlush();
	audioSetAlarm();
	msg[0] = 158; msg[1] = 99; msg[2] = 0;
	for(i = 0;i < 3;i++) audioPost(msg);
}

void MentForcedOpenAlarm(void)
{
	unsigned char	msg[4];
	int		i;

	audioFlush();
	audioSetAlarm();
	msg[0] = 159; msg[1] = 99; msg[2] = 0;
	for(i = 0;i < 3;i++) audioPost(msg);
}

int GetCapturePromptMent(int mode)
{
	int		no;
	
	if(mode == USER_CARD)						  no = 103;
	else if(mode == USER_FP)					  no = 104;
	else if(mode == USER_PIN)					  no = 105;
	else if(mode == (USER_CARD|USER_FP))		  no = 106;
	else if(mode == (USER_CARD|USER_PIN))		  no = 107;
	else if(mode == (USER_FP|USER_PIN))			  no = 108;
	else if(mode == (USER_CARD|USER_FP|USER_PIN)) no = 109;
	else	no = 0;
	return no;
}

void MentUserNormal(int mentNo)
{
	unsigned char	msg[4];

	if(GetLanguage() == LANG_ENG && mentNo >= 100 && mentNo < 120) mentNo += 30;
	audioFlush();
	msg[0] = mentNo; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
	audioPost(msg);
}

void MentUserNormalMultiple(unsigned char *mentNos, int count)
{
	unsigned char	msg[4];
	int		i, mentNo;

	audioFlush();
	for(i = 0;i < count;i++) {
		mentNo = mentNos[i];
		if(GetLanguage() == LANG_ENG && mentNo >= 100 && mentNo < 120) mentNo += 30;
		msg[0] = mentNo; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
		audioPost(msg);
	}
}

void MentAddUserNormal(int mentNo)
{
	unsigned char	msg[4];

	//audioFlush();	// commented by 2024.3.11
	if(GetLanguage() == LANG_ENG && mentNo >= 100 && mentNo < 120) mentNo += 30;
	msg[0] = mentNo; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
	audioPost(msg);
}

void AddResultMent(int msgIndex)
{
	int		no;

	no = 0;
	switch(msgIndex) {
	case R_USER_NOT_FOUND:		no = 102; break;
	case R_USER_DELETED:
	case R_ALL_USER_DELETED:
	case R_ALL_DELETED:			no = 114; break;
	case R_DATA_SAVED:			no = 115; break;
	}
	if(no) MentUserNormal(no);
}

char *GetPromptMsg(int msgIndex)
{
	return xmsgs[msgIndex];
}

char *GetResultMsg(int msgIndex)
{
	return xrsp_msgs[msgIndex];
}

void GetResultMsgNoLine(int msgIndex, char *buf)
{
	char	*p;

	p = xrsp_msgs[msgIndex];
	strcpy_swap(buf, p, '\n', ' ');
}

int GetCaptureMsgs(int *msgs, int mode, int retry)
{
	int		cnt, no;
	
	cnt = 0;
	if(mode & USER_FP) {
		if(retry) no = M_PLACE_YOUR_FINGER_AGAIN;
		else	  no = M_PLACE_YOUR_FINGER;
		msgs[cnt++] = no;
	}
	if(mode & USER_CARD) {
		if(retry) no = M_TOUCH_YOUR_CARD_AGAIN;
		else	  no = M_TOUCH_YOUR_CARD;
		msgs[cnt++] = no;
	}
	if(mode & USER_PIN) {
		if(retry) no = M_ENTER_PASSWORD;
		else	  no = M_ENTER_PASSWORD;
		msgs[cnt++] = no;
	}
	return cnt;
}

void GetIDTitle(long nID, char *Title)
{
	char	*p;
	
	p = Title;
	*p++ = 'I'; *p++ = 'D'; *p++ = ':'; *p++ = ' ';
	if(nID) sprintf(p, "%ld", nID);
	else	*p = 0;
}

void GetIDStrTitle(char *StrID, char *Title)
{
	char	*p;
	
	p = Title;
	*p++ = 'I'; *p++ = 'D'; *p++ = ':'; *p++ = ' ';
	if(StrID) strcpy(p, StrID);
	else	*p = 0;
}

#ifdef _HOST_EMU

int mentPut(int No, int Volume, int Type)
{
	return 0;
}

void mentStart(void)
{
}

void mentStop(int stopType)
{
}

int timerCreate(int ID)
{
	return 0;
}

void timerDelete(int ID)
{
}

int timerStart(int ID, int msec, int periodic)
{
	return 0;
}

#endif

void RefreshTopScreen(void)
{
}
