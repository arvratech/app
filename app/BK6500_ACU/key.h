#ifndef _KEY_H_
#define _KEY_H_

#define KEY_UP				'F'
#define KEY_DOWN			'G'
#define KEY_OK				'D'
#define KEY_CANCEL			'C'
#define KEY_MENU			'B'
#define KEY_CALL			'A'
#define KEY_LONG			0x17

#define KEY_F1				'E'
#define KEY_F2				'F'
#define KEY_F3				'G'
#define KEY_F4				'H'

#define KEY_PERIOD			'E'

#define KEY_FINGER_TOUCH	0x83
#define KEY_EXT1			0x81
#define KEY_EXT2			0x82


void keyInit(void (*KeyCallBack)());
void keyExit(void);
void keySetCallBack(void (*KeyCallBack)());
void ctKey(void *arg);
int  keyGet(void);
int  keyGetScanCode(void);
int  keyGetEx(int timeout);
int  keyGetWithExternalCancel(int timeout, int (*ExternalCallBack)());
void keyFlush(void);
int  keyGetCallButton(void);
void keyOnBackLight(void);
void keyOffBackLight(void);
void keyOnFuncLED(int code);

#endif	/* _KEY_H_ */
