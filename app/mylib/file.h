#ifndef _FILE_H_
#define _FILE_H_

int  diskAvailableSpace(char *path, unsigned long *psize);
int  fileSize(char *path, unsigned long *psize);
int  fileCopy(char *srcPath, char *dstPath, void (*callback)(unsigned long size));
int  fileCopyToMem(char *path, unsigned char *buf);
int  dirUsage(char *path);
int  dirCreate(char *path);
int  dirDelete(char *path);
int  dirCopyAppend(char *srcPath, char *dstPath, void (*callback)(unsigned long size));
int  dirCopy(char *srcPath, char *dstPath, void (*callback)(unsigned long size));
void TaskCopying(unsigned long size);
void TaskSetCopySize(unsigned long size);
void LOG(int errnum, ...);


#endif
