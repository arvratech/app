#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "lcdc.h"
#include "usbd.h"
#include "syscfg.h"
#include "gfont.h"
#include "acad.h"
#include "acap.h"
#include "user.h"
#include "cr.h"
#include "msg.h"
#include "menuprim.h"
#include "desktop.h"
#include "wnds.h"
#include "topprim.h"
#include "toptask.h"
#include "adm.h"
#include "admcon.h"


// Return	-2:Error  1:Lockout  2:OK
int MenuConsoleLogin(void)
{
	char	buf1[20], buf2[20];
	int		rval, cnt, timeout;
	
	cnt = 0;
	cPutText("\n"); if(menu_errno) return GWND_ERROR;
	if(menucGetMode() == 1) timeout = 600; else timeout = -1;
	while(1) {
		buf1[0] = 0;
		rval = menucInputStringEx("Login: ", buf1, 16, timeout);	// Prompt,data,MaxLength=16,Timeout=-1
		if(rval < 1) {
			if(rval == GWND_TIMEOUT) menu_errno = 99;
			break;
		} else if(rval == 1) continue;
		buf2[0] = 0;
		rval = menucInputPassword("Password: ", buf2, 16);	// Prompt,data,MaxLength=16
		if(rval < 1) break;
		if(!strcmp(buf1, "biokey") && !strcmp(buf2, sys_cfg->AdminPIN)) rval = 2;
		else {
			cPutText("Login incorrect\r\n"); if(menu_errno) return GWND_ERROR;
			cnt++;
			if(cnt >= 3) rval = 1;
			else	rval = 0;
		}
		if(rval) break;
	}
	return rval;
}

void MenuConsoleLockout(void)
{
	char	buf[64];
	unsigned long	timer;

	cPutText("\r\nLockout...\r\n"); if(menu_errno) return;
	timer = DS_TIMER + 600L;
	while(DS_TIMER <= timer) {	
		cReadText(buf, 64);
		if(menu_errno) break;
		cUsbWriteNull();
		if(menu_errno) break;
		taskYield();
	}
}

void MenuConsoleShell(void)
{
	char	buf[80];
	int		rval, lang;

	cPutText("\r\nWelcome to system\r\n"); if(menu_errno) return;
	cPrintf("Device ID: %d\r\n", (int)sys_cfg->DeviceID);
	cPutText("\r\n");
	while(1) {
		buf[0] = 0;
		rval = menucInputString("> ", buf, 64);	// Prompt,data,MaxLength=16
		if(rval < 1) break;
		else if(rval == 1) continue;
		if(!strcmp(buf, "exit") || !strcmp(buf, "logout")) break;
		else if(!strcmp(buf, "admin")) {
			OnBackLight();
			lcdBacklight(1);
			DesktopRemoteLogin();
			taskReset(t_key); //taskReset(t_tcp);
			taskReset(t_main);
			lang = GetLanguage();
			SetLanguage(1);
			msgInitialize();
			MenuSetAdminConsole(1);

			MenuAdm();

			MenuSetAdminConsole(0);	
			SetLanguage(lang);
			msgInitialize();
			ctMainInit();
			taskBegin(t_main);
			taskBegin(t_key); //taskBegin(t_tcp);
			OnBackLight();
			DesktopDrawAll();
		} else	cPutText("Invalid command\r\n");
		if(menu_errno) break;
	}
	if(!menu_errno) cPutText("\r\nLogout. Bye...\r\n"); 
}
