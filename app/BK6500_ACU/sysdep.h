#ifndef _SYS_DEP_H_
#define _SYS_DEP_H_


void GetMACAddress(unsigned char *addr);
int  GetAppProgramStartBlock(void);
int  WriteAppProgram(unsigned char *buf, int nSize);
int  GetUserStartBlock(void);
int  GetMaxUserSize(void);
int  GetMaxFPTemplateSize(void);
int  GetMaxFPIdentifySize(void);
int  GetMaxPhoneNoSize(void);
int  GetUserFileSize(void);
int  GetFPTemplateFileSize(void);
int  GetPhoneNoFileSize(void);
int  GetCamImageFileSize(void);
unsigned char *GetXferBuffer(int XferFile);
int  GetXferBufferSize(int XferFile);
int  GetXferSize(int XferFile);
void SetXferSize(int XferFile, int nSize);
unsigned char *GetMainBuffer(void);
int GetMainBufferSize(void);
void InitTrpls(void);
void UpdateTrpls(void);


#endif

