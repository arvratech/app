#ifndef _CBUF_H_
#define _CBUF_H_


void cbufInit(void *cbuf, int DataSize);
int  cbufLength(void *cbuf);
int  cbufUsed(void *cbuf);
int  cbufFree(void *cbuf);
void cbufFlush(void *cbuf);
int  cbufEmpty(void *cbuf);
int  cbufWrlock(void *cbuf);
void cbufWrunlock(void *cbuf);
int  cbufRdlock(void *cbuf);
void cbufRdunlock(void *cbuf);
int  cbufPeek(void *cbuf);
int  cbufGetch(void *cbuf);
int  cbufPutch(void *cbuf, int c);
int  cbufGet(void *cbuf, unsigned char *buf, int len);
int  cbufPut(void *cbuf, unsigned char *buf, int len);


#endif

