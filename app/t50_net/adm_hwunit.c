void AdmHwUnitStart(void *pss);

void _AdmHwUnitListPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *unit;
	int		resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("UnitListPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode >= RESULT_OK) {
		unit = PScontext(ps);
		PSsetSubtitle(ps, unitModelName(unit));
		PrefReloadRowAtIndex(act, index);
	}
}

void AdmHwUnitListStart(void *pss)
{
	void	*ps, *ps2, *unit;
	char	temp[80];
	int		i, j, size, cnt, models[8];

	PSsetOnPrefChanged(pss, _AdmHwUnitListPrefChanged);
	size = unitsMaxSize();
	for(i = 0;i < size;i++) {
		unit = unitsGet(i);
		ps = PSaddSwitch(pss, unitAddress(unit), unitIsNonNull, NULL, AdmHwUnitStart);
		PSsetSubtitle(ps, unitModelName(unit));
		PSsetContext(ps, unit);
		if(devLocalUnitModel(NULL) && i == 0) PSsetAccessoryEnabled(ps, FALSE);
		else {
			cnt = unitGetAvailableModels(unit, models);
			ps2 = PSspinnerAlloc();
			PSspinnerInit(ps2, PStitle(ps), NULL, unitCreate);
			PSsetContext(ps2, unit);
			for(j = 0;j < cnt;j++) PSspinnerAddTitleValue(ps2, xmenu_unit_model[models[j]], models[j]);
			PSswitchSetOnDialog(ps, ps2);
			ps2 = PSalertAlloc();
			PSalertInit(ps2, PStitle(ps), GetPromptMsg(M_DELETE_CONFIRM), unitDelete);  
			PSsetContext(ps2, unit);
			PSswitchSetOffDialog(ps, ps2);
		}
	}
}

void AdmHwUnitStart(void *pss)
{
	void	*ps, *unit;
	char	*p, temp[80];
	int		i, id, model;

	unit = PScontext(pss);
	sprintf(temp, "%d", unitId(unit));
	PSaddTitle(pss, xmenu_unit_ch[4], temp, PS_TITLE_STYLE_SUBTITLE);
	PSaddTitle(pss, admModelTitle(), unitModelName(unit), PS_TITLE_STYLE_SUBTITLE);
	model = unitModel(unit);
	if(model == 4) {
		PSaddTitle(pss, xmenu_hardware[3], NULL, PS_TITLE_STYLE_GROUP);
		id = UNIT2XBO(unitId(unit));
		for(i = 0;i < 12;i++) _AdmAddBo(pss, id+i);
	} else if(model == 3) {
		PSaddTitle(pss, xmenu_hardware[2], NULL, PS_TITLE_STYLE_GROUP);
		id = UNIT2XBI(unitId(unit));
		for(i = 0;i < 24;i++) _AdmAddBi(pss, id+i);
	} else {
		id = UNIT2CH(unitId(unit));
		if(unitId(unit) == 0 && model == 0 && !devLocalUnitModel(NULL)) i = 1; else i = 0;
		for( ;i < 4;i++) {
			ps = PSaddScreen(pss, xmenu_unit_ch[i], AdmHwUnitChStart);
			PSsetContext(ps, (void *)(id+i));
		}
	}
}

