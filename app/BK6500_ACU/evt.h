#ifndef _EVT_H_
#define _EVT_H_


void evtInit(void);
int  evtIsGranted(int evtId);
int  evtIsLock(int evtId);
int  evtIsAccess(int evtId);
int  evtIsOther(int evtId);
int  evtCheckMask(int evtId);
void evtAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData);
void evtAddRaw(unsigned char *buf, int length);
int  evtRead(unsigned char *data);
void evtClear(void);
void evtClearPending(void);
int  evtIsPending(void);
void evtEnableSync(void);
void evtDisableSync(void);
void evtEnableNet(void);
void evtDisableNet(void);
int  evtIsNetPending(void);
void evtClearNetPending(void);

int  evtcamAdd(unsigned char *buf, void *yuvBuffer);
int  evtcamCopy(void *buffer);
void evtcamClear(int fileSize);
void evtcamFlush(void);
void *evtcamGetBuffer(void);


#endif
