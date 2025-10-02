#ifndef _DEV_PRIM_H_
#define _DEV_PRIM_H_


CREDENTIAL_READER *_GetCredentialReader(int ID);
void ConsoleCatptureCardStart(void);	
void ConsoleCatptureCardEnd(void);	
int  ConsoleCaptureCard(unsigned char *Data);
void ConsoleClearCard(void);
void InitLocalUnit(void);
void ProcessLocalUnit(void);
void ProcessRemoteIOU(int ID, unsigned char *buf);
void ProcessRemoteCRU(int ID, unsigned char *buf);
void ProcessRemoteACUnitNone(int ID, unsigned char *buf);
void ProcessRemoteACUnitRequest(int ID, unsigned char *buf);
void ProcessRemoteACUnitResponse(int ID, unsigned char *buf);
void ProcessRemoteACUnitSegResponse(int ID, unsigned char *buf);
void ProcessRemoteACUnitSegRequest(int ID, unsigned char *buf);
void ProcessRemoteIU(int ID, unsigned char *buf);
void ProcessAuthResult(ACCESS_POINT *ap, CREDENTIAL_READER *cr, FP_USER *user);
void ProcessCommandResult(CREDENTIAL_READER *cr, int Result, int DataSize);
void ProcessDoors(void);
void ProcessDoorsLockSchedule(void);
void ProcessAlarmAction(void);
int  IsBlackWhiteMode(void);
int  GetSvrSioPort(void);


#endif	/* _DEV_PRIM_H_ */