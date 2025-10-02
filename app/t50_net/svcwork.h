#ifndef _SVC_WORK_H_
#define _SVC_WORK_H_


void svcWorkInit(void);
void svcWorkExit(void);
int  svcWorkPostMessage(unsigned char *msg);
void svcWorkFlush(void);


#endif

