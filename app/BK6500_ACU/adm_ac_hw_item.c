int _UnitItem(GWND *WizardWnd, int *Wizard, int Item, UNITS_CFG *NewUnits);

void WizardUnitItem(int Item, UNITS_CFG *NewUnits)
{
	GWND	*wnd, _wnd;
	char	title[20];
	int		rval, Stage, Wizard;

	wnd = &_wnd;
	GetUnitName(Item<<1, title); title[strlen(title)-2] = 0;
	WizardWndInitialize(wnd, title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 1) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _UnitItem(wnd, &Wizard, Item, NewUnits); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				Stage = 0;
			}
		}
		taskYield();
	}
}

void _InitUnitType(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg);
void _InitUnitEnable(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg);
void _InitDoorEnable(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg);
void _InitReaderType(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg);
void _UnitTypeChanged(GRADIO_GROUP *rdGroup, int Position, void *Context);

int _UnitItem(GWND *WizardWnd, int *Wizard, int Item, UNITS_CFG *NewUnits)
{
	GRADIO_GROUP	rdgUnitType, rdgUnitEnable[2], rdgDoorEnable[2], rdgReaderType[2];
	GRADIO	rdUnitType[3], rdUnitEnable[2][2], rdDoorEnable[2][2], rdReaderType[2][3];
	void	*Context[3];
	int		rval, i, y, h, val, UnitID;

	UnitID = Item << 1;	
	wndDrawClientArea(WizardWnd);
	if(UnitID > 1) radioGroupInitialize(&rdgUnitType, rdUnitType, 3);
	else	radioGroupInitialize(&rdgUnitType, rdUnitType, 2);
	for(i = 0;i < 2;i++) {
		radioGroupInitialize(&rdgUnitEnable[i], rdUnitEnable[i], 2);
		radioGroupInitialize(&rdgDoorEnable[i], rdDoorEnable[i], 2);
		radioGroupInitialize(&rdgReaderType[i], rdReaderType[i], 2);
	}	
	h = 22; y = 2;
	unitscfgSet(NewUnits);
	_InitUnitType(UnitID, WizardWnd, y, &rdgUnitType);
	y += h + h + 0;
	for(i = 0;i < 2;i++) {
		_InitUnitEnable(UnitID+i, WizardWnd, y, &rdgUnitEnable[i]);
		y += h + 0;
		_InitDoorEnable(UnitID+i, WizardWnd, y, &rdgDoorEnable[i]);
		y += h + 0;
		_InitReaderType(UnitID+i, WizardWnd, y, &rdgReaderType[i]);
		y += h + 0;
	}
	syscfgSetUnits(sys_cfg);
	radioGroupDrawWindow(&rdgUnitType);
	for(i = 0;i < 2;i++) {
		radioGroupDrawWindow(&rdgUnitEnable[i]);
		radioGroupDrawWindow(&rdgDoorEnable[i]);
		radioGroupDrawWindow(&rdgReaderType[i]);
	}
	while(1) {
		Context[0] = (void *)UnitID; Context[1] = &rdgReaderType[0]; Context[2] = &rdgReaderType[1];
		rval = RadioGroupWizardEx(&rdgUnitType, *Wizard, _UnitTypeChanged, Context);
		for(i = 0;i < 2;i++) {
			if(rval >= 3) rval = RadioGroupWizard(&rdgUnitEnable[i], *Wizard);
			if(rval >= 3) rval = RadioGroupWizard(&rdgDoorEnable[i], *Wizard);
			if(rval >= 3 && radioGroupIsEnabled(&rdgReaderType[i])) rval = RadioGroupWizard(&rdgReaderType[i], *Wizard);
		}
		if(rval == 1 || rval == 2) {
			unitscfgSet(NewUnits);
			val = radioGroupGetChecked(&rdgUnitType);
			if(UnitID < 2 && val) val = 2;
			unitSetType(UnitID, val);
			for(i = 0;i < 2;i++) {
				val = radioGroupGetChecked(&rdgUnitEnable[i]);
				unitSetEnable(UnitID+i, val);
				val = radioGroupGetChecked(&rdgDoorEnable[i]);
				adSetEnable(UnitID+i, val);
				val = radioGroupGetChecked(&rdgReaderType[i]);
				crSetType(UnitID+i, val);
			}
			syscfgSetUnits(sys_cfg);
		}
		if(rval < 3) break;
	}
	return rval;
}

#define RDG_LABEL_WIDTH		104

