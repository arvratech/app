BINARY_INPUT_CFG	binaryInputs[4];
BINARY_OUTPUT_CFG	binaryOutputs[2];
ACCESS_DOOR_CFG		accessDoors[1];
CREDENTIAL_READER_CFG	credentialReaders[1];
int _adBackup;


int _AdIsBackup(void)
{
	return _adBackup;
}

void _AdSetBackup(int val)
{
	_adBackup = val;
}

void _AdSetNonNull(void *ad, BOOL bVal)
{
	int		i;

	if(bVal) {
		if(_adBackup) {
printf("_AdSetNonNull: ad restore\n");
			for(i = 0;i < 4;i++) memcpy(&sys_cfg->binaryInputs[i], &binaryInputs[i], sizeof(BINARY_INPUT_CFG)); 
			for(i = 0;i < 2;i++) memcpy(&sys_cfg->binaryOutputs[i], &binaryOutputs[i], sizeof(BINARY_OUTPUT_CFG)); 
			memcpy(&sys_cfg->accessDoors[0], &accessDoors[0], sizeof(ACCESS_DOOR_CFG)); 
			memcpy(&sys_cfg->credentialReaders[0], &credentialReaders[0], sizeof(CREDENTIAL_READER_CFG)); 
		} else {
printf("_AdSetNonNull: adCreate\n");
			adCreate(ad);
		}
	} else {
printf("_AdSetNonNull: adDelete\n");
		for(i = 0;i < 4;i++) memcpy(&binaryInputs[i], &sys_cfg->binaryInputs[i], sizeof(BINARY_INPUT_CFG)); 
		for(i = 0;i < 2;i++) memcpy(&binaryOutputs[i], &sys_cfg->binaryOutputs[i], sizeof(BINARY_OUTPUT_CFG));
		memcpy(&accessDoors[0], &sys_cfg->accessDoors[0], sizeof(ACCESS_DOOR_CFG)); 
		memcpy(&credentialReaders[0], &sys_cfg->credentialReaders[0], sizeof(CREDENTIAL_READER_CFG)); 
		_adBackup = 1;
		adDelete(ad);
	}
}

void _BiSetDoorType(void *bi, BOOL bVal)
{
	if(bVal) biCreate(bi, BI_TYPE_DOOR);
	else	 biDelete(bi);
}

void _BoSetDoorType(void *bo, BOOL bVal)
{
	if(bVal) boCreate(bo, BO_TYPE_DOOR);
	else	 boDelete(bo);
}

int _AdIoSet(void *ad)
{
	int		id;

	id = adIoSet(ad);
	if(id >= 0 && id < 64) id >>= 5;	// 0, 1
	else	id = -1;
	return id;
}

void _AdSetIoSet(void *ad, int val)
{
	int		rval, ids[2];

	rval = adGetCreatableIoChs(ad, ids);
	if(val) adChangeIoCh(ad, ids[1]);
	else	adChangeIoCh(ad, ids[0]);
}

void _AdmAddAdSwitch(void *pss, int id)
{
	void	*ps, *ad;

	if(devMode(NULL)) {
		ad = adsGet(0);
		ps = PSaddSwitch(pss, adAddress(ad), adIsNonNull, _AdSetNonNull, AdmHwAdStart);
	} else {
		ad = adsGet(id);
		ps = PSaddSwitch(pss, adAddress(ad), adIsNonNull, _AdSetNonNull, AdmHwAdStart);
	}
	PSsetContext(ps, ad);
	_AdSetBackup(0);
}

static unsigned char	data, sync;

void AdmAcAa(void *pss);

