int _ProcessMeals(GWND *WizardWnd, int Wizard, int MenuID, unsigned char *Tms, unsigned char *Count);
int _AvoidMealsOverlap(GWND *WizardWnd, int Wizard, unsigned char *Option);
int _MealsPrinter(GWND *WizardWnd, int Wizard, unsigned char *Option);
#ifdef MEALS_NUMBER
int _MealsInputNumber(GWND *WizardWnd, int Wizard, unsigned char *Option);
#endif
int _MealsFinalStage(GWND *WizardWnd, int Wizard, unsigned char Tms[][4], unsigned char *Count, unsigned char Option);
int _SaveMealsSetup(GWND *WizardWnd, char *Title, unsigned char NewTms[][4], unsigned char *NewCount, unsigned char NewOption);

void WizardSetupMeals(char *Title)
{
	GWND		*wnd, _wnd;
	unsigned char	Tms[5][4], Count[5], Option;
	int		rval, Stage, Wizard, max;

	AdminConsoleUnsupported();
	for(Stage = 0;Stage < 5;Stage++) {
		memcpy(Tms[Stage], sys_cfg->MealTimeSection[Stage], 4);
		Count[Stage] = sys_cfg->MealMenu[Stage];
	}
	Option = sys_cfg->MealOption;
	wnd = &_wnd;
	WizardWndInitialize(wnd, Title);
	wndDrawWindow(wnd);
	Stage = 1;
#ifdef MEALS_NUMBER
	max = 9;
#else
	max = 8;
#endif
	while(Stage) {
		Wizard = GWND_WIZARD;
		if(Stage < max) Wizard |= GWND_WIZARD_NEXT;
		if(Stage > 1) Wizard |= GWND_WIZARD_BACK;
		switch(Stage) {
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:	rval = _ProcessMeals(wnd, Wizard, Stage, Tms[Stage-1], &Count[Stage-1]); break;
		case 6: rval = _AvoidMealsOverlap(wnd, Wizard, &Option); break;
		case 7: rval = _MealsPrinter(wnd, Wizard, &Option); break;
#ifdef MEALS_NUMBER
		case 8: rval = _MealsNumber(wnd, Wizard, &Option); break;
		case 9: rval = _MealsFinalStage(wnd, Wizard, Tms, Count, Option); break;
#else		
		case 8: rval = _MealsFinalStage(wnd, Wizard, Tms, Count, Option); break;
#endif
		}
		if(rval <= 0) {
			Stage = 0;
		} else if(rval == 1) {
			Stage--;
		} else {
			if(Wizard & GWND_WIZARD_NEXT) Stage++;
			else {	// Final Stage
				_SaveMealsSetup(wnd, Title, Tms, Count, Option);
				Stage = 0;
			}
		}
		taskYield();
	}
}

