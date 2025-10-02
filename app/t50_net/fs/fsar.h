#ifndef _FS_AR_H_
#define _FS_AR_H_

#include "..\BootLoader\flash.h"
#include "..\BootMonitor\fs.h"
#include "acar.h"
#include "cal.h"
#include "sche.h"
//#include "oc.h"


int  fsOpenScheFiles(void);

int  arfsSeek(long position);
long arfsTell(void);
int  arfsEof(void);
int  arfsGet(ACCESS_RIGHTS *ar);
int  arfsGetEncoded(long nID, unsigned char *data);
int  arfsAddEncoded(long nID, unsigned char *data);
int  arfsReadBulk(unsigned char *Data, int Size);
int  arfsAddBulk(unsigned char *Data, int Size);
int  arfsRemove(long nID);
int  arfsRemoveAll(void);
int  arfsGetCount(void);

int  calfsSeek(long position);
long calfsTell(void);
int  calfsEof(void);
int  calfsGet(CALENDAR *cal);
int  calfsGetEncoded(long nID, unsigned char *data);
int  calfsAddEncoded(long nID, unsigned char *data);
int  calfsReadBulk(unsigned char *Data, int Size);
int  calfsAddBulk(unsigned char *Data, int Size);
int  calfsRemove(long nID);
int  calfsRemoveAll(void);
int  calfsGetCount(void);

int  schefsSeek(long position);
long schefsTell(void);
int  schefsEof(void);
int  schefsGet(SCHEDULE *sche);
int  schefsGetEncoded(long nID, unsigned char *buf);
int  schefsAddEncoded(long nID, unsigned char *buf);
int  schefsReadBulk(unsigned char *Data, int Size);
int  schefsAddBulk(unsigned char *Data, int Size);
int  schefsRemove(long nID);
int  schefsRemoveAll(void);
int  schefsGetCount(void);

/*
int  ocfsSeek(long position);
long ocfsTell(void);
int  ocfsEof(void);
int  ocfsGet(OUTPUT_COMMAND *oc);
int  ocfsGetEncoded(long nID, unsigned char *buf);
int  ocfsAddEncoded(long nID, unsigned char *buf);
int  ocfsReadBulk(unsigned char *Data, int Size);
int  ocfsAddBulk(unsigned char *Data, int Size);
int  ocfsRemove(long nID);
int  ocfsRemoveAll(void);
int  ocfsGetCount(void);
*/


#endif	/* _FS_AR_H_ */