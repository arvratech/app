void _AdmHwDevId(void *pss)
{
	void	*ps;
	char	temp[64];
	int		i;

	ps = PSaddPicker(pss, devSlaveAddressTitle(), devGetSlaveAddress, devSetSlaveAddress);
	PSsetContext(ps, sys_cfg);
	PSpickerAddItem(ps, devUnitTitle());
	for(i = 0;i < 11;i++) {
		if(i == 0) strcpy(temp, devLocalTitle());
		else	sprintf(temp, "%s %d", devUnitTitle(), i-1);
		PSpickerAtIndexAddTitle(ps, 0, temp);
	}
	PSpickerAddItem(ps, devChannelTitle());
	for(i = 0;i < 2;i++) {
		sprintf(temp, "%s %d", devChannelTitle(), i+1);
		PSpickerAtIndexAddTitle(ps, 1, temp);
	}
}

void _AdmHwDevAdOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
{
	void	*ps, *pss, *a;
	int		index;

	ps = activityPsContext(act);
	if(status) {
		a = activityParent(act);
		AcuRequestDestroy(act);
		pss = activityPsContext(a);
		index = PSindexForObject(pss, ps);
		PrefReloadRowAtIndex(a, index);
	} else {
		if(buf[0]) AcuRequestSetResult(act, R_FAILED);
		else {
			_PSobjectRestoreValue(ps);
			AcuRequestDestroy(act);
			slvmWriteLocalIoConfig();
		}
	}
	_AdSetBackup(0);
}

void _AdmHwDevIntrusionOnSuccess(GACTIVITY *act)
{
	intrusionDisarm(NULL);
}

void _AdmSetTamperOnSuccess(GACTIVITY *act)
{
	tamperClearPresentValue(NULL);
}

void _AdmHwDevChanged(void *pss, int index, void *act)
{
	void	*ps, *cr, *ad;
	BOOL	bVal;

	switch(index) {
	case 4:
printf("### AD switch changed...\n");
		ps = PSobjectAtIndex(pss, index);
		ad = adsGet(0);
		AcuRequestStart(NULL, ps, _AdmHwDevAdOnResult);
		acuReqWriteAccessDoor(ad);
		PSswitchRestoreValue(ps);
		break;
	case 5:
printf("### Intrusion...\n");
		ps = PSobjectAtIndex(pss, index);
		AcuRequestStart(NULL, ps, NULL);
		AcuRequestSetOnSuccess(_AdmHwDevIntrusionOnSuccess);
		cr = crsGet(0);
		acuReqWriteCredentialReader(cr);
		PScheckRestoreValue(ps);
//printf("ps=%x value=%d %d\n", ps, PScheckValue(ps), devEnableIntrusion(NULL));
		break;
	case 7:
printf("### Tamper...\n");
		ps = PSobjectAtIndex(pss, index);
		AcuRequestStart(NULL, ps, NULL);
		AcuRequestSetOnSuccess(_AdmSetTamperOnSuccess);
		cr = crsGet(0);
		acuReqWriteCredentialReader(cr);
		PScheckRestoreValue(ps);
		break;
	}
}

void AdmHwCrCredFmts(void *pss);
void AdmSetAuthResultStart(void *pss);

void AdmHwDevStart(void *pss)
{
	void	*ps, *cr, *ad;
	int		dlu, idx;

	PSsetOnPrefChanged(pss, _AdmHwDevChanged);
	_AdmHwDevId(pss);
	PSaddTitle(pss, admModelTitle(), devModelName(sys_cfg), PS_TITLE_STYLE_SUBTITLE);
	dlu = devLocalUnitModel(NULL);
//	ps = PSaddScreen(pss, xmenu_device[0], AdmHwNwpStart);
	ps = PSaddScreen(pss, xmenu_access_point[0], AdmHwCrCredFmts);
	cr = crsGet(0);
	PSsetContext(ps, cr);
	ps = PSaddScreen(pss, admInputsOutputsTitle(), AdmHwDevChStart);
	_AdmAddAdSwitch(pss, 0);
	ad = adsGet(0);
	ps = PSaddCheck(pss, xmenu_armed[1], devEnableIntrusion, devSetEnableIntrusion);
	PSaddScreen(pss, xmenu_readerop[1], AdmSetAuthResultStart);
	ps = PSaddCheck(pss, xmenu_lockout[0], devEnableTamper, devSetEnableTamper);
}

void AdmSetAuthResultStart(void *pss)
{
	void	*ps;

	ps = PSaddStepper(pss, xmenu_authresult[1], syscfgAuthWaitTime, syscfgSetAuthWaitTime, 1, 255, 1, admSecondUnitTitle());
	PSstepperSetUnitRatio(ps, -10);	// desi-second
	ps = PSaddStepper(pss, xmenu_authresult[2], syscfgAuthResultTime, syscfgSetAuthResultTime, 1, 255, 1, admSecondUnitTitle());
	PSstepperSetUnitRatio(ps, -10);	// desi-second
	PSaddCheck(pss, xmenu_authresult[3], syscfgResultInhibitAuth, syscfgSetResultInhibitAuth);
}

