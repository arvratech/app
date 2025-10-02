static CF_CARD_CFG	_cfcfg;
static CF_CARD		*cfrd, _cfwr;


void _AdmCfPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *aa, *ad;
	char	temp[64];
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("AdmCfPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(PStype(ps) == PS_TYPE_RADIO_GROUP) {
printf("RadioGroup=%d\n");
		PSsetBackResultCode(pss, RESULT_OK+1);
	}
}

void AdmCfRd(void *pss);
void AdmCfWr(void *pss);

void AdmCfSc(void *pss)
{
	void	*ps, *psg, *cr, *cf;
	char	*p;
	int		i;

	PSsetOnPrefChanged(pss, _AdmCfPrefChanged);
printf("AdmcfSc...tag=%d\n", PStag(pss));
	cr = PScontext(pss);
	if(PStag(pss)) {
		psg = PSaddRadioGroup(pss, PStitle(pss), crScReaderFormat, crSetScReaderFormat);
	}
	for(i = 0;i < 3;i++) {
		cf = cfCardsAt(i);
		p = cfCardName(cf);
		if(PStag(pss)) {
			ps = PSaddRadio(pss, p, i, psg);
			PSsetButton(ps, 1, AdmCfRd);
			((CF_CARD *)cf)->ctx = cr;
		} else {
			ps = PSaddButton(pss, p, NULL); 
			PSsetIntAction(ps, AdmCfRd);
		}
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, cf);
	}
#ifdef _HYNIX
	//ps = PSaddCheck(pss, xmenu_card_format[7], crIsEnableHynixEmvReader, crSetEnableHynixEmvReader);
	//PSsetContext(ps, cr);
#endif
}

