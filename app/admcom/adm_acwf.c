static CF_WIEGAND_CFG	_wfcfg;
static CF_WIEGAND		*wfrd, _wfwr;
static unsigned int		_wfType;


void AdmWfRd(void *pss);
void AdmWfWr(void *pss);
void _AdmWfPrefChanged(void *pss, int index, void *act);

void AdmWf(void *pss)
{
	void	*ps, *cr, *wf;
	char	temp[64];
	int		i, tag, count;

	PSsetOnPrefChanged(pss, _AdmWfPrefChanged);
	cr = PScontext(pss);
	tag = PStag(pss);
	if(tag == 0 || tag == 1) _wfType = 0;
	else	_wfType = 1;
	count = cfWiegandsCount(_wfType);
	for(i = 0;i < count;i++) {
		wf = cfWiegandsAt(i, _wfType);
		cfWiegandGetName(wf, temp);
		if(tag == 1) {
			ps = PSaddCheck(pss, temp, cfWiegandIsEnableReader, cfWiegandSetEnableReader);
			PSsetButton(ps, 1, AdmWfRd);
		} else {
			ps = PSaddButton(pss, temp, NULL);  
			PSsetIntAction(ps, AdmWfRd);
		}
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		((CF_WIEGAND *)wf)->ctx = cr;
		PSsetContext(ps, wf);
	}
	if(count < MAX_CF_WIEGAND_SZ) {
		strcpy(temp, cfAddTitle());
		ps = PSaddButton(pss, temp, NULL);
		PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
		PSbuttonSetImageFile(ps, "add.bmp");
		_wfwr.cfg = &_wfcfg;
		PSsetContext(ps, &_wfwr);
		PSsetTag(ps, 1);
		PSsetIntAction(ps, AdmWfWr);
	}
}

