static ALARM_ACTION_CFG	_aacfg;
static ALARM_ACTION		*aard, _aawr;

void AdmAaRd(void *pss);
void AdmAaWr(void *pss);
void _AdmAaPrefChanged(void *pss, int index, void *act);

void AdmAcAa(void *pss)
{
	void	*ps, *psg, *aa, *objs[MAX_AA_SZ];
	char	temp[64];
	int		i, count;

printf("AdmAcAa...\n");
	PSsetOnPrefChanged(pss, _AdmAaPrefChanged);
	if(PStag(pss)) {
		psg = PSaddRadioGroup(pss, PStitle(pss), adAlarmAction, adSetAlarmAction);
		PSsetContext(psg, PScontext(pss));
	}
	count = aasGetList(objs);
	for(i = 0;i < count;i++) {
		aa = objs[i];
		aaGetName(aa, temp);
		if(PStag(pss)) {
			ps = PSaddRadio(pss, temp, i, psg);
			PSsetButton(ps, 1, AdmAaRd);
		} else {
			ps = PSaddButton(pss, temp, NULL);  
			PSsetIntAction(ps, AdmAaRd);
		}
		PSsetViewStyle(ps, PS_VIEW_STYLE_READONLY);
		PSsetContext(ps, aa);
	}
	if(count < MAX_AA_SZ) {
		strcpy(temp, cfAddTitle());
		ps = PSaddButton(pss, temp, NULL);
		PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
		PSbuttonSetImageFile(ps, "add.bmp");
		_aawr.cfg = &_aacfg;
		PSsetContext(ps, &_aawr);
		PSsetTag(ps, 1);
		PSsetIntAction(ps, AdmAaWr);
	}
}

void _AdmAaPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *aa, *ad;
	char	temp[64];
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("AdmAaPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode == RESULT_PREF_DELETED) {
printf("deleted: index=%d resultCode=%d\n", index, resultCode);
		aa = PScontext(ps);
		aasRemove(aaId(aa));
		PSremoveChildren(pss);
		AdmAcAa(pss);
		PrefReloadData(act);
		PSsetBackResultCode(pss, RESULT_OK+1);
	} else if(resultCode == RESULT_OK && PStag(ps)) {
printf("added: index=%d resultCode=%d\n", index, resultCode);
		aa = PScontext(ps);
		aa = aasAdd(aa);
		PSremoveChildren(pss);
		AdmAcAa(pss);
		PrefReloadData(act);
		ps = PSobjectWithContext(pss, aa);
		if(ps) {
			index = PSindexForObject(pss, ps);
			if(PStag(pss)) PrefButtonCreate(ACT_PREF+index, ps);	
			else	PrefCreate(ACT_PREF+index, ps);
		}
	} else if(resultCode == RESULT_PREF_EDITED) {
		PrefReloadRowAtIndex(act, index);
	} else if(PStype(ps) == PS_TYPE_RADIO_GROUP) {
printf("RadioGroup=%d\n");
		PSsetBackResultCode(pss, RESULT_OK+1);
	}
}

void _AdmAaRdLoadData(void *pss)
{
	void	*ps, *aa, *aaac;
	char	temp[64];
	int		i, id, column[4];

	aa = PScontext(pss);
	id = aaId(aa);
	ps = PSaddButton(pss, NULL, NULL);	// Edit Button
	PSsetViewStyle(ps, PS_VIEW_STYLE_EDIT);
	_aawr.cfg = &_aacfg;
	PSsetContext(ps, &_aawr);
	PSsetIntAction(ps, AdmAaWr);
	if(id > 0) ps = PSaddButton(pss, NULL, NULL);	// Trash Button
	aaGetName(aa, temp);
	PSsetTitle(pss, temp);
	ps = PSaddTitle(pss, temp, NULL, PS_TITLE_STYLE_DEFAULT);
	column[0] = 16; column[1] = 100; column[2] = 184; column[3] = 320;
	sprintf(temp, "%s,%s,%s", xmenu_action_cmd[1], xmenu_action_cmd[2], xmenu_action_cmd[3]);
	ps = PSaddTitle(pss, xmenu_action_cmd[0], temp, PS_TITLE_STYLE_SUBTITLE | PS_TITLE_STYLE_GROUP | PS_TITLE_STYLE_COLUMN);
	PStitleSetColumn(ps, column);
	for(i = AC_INDEX_FIRE_ALARM;i <= AC_INDEX_ACCESS_DENIED;i++) {
		aaac = aaActionCommandAtIndex(aa, i);
		if(aaacEnable(aaac)) {
			sprintf(temp, "%d.%d,%d.%d,%d", aaacOnTime(aaac)/10, aaacOnTime(aaac)%10,
									aaacOffTime(aaac)/10, aaacOffTime(aaac)%10, aaacRepeatCount(aaac));
			ps = PSaddTitle(pss, xmenu_action_idx[i], temp, PS_TITLE_STYLE_SUBTITLE | PS_TITLE_STYLE_COLUMN);
			PStitleSetColumn(ps, column);
		} else {
			ps = PSaddTitle(pss, xmenu_action_idx[i], NULL, PS_TITLE_STYLE_SUBTITLE);
		}
	}
}

void _AdmAaRdPrefChanged(void *pss, int index, void *act)
{
	void	*aa;

printf("AdmAaRdPrefChanged...index=%d\n", index);
	aa = PScontext(pss);
	if(index == 0) {	// Edit
		aaCopy(aa, &_aawr);
		PSremoveChildren(pss);
		_AdmAaRdLoadData(pss);
		PrefReloadData(act);
	}
}

void AdmAaRd(void *pss)
{
	PSsetOnPrefChanged(pss, _AdmAaRdPrefChanged);
	aard = PScontext(pss);
printf("AdmAaRd...aa=%x\n", aard);
	_AdmAaRdLoadData(pss);
}

void AdmAaAc(void *pss);

void AdmAaWr(void *pss)
{
	void	*ps, *aa, *aaac;
	char	temp[64];
	int		i;

	aa = PScontext(pss);
	if(PStag(pss)) {
		//aasRemove(aa);
		aaGetName(NULL, temp);
	} else {
		aaCopy(aa, aard);
		aaGetName(aa, temp);
	}
	ps = PSaddTitle(pss, temp, NULL, PS_TITLE_STYLE_DEFAULT);
	for(i = AC_INDEX_FIRE_ALARM;i <= AC_INDEX_ACCESS_DENIED;i++) {
		aaac = aaActionCommandAtIndex(aa, i);
		ps = PSaddSwitch(pss, aaActionIndexName(aa, i), aaacEnable, aaacSetEnable, AdmAaAc);
		PSsetContext(ps, aaac);
	}
}

void AdmAaAc(void *pss)
{
	void	*ps, *aaac;

	aaac = PScontext(pss);
	ps = PSaddStepper(pss, xmenu_action_cmd[1], aaacOnTime, aaacSetOnTime, 1, 255, 1, admSecondUnitTitle());
	PSsetContext(ps, aaac);
	PSstepperSetUnitRatio(ps, -10);
	ps = PSaddStepper(pss, xmenu_action_cmd[2], aaacOffTime, aaacSetOffTime, 1, 255, 1, admSecondUnitTitle());
	PSsetContext(ps, aaac);
	PSstepperSetUnitRatio(ps, -10);
	ps = PSaddStepper(pss, xmenu_action_cmd[3], aaacRepeatCount, aaacSetRepeatCount, 0, 255, 1, "");
	PSsetContext(ps, aaac);
}

