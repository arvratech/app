#ifndef _TOPTASK_H_
#define _TOPTASK_H_


extern CO_TASK	*t_main, *t_clock, *t_key, *t_svr, *t_sin, *t_units, *t_console;


void ctMainInit(void);
void ctMain(void *arg);
void ctClock(void *arg);
void ctSvrInit(void);
void ctSvr(void *arg);
void ctSinInit(void);
void ctSin(void *arg);
void ctUnitsInit(void);
void ctUnits(void *arg);
void ctConsoleInit(void);
void ctConsole(void *arg);


#endif	/* _TOPTASK_H_ */