int _ProcessMeals(GWND *WizardWnd, int Wizard, int MenuID, unsigned char *Tms, unsigned char *Count)
{
	GLABEL	*lblMenuName, _lblMenuName, *lblTime, _lblTime, *lblTimeRange, _lblTimeRange;
	GLABEL	*lblCount, _lblCount, *lblCountGuide, _lblCountGuide;
	GEDIT	*edtCount, _edtCount;
	GDATE	*tmsInput, _tmsInput;
	RECT	rect;
	unsigned long	NewVal;
	char	buf[64];
	int		rval, c, y;

	wndDrawClientArea(WizardWnd);
	y = 8;
	lblMenuName = &_lblMenuName;
	labelInitialize(lblMenuName, WizardWnd, 8, y, 150, 24);
	labelSetWindowText(lblMenuName, xmenu_meal[MenuID]); 
	y += 24;
	lblTime = &_lblTime;
	labelInitialize(lblTime, WizardWnd, 16, y, 150, 24);
	labelSetWindowText(lblTime, xmenu_meal_sub[0]); 
	y += 23;
	tmsInput = &_tmsInput;
	dateInitialize(tmsInput, WizardWnd, 24, y, 130, 26); 
	dateSetFont(tmsInput, GWND_NORMAL_FONT);
	dateModifyStyle(tmsInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
	dateSetType(tmsInput, DT_TIME_SECTION);		
	memcpy(tmsInput->Data, Tms, 4);				
	lblTimeRange = &_lblTimeRange;
	labelInitialize(lblTimeRange, WizardWnd, 160, y+1, 100, 24);
	labelSetWindowText(lblTimeRange, "(hh:mm-hh:mm)"); 
	wndGetClientArea(WizardWnd, &rect);
	y += 27;	
	y += 32;
	lblCount = &_lblCount;
	labelInitialize(lblCount, WizardWnd, 16, y, 80, 24);
	labelSetWindowText(lblCount, xmenu_meal_sub[1]);
	y += 24;
	edtCount = &_edtCount;
	editInitialize(edtCount, WizardWnd, 24, y, 32, 26); 
	editSetFont(edtCount, GWND_NORMAL_FONT);
	editLimitText(edtCount, 1);
	editModifyStyle(edtCount, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	sprintf(edtCount->Text, "%d", (int)*Count);
	lblCountGuide = &_lblCountGuide;
	labelInitialize(lblCountGuide, WizardWnd, 68, y+1, 160, 24);
	MakeGuideNumberNormal(buf, 1, 4);
	labelSetWindowText(lblCountGuide, buf);
	while(1) {
		labelDrawWindow(lblMenuName);
		labelDrawWindow(lblTime);
		labelDrawWindow(lblTimeRange);
		dateDrawWindow(tmsInput);
		y = 82;		// 8 + 24 + 23 + 27	
		DrawInputDateTimeDesp(rect.x+40, rect.y+y);
		labelDrawWindow(lblCount);
		editDrawWindow(edtCount);
		labelDrawWindow(lblCountGuide);
		y += 56;	// 32 + 24					
		DrawInputDigitDesp(rect.x+140, rect.y+y);
		rval = dateProcessWindow(tmsInput, Wizard);
		if(rval > 0) {
			dateDrawWindow(tmsInput); c = tmsInput->Edit.LastKey;
		}
		if(rval > 0 && c == KEY_OK) {
			rval = editProcessWindow(edtCount, Wizard);
			if(rval > 0) {
				editDrawWindow(edtCount); c = edtCount->LastKey;
			}
		}
		if(rval < 1) break;
		rval = 0;
		if(c == KEY_F1 || c == KEY_F4) {
			NewVal = n_check_atou(edtCount->Text);
			if(!tmsIsNull(tmsInput->Data) && !tmsValidateEx(tmsInput->Data)) {
				ResultMsg(WizardWnd, xmenu_meal_sub[0], R_INVALID_DATA);
			} else if(NewVal < 1 || NewVal > 4) {
				ResultMsg(WizardWnd, xmenu_meal_sub[1], R_INVALID_DATA);
			} else {
				*Count = (unsigned short)NewVal;	
				memcpy(Tms, tmsInput->Data, 4);
				if(c == KEY_F1) rval = 1; else rval = 2;
			}
		}
		if(rval) break;
		taskYield();
	}
	return rval;
}

int _AvoidMealsOverlap(GWND *WizardWnd, int Wizard, unsigned char *Option)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuNoYesWizard(WizardWnd, xmenu_meal[6], Option, MEALS_AVOID_OVERLAP, Wizard);
	return rval;
}

int _MealsPrinter(GWND *WizardWnd, int Wizard, unsigned char *Option)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuNoYesWizard(WizardWnd, xmenu_meal[7], Option, MEALS_PRINTER, Wizard);
	return rval;
}

#ifdef MEALS_NUMBER

int _MealsNumber(GWND *WizardWnd, int Wizard, unsigned char *Option)
{
	int		rval;

	wndDrawClientArea(WizardWnd);
	rval = MenuNoYesWizard(WizardWnd, xmenu_meal[8], Option, ENABLE_MEALS_COUNT, Wizard);
	return rval;
}

