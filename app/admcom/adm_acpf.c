static CF_PIN_CFG	_pfcfg;
static CF_PIN		*pfrd, _pfwr;

void AdmPfRd(void *pss);
void AdmPfWr(void *pss);

void _AdmPfRdLoadData(void *pss)
{
	void	*ps, *pf;
	char	*p, temp[80];

	pf = PScontext(pss);
	cfPinValidate(pf);
	ps = PSaddButton(pss, NULL, NULL);	// Edit Button
	PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
	_pfwr.cfg = &_pfcfg;
	PSsetContext(ps, &_pfwr);
	PSsetIntAction(ps, AdmPfWr);
	p = cfPinName(pf);
	PSsetTitle(pss, p);
	ps = PSaddTitle(pss, p, NULL, PS_TITLE_STYLE_DEFAULT);
	ps = PSaddTitle(pss, cfOutputWiegandTitle(), cfPinOutputWiegandName(pf), PS_TITLE_STYLE_SUBTITLE);
	cfPinGetEntryIntervalName(pf, temp);
	ps = PSaddTitle(pss, cfPinEntryIntervalTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	cfPinGetEntryStopSentinelName(pf, temp);
	ps = PSaddTitle(pss, cfPinEntryStopSentinelTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	cfPinGetMaximumLengthName(pf, temp);
	ps = PSaddTitle(pss, cfPinMaximumLengthTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
}

void _AdmPfRdPrefChanged(void *pss, int index, void *act)
{
	void	*pf;

printf("AdmPfRdPrefChanged...%d\n", index);
	pf = PScontext(pss);
	if(index == 0) {
		cfPinCopy(pf, &_pfwr);
		PSremoveChildren(pss);
		_AdmPfRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmPfRd(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmPfRdPrefChanged);
	pfrd = PScontext(pss);
	_AdmPfRdLoadData(pss);
}

void _RefreshPinOutputWiegandSpinner(void *ps)
{
	void	*pf;
	BOOL	bVal;

	pf = PScontext(ps);
	if(cfPinEntryStopSentinel(pf)) bVal = FALSE;
	else	bVal = TRUE;
	PSspinnerSetEnableAtIndex(ps, bVal, 3);
}

void _AdmPfWrLoadData(void *pss)
{
	void	*ps, *pf;
	char	*p, temp[80];
	int		i, val, max;

	pf = PScontext(pss);
	p = cfPinName(pf);
//printf("id=%d cred=%d name=[%s]\n", (int)wf->id, wfCredential(wf), temp);
	ps = PSaddTitle(pss, p, NULL, PS_TITLE_STYLE_DEFAULT);
	ps = PSaddSpinner(pss, cfOutputWiegandTitle(), cfPinOutputWiegand, cfPinSetOutputWiegand);
	PSsetContext(ps, pf);
	for(i = 0;i < 4;i++) PSspinnerAddTitleValue(ps, cfWgPinName(cfWgPinsGet(i)), i);
	_RefreshPinOutputWiegandSpinner(ps);
	ps = PSaddStepper(pss, cfPinEntryIntervalTitle(), cfPinEntryInterval, cfPinSetEntryInterval, 1, 30, 1, admSecondUnitTitle());
	PSsetContext(ps, pf);
	ps = PSaddSpinner(pss, cfPinEntryStopSentinelTitle(), cfPinEntryStopSentinel, cfPinSetEntryStopSentinel);
	PSsetContext(ps, pf);
	val = cfPinEntryStopSentinel(pf);
	for(i = 0;i < 2;i++) {
		cfPinSetEntryStopSentinel(pf, i);
		cfPinGetEntryStopSentinelName(pf, temp);
		PSspinnerAddTitleValue(ps, temp, i);
	}
	cfPinSetEntryStopSentinel(pf, val);
	if(cfPinOutputWiegand(pf) == 2) max = 5;
	else	max = 12;
	ps = PSaddStepper(pss, cfPinMaximumLengthTitle(), cfPinMaximumLength, cfPinSetMaximumLength, 1, max, 1, admDigitUnitTitle());
	PSsetContext(ps, pf);
}

void _AdmPfWrPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *pf;
	char	temp[80];
	int		val;

printf("AdmPfWrPrefChanged...%d\n", index);
	if(index == 1) {
		ps = PSobjectAtIndex(pss, index);
		pf = PScontext(ps);
		val = cfPinValidateOutputWiegandFormat(pf);
		if(val) {
			PSsetSubtitle(ps, cfPinOutputWiegandName(pf));
			PrefReloadRowAtIndex(act, index);
		}
		val = cfPinMaximumLength(pf);
		cfPinValidate(pf);
		if(val != cfPinMaximumLength(pf)) {
			ps = PSobjectAtIndex(pss, 4);
			PSstepperSetValue(ps, cfPinMaximumLength(pf));
			PrefReloadRowAtIndex(act, 4);
		}
	} else if(index == 3) {
		ps = PSobjectAtIndex(pss, index);
		pf = PScontext(ps);
		val = cfPinValidateOutputWiegandFormat(pf);
		ps = PSobjectAtIndex(pss, 1);
		if(val) {
			PSspinnerSetValue(ps, cfPinOutputWiegand(pf));
			PrefReloadRowAtIndex(act, 1);
		}
		_RefreshPinOutputWiegandSpinner(ps);
	}
}

void AdmPfWr(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmPfWrPrefChanged);
	cfPinCopy(&_pfwr, pfrd);
	_AdmPfWrLoadData(pss);
}

