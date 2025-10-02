void _AdmAddAdSwitch(void *pss, int id);
void _AdmAddSubdevSwitch(void *pss, int id);
void _AdmAddBi(void *pss, int id);
void _AdmAddBo(void *pss, int id);


void AdmHwDevChStart(void *pss)
{
	int		i;

	if(!devMode(NULL)) _AdmAddCr(pss, 0);
	if(bisGet(0)) {
		PSaddTitle(pss, xmenu_hardware[2], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 4;i++) _AdmAddBi(pss, i);
		PSaddTitle(pss, xmenu_hardware[3], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 2;i++) _AdmAddBo(pss, i);
	}
}

void _AdmHwChPrefChanged(void *pss, int index, void *act)
{
	void	*ps, *obj;
	int		cls, resultCode;

	ps = PSobjectAtIndex(pss, index);
	resultCode = PSresultCode(pss);
printf("HwChPrefChanged: index=%d resultCode=%d\n", index, resultCode);
	if(resultCode >= RESULT_OK) {
		obj = PScontext(ps);
		cls = hwObjectClass(obj);
		switch(cls) {
		case CLS_SUBDEVICE:
			PSsetSubtitle(ps, subdevModelName(obj));
			PrefReloadRowAtIndex(act, index);
			break;
		}
	}
}

void AdmHwUnitChStart(void *pss)
{
	void	*subdev, *cr;
	int		i, id, cid, dlu;

printf("AdmhwUnitCh...\n");
	PSsetOnPrefChanged(pss, _AdmHwChPrefChanged);
	id = (int)PScontext(pss);
	dlu = devLocalUnitModel(NULL);
	if(dlu || id > 0) {
		subdev = subdevsGet(id);
		cr = crsGet(id);
		if(!subdevIsNonNull(subdev) || !crIsNonNull(cr)) _AdmAddSubdevSwitch(pss, id);
		else	_AdmAddSubdev(pss, id);
	}
	if(dlu || id > 0) cid = CH2UNITBI(id);
	else	cid = 0;
	_AdmAddCr(pss, id);
	if(bisGet(cid)) {
		PSaddTitle(pss, xmenu_hardware[2], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 4;i++) _AdmAddBi(pss, cid+i);
		if(dlu || id > 0) cid = CH2UNITBO(id);
		else	cid = 0;
		PSaddTitle(pss, xmenu_hardware[3], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 2;i++) _AdmAddBo(pss, cid+i);
	}
}

void AdmHwSubdevChStart(void *pss)
{
	void	*subdev;
	int		i, id, cid;

	subdev = PScontext(pss);
	id = subdevId(subdev);
	cid = CH2SUBDEVBI(id);
	if(bisGet(cid)) {
		PSaddTitle(pss, xmenu_hardware[2], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 4;i++) _AdmAddBi(pss, cid+i);
		cid = CH2SUBDEVBO(id);
		PSaddTitle(pss, xmenu_hardware[3], NULL, PS_TITLE_STYLE_GROUP);
		for(i = 0;i < 2;i++) _AdmAddBo(pss, cid+i);
	}
}

