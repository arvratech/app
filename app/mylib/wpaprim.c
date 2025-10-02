#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
#include "lang.h"
#include "msg.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "wpactrl.h"
#include "wpanet.h"
#include "wpaprim.h"

extern unsigned char	wpaScanResult;


char *wpaStateName(int wpaState)
{
	int		idx;

	idx = wpaState;
	if(wpaState == WPA_COMPLETED && niState() < S_NI_READY) idx++;
	return xmenu_wpa_state[idx];
}

char *wpaSecurityName(int security)
{
	return xmenu_wifi_security[security];
}

void wpaGetSecuredName(int security, char *name)
{
	char	temp[32];
	int		lang;

	if(security == KEY_NONE) name[0] = 0;
	else {
		lang = GetLanguage();
		strcpy_chr(temp, xmenu_wifi_security[security], ' ');
//printf("[%s] %d [%s]\n", temp, security, xmenu_wifi_security[security]);
		if(lang == LANG_KOR) sprintf(name, "%s%s", temp, xmenu_wifi_state[3]);
		else	sprintf(name, "%s %s", xmenu_wifi_state[3], temp);
	}
}

int wpaFrequency2Channel(int freq)
{
	int		ch;

	if(freq >= 2412 && freq <= 2484) {
		if(freq == 2484) ch = (freq-2412) / 5;
		else	ch = (freq-2412) / 5 + 1;
	}else if(freq >= 5170 && freq <= 5825) {
		ch = (freq-5170) / 5 + 34;
	} else {
		ch = -1;
	}
	return ch;
}