void _AdmWfPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *wf;
	char	temp[64];
	int		resultCode;

	resultCode = PSresultCode(pss);
	ps = PSobjectAtIndex(pss, index);
	if(resultCode == RESULT_PREF_DELETED) {
printf("AdmWfPrefChanged: index=%d DELETED\n", index);
		wf = PScontext(ps);
		cfWiegandSetEnableReader(wf, FALSE);
		cfWiegandsRemove(cfWiegandId(wf), _wfType);
		PSremoveChildren(pss);
		AdmWf(pss);
		PrefReloadData(act);
	} else if(resultCode == RESULT_OK && PStag(ps) == 1) {
printf("added: index=%d Write-RESULT_OK\n", index);
		wf = cfWiegandsAdd(&_wfwr, _wfType);
		PSremoveChildren(pss);
		AdmWf(pss);
		PrefReloadData(act);
		ps = PSobjectWithContext(pss, wf);
		if(ps) {
			index = PSindexForObject(pss, ps);
			if(PStag(pss) == 1) PrefButtonCreate(ACT_PREF+index, ps);	
			else	PrefCreate(ACT_PREF+index, ps);
		}
	} else if(resultCode == RESULT_PREF_EDITED) {
printf("added: index=%d EDITED\n", index);
		wf = PScontext(ps);
		cfWiegandGetName(wf, temp);
		PSsetTitle(ps, temp);
		PrefReloadRowAtIndex(act, index);
	} else {
printf("AdmWfPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	}
}

void _AdmWfRdLoadData(void *pss)
{
	void	*wf, *ps, *fld;
	char	temp[64];
	int		i, id;

	wf = PScontext(pss);
	id = cfWiegandId(wf);
	if(id >= 4) {
		ps = PSaddButton(pss, NULL, NULL);	// Edit Button
		PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
		_wfwr.cfg = &_wfcfg;
		PSsetContext(ps, &_wfwr);
		PSsetIntAction(ps, AdmWfWr);
		ps = PSaddButton(pss, NULL, NULL);	// Trash Button
	}
	cfWiegandGetName(wf, temp);
	PSsetTitle(pss, temp);
	ps = PSaddTitle(pss, temp, NULL, PS_TITLE_STYLE_DEFAULT);
	ps = PSaddTitle(pss, credTitle(), cfWiegandCredentialName(wf), PS_TITLE_STYLE_SUBTITLE);
	if(_wfType == 0) {
		cfWiegandGetOutputWiegandName(wf, temp);
		ps = PSaddTitle(pss, cfOutputWiegandTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	}
	ps = PSaddTitle(pss, cfCodingTitle(), cfWiegandCodingName(wf), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfParityTitle(), cfWiegandParityName(wf), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfCardFieldsTitle(), NULL, PS_TITLE_STYLE_GROUP);
	for(i = 0;i < MAX_CARD_FIELD_SZ;i++) {
		fld = cfWiegandCardFieldAtIndex(wf, i);
		if(cardFieldType(fld) < 0) break;
		cardFieldGetOffsetBitName(fld, temp);
		ps = PSaddTitle(pss, cardFieldOffsetTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
		cardFieldGetLengthBitName(fld, temp);
		ps = PSaddTitle(pss, cardFieldTypeName(fld), temp, PS_TITLE_STYLE_SUBTITLE);
	}
	cfWiegandGetPaddingName(wf, temp);
	ps = PSaddTitle(pss, cfPaddingTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
}

void _AdmWfRdPrefChanged(void *pss, int index, void *act)
{
	void	*wf;

printf("AdmWfRdPrefChanged...%d\n", index);
	wf = PScontext(pss);
	if(index == 0) {
		cfWiegandCopy(wf, &_wfwr);
		PSremoveChildren(pss);
		_AdmWfRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmWfRd(void *pss)
{
printf("AdmWfRd...\n");
	PSsetOnPrefChanged(pss, _AdmWfRdPrefChanged);
	wfrd = PScontext(pss);
printf("AdmWfRd...1\n");
	_AdmWfRdLoadData(pss);
printf("AdmWfRd...2\n");
}

void _RefreshOutputWiegandSpinner(void *ps);

void _AdmWfWrLoadData(void *pss)
{
	void	*wf, *twf, *ps, *fld;
	char	temp[64];
	int		i, id, min, max, count, arr[8];

	wf = PScontext(pss);
	id = cfWiegandId(wf);
	cfWiegandGetName(wf, temp);
printf("id=%d cred=%d name=[%s]\n", id, cfWiegandCredential(wf), temp);
	ps = PSaddTitle(pss, temp, NULL, PS_TITLE_STYLE_DEFAULT);
	ps = PSaddSpinner(pss, credTitle(), cfWiegandCredential, cfWiegandSetCredential);
	PSsetContext(ps, wf);
	for(i = 0;i < MAX_CREDENTIAL_SZ;i++) PSspinnerAddTitleValue(ps, credName(i), i);
	if(_wfType == 0) {
		ps = PSaddSpinner(pss, cfOutputWiegandTitle(), cfWiegandOutputWiegand, cfWiegandSetOutputWiegand);
		PSsetContext(ps, wf);
		PSspinnerAddTitleValue(ps, admDisableTitle(), 255);
		for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) {
			twf = cfWiegandsGet(i, 1);
			if(twf) {
				cfWiegandGetName(twf, temp);
				PSspinnerAddTitleValue(ps, temp, cfWiegandId(twf));
			}
		}
		_RefreshOutputWiegandSpinner(ps);
	}
	ps = PSaddSpinner(pss, cfCodingTitle(), cfWiegandCoding, cfWiegandSetCoding);
	PSsetContext(ps, wf);
	count = cfWiegandGetAvailableCodings(wf, arr);
	for(i = 0;i < count;i++) PSspinnerAddTitleValue(ps, cfCodingName(arr[i]), arr[i]);
	ps = PSaddSpinner(pss, cfParityTitle(), cfWiegandParity, cfWiegandSetParity);
	PSsetContext(ps, wf);
	for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfParityName(i), i);
	for(i = 0;i < MAX_CARD_FIELD_SZ;i++) {
		fld = cfWiegandCardFieldAtIndex(wf, i);
		if(cardFieldType(fld) < 0) break;
		ps = PSaddTitle(pss, cfCardFieldsTitle(), NULL, PS_TITLE_STYLE_GROUP);
		max = cfWiegandMaximumCardFieldOffset(wf, i);
		ps = PSaddStepper(pss, cardFieldOffsetTitle(), cardFieldOffset, cardFieldSetOffset, 0, max, 1, admBitUnitTitle());
		PSsetContext(ps, fld);
		min = cfWiegandMinimumCardFieldLength(wf, i);
		max = cfWiegandMaximumCardFieldLength(wf, i);
		ps = PSaddStepper(pss, cardFieldTypeName(fld), cardFieldLength, cardFieldSetLength, min, max, 1, admBitUnitTitle());
		PSsetContext(ps, fld);
		min = cfWiegandCoding(wf);
		if(min == CF_CODING_BCD) max = 4;
		else if(min == CF_CODING_ASCII) max = 8;
		else	max = 0;
		PSstepperSetUnitRatio(ps, max);
	}
	max = cfWiegandMaximumPadding(wf);
	ps = PSaddStepper(pss, cfPaddingTitle(), cfWiegandPadding, cfWiegandSetPadding, 0, max, 1, admBitUnitTitle());
	PSsetContext(ps, wf);
}

void _AdmWfWrPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *wf;
	char	temp[64];
	int		id;
	
printf("AdmWfWrPrefChanged...%d\n", index);
	wf = PScontext(pss);
	if(index == 1) {				// credential
		PSremoveChildren(pss);
		cfWiegandSetId(wf, 4);
		cfWiegandSetDefault(wf, cfWiegandCredential(wf));
		_AdmWfWrLoadData(pss);
		PrefReloadData(act);
	} else if(!_wfType && index >= 5 || _wfType && index >= 4) {	// parity, cardFields, padding
		cfWiegandGetName(wf, temp);
		ps = PSobjectAtIndex(pss, 0);
		PSsetTitle(ps, temp);
		PrefReloadRowAtIndex(act, 0);
	}
}

void AdmWfWr(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmWfWrPrefChanged);
	if(PStag(pss)) {
		cfWiegandSetId(&_wfwr.id, 4);
		cfWiegandSetDefault(&_wfwr, CRED_INT32);
	} else	cfWiegandCopy(&_wfwr, wfrd);
	_AdmWfWrLoadData(pss);
printf("AdmWfWr...\n");
}

