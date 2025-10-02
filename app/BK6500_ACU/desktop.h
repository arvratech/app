#ifndef _DESKTOP_H_
#define _DESKTOP_H


void DesktopInitialize(void);
int  DesktopGetColor(void);
void DesktopSetColorMode(void);
int  DesktopIsIdle(void);
int  DesktopProcess(void);
void DesktopDrawAll(void);
void DesktopDrawMain(void);
void DesktopClearMain(void);
void DesktopEnterMenu(void);
void DesktopExitMenu(void);
void DesktopInitClock(void);
void DesktopDrawClock(void);
void DesktopOnClockBlink(void);
void DesktopOffClockBlink(void);
void DesktopSetStat(char *Text);
void DesktopDrawStat(void);
void DesktopRemoteLogin(void);
void DesktopRestoreWindow(GWND *wnd);
void DesktopRestoreRect(RECT *rect);
void DesktopRestoreArea(int x, int y, int Width, int Height);
void DesktopDrawBackGround(void);

int  DesktopEnterPassword(char *Title, char *Data);
void DesktopResult(char *Caption, int MsgIndex);

int  DesktopIconIsVisible(void);
void DesktopSetIconsVisible(int Enable);
void DesktopDrawIcons(void);
void DesktopDrawServerNetState(void);
void DesktopDrawCamState(void);
void DesktopDrawSDCard(void);



#endif	/* _DESKTOP_H_ */