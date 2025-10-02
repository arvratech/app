void _AdmAddCr(void *pss, int id)
{
	void	*ps, *cr;

	cr = crsGet(id);
	ps = PSaddScreen(pss, crAddressName(cr), AdmHwCrStart);
	PSsetInteractionEnabled(ps, crIsNonNull(cr));
	PSsetSubtitle(ps, crTypeName(cr));
	PSsetContext(ps, cr);
}

void _CrSetSecondary(void *cr, BOOL bVal)
{
	if(bVal) crSetType(cr, CR_TYPE_SECONDARY);
	else	 crSetType(cr, -1);
}

void _CrSetGeneral(void *cr, BOOL bVal)
{
	if(bVal) crCreate(cr, CR_TYPE_GENERAL);
	else	 crDelete(cr);
}

void _AdmAddCrSwitch(void *pss, int id)
{
	void	*ps, *cr;

	cr = crsGet(id);
	ps = PSaddSwitch(pss, crAddress(cr), crIsNonNull, _CrSetGeneral, AdmHwCrStart);
	PSsetContext(ps, cr);
}

void AdmHwCrStart(void *pss)
{
	void	*ps, *cr, *ad;

	cr = PScontext(pss);
	PSaddTitle(pss, admAddressTitle(), crAddress(cr), PS_TITLE_STYLE_SUBTITLE);
	if(crType(cr) > 0) {
		ad = crAssignedDoor(cr);
		PSaddTitle(pss, xmenu_hardware[6], adAddress(ad), PS_TITLE_STYLE_SUBTITLE);
	}
	PSaddTitle(pss, admTypeTitle(), crTypeName(cr), PS_TITLE_STYLE_SUBTITLE);
}

