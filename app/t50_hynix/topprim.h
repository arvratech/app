#ifndef _TOP_PRIM_H_
#define _TOP_PRIM_H_

#define MSG_TIMEOUT			15
#define BEEP_KEY			1
#define BEEP_ACCESS_GRANTED	2
#define BEEP_ACCESS_DENIED	3
#define BEEP_TAMPER			4
#define BEEP_FP_CAPTURED	5
#define BEEP_CARD_CAPTURED	6


//void PlayBeepMent(int beep, int ment_no);
void PutBeep(int Beep);
void StopBeep(void);
void KeyBeep(void);
void CardCaptureBeep(void);
void FPCaptureBeep(void);
void MentTamperAlarm(void);
void MentForcedOpenAlarm(void);
void MentOpenTooLongAlarm(void);
int  GetCapturePromptMent(int mode);
void MentUserNormal(int mentNo);
void MentUserNormalMultiple(unsigned char *mentNos, int count);
void MentAddUserNormal(int mentNo);
void MentAdminNormal(int mentNo);
void AddResultMent(int msgIndex);

char *GetPromptMsg(int msgIndex);
char *GetResultMsg(int msgIndex);
void GetResultMsgNoLine(int msgIndex, char *buf);
int  GetCaptureMsgs(int *msgs, int mode, int retry);
void GetIDTitle(long nID, char *Title);
void GetIDStrTitle(char *StrID, char *Title);

void RefreshTopScreen(void);
int EncodeTermStatus(void *buf);
int EncodeStatus(void *buf);


#endif

