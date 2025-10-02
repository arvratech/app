#ifndef _VOX_H_
#define _VOX_H_


int  voxOpen(void);
int  voxClose(void);
int  voxPlayWave(char *fileName, int volume);
void voxStopWave(void);
int  voxState(void);


#endif