void _InitUnitType(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, idx, val;

	w = 100; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text; strcpy(p, xmenu_hw_config[0]); p += strlen(p); *p++ = ' '; strcpy(p, xmenu_hw_config[7]);
	labelSetWindowText(&rdg->lblTitle, text);
	idx = 0;
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, idx); idx++;
	radioInitialize(radio, wnd, x, y, w, h); x += w+8;
	if(UnitID > 1) p = xmenu_unit_type[0]; else p = xmenu_hw_config[4];
	radioSetWindowText(radio, p);
	if(UnitID > 1) {
		radio = radioGroupAt(rdg, idx); idx++;
		radioInitialize(radio, wnd, x, y, w, h);
		radioSetWindowText(radio, xmenu_unit_type[1]);
	} 
	x = RDG_LABEL_WIDTH;
	y += 22;
	radio = radioGroupAt(rdg, idx);
	p = xmenu_unit_type[2];
	radioInitialize(radio, wnd, x, y, GetStringWidth(p)+26, h);
	radioSetWindowText(radio, p); 
	val = unitGetType(UnitID);
	if(UnitID < 2) {
		if(val == 2) val = 1; else val = 0;
	} 
	radioGroupSetChecked(rdg, val);
}

void _InitUnitEnable(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, val;
	
	w = 100; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text;	
	if(UnitID & 1) strcpy(p, xmenu_hw_config[10]); else strcpy(p, xmenu_hw_config[9]); 
	labelSetWindowText(&rdg->lblTitle, text);
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, 0);
	radioInitialize(radio, wnd, x, y, w, h); x += w+8; 
	radioSetWindowText(radio, xmenu_inuse[0]);
	radio = radioGroupAt(rdg, 1);
	radioInitialize(radio, wnd, x, y, w, h);
	radioSetWindowText(radio, xmenu_inuse[1]);
	if(unitGetEnable(UnitID)) val = 1; else val = 0;
	radioGroupSetChecked(rdg, val);
}

void _InitDoorEnable(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, val;
	
	w = 100; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text; *p++ = ' '; *p++ = ' '; strcpy(p, xmenu_hw_config[6]);
	labelSetWindowText(&rdg->lblTitle, text);
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, 0);
	radioInitialize(radio, wnd, x, y, w, h); x += w+8; 
	radioSetWindowText(radio, xmenu_inuse[0]);
	radio = radioGroupAt(rdg, 1);
	radioInitialize(radio, wnd, x, y, w, h);
	radioSetWindowText(radio, xmenu_inuse[1]); 
	if(adGetEnable(UnitID)) val = 1; else val = 0;
	radioGroupSetChecked(rdg, val);
}

void _InitReaderType(int UnitID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, val, type;
	
	w = 100; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text; *p++ = ' '; *p++ = ' '; strcpy(p, xmenu_hw_config[1]);
	labelSetWindowText(&rdg->lblTitle, text);
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, 0);
	radioInitialize(radio, wnd, x, y, w+4, h); x += w+8;
	type = unitGetType(UnitID);
	if(type == 2) val = 3; else val = 2;
	radioSetWindowText(radio, xmenu_hw_config[val]);
	radio = radioGroupAt(rdg, 1);
	radioInitialize(radio, wnd, x, y, w, h);
	if(type == 2) val = 18; else val = 3;
	radioSetWindowText(radio, xmenu_hw_config[val]);
	val = crGetType(UnitID);
	radioGroupSetChecked(rdg, val);
//	if(type == 2) radioGroupModifyStyle(rdg, 0, WS_DISABLED);	// add
}

void _UnitTypeChanged(GRADIO_GROUP *rdg, int Position, void *Context)
{
	GRADIO_GROUP	*rdg1, *rdg2;
	GRADIO	*radio;
	void	**pContext;
	int		UnitID;

	pContext	= (void **)Context;
	UnitID		= (int)pContext[0];
	rdg1		= (GRADIO_GROUP *)pContext[1];
	rdg2		= (GRADIO_GROUP *)pContext[2];
	radioGroupDrawBackGround(rdg1);
	radioGroupDrawBackGround(rdg2);
	if(UnitID < 2 && Position == 1 || UnitID > 1 && Position == 2) {
//		radioGroupModifyStyle(rdg1, 0, WS_DISABLED);	// add
		//radioGroupSetChecked(rdg1, 1);
//		radioGroupModifyStyle(rdg2, 0, WS_DISABLED);	// add
		radio = radioGroupAt(rdg1, 0);
		radioSetWindowText(radio, xmenu_hw_config[3]);
		radio = radioGroupAt(rdg1, 1);
		radioSetWindowText(radio, xmenu_hw_config[18]);
		radio = radioGroupAt(rdg2, 0);
		radioSetWindowText(radio, xmenu_hw_config[3]);
		radio = radioGroupAt(rdg2, 1);
		radioSetWindowText(radio, xmenu_hw_config[18]);		
	} else {
//		radioGroupModifyStyle(rdg1, WS_DISABLED, 0);	// remove
		//radioGroupSetChecked(rdg1, index);
//		radioGroupModifyStyle(rdg2, WS_DISABLED, 0);	// remove
		radio = radioGroupAt(rdg1, 0);
		radioSetWindowText(radio, xmenu_hw_config[2]);
		radio = radioGroupAt(rdg1, 1);
		radioSetWindowText(radio, xmenu_hw_config[3]);
		radio = radioGroupAt(rdg2, 0);
		radioSetWindowText(radio, xmenu_hw_config[2]);
		radio = radioGroupAt(rdg2, 1);
		radioSetWindowText(radio, xmenu_hw_config[3]);
	}
	radioGroupDrawWindow(rdg1);
	radioGroupDrawWindow(rdg2);
}

