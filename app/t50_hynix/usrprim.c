#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "fsuser.h"
#include "sysdep.h"
#include "syscfg.h"
#include "ta.h"
#include "gactivity.h"
#include "admprim.h"
#include "admtitle.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "usrprim.h"


void GetUserAccessMode(int accessMode, char *buf)
{
	char	*p;
	int		i, count, msk;

	p = buf; count = 0;
	msk = 0x01;
	for(i = 0;i < 7;i++, msk <<= 1) {
		if(accessMode & msk) {
			if(count) *p++ = ',';
			strcpy(p, xmenu_access_mode[i]); p += strlen(p);
			count++;
		}
	}
	*p = 0;
}

void GetUserCardData(unsigned char *cardData, char *buf)
{
	if(cardData[0]) string_co2db(cardData, 9, buf);
	else	buf[0] = 0;
}

void GetUserTitleCardData(unsigned char *cardData, char *buf)
{
	char	*p;

	p = buf; strcpy(p, xmenu_user_reg[4]); p += strlen(p); *p++ = ':'; *p++ = ' ';
	if(cardData[0]) string_co2db(cardData, 9, p);
	else	*p = 0;
}

void GetUserFpData(int fpExist, char *buf)
{
	int		val;

	if(fpExist) val = 0; else val = 1;
	strcpy(buf, xmenu_yesno[val]);
}

void GetUserAccessRights(unsigned char accessRights, char *buf)
{
	if(accessRights == 0) strcpy(buf, admNeverTitle()); 
	else if(accessRights == ALWAYS_VAL) strcpy(buf, admAlwaysTitle()); 
	else	sprintf(buf, "%s %d", admScheduleTitle(), (int)accessRights);
}

void GetUserDate(unsigned char *ctm, char *buf)
{
	if(date_validate(ctm)) sprintf(buf, "%04d-%02d-%02d", 2000+ctm[0], (int)ctm[1], (int)ctm[2]);
	else	strcpy(buf, admUnspecifiedTitle()); 
}

