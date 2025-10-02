#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "schecfg.h"
#include "tacfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "prefspec.h"
#include "psmem.h"
#include "fsuser.h"
#include "dlg.h"
#include "acu.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "pref.h"
#include "appact.h"


void PhoneCallAction(int requestCode, void *pss)
{
	char	temp[128];

	sprintf(temp, "name=%s\nurl=%s\n", PStitle(pss), PSsubtitle(pss));
//	PjsCallStart(requestCode, temp);
	PjsCallRestart(requestCode, temp);
}

void PhoneCallStart(void *pss)
{
	ACCT	*acct, _acct;
	void	*ps;
	int		rval;

	acct = &_acct;
	acctfsSeek(0L);
	while(1) {
		rval = acctfsRead(acct);
		if(rval <= 0) break;
		ps = PSaddButton(pss, acct->acctName, PhoneCallAction);
		PSsetSubtitle(ps, acct->url);
	}
}

extern PS_SCREEN	_psscreen;

void PhoneCall(int requestCode)
{
	void	*ps;

	ps = &_psscreen;
	PSscreenInit(ps, xmenu_phone[5], PhoneCallStart);
	PrefCreate(requestCode, ps);
}