int _IOUnitItem(GWND *WizardWnd, int *Wizard, int Item, UNITS_CFG *NewUnits);

void WizardIOUnitItem(int Item, UNITS_CFG *NewUnits)
{
	GWND	*wnd, _wnd;
	char	title[20];
	int		rval, Stage, Wizard;

	wnd = &_wnd;
	GetIOUnitName(Item, title);
	WizardWndInitialize(wnd, title);
	wndDrawWindow(wnd);
	Stage = 1;
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < 1) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:	rval = _IOUnitItem(wnd, &Wizard, Item, NewUnits); break;
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				Stage = 0;
			}
		}
		taskYield();
	}
}

void _InitIOUnitType(int ID, GWND *wnd, int y, GRADIO_GROUP *rdg);
void _InitIOUnitEnable(int ID, GWND *wnd, int y, GRADIO_GROUP *rdg);

int _IOUnitItem(GWND *WizardWnd, int *Wizard, int Item, UNITS_CFG *NewUnits)
{
	GRADIO_GROUP	rdgUnitType, rdgUnitEnable;
	GRADIO	rdUnitType[2], rdUnitEnable[2];
	int		rval, y, h, val, ID;

	ID = Item;	
	wndDrawClientArea(WizardWnd);
	radioGroupInitialize(&rdgUnitType, rdUnitType, 2);
	radioGroupInitialize(&rdgUnitEnable, rdUnitEnable, 2);
	h = 22; y = 12;
	unitscfgSet(NewUnits);
	_InitIOUnitType(ID, WizardWnd, y, &rdgUnitType);
	y += h + h + h;
	_InitIOUnitEnable(ID, WizardWnd, y, &rdgUnitEnable);
	syscfgSetUnits(sys_cfg);
	radioGroupDrawWindow(&rdgUnitType);
	radioGroupDrawWindow(&rdgUnitEnable);
	while(1) {
		if(iounitCanTypeInput(ID)) rval = RadioGroupWizard(&rdgUnitType, *Wizard);
		else	rval = 3;
		if(rval >= 3) rval = RadioGroupWizard(&rdgUnitEnable, *Wizard);
		if(rval == 1 || rval == 2) {
			unitscfgSet(NewUnits);
			val = radioGroupGetChecked(&rdgUnitType);
			iounitSetType(ID, val);
			val = radioGroupGetChecked(&rdgUnitEnable);
			iounitSetEnable(ID, val);
			syscfgSetUnits(sys_cfg);
		}
		if(rval < 3) break;
	}
	return rval;
}

#define RDG_LABEL_WIDTH		104

void _InitIOUnitType(int ID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, val;
	
	w = 160; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text; strcpy(p, xmenu_hw_config[0]); p += strlen(p); *p++ = ' '; strcpy(p, xmenu_hw_config[7]);
	labelSetWindowText(&rdg->lblTitle, text);
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, 0);
	radioInitialize(radio, wnd, x, y, w, h); 
	radioSetWindowText(radio, xmenu_iounit_type[0]);
	radio = radioGroupAt(rdg, 1);
	radioInitialize(radio, wnd, x, y+h, w, h);
	radioSetWindowText(radio, xmenu_iounit_type[1]);
	if(iounitCanTypeInput(ID)) {
		val = iounitGetType(ID);
		radioGroupSetChecked(rdg, val);
	} else {	
		radioGroupModifyStyle(rdg, 0, WS_DISABLED);	// add
		radioGroupSetChecked(rdg, 1);
	}
}

void _InitIOUnitEnable(int ID, GWND *wnd, int y, GRADIO_GROUP *rdg)
{
	GRADIO	*radio;
	char	*p, text[40];
	int		x, w, h, val;
	
	w = 100; h = 22;
	x = 2;
	labelInitialize(&rdg->lblTitle, wnd, x, y-1, RDG_LABEL_WIDTH, 24);
	p = text;
	strcpy(p, xmenu_hw_config[16]); 
	labelSetWindowText(&rdg->lblTitle, text);
	x = RDG_LABEL_WIDTH;
	radio = radioGroupAt(rdg, 0);
	radioInitialize(radio, wnd, x, y, w, h); 
	radioSetWindowText(radio, xmenu_inuse[0]);
	radio = radioGroupAt(rdg, 1);
	radioInitialize(radio, wnd, x, y+h, w, h);
	radioSetWindowText(radio, xmenu_inuse[1]); 
	if(iounitGetEnable(ID)) val = 1; else val = 0;
	radioGroupSetChecked(rdg, val);
}

