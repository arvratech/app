#ifndef _FPM_H_
#define _FPM_H_


void fpmTouched(void);
int  fpmCommand(void);
void fpmWriteRequest(void);
void fpmReadResponse(unsigned char *buf, int size);
void fpmCopyTemplate(unsigned char *buf);
void fpmRequestCapture(void);
void fpmResponseCapture(void);
void fpmRequestIdentify(void);
void fpmResponseIdentify(void);
void fpmRequestIdentifyMatch(void);
void fpmResponseIdentifyMatch(void);
void fpmRequestFind(unsigned long fpId);
int  fpmResponseFind(void);
void fpmRequestCreateTemplate(unsigned char *tmpl);
int  fpmResponseCreateTemplate(void);
void fpmRequestAddTemplate(unsigned long fpId, unsigned char *tmpl);
int  fpmResponseAddTemplate(void);
void fpmRequestDeleteTemplate(unsigned long fpId);
int  fpmResponseDeleteTemplate(void);
void fpmRequestDeleteAllTemplate(void);
int  fpmResponseDeleteAllTemplate(void);
void fpmRequestTemplateCount(void);
int  fpmResponseTemplateCount(void);


#endif

