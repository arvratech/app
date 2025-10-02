#ifndef _ADM_PRIM_H_
#define _ADM_PRIM_H_


void EnterSleepMode(void);
void WakeupSleepMode(void);
void *AdmActInitWithTableView(GACTIVITY *act, char *title);
void *AdmActInitWithTableButtonView(GACTIVITY *act, char *btnTitle);
void *AdmActInitWithScrollView(GACTIVITY *act, char *title);
void AdmActExit(GACTIVITY *act);
void *AdmInitTableViewCell(void *tblv, UITableViewCellStyle style, UITableViewCellAccessoryType accessoryType, char *lblTitle);
void *TableCreateSubtitleCell(void *tblv, char *title, char *subtitle);
void *TableCreateLabelCell(void *tblv, char *title);
void *TableCreateTopLabelCell(void *tblv, char *title);
char *GetTableViewCellText(void *tblv, int index);
int  AdmAddTitleLabel(void *view, int y, char *text);
int  AdmAddTopTitleLabel(void *view, int y, char *text);
int  AdmAddKeyValueLabel(void *view, int y, char *keyText, char *valueText);
int  AdmAddKeyLabel(void *view, int y, char *text);
int  AdmAddValueLabel(void *view, int y, char *text);
int  AdmAddValueLabel2(void *view, int tag, int y);
void AdmSetContentSize(void *view, int contentHeight);
BOOL AdmDoneValidateInt8(char *intent, char *result);
BOOL AdmNetDoneValidateDeviceID(char *intent, char *result);
BOOL AdmDoneValidateIpAddress(char *intent, char *result);
BOOL AdmNetDoneValidateServer(char *intent, char *result);
BOOL AdmDoneValidatePIN(char *intent, char *result);
BOOL AdmDoneValidateUserID(char *intent, char *result);
BOOL AdmDoneValidateNewUserID(char *intent, char *result);
BOOL AdmDoneValidateMifareKey(char *intent, char *result);
BOOL AdmDoneValidateAid(char *intent, char *result);
BOOL AdmDoneValidateMacFilter(char *intent, char *result);
BOOL AdmDoneValidateAcctUrl(char *intent, char *result);
void parseKeyValuePairs(char *buf, void (*Callback)(char *key, char *value));
void GetTimeString(char *buf, int hh, int mm);
void AlertResultMsg(char *title, int msgIndex);
void *ViewAddFuncLabel(void *wnd);
void ViewRefreshFuncLabel(void *view, int funcKey);
void ViewAddFuncButtons(void *wnd, void *funcBtns[]);
void ViewRefreshFuncButtons(void *funcBtns[]);
void *SlvnetsIpcnetCreateReqTxn(void);
void *SvvnetsSvrnetsCreateReqTxn(void);


#endif

