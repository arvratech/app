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
#include "oc.h"
#include "cf.h"
#include "aa.h"
#include "cfprim.h"
#include "pfprim.h"
#include "aaprim.h"
#include "dlg.h"
#include "pref.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "appact.h"


#include "../admcom/adm_accf.c"
#include "../admcom/adm_acwf.c"
#include "../admcom/adm_acpf.c"
#include "../admcom/adm_acpwf.c"
#include "../admcom/adm_acaa.c"


void AdmAcStart(void *pss)
{
	void	*ps;

	ps = PSaddScreen(pss, xmenu_ac[0], AdmAcCfStart);
//	ps = PSaddScreen(pss, xmenu_ac[1], AdmAcAaStart);
}

void AdmAcCfStart(void *pss)
{
	void	*ps, *cf, *pf;

	ps = PSaddScreen(pss, credClassName(0), AdmCfScStart);
	cf = cfCardsGet(CFCARD_EM);
	ps = PSaddButton(pss, cfCardName(cf), NULL);
	PSsetOnStart(ps, AdmCfRdStart);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cf);
	ps = PSaddScreen(pss, credClassName(1), AdmWfStart);
	cf = cfCardsGet(CFCARD_SERIAL);
	ps = PSaddButton(pss, cfCardName(cf), NULL);
	PSsetOnStart(ps, AdmCfRdStart);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cf);
	pf = cfPinGet();
	ps = PSaddButton(pss, cfPinName(pf), NULL);
	PSsetOnStart(ps, AdmPfRdStart);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, pf);
	ps = PSaddScreen(pss, credClassName(3), AdmPwfStart);
}

