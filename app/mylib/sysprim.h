#ifndef _SYS_PRIM_H_
#define _SYS_PRIM_H_



int  ExecForeground(const char *path, const char *arg, ...);
int  ExecOnly(const char *path, const char *arg, ...);
int  ExecOnlyWithArray(const char *path, char **array);
int  ExecPipe(const char *path, const char *arg, ...);
int  ExecPipeFinal(char *outputBuffer, const char *path, const char *arg, ...);
void setExitSignal(int bg, void (*sigHandler)(int));
void restoreExitSignal(void);
int  initDaemon(void);
int  mountUsb(void);
void umountUsb(void);
void getKernelVersion(char *version);
void getKernelLocalVersion(char *version);


#endif

