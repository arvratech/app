#ifndef _MAIN_INPUT_H_
#define _MAIN_INPUT_H_


void *MainLoop(void);

int  MainOpenSlvm(void);
void MainCloseSlvm(void);
void MainOnReadSlvm(void);

int  MainOpenBlem(void);
void MainCloseBlem(void);
void MainOnReadBlem(void);

int  MainOpenPipe(void);
void MainClosePipe(void);
void MainStartPipe(void);
void MainOnReadPipe(void);
void MainPostPipe(unsigned char *buf, int length);

int  MainOpenSio(void);
void MainCloseSio(void);
void MainStartSio(void);
void MainOnReadSio(void);


#endif