#endif

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Continue  2=Back Key  3=Next key
int _MealsFinalStage(GWND *WizardWnd, int Wizard, unsigned char Tms[][4], unsigned char *Count, unsigned char Option)
{
	RECT	rect;
	char	temp[32];
	unsigned long	timer;
	int		c, x, y, fh, BtnFile, Color, timeout, contflg;

	wndDrawClientArea(WizardWnd);
	Color = wndGetBackGroundTextColor(WizardWnd);
	wndGetClientArea(WizardWnd, &rect);
	fh = GetFontHeight() + 2;	// +4
	x = rect.x + 10; y = rect.y + 10;
	lcdPutText(x+110, y, xmenu_meal_sub[0], Color);
	c = GetStringWidth(xmenu_meal_sub[1])+2;
	lcdPutText(lcdGetWidth()-c, y, xmenu_meal_sub[1], Color);
	for(c = 0;c < 5;c++) {
		y += fh - 2;
		lcdPutText(x, y, xmenu_meal[c+1], Color);
		TimeSection2Str(Tms[c], temp);
		lcdPutText(x+110, y, temp, Color);
		temp[0] = Count[c] + '0'; temp[1] = 0;
		lcdPutText(x+250, y, temp, Color);
	}
	y += fh;
	lcdPutText(x, y, xmenu_meal[6], Color);
	if(Option & MEALS_AVOID_OVERLAP) c = BOX_CHECKED; else c = BOX_UNCHECKED;
	lcdPutIconFile(x+110, y, fh+2, c);
	y += fh;
	lcdPutText(x, y, xmenu_meal[7], Color);
	if(Option & MEALS_PRINTER) c = BOX_CHECKED; else c = BOX_UNCHECKED;
	lcdPutIconFile(x+110, y, fh+2, c);

	if(Wizard & GWND_WIZARD_BACK) c = BTN_BACK; else c = 0;
	_DrawLowerButtons(c, 0, 0, BTN_OK, 0);
	timer = DS_TIMER; timeout = 150;
	contflg = 1;
	while(contflg) {
		//if(menuGetGlobalExit()) c = KEY_CANCEL;
		//else	c = keyGet();
		c = keyGet();
		if(!c) {
			if((DS_TIMER-timer) >= timeout) c = -1;
		} else {
			timer = DS_TIMER;
		}
		switch(c) {
		case -1:
			c = GWND_TIMEOUT; contflg = 0;
			break;
		case KEY_CANCEL:
			c = GWND_QUIT; contflg = 0;
			break;
		case KEY_F1:
			if(Wizard & GWND_WIZARD_BACK) {
				c = 1; contflg = 0;
			} else if(!(Wizard & GWND_WIZARD) && menuGetGlobalExit()) {
				c = GWND_QUIT; contflg = 0;
			}
			break;
		case KEY_OK:
		case KEY_F4:
			if(BtnFile) {
				c = 2; contflg = 0;
			} 
			break;
		}
		taskYield();
	}
	return c;
}

int _SaveMealsSetup(GWND *WizardWnd, char *Title, unsigned char NewTms[][4], unsigned char *NewCount, unsigned char NewOption)
{
	unsigned char	OldTms[5][4], OldCount[5], OldOption;
	int		i, rval;

	for(i = 0;i < 5;i++) {
		memcpy(OldTms[i], sys_cfg->MealTimeSection[i], 4);
		OldCount[i] = sys_cfg->MealMenu[i];
	}
	OldOption = sys_cfg->MealOption;
	if(n_memcmp(OldTms, NewTms, 20) || n_memcmp(OldCount, NewCount, 5) || OldOption != NewOption) {
		for(i = 0;i < 5;i++) {
			memcpy(sys_cfg->MealTimeSection[i], NewTms[i], 4);
			sys_cfg->MealMenu[i] = NewCount[i];
		}
		sys_cfg->MealOption = NewOption; 
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			ResultMsg(WizardWnd, Title, R_DATA_SAVED);
			rval = 1;
		} else {
			for(i = 0;i < 5;i++) {
				memcpy(sys_cfg->MealTimeSection[i], OldTms[i], 4);
				sys_cfg->MealMenu[i] = OldCount[i];
			}
			sys_cfg->MealOption = OldOption; 
			ResultMsg(WizardWnd, Title, R_R_SYSTEM_ERROR);
			rval = -1;
		}
	} else	rval = 0;
	return rval;
}

/*
void MenuMeals(GMENU *ParentMenu, char *Title, int Index);

void MenuMealsSetup(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, &xmenu_meal[1], 6);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuMeal(menu, ItemText, 0);
			break;
		case 2:
			MenuMeal(menu, ItemText, 1);
			break;
		case 3:
			MenuMeal(menu, ItemText, 2);
			break;
		case 4:
			MenuMeal(menu, ItemText, 3);
			break;
		case 5:
			MenuMeal(menu, ItemText, 4);	
			break;
		case 6:
			MenuNoYes((GWND *)menu, ItemText, &sys_cfg->MealOption, MEALS_AVOID_OVERLAP, 1);
			break;
		//case 7:		
		//	MenuNoYes((GWND *)menu, ItemText, &sys_cfg->MealOption, MEALS_PRINTER, 1);
		//	break;
		}
	}
	MenuTerminate(menu);
}

void MenuMeals(GMENU *ParentMenu, char *Title, int Index)
{
	GMENU	*menu, _menu;
	char	temp[32];
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_meal_sub, 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		switch(rval) {
		case 1:
			sprintf(temp, "%s %s", Title, xmenu_meal_sub[0]);
			WndInputTimeSection((GWND *)menu, temp, sys_cfg->MealTimeSection[Index], 1);	
			break;
		case 2:
			sprintf(temp, "%s %s", Title, xmenu_meal_sub[1]);
			WndInputNumber((GWND *)menu, temp, NULL, &sys_cfg->MealMenu[Index], 1, 1, 4, R_INVALID_DATA, 1);
			break;
		}
	}
	MenuTerminate(menu);
}
*/
