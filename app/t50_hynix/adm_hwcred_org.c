void AdmHwCrCredFmtsStart(void *pss);

void AdmHwCredStart(void *pss)
{
	void	*ps, *cr;

	cr = PScontext(pss);
	ps = PSaddScreen(pss, xmenu_access_point[0], AdmHwCrCredFmtsStart);
	PSsetContext(ps, cr);
}

void _AdmHwCrCredPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *cr, *cf;
	char	temp[64];
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("HwCrCredPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode >= RESULT_OK) {
		if(index== 0) {
			cr = PScontext(ps);
			if(crIsEnableScReader(cr)) {
				cf = cfCardsAt(crScFormat(cr));
				strcpy(temp, cfCardName(cf));
			} else {
				temp[0] = ' '; temp[1] = 0;
			}
			PSsetSubtitle(ps, temp);
			PrefReloadRowAtIndex(act, index);
		}
	}
}

void AdmHwCrCredFmtsStart(void *pss)
{
	void	*ps, *cr, *cf, *subdev;
	char	temp[64];
	int		id, model;

	PSsetOnPrefChanged(pss, _AdmHwCrCredPrefChanged);
	cr = PScontext(pss);
	id = crId(cr);
	if(!devLocalUnitModel(NULL) && id == 0) model = devModel(NULL);
	else {
		subdev = subdevsGet(id);
		model = subdevModel(subdev);
	}
printf("Model=%d\n", model);
	if(model > 1) {
		ps = PSaddSwitch(pss, crName(cr, 0), crIsEnableScReader, crSetEnableScReader, AdmCfScStart);
		if(crIsEnableScReader(cr)) {
			cf = cfCardsAt(crScFormat(cr));
			strcpy(temp, cfCardName(cf));
		} else {
			temp[0] = ' '; temp[1] = 0;
		}
		PSsetSubtitle(ps, temp);
		PSsetTag(ps, 10);

		ps = PSaddSwitch(pss, crName(cr, 1), crIsEnableEmReader, crSetEnableEmReader, AdmCfRdStart);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, cfCardsGet(CFCARD_EM));
	}
	ps = PSaddSwitch(pss, crName(cr, 2), crIsEnableWiegandReader, crSetEnableWiegandReader, AdmWfStart);
	PSsetTag(ps, 1);
	if(model > 1) {
		ps = PSaddSwitch(pss, crName(cr, 3), crIsEnableSerialReader, crSetEnableSerialReader, AdmCfRdStart);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, cfCardsGet(CFCARD_SERIAL));
	}
	if(model > 0) {
		ps = PSaddSwitch(pss, crName(cr, 4), crIsEnablePinReader, crSetEnablePinReader, AdmPfRdStart);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, cfPinGet());
		ps = PSaddSwitch(pss, crName(cr, 5), crIsEnableFpReader, crSetEnableFpReader, AdmPfRdStart);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, cfFpGet());
	}
}

