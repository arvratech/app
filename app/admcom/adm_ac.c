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
#include "aa.h"
#include "cf.h"
#include "cfprim.h"
#include "pfprim.h"
#ifndef _WIEGAND_READER 
#include "aaprim.h"
#endif
#include "dlg.h"
#include "pref.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "admtitle.h"
#include "actalert.h"
#include "appact.h"

#include "adm_accf.c"
#include "adm_acwf.c"
#include "adm_acpf.c"
#include "adm_acwpf.c"
#ifndef _WIEGAND_READER 
//#include "adm_acaa.c"
#endif

void AdmAcInputFmt(void *pss);
void AdmAcWgOutFmt(void *pss);


void AdmAc(void *pss)
{
/*
	void	*ps;

	ps = PSaddScreen(pss, admInputFormatsTitle(), AdmAcInputFmt);
	ps = PSaddScreen(pss, admWiegandOutputFormatsTitle(), AdmAcWgOutFmt);
#ifndef _WIEGAND_READER 
	ps = PSaddScreen(pss, admAlarmCommandsTitle(), AdmAcAa);
#endif
*/
}

/*
void AdmAcInputFmt(void *pss)
{
	void	*ps, *cf, *pf;

	ps = PSaddScreen(pss, credClassName(0), AdmCfSc);
#ifndef _WIEGAND_READER 
	cf = cfCardsGet(CFCARD_EM);
	ps = PSaddButton(pss, cfCardName(cf), NULL);
	PSsetIntAction(ps, AdmCfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cf);
	ps = PSaddScreen(pss, credClassName(1), AdmWf);
	PSsetTag(ps, 0);
#endif
	cf = cfCardsGet(CFCARD_SERIAL);
	ps = PSaddButton(pss, cfCardName(cf), NULL);
	PSsetIntAction(ps, AdmCfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cf);
	pf = cfPinGet();
	ps = PSaddButton(pss, cfPinName(pf), NULL);
	PSsetIntAction(ps, AdmPfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, pf);
}

void AdmAcWgOutFmt(void *pss)
{
	void	*ps;

	ps = PSaddScreen(pss, credClassName(1), AdmWf);
	PSsetTag(ps, 2);
	ps = PSaddScreen(pss, credClassName(3), AdmWpf);
}
*/
