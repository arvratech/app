static CF_WIEGAND_PIN	*wpfrd, _wpfwr;


void _AdmWpfPrefChanged(void *pss, int index, void *act)
{
	void	*ps;
	char	temp[64];
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("AdmWpfPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode == RESULT_PREF_EDITED) {
		PrefReloadRowAtIndex(act, index);
	}
}

void AdmWpfRd(void *pss);
void AdmWpfWr(void *pss);

void AdmWpf(void *pss)
{
	void	*ps, *wpf;
	int		i;

	PSsetOnPrefChanged(pss, _AdmWpfPrefChanged);
	for(i = 0;i < MAX_CF_WIEGAND_PIN_SZ;i++) {
		wpf = cfWgPinsGet(i);
		ps = PSaddButton(pss, cfWgPinName(wpf), NULL);
		PSsetIntAction(ps, AdmWpfRd);
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, wpf);
	}
}

static CF_WIEGAND_PIN_CFG	 _wpfcfg;

void _AdmWpfRdLoadData(void *pss)
{
	void	*ps, *wpf;
	char	*p, temp[64];
	int		i, id;

	wpf = PScontext(pss);
	id = cfWgPinId(wpf);
	if(id != 1) {
		ps = PSaddButton(pss, NULL, NULL);	// Edit Button
		PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
		_wpfwr.cfg = &_wpfcfg;
		PSsetContext(ps, &_wpfwr);
		PSsetIntAction(ps, AdmWpfWr);
	}
	p = cfWgPinName(wpf);
	PSsetTitle(pss, p);
	ps = PSaddTitle(pss, p, NULL, PS_TITLE_STYLE_DEFAULT);
	if(id != 1) ps = PSaddTitle(pss, cfParityTitle(), cfWgPinParityName(wpf), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfCodingTitle(), cfWgPinCodingName(wpf), PS_TITLE_STYLE_SUBTITLE);
	if(id == 2) {
		sprintf(temp, "8 %s  %d", admBitUnitTitle(), cfWgPinFacilityCode(wpf));
		ps = PSaddTitle(pss, cfPinFacilityCodeTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	}
	cfWgPinGetPinNumberName(wpf, temp);
	ps = PSaddTitle(pss, cfPinNumberTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
}

void _AdmWpfRdPrefChanged(void *pss, int index, void *act)
{
	void	*wpf;

printf("AdmWpfRdPrefChanged...%d\n", index);
	wpf = PScontext(pss);
	if(index == 0) {
		cfWgPinCopy(wpf, &_wpfwr);
		PSremoveChildren(pss);
		_AdmWpfRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmWpfRd(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmWpfRdPrefChanged);
	wpfrd = PScontext(pss);
	_AdmWpfRdLoadData(pss);
}

void _AdmWpfWrLoadData(void *pss)
{
	void	*ps, *wpf;
	char	*p, temp[64];
	int		i, id;

	wpf = PScontext(pss);
	id = cfWgPinId(wpf);
	p = cfWgPinName(wpf);
//printf("id=%d cred=%d name=[%s]\n", (int)wf->id, wfCredential(wf), temp);
	ps = PSaddTitle(pss, p, NULL, PS_TITLE_STYLE_DEFAULT);
	if(id == 0 || id == 3) {
		ps = PSaddSpinner(pss, cfParityTitle(), cfWgPinParity, cfWgPinSetParity);
		PSsetContext(ps, wpf);
		for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfParityName(i), i);
	} else if(id == 2) {
		ps = PSaddTextField(pss, cfPinFacilityCodeTitle(), AdmDoneValidateInt8);
		PStextFieldAddItem(ps, PStitle(ps), cfWgPinFacilityCodeName, cfWgPinSetFacilityCodeName, TI_NUMERIC, 3);
		PSsetContext(ps, wpf);
	}
}

void _AdmWpfWrPrefChanged(void *pss, int index, void *act)
{
printf("AdmWpfWrPrefChanged...%d\n", index);
}

void AdmWpfWr(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmWpfWrPrefChanged);
	cfWgPinCopy(&_wpfwr, wpfrd);
	_AdmWpfWrLoadData(pss);
}

