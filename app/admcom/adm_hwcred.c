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
#include "hw.h"
#include "hwprim.h"
#include "cf.h"
#include "cfprim.h"
#include "pfprim.h"
#include "dlg.h"
#include "pref.h"
#include "psmem.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admprim.h"
#include "actalert.h"
#include "pref.h"
#include "appact.h"


void _AdmHwCrCredPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *cr, *cf;
	char	temp[64];
	BOOL	bVal;
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("HwCrCredPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode >= RESULT_OK) {
		if(index== 0) {
			cr = PScontext(ps);
			bVal = crIsEnableScReader(cr);
			if(bVal) {
				cf = cfCardsAt(crScReaderFormat(cr));
				strcpy(temp, cfCardName(cf));
			} else {
				temp[0] = ' '; temp[1] = 0;
			}
			PSsetSubtitle(ps, temp);
			PrefReloadRowAtIndex(act, index);
		}
	}
}

void AdmCfSc(void *pss);
void AdmCfRd(void *pss);
void AdmWf(void *pss);
void AdmPfRd(void *pss);

void AdmHwCrCredFmts(void *pss)
{
	void	*ps, *cr, *cf, *subdev;
	char	temp[64];
	int		id, model;

	PSsetOnPrefChanged(pss, _AdmHwCrCredPrefChanged);
	cr = PScontext(pss);
	id = crId(cr);
	model = devModel(NULL);
	ps = PSaddSwitch(pss, crName(cr, 0), crIsEnableScReader, crSetEnableScReader, AdmCfSc);
	PSsetContext(ps, cr);
	if(crIsEnableScReader(cr)) {
		cf = cfCardsAt(crScReaderFormat(cr));
		strcpy(temp, cfCardName(cf));
	} else {
		temp[0] = ' '; temp[1] = 0;
	}
	PSsetSubtitle(ps, temp);
	PSsetTag(ps, 10);
#ifndef _WIEGAND_READER
	ps = PSaddSwitch(pss, crName(cr, 1), crIsEnableEmReader, crSetEnableEmReader, AdmCfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cfCardsGet(CFCARD_EM));
	ps = PSaddSwitch(pss, crName(cr, 2), crIsEnableWiegandReader, crSetEnableWiegandReader, AdmWf);
	PSsetTag(ps, 1);
#endif
	ps = PSaddSwitch(pss, crName(cr, 3), crIsEnableSerialReader, crSetEnableSerialReader, AdmCfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cfCardsGet(CFCARD_SERIAL));
	ps = PSaddSwitch(pss, crName(cr, 4), crIsEnablePinReader, crSetEnablePinReader, AdmPfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cfPinGet());
#ifdef _FP_READER
	ps = PSaddSwitch(pss, crName(cr, 5), crIsEnableFpReader, crSetEnableFpReader, AdmPfRd);
	PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
	PSsetContext(ps, cfFpGet());
#endif
}

