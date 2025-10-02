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
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "aa.h"
#include "aaprim.h"


void aaGetName(void *self, char *name)
{
	ALARM_ACTION	*aa = self;
	char	*p;

	if(!aa) strcpy(name, xmenu_ac[1]);
	else	sprintf(name, "%s %d", xmenu_ac[1], aaId(aa)+1);
}

char *aaActionIndexName(void *self, int index)
{
	return xmenu_action_idx[index];
}

	


