#ifndef _IODEV_H_
#define _IODEV_H_


void ioInit(void);
void ioProcessInputAlarm(void);
void ioOnLock(int ch);
void ioOffLock(int ch);
void ioOnAuth(int ch);
void ioOffAuth(int ch);
void ioOnBuzzer(int ch);
void ioOffBuzzer(int ch);
void ioOnHold(int ch);
void ioOffHold(int ch);

void wdtOpen(void);
void wdtReset(void);
void wdtResetLong(void);
void wdtResetSystem(void);

void beepOpen(void);
void beepPlay(void);


#endif	/* _IODEV_H_ */
