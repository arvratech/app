#ifndef _SC_TASK_H_
#define _SC_TASK_H_


int  scOpen(void);
void scClose(void);
void scTaskInit(void);
void scTaskOpen(void);
void scTaskClose(void);
void scTask(void);
void scTaskEnterSleepMode(void);
void scTaskWakeupSleepMode(void);


#endif