void _AdmHwAdReload(void *pss)
{
	void	*ps, *ad, *cr, *subdev, *bi, *bo, *bo2;
	char	temp[64];
	int		i, id, ioId, ids[2];

	ad = PScontext(pss);
	id = adId(ad);
	PSaddTitle(pss, admAddressTitle(), adAddress(ad), PS_TITLE_STYLE_SUBTITLE);
	if(!devLocalUnitModel(NULL) && id == 0) {
		ps = PSaddScreen(pss, xmenu_door[0], NULL);
		PSsetSubtitle(ps, "Reader");
	} else {
		subdev = subdevsGet(id);
		ps = PSaddScreen(pss, xmenu_door[0], AdmHwSubdevStart);
		PSsetSubtitle(ps, subdevAddress(subdev));
		PSsetContext(ps, subdev);
	}
	data = sync = 0;
	cr = adSecondaryReader(ad);
	if(cr && crIsCreatableSecondary(cr)) {
		i = crId(cr);
printf("secondary id=%d\n", i);
		if(!devLocalUnitModel(NULL) && i == 0) {
			ps = PSaddSwitch(pss, xmenu_door[1], crIsNonNull, _CrSetSecondary, NULL);
			PSsetSubtitle(ps, devAddress(NULL));
		} else {
			ps = PSaddSwitch(pss, xmenu_door[1], crIsNonNull, _CrSetSecondary, AdmHwSubdevFromCrStart);
			subdev = subdevsGet(i);
			PSsetSubtitle(ps, subdevAddress(subdev));
		}
		PSsetContext(ps, cr);
		PSsetTag(ps, 1);
		if(crIsNonNull(cr)) data |= 0x01;
	}
	i = adGetCreatableIoChs(ad, ids);
printf("IoCh 1st: %d\n", ids[0]);
	if(i > 1) {
printf("IoCh 2nd: %d\n", ids[1]);
		ps = PSaddSpinner(pss, xmenu_door[2], _AdIoSet, _AdSetIoSet);
		PSsetContext(ps, ad);
		PSsetTag(ps, 2);
		if(!devLocalUnitModel(NULL) && id == 0) {
			PSspinnerAddTitle(ps, devAddress(NULL));
			PSspinnerAddTitle(ps, chAddress(id));
		} else {
			subdev = subdevsGet(id);
			PSspinnerAddTitle(ps, subdevAddress(subdev));
			PSspinnerAddTitle(ps, chAddress(id));
		}
	}
	ioId = adIoSet(ad);
	id = IO2BO(ioId);
	bo = bosGet(id);
	ps = PSaddScreen(pss, xmenu_door[3], AdmHwBoStart);
	PSsetSubtitle(ps, boAddress(bo));
	PSsetContext(ps, bo);
	id = IO2BI(ioId);
	bi = bisGet(id);
	ps = PSaddSwitch(pss, xmenu_door[4], biIsNonNull, _BiSetDoorType, AdmHwBiStart);
	PSsetSubtitle(ps, biAddress(bi));
	PSsetContext(ps, bi);
	PSsetTag(ps, 3);
	if(biIsCreatableDoor(bi)) {
		if(biIsNonNull(bi)) data |= 0x02;
	} else {
		PSsetAccessoryEnabled(ps, FALSE);
	}
	bi = bisGet(id+1);
	ps = PSaddSwitch(pss, xmenu_door[5], biIsNonNull, _BiSetDoorType, AdmHwBiStart);
	PSsetSubtitle(ps, biAddress(bi));
	PSsetContext(ps, bi);
	PSsetTag(ps, 4);
	if(biIsCreatableDoor(bi)) {
		if(biIsNonNull(bi)) data |= 0x04;
	} else {
		PSsetAccessoryEnabled(ps, FALSE);
	}
	if(devMode(NULL)) {
		bo = bosGet(boId(bo)+1);
		ps = PSaddSwitch(pss, xmenu_door[7], boIsNonNull, _BoSetDoorType, AdmHwBoStart);
		PSsetSubtitle(ps, boAddress(bo));
		PSsetContext(ps, bo);
		PSsetTag(ps, 5);
	} else {
		bo = bosGet(boId(bo)+1);
		ps = PSaddSwitch(pss, xmenu_door[6], boIsNonNull, _BoSetDoorType, AdmHwBoStart);
		PSsetSubtitle(ps, boAddress(bo));
		PSsetContext(ps, bo);
		PSsetTag(ps, 5);
		if(boIsCreatableDoor(bo)) {
			if(boIsNonNull(bo)) data |= 0x08;
		} else {
			PSsetAccessoryEnabled(ps, FALSE);
		}
		ps = PSaddScreen(pss, xmenu_door[7], AdmAcAa);
		aaGetName(aasGet(adAlarmAction(ad)), temp);
		PSsetSubtitle(ps, temp);
		PSsetTag(ps, 6);
		PSsetContext(ps, ad);
	}
	ps = PSaddStepper(pss, xmenu_door[9], adDoorPulseTime, adSetDoorPulseTime, 0, 255, 1, admSecondUnitTitle());
	PSsetContext(ps, ad);
	PSsetTag(ps, 7);
	ps = PSaddSpinner(pss, xmenu_door[11], adLockType, adSetLockType);
	PSsetContext(ps, ad);
	PSsetTag(ps, 8);
	for(i = 0;i < 2;i++) PSspinnerAddTitle(ps, xmenu_lock_type[i]);
}

