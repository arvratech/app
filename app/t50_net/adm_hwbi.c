void _AdmAddBi(void *pss, int id)
{
	void	*ps, *bi;

	bi = bisGet(id);
	ps = PSaddScreen(pss, biAddressName(bi), AdmHwBiStart);
	PSsetInteractionEnabled(ps, biIsNonNull(bi));
	PSsetSubtitle(ps, biTypeName(bi));
	PSsetContext(ps, bi);
}

void _BiSetGeneral(void *bi, BOOL bVal)
{
	if(bVal) biCreate(bi, BI_TYPE_GENERAL);
	else	 biDelete(bi);
}

void _AdmAddBiSwitch(void *pss, int id)
{
	void	*ps, *bi;

	bi = bisGet(id);
	ps = PSaddSwitch(pss, biAddress(bi), biIsNonNull, _BiSetGeneral, AdmHwBiStart);
	PSsetContext(ps, bi);
}

void _AdmHwBiChanged(void *pss, int index, void *act)
{
	void	*ps, *bi;
	int		polarity;

	if(index == 3) {
		ps = PSobjectAtIndex(pss, index);
		bi = PScontext(ps);
		AcuRequestStart(NULL, ps, NULL);
		acuReqWriteBinaryInput(bi);
		PSspinnerRestoreValue(ps);
	}
}

void AdmHwBiStart(void *pss)
{
	void	*ps, *bi, *ad;

	PSsetOnPrefChanged(pss, _AdmHwBiChanged);
	bi = PScontext(pss);
	PSaddTitle(pss, admAddressTitle(), biAddress(bi), PS_TITLE_STYLE_SUBTITLE);
	if(biType(bi) > 0) {
		ad = biAssignedDoor(bi);
		PSaddTitle(pss, xmenu_hardware[6], adAddress(ad), PS_TITLE_STYLE_SUBTITLE);
	}
	PSaddTitle(pss, admTypeTitle(), biTypeName(bi), PS_TITLE_STYLE_SUBTITLE);
	ps = PSaddSpinner(pss, xmenu_input_output[0], biPolarity, biSetPolarity);
	PSsetContext(ps, bi);
	PSspinnerAddTitleValue(ps, xmenu_io_polarity[0], 0);
	PSspinnerAddTitleValue(ps, xmenu_io_polarity[1], 1);
}

