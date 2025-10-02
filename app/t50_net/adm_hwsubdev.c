void _AdmAddSubdev(void *pss, int id)
{
	void	*ps, *subdev;

	subdev = subdevsGet(id);
	ps = PSaddScreen(pss, subdevAddressName(subdev), AdmHwSubdevStart);
	PSsetInteractionEnabled(ps, subdevIsNonNull(subdev));
	PSsetSubtitle(ps, subdevModelName(subdev));
	PSsetContext(ps, subdev);
}

void _AdmAddSubdevSwitch(void *pss, int id)
{
	void	*ps, *subdev, *ps2, *cr;
	int		i, cnt, models[16];

	subdev = subdevsGet(id);
	ps = PSaddSwitch(pss, subdevAddressName(subdev), subdevIsNonNull, NULL, AdmHwSubdevStart);
	PSsetSubtitle(ps, subdevModelName(subdev));
	PSsetContext(ps, subdev);
	cnt = subdevGetAvailableModels(subdev, models);
	ps2 = PSspinnerAlloc();
	PSspinnerInit(ps2, PStitle(ps), NULL, subdevCreate);
	PSsetContext(ps2, subdev);
	for(i = 0;i < cnt;i++) PSspinnerAddTitleValue(ps2, xmenu_dev_model[models[i]+2], models[i]);
	PSswitchSetOnDialog(ps, ps2);
	ps2 = PSalertAlloc();
	PSalertInit(ps2, PStitle(ps), GetPromptMsg(M_DELETE_CONFIRM), subdevDelete);  
	PSsetContext(ps2, subdev);
	PSswitchSetOffDialog(ps, ps2);
//	PSsetAccessoryEnabled(ps, subdevNonNull(subdev));
}

void AdmHwCrCredFmts(void *pss);

void AdmHwSubdevStart(void *pss)
{
	void	*ps, *subdev, *cr, *ad;
	char	temp[80];
	int		model;

	subdev = PScontext(pss);
	PSaddTitle(pss, admAddressTitle(), subdevAddress(subdev), PS_TITLE_STYLE_SUBTITLE);
	PSaddTitle(pss, admModelTitle(), subdevModelName(subdev), PS_TITLE_STYLE_SUBTITLE);
	model = subdevModel(subdev);
	if(model > 1) ps = PSaddScreen(pss, xmenu_device[0], AdmHwNwpStart);
	ps = PSaddScreen(pss, xmenu_device[1], AdmHwCrCredFmts);
	PSsetContext(ps, cr);
//	if(model) {
//		sprintf(temp, "%d", subdevId(subdev));
//		PSaddTitle(pss, xmenu_unit_ch[4], temp, PS_TITLE_STYLE_SUBTITLE);
//	}
	if(subdevHaveIo(subdev)) {
		ps = PSaddScreen(pss, xmenu_device[4], AdmHwSubdevChStart);
		PSsetContext(ps, subdev);
	}
}

void AdmHwSubdevFromCrStart(void *pss)
{
	void	*ps, *subdev, *cr, *ad;
	char	temp[80];
	int		model;

	cr = PScontext(pss);
	subdev = subdevsGet(crId(cr));
	PSaddTitle(pss, admAddressTitle(), subdevAddress(subdev), PS_TITLE_STYLE_SUBTITLE);
	PSaddTitle(pss, admModelTitle(), subdevModelName(subdev), PS_TITLE_STYLE_SUBTITLE);
	if(crType(cr) >= 0) {
		ad = crAssignedDoor(cr);
		PSaddTitle(pss, xmenu_hardware[6], adAddress(ad), PS_TITLE_STYLE_SUBTITLE);
	} else {
		PSaddTitle(pss, xmenu_hardware[6], " ", PS_TITLE_STYLE_SUBTITLE);
	}
	PSaddTitle(pss, admTypeTitle(), crTypeName(cr), PS_TITLE_STYLE_SUBTITLE);
	model = subdevModel(subdev);
	if(model > 1) ps = PSaddScreen(pss, xmenu_device[0], AdmHwNwpStart);
	ps = PSaddScreen(pss, xmenu_device[1], AdmHwCrStart);
	PSsetContext(ps, cr);
//	if(model) {
//		sprintf(temp, "%d", subdevId(subdev));
//		PSaddTitle(pss, xmenu_unit_ch[4], temp, PS_TITLE_STYLE_SUBTITLE);
//	}
	if(subdevHaveIo(subdev)) {
		ps = PSaddScreen(pss, xmenu_device[4], AdmHwSubdevChStart);
		PSsetContext(ps, subdev);
	}
}

