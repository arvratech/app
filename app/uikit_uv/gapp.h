#ifndef _GAPP_H
#define _GAPP_H

#include "NSEnum.h"


#define GM_ACTIVITY			0x31
#define GM_USER				0x80


int  appInit(void *loop, void (*onCreate)(GACTIVITY *));
void appExit(void);
void appProcActivity(void);
void appPostMessage(unsigned char *msg);
void appSendMessage(unsigned char *msg);
void appDrawCurrentActivity(void);
void appDrawView(void);
void appDrawActivity(CGRect *rect);
int  appStartActivity(GACTIVITY *act, char *intent, int flag);
int  appStartActivityForResult(GACTIVITY *act, int requestCode, char *intent, int flag);
int  appDestroyActivity(GACTIVITY *act);
int  appDestroyActivityAffnity(GACTIVITY *act);
void appStopActivityTimer(GACTIVITY *act);
void appStartActivityTimer(GACTIVITY *act);
void appSetSoundSetting(int volume, int option);
int  appKeypadTone(void);
int  appTouchSounds(void);
void appMuteSounds(void);
void appRecoverSounds(void);
GACTIVITY *appCurrentActivity(void);


#endif