void _PSobjectRestoreValue(void *ps);

void _AdmHwAdPostResult(GACTIVITY *act, void *ps)
{
	void	*pss, *obj;
	char	temp[80];
	BOOL	bVal;
	int		index, resultCode;

	pss = activityPsContext(act);
	index = PSindexForObject(pss, ps);
	obj = PScontext(ps);
	switch(PStag(ps)) {
	case 3:
		bVal = biIsNonNull(obj);
//		PSsetInteractionEnabled(ps, bVal);
		PrefReloadRowAtIndex(act, index);
		if(bVal && !(data & 0x02) || !bVal && (data & 0x02)) sync |= 0x02;
		break;
	case 4:
		bVal = biIsNonNull(obj);
//		PSsetInteractionEnabled(ps, bVal);
		PrefReloadRowAtIndex(act, index);
		if(bVal && !(data & 0x04) || !bVal && (data & 0x04)) sync |= 0x04;
		break;
	case 5:
		bVal = boIsNonNull(obj);
		PSsetInteractionEnabled(ps, bVal);
//		PrefReloadRowAtIndex(act, index);
		if(bVal && !(data & 0x08) || !bVal && (data & 0x08)) sync |= 0x08;
		break;
	case 6:
		if(devMode(NULL)) {

		} else {
			obj = PScontext(ps);
			aaGetName(aasGet(adAlarmAction(obj)), temp);
			PSsetSubtitle(ps, temp);
			PrefReloadRowAtIndex(act, index);
		}
		break;
	}
//	if(sync) resultCode = RESULT_OK+1; else resultCode = RESULT_CANCELLED;
resultCode = RESULT_CANCELLED;
	PSsetBackResultCode(pss, resultCode);
}

void AdmHwAdOnResult(GACTIVITY *act, int status, unsigned char *buf, int len)
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
			a = activityParent(act);
			AcuRequestDestroy(act);
			slvmWriteLocalIoConfig();
			_AdmHwAdPostResult(a, ps);
		}
	}
}

void _AdmHwAdPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *obj, *ad;
	char	temp[64];
	BOOL	bVal;
	int		cls, resultCode;

	ad = PScontext(pss);
	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("HwAdPrefChanged: index=%d resultCode=%d tag=%d\n", index, resultCode, PStag(ps));
	if(resultCode < RESULT_OK) return;
	obj = PScontext(ps);
	switch(PStag(ps)) {
	case 1:
		bVal = crIsNonNull(obj);
//		PSsetInteractionEnabled(ps, bVal);
		PrefReloadRowAtIndex(act, index);
		if(bVal && !(data & 0x01) || !bVal && (data & 0x01)) sync |= 0x01; else sync &= 0xfe;
		break;
	case 2:
printf("AD changed....\n");
		PSremoveChildren(pss);
		_AdmHwAdReload(pss);
		PrefReloadData(act);
		sync |= 0x80;
		break;
	case 3:
	case 4:
	case 5:
		AcuRequestStart(NULL, ps, AdmHwAdOnResult);
		acuReqWriteAccessDoor(ad);
		PSswitchRestoreValue(ps);
		break;
	case 6:
		if(devMode(NULL)) {
			AcuRequestStart(NULL, ps, AdmHwAdOnResult);
			acuReqWriteAccessDoor(ad);
			PSswitchRestoreValue(ps);
		} else {
			obj = PScontext(ps);
			aaGetName(aasGet(adAlarmAction(obj)), temp);
			PSsetSubtitle(ps, temp);
			PrefReloadRowAtIndex(act, index);
		}
		break;
	case 7:
		AcuRequestStart(NULL, ps, AdmHwAdOnResult);
		acuReqWriteAccessDoor(ad);
		PSstepperRestoreValue(ps);
		break;
	case 8:
		AcuRequestStart(NULL, ps, AdmHwAdOnResult);
		acuReqWriteAccessDoor(ad);
		PSspinnerRestoreValue(ps);
		break;
	}
//	if(sync) resultCode = RESULT_OK+1; else resultCode = RESULT_CANCELLED;
	resultCode = RESULT_CANCELLED;
	PSsetBackResultCode(pss, resultCode);
}

void AdmHwAdStart(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmHwAdPrefChanged);
	PSsetTag(pss, 0);
	_AdmHwAdReload(pss);
}

