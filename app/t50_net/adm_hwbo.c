void _AdmAddBo(void *pss, int id)
{
	void	*ps, *bo;

	bo = bosGet(id);
	ps = PSaddScreen(pss, boAddressName(bo), AdmHwBoStart);
	PSsetInteractionEnabled(ps, boIsNonNull(bo));
	PSsetSubtitle(ps, boTypeName(bo));
	PSsetContext(ps, bo);
}

void _BoSetGeneral(void *bo, BOOL bVal)
{
	if(bVal) boCreate(bo, BO_TYPE_GENERAL);
	else	 boDelete(bo);
}

void _AdmAddBoSwitch(void *pss, int id)
{
	void	*ps, *bo;

	bo = bosGet(id);
	ps = PSaddSwitch(pss, boAddress(bo), boIsNonNull, _BoSetGeneral, AdmHwBoStart);
	PSsetContext(ps, bo);
}

void _AdmHwBoChanged(void *pss, int index, void *act)
{
	void	*ps, *bo;
	int		polarity;

	if(index == 3) {
		ps = PSobjectAtIndex(pss, index);
		bo = PScontext(ps);
		AcuRequestStart(NULL, ps, NULL);
		acuReqWriteBinaryOutput(bo);
		PSspinnerRestoreValue(ps);
	}
}

void AdmHwBoStart(void *pss)
{
	void	*ps, *bo, *ad;

	PSsetOnPrefChanged(pss, _AdmHwBoChanged);
	bo = PScontext(pss);
	PSaddTitle(pss, admAddressTitle(), boAddress(bo), PS_TITLE_STYLE_SUBTITLE);
	if(boType(bo) > 0) {
		ad = boAssignedDoor(bo);
		PSaddTitle(pss, xmenu_hardware[6], adAddress(ad), PS_TITLE_STYLE_SUBTITLE);
	}
	PSaddTitle(pss, admTypeTitle(), boTypeName(bo), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddSpinner(pss, xmenu_input_output[0], boPolarity, boSetPolarity);
	PSsetContext(ps, bo);
	PSspinnerAddTitleValue(ps, xmenu_io_polarity[0], 0);
	PSspinnerAddTitleValue(ps, xmenu_io_polarity[1], 1);
}

