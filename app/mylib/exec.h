#ifndef _EXEC_H_
#define _EXEC_H_


// MP4 state definitions
#define MP_NULL			0
#define MP_IDLE			1
#define MP_RUN			2
#define MP_PAUSE		3


void runWpaSupplicant(void);
void stopWpaSupplicant(void);
void runScriptFile(char *scriptFile);

int  mpFileCount(void);
void mpArrangeFile(void);
int  mpGetFile(int index, char *fileName);
void mpFullPath(char *path, char *fileName);
int  mpGetFileSize(char *fileName, unsigned long *fileSize);
int  mpDeleteFile(char *fileName);
void mpDeleteAll(void);


#endif

