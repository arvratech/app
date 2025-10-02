#ifndef _EVT_H_
#define _EVT_H_


void evtInit(void);
int  evtIsGranted(int EventID);
int  evtIsAccess(int evtId);
void evtMake(unsigned char *buf, int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData);
int  evtAddRaw(unsigned char *buf, int length);
int  evtRead(unsigned char *buf);
void evtClear(void);
void evtClearError(void);
int  evtGetSize(void);
void evtAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData);
void EventAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData);

int  evtcamAdd(unsigned char *evtbuf);
int  evtcamRead(unsigned char *buf);
void evtcamClear(void);
void evtcamFlush(void);


#endif