void _AdmCfRdLoadData(void *pss)
{
	void	*cf, *ps, *fld;
	char	temp[64];
	int		i, id;

	cf = PScontext(pss);
	cfCardValidate(cf);
printf("cf=%x\n", cf);
	id = cfCardId(cf);
	ps = PSaddButton(pss, NULL, NULL);	// Edit Button
	PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
	_cfwr.cfg = &_cfcfg;
	PSsetContext(ps, &_cfwr);
	PSsetIntAction(ps, AdmCfWr);
	ps = PSaddTitle(pss, cfCardName(cf), NULL, PS_TITLE_STYLE_DEFAULT); 
	ps = PSaddTitle(pss, credTitle(), cfCardCredentialName(cf), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfCardTypeTitle(), cfCardCardTypeName(cf), PS_TITLE_STYLE_SUBTITLE);
	if(id == CFCARD_SC_BLOCK || id == CFCARD_SC_FILE || id == CFCARD_SERIAL) {
		ps = PSaddTitle(pss, cfCardAppTitle(), cfCardCardAppName(cf), PS_TITLE_STYLE_SUBTITLE);
	}
	cfCardGetOutputWiegandName(cf, temp);
	ps = PSaddTitle(pss, cfOutputWiegandTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfCodingTitle(), cfCardCodingName(cf), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddTitle(pss, cfCardFieldsTitle(), NULL, PS_TITLE_STYLE_GROUP);
	for(i = 0;i < MAX_CARD_FIELD_SZ;i++) {
		fld = cfCardCardFieldAtIndex(cf, i);
		if(cardFieldType(fld) < 0) break;
		cardFieldGetOffsetByteName(fld, temp);
		ps = PSaddTitle(pss, cardFieldOffsetTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
		cardFieldGetLengthByteName(fld, temp);
		ps = PSaddTitle(pss, cardFieldTypeName(fld), temp, PS_TITLE_STYLE_SUBTITLE);
	}
	if(id == CFCARD_SC_BLOCK) {
		ps = PSaddTitle(pss,  cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		cfCardGetMifareKeyName(cf, temp);
		ps = PSaddTitle(pss, cfMifareKeyTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
		cfCardGetBlockNoName(cf, temp);
		ps = PSaddTitle(pss, cfBlockNoTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	} else if(id == CFCARD_SC_FILE) {
		ps = PSaddTitle(pss,  cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		cfCardGetAidName(cf, temp);
		ps = PSaddTitle(pss, cfAidTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
	} else if(id == CFCARD_SERIAL) {
		ps = PSaddTitle(pss,  cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		ps = PSaddTitle(pss, cfSerialSpeedTitle(), cfCardSerialSpeedName(cf), PS_TITLE_STYLE_SUBTITLE);
		ps = PSaddTitle(pss, cfSerialParityTitle(), cfCardSerialParityName(cf), PS_TITLE_STYLE_SUBTITLE);
		ps = PSaddTitle(pss, cfSerialStopBitsTitle(), cfCardSerialStopBitsName(cf), PS_TITLE_STYLE_SUBTITLE);
	}
printf("end cf=%x\n", cf);
}

void _AdmCfRdPrefChanged(void *pss, int index, void *act)
{
	void	*cf, *fld;

printf("AdmCfRdPrefChanged...%d\n", index);
	cf = PScontext(pss);
	if(index == 0) {
		cfCardCopy(cf, &_cfwr);
		PSremoveChildren(pss);
		_AdmCfRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmCfRd(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmCfRdPrefChanged);
	cfrd = PScontext(pss);
printf("AdmCrRd: %x\n", cfrd);
	_AdmCfRdLoadData(pss);
}

void _RefreshOutputWiegandSpinner(void *ps)
{
	void	*cf;
	char	temp[64];
	int		i, index, count, arr[8];

	cf = PScontext(ps);
	count = PSspinnerItemCount(ps);
	for(i = 1;i < count;i++) PSspinnerSetEnableAtIndex(ps, FALSE, i);
	count = cfCardGetAvailableOutputWiegands(cf, arr);
	for(i = 0;i < count;i++) {
		index = PSspinnerIndexWithValue(ps, arr[i]);
		if(index >= 0) {
			cfWiegandGetName(cfWiegandsGet(arr[i], 1), temp);
			PSspinnerSetTitleValueAtIndex(ps, temp, arr[i], index);
			PSspinnerSetEnableAtIndex(ps, TRUE, index);
		}
	}
}

void _AdmCfWrLoadData(void *pss)
{
	void	*cf, *ps, *wf, *fld;
	char	temp[64];
	int		i, id, count, min, max, arr[8];

	cf = PScontext(pss);
	id = cfCardId(cf);
	ps = PSaddTitle(pss, cfCardName(cf), NULL, PS_TITLE_STYLE_DEFAULT);
	if(id != CFCARD_SERIAL || cfCardCardType(cf) != 1) { 
		ps = PSaddSpinner(pss, credTitle(), cfCardCredential, cfCardSetCredential);
		PSsetContext(ps, cf);
		count = cfCardGetAvailableCredentials(cf, arr);
		for(i = 0;i < count;i++) PSspinnerAddTitleValue(ps, credName(arr[i]), arr[i]);
	}
	if(id == CFCARD_SC_UID || id == CFCARD_SC_BLOCK || id == CFCARD_SC_FILE || id == CFCARD_SERIAL) { 
		ps = PSaddSpinner(pss, cfCardTypeTitle(), cfCardCardType, cfCardSetCardType);
		PSsetContext(ps, cf);
		if(id == CFCARD_SC_UID) {
			PSspinnerAddTitleValue(ps, cfScUidCardType(0), 0);
			if(cfCardCredential(cf) == 1) PSspinnerAddTitleValue(ps, cfScUidCardType(1), 1);
		} else if(id == CFCARD_SC_BLOCK) {
			for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfScBlockCardType(i), i);
		} else if(id == CFCARD_SC_FILE) {
			for(i = 0;i < 3;i++) PSspinnerAddTitleValue(ps, cfScFileCardType(i), i);
		} else if(id == CFCARD_SERIAL) {
			for(i = 0;i < 3;i++) PSspinnerAddTitleValue(ps, cfSerialCardType(i), i);
		}
	}
	if(id == CFCARD_SC_BLOCK || id == CFCARD_SC_FILE || id == CFCARD_SERIAL && cfCardCardType(cf) != 1) { 
		ps = PSaddSpinner(pss, cfCardAppTitle(), cfCardCardApp, cfCardSetCardApp);
		PSsetContext(ps, cf);
		if(id == CFCARD_SC_BLOCK) {
			for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfScBlockCardAppName(i), i);
		} else if(id == CFCARD_SC_FILE) {
			for(i = 0;i < 3;i++) PSspinnerAddTitleValue(ps, cfScFileCardAppName(i), i);
		} else if(id == CFCARD_SERIAL) {
			for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfSerialCardAppName(i), i);
		}
	}
	ps = PSaddSpinner(pss, cfOutputWiegandTitle(), cfCardOutputWiegand, cfCardSetOutputWiegand);
	PSsetContext(ps, cf);
	PSspinnerAddTitleValue(ps, admDisableTitle(), 255);
	for(i = 0;i < MAX_CF_WIEGAND_SZ;i++) {
		wf = cfWiegandsGet(i, 1);
		if(wf) {
			cfWiegandGetName(wf, temp);
			PSspinnerAddTitleValue(ps, temp, cfWiegandId(wf));
		}
	}
	_RefreshOutputWiegandSpinner(ps);
	if(id == CFCARD_SC_BLOCK || id == CFCARD_SC_FILE || id == CFCARD_SERIAL && cfCardCardType(cf) != 1) { 
		ps = PSaddSpinner(pss, cfCodingTitle(), cfCardCoding, cfCardSetCoding);
		PSsetContext(ps, cf);
		count = cfCardGetAvailableCodings(cf, arr);
		for(i = 0;i < count;i++) PSspinnerAddTitleValue(ps, cfCodingName(arr[i]), arr[i]);
		ps = PSaddTitle(pss, cfCardFieldsTitle(), NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < MAX_CARD_FIELD_SZ;i++) {
			fld = cfCardCardFieldAtIndex(cf, i);
			if(cardFieldType(fld) < 0) break;
			max = cfCardMaximumCardFieldOffset(cf, i);
			ps = PSaddStepper(pss, cardFieldOffsetTitle(), cardFieldOffset, cardFieldSetOffset, 0, max, 1, admByteUnitTitle());
			PSsetContext(ps, fld);
			min = cfCardMinimumCardFieldLength(cf, i);
			max = cfCardMaximumCardFieldLength(cf, i);
			ps = PSaddStepper(pss, cardFieldTypeName(fld), cardFieldLength, cardFieldSetLength, min, max, 1, admByteUnitTitle());
			PSsetContext(ps, fld);
		}
	}
	if(id == CFCARD_SC_BLOCK) {
		ps = PSaddTitle(pss, cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		ps = PSaddTextField(pss, cfMifareKeyTitle(), AdmDoneValidateMifareKey);
		PSsetContext(ps, cf);
		PStextFieldAddItem(ps, PStitle(ps), cfCardMifareKeyName, cfCardSetMifareKeyName, TI_HEXANUMERIC, 12);
		ps = PSaddPicker(pss, cfBlockNoTitle(), cfCardGetBlockNo, cfCardSetBlockNo);
		PSsetContext(ps, cf);
		if(cfCardCardType(cf)) {
			PSpickerAddItem(ps, cfBlockTitle());
			for(i = 0;i < 20;i++) {
				sprintf(temp, "%s %d", cfBlockTitle(), i);
				PSpickerAtIndexAddTitle(ps, 0, temp);
			}
		} else {
			PSpickerAddItem(ps, cfSectorTitle());
			for(i = 0;i < 16;i++) {
				sprintf(temp, "%s %d", cfSectorTitle(), i);
				PSpickerAtIndexAddTitle(ps, 0, temp);
			}
			PSpickerAddItem(ps, cfBlockTitle());
			for(i = 0;i < 4;i++) {
				sprintf(temp, "%s %d", cfBlockTitle(), i);
				PSpickerAtIndexAddTitle(ps, 1, temp);
			}
		}
	} else if(id == CFCARD_SC_FILE) {
		ps = PSaddTitle(pss, cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		ps = PSaddTextField(pss, cfAidTitle(), AdmDoneValidateAid);
		PSsetContext(ps, cf);
		PStextFieldAddItem(ps, PStitle(ps), cfCardAidName, cfCardSetAidName, TI_HEXANUMERIC, 20);
	} else if(id == CFCARD_SERIAL) {
		ps = PSaddTitle(pss, cfReaderSettingTitle(), NULL, PS_TITLE_STYLE_GROUP);
		ps = PSaddSpinner(pss, cfSerialSpeedTitle(), cfCardSerialSpeed, cfCardSetSerialSpeed);
		PSsetContext(ps, cf);
		for(i = 0;i < 6;i++) PSspinnerAddTitleValue(ps, cfSerialSpeedName(i), i);
		ps = PSaddSpinner(pss, cfSerialParityTitle(), cfCardSerialParity, cfCardSetSerialParity);
		PSsetContext(ps, cf);
		for(i = 0;i < 3;i++) PSspinnerAddTitleValue(ps, cfSerialParityName(i), i);
		ps = PSaddSpinner(pss, cfSerialStopBitsTitle(), cfCardSerialStopBits, cfCardSetSerialStopBits);
		PSsetContext(ps, cf);
		for(i = 0;i < 2;i++) PSspinnerAddTitleValue(ps, cfSerialStopBitsName(i), i);
	}
}
	
void _AdmCfWrPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *cf, *fld;
	char	temp[64];
	int		id;
	
printf("AdmCfWrPrefChanged...%d\n", index);
	cf = PScontext(pss);
	if(index == 1) {				// credential
		PSremoveChildren(pss);
		cfCardSetDefault(cf, cfCardCredential(cf));
		_AdmCfWrLoadData(pss);
		PrefReloadData(act);
	} else if(index == 2) {
		if(cfCardId(cf) == CFCARD_SERIAL && cfCardCardType(cf) == 1) {
			PSremoveChildren(pss);
			cfCardSetDefaultSeialBarCode(cf);
			_AdmCfWrLoadData(pss);
fld = cfCardCardFieldAtIndex(cf, 0);
printf("C %x length=%d\n", fld, cardFieldLength(fld));
			PrefReloadData(act);
		}
	}
}

void AdmCfWr(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmCfWrPrefChanged);
	cfCardCopy(&_cfwr, cfrd);
	_AdmCfWrLoadData(pss);
}

