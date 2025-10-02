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
int  GetCapturePromptMent(int mode);
void MentUserNormal(int MentNo);
void MentUserNormalTwo(int MentNo1, int MentNo2);
void MentAddUserNormal(int MentNo);
void MentAdminNormal(int MentNo);
void MentAddUser(int MentNo, int MentType);
void AddResultMent(int MsgIndex);
char *GetPromptMsg(int MsgIndex);
void GetPromptMsgWithoutNewLine(char *Msg, int MsgIndex);
void PromptMsg(GWND *ParentWnd, char *Caption, int MsgIndex);
void DestroyPromptMsg(void);
char *GetResultMsg(int MsgIndex);
void GetResultWithoutNewLine(char *Msg, int MsgIndex);
void ResultMsgNoWait(GWND *ParentWnd, char *Caption, int MsgIndex);
void ResultMsgNoWaitRight(GWND *ParentWnd, char *Caption, int MsgIndex);
void ResultMsg(GWND *ParentWnd, char *Caption, int MsgIndex);
void ResultMsgRight(GWND *ParentWnd, char *Caption, int MsgIndex);
void ResultMsgEx(GWND *ParentWnd, char *Caption, int Timeout, int Icon, char *fmt, ...);
void HalfResultMsgNoWait(GWND *ParentWnd, char *Caption, int MsgIndex);
void DestroyResultMsg(void);
void DestroyWindow(GWND *wnd);
void GetIDTitle(long nID, char *Title);
void GetIDStrTitle(char *StrID, char *Title);

void EnrollMsgProcessing(void);
void OnIndicator(int err);
void OffIndicator(void);
void PutBackNextWizard(int Stage, int End);
void OnBackLight(void);
void OffBackLight(void);
int  GetBackLight(void);
void StartBackLightTimer(void);
void ProcessBackLightTimer(void);
void StartCursor(int x, int y, int bmpSize);
void EndCursor(void);
void RestoreCursor(void);
void SetServerNetState(int NetState);
void CtrlerStateChanged(void);
void UnlockByGranted(int adID, unsigned char *ctm);
void UnlockByRequestToExit(int adID, unsigned char *ctm);
void UnlockByCall(int adID);
void DoorSetByCommand(int adID, int PresentValue);
void ResetSystem(void);
void InitializeVox(void);
void InitializeCam(void);
int  RunCam(void);
void StopCam(void);
int  CaptureCamJpeg(void *JPEGBuffer, void *YUVBuffer, unsigned long *CamFrameCount);
int  CaptureSlowCamJpeg(void *JPEGBuffer, void *YUVBuffer, unsigned long *CamFrameCount);
int  CaptureSlowOldCamJpeg(void *JPEGBuffer, void *YUVBuffer, unsigned long *CamFrameCount);
int  MakeJpegHeader(void *JPEGBuffer);
int  EncodeDeviceStatus(void *buf);
int  EncodeStatus(void *buf);


#endif	/* _TOP_PRIM_H_ */