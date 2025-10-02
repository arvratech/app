#ifndef _SYS_DEP_H_
#define _SYS_DEP_H_


int  WriteProgram(unsigned char *addr, int nSize);
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


#endif
