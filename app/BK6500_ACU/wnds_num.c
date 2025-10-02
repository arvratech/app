void MakeGuideNumberNormal(char *GuideMsg, int Min, int Max)
{
	if(Min <= 0) Min = 0;
	if(GetBidi()) sprintf(GuideMsg, "%d-%d", Max, Min);
	else	sprintf(GuideMsg, "%d-%d", Min, Max);
}

void MakeGuideNumberMinute(char *GuideMsg, int Min, int Max)
{
	char	*mp1;
	
	if(Min <= 0) Min = 0;
	mp1 = xmenu_other[4];
	if(GetBidi()) sprintf(GuideMsg, "%s %d-%d", mp1, Max, Min);
	else		  sprintf(GuideMsg, "%d-%d %s", Min, Max, mp1);
}

void MakeGuideNumberSecond(char *GuideMsg, int Min, int Max)
{
	char	*mp1;
	
	if(Min <= 0) Min = 0;
	mp1 = xmsgs[M_SECOND];
	if(GetBidi()) sprintf(GuideMsg, "%s %d-%d", mp1, Max, Min);
	else		  sprintf(GuideMsg, "%d-%d %s", Min, Max, mp1);
}

void MakeGuideNumberMinuteAlways(char *GuideMsg, int Max)
{
	char	*mp1, *mp2;
	
	mp1 = xmenu_other[4]; mp2 = xmsgs[M_ALWAYS];
	if(GetBidi()) sprintf(GuideMsg, "%s %d-1 %s:0", mp1, Max, mp2);
	else		  sprintf(GuideMsg, "0:%s 1-%d %s", mp2, Max, mp1);
}

void MakeGuideNumberSecondAlways(char *GuideMsg, int Max)
{
	char	*mp1, *mp2;

	mp1 = xmsgs[M_SECOND]; mp2 = xmsgs[M_ALWAYS];
	if(GetBidi()) sprintf(GuideMsg, "%s %d-1 %s:0", mp1, Max, mp2);
	else		  sprintf(GuideMsg, "0:%s 1-%d %s", mp2, Max, mp1);
}

void MakeGuideNumberMinuteUnlimited(char *GuideMsg, int Max)
{
	if(GetBidi()) sprintf(GuideMsg, "%s %d-1 %s:0", xmenu_other[4], Max, xmenu_other[5]);
	else		  sprintf(GuideMsg, "0:%s 1-%d %s", xmenu_other[5], Max, xmenu_other[4]);
}

void MakeGuideNumberSecondPulse(char *GuideMsg, int Max)
{
	char	*mp1, *mp2;
	
	mp1 = xmsgs[M_SECOND]; mp2 = xmenu_other[10];
	if(GetBidi()) sprintf(GuideMsg, "%s %d-1 %s:0", mp1, Max, mp2);
	else		  sprintf(GuideMsg, "0=%s 1-%d %s", mp2, Max, mp1);
}

void MakeGuideNumberDeciSecond(char *GuideMsg, int Min, int Max)
{
	char	*mp1;
	
	if(Min <= 0) Min = 0;
	mp1 = xmsgs[M_SECOND];
	if(GetBidi()) sprintf(GuideMsg, "%s0.1 %d-%d", mp1, Max, Min);
	else	sprintf(GuideMsg, "%d-%d 0.1%s", Min, Max, mp1);
}

void MakeGuideNumberCentiSecond(char *GuideMsg, int Min, int Max)
{
	char	*mp1;
	
	if(Min <= 0) Min = 0;
	mp1 = xmsgs[M_SECOND];
	if(GetBidi()) sprintf(GuideMsg, "%s0.01 %d-%d", mp1, Max, Min);
	else	sprintf(GuideMsg, "%d-%d 0.01%s", Min, Max, mp1);
}

void MakeGuideNumberSchedule(char *GuideMsg)
{
	if(GetBidi()) sprintf(GuideMsg, "224-2 %s:1 %s:0 %s:255", xmsgs[M_ALWAYS], xmenu_other[12], xmenu_other[11]);
	else	sprintf(GuideMsg, "255:%s 0:%s 1:%s 2-224", xmenu_other[11], xmenu_other[12], xmsgs[M_ALWAYS]);
}

static void _GetPromptInt(char *buf, int val)
{
	if(GetBidi()) {
		strcpy(buf, " <= "); sprintf(buf+4, "%d", val);
	} else {
		sprintf(buf, "%d", val); strcat(buf, " => ");
	}	
}

static void _GetPromptString(char *buf, char *String)
{
	if(GetBidi()) {
		strcpy(buf, " <= "); strcpy(buf+4, String);
	} else {
		strcpy(buf, String); strcat(buf, " => ");
	}	
}

void DrawInputDigitDesp(int x, int y)
{
	lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26;
	lcdPutIconFile(x, y, 24, BTN_SHARP);
}

void _DrawInputDigitDesp(GWND *wnd)
{
	RECT	rect;
	int		x, y;

	wndGetClientArea(wnd, &rect);
	x = rect.x + 105; y = rect.y + 60;
	DrawInputDigitDesp(x, y);
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
static int _WndInputNumber(GWND *ParentWnd, char *Title, void *pVal, int Length, int Min, int Max, int InvalidMsg, int Section)
{
	char	data[20], temp[60];
	int		rval, NewVal;

	if(Min <= 0) Min = 0;
	NewVal = GetIntValue(pVal, Length);
	_GetPromptInt(temp, NewVal);
	sprintf(data, "%d", Max); rval = isdigits(data);
	if(MenuGetAdminConsole()) {
		sprintf(data, "%d", NewVal);
		rval = menucInputString(temp, data, rval);
		NewVal = n_check_atou(data);
	} else {
		_DrawInputDigitDesp(wndInput);
		sprintf(edtInput->Text, "%d", NewVal);
		editDrawWindow(edtInput);
		rval = editProcessWindow(edtInput, 0);
		NewVal = n_atol(edtInput->Text);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	if(NewVal < Min || NewVal > Max) { 
		ResultMsg(ParentWnd, Title, InvalidMsg);
		rval = 1;
	} else {
		rval = WndSaveInteger(ParentWnd, Title, pVal, Length, NewVal, Section);
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputNumber(GWND *ParentWnd, char *Title, char *GuideMsg, void *pVal, int Length, int Min, int Max, int InvalidMsg, int Section)
{
	char	*msg, _msg[64];

	if(!MenuGetAdminConsole()) {
		wndSetParentWnd(wndInput, ParentWnd);
		wndSetSize(wndInput, 60, 63, 200, 114); 
		wndSetWindowText(wndInput, Title);		// Title
		labelSetSize(lblInput, 6, 8, 150, 24); 	
		editSetSize(edtInput, 24, 32, 150, 26); 
		editSetFont(edtInput, GWND_NORMAL_FONT);
		editLimitText(edtInput, 8);
		editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	}
	if(GuideMsg) msg = GuideMsg;
	else {
		msg = _msg;
		MakeGuideNumberNormal(msg, Min, Max);
	}
	if(!MenuGetAdminConsole()) {
		labelSetWindowText(lblInput, msg); 
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
	} else {
		cPutTextMultiLine(msg);	
		if(menu_errno) return GWND_ERROR;
	}
	return _WndInputNumber(ParentWnd, Title, pVal, Length, Min, Max, InvalidMsg, Section);
}

static int _WndInputDigitString(GWND *ParentWnd, char *Title, char *Data, int MaxLength, int Section)
{
	char	temp[32], NewData[32];
	int		rval, w;

	if(MenuGetAdminConsole()) {
		_GetPromptString(temp, Data);
		strcpy(NewData, Data);
		rval = menucInputString(temp, NewData, MaxLength);		
	} else {	
		strcpy(edtInput->Text, Data);
		editDrawWindow(edtInput);
		rval = editProcessWindow(edtInput, 0);
		strcpy(NewData, edtInput->Text);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	w = isdigits(NewData);
	if(w < 0) {
		ResultMsg(ParentWnd, Title, R_INVALID_DATA);
		rval = 1;
	} else {
		rval = WndSaveString(ParentWnd, Title, Data, NewData, Section);
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputDigitString(GWND *ParentWnd, char *Title, char *Data, int MaxLength, int Section)
{
	if(!MenuGetAdminConsole()) {
		wndSetParentWnd(wndInput, ParentWnd);
		wndSetSize(wndInput, 60, 63, 200, 114); 
		wndSetWindowText(wndInput, Title);		// Title
		labelSetSize(lblInput, 6, 10, 150, 24); 	
		editSetSize(edtInput, 20, 38, 150, 26); 
		editSetFont(edtInput, GWND_NORMAL_FONT);
		editLimitText(edtInput, MaxLength);
		editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
		labelSetWindowText(lblInput, Data); 
 		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
	}
	return _WndInputDigitString(ParentWnd, Title, Data, MaxLength, Section);
}

static void _PutGuideIPString(GWND *wnd)
{
	RECT	rect;
	int		x, y;

	if(!MenuGetAdminConsole()) {
		wndGetClientArea(wnd, &rect);
		x = rect.x + 80; y = rect.y + 52;
		lcdPutIconFile(x, y, 24, BTN_PERIOD); x += 24;
		lcdPutIconFile(x, y, 24, BTN_ASTERISK); x += 40;
		lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26;
		lcdPutIconFile(x, y, 24, BTN_SHARP);
	}
}

static int _WndInputIPString(GWND *ParentWnd, GIPSTRING *ipstr, char *Title, char *Data, int Section)
{
	char	temp[20], NewData[32];
	unsigned char	IPAddr[4];
	int		rval;

	if(MenuGetAdminConsole()) {
		_GetPromptString(temp, Data);
		strcpy(NewData, Data);
		rval = menucInputString(temp, NewData, 15);
		//if(rval == 1) rval = 0;
	} else {
		strcpy(ipstr->Edit.Text, Data);
		ipstringDrawWindow(ipstr);
		rval = ipstringProcessWindow(ipstr, 0);
		strcpy(NewData, ipstr->Edit.Text);
	}		
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	rval = addr2inet(NewData, IPAddr);
	if(rval < 0) {
		ResultMsg(ParentWnd, Title, R_INVALID_IP_ADDRESS);
		rval = 1;
	} else {	
		rval = WndSaveString(ParentWnd, Title, Data, NewData, Section);
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputIPString(GWND *ParentWnd, char *Title, char *Data, int Section)
{
	GWND	*wnd, _wnd;
	GIPSTRING	*ipstr, _ipstr;
	
	if(!MenuGetAdminConsole()) {
	 	wnd = &_wnd;
		wndInitialize(wnd, Title, 35, 65, 250, 110); 
		ipstr = &_ipstr;
		ipstringInitialize(ipstr, wnd, 20, 20, 200, 26); 
		ipstringSetFont(ipstr, GWND_NORMAL_FONT);
		ipstringLimitText(ipstr, 15);
		ipstringModifyStyle(ipstr, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);	
		wndDrawWindow(wnd);
		_PutGuideIPString(wnd);
	}
	return _WndInputIPString(ParentWnd, ipstr, Title, Data, Section);
}

// Return	-1=Timeout  0=Cancelled  1=OK
int _WndInputAnalog(GWND *ParentWnd, GWND *wnd, int *curValue, int minValue, int maxValue, void (*CallBack)())
{
	RECT	rect;	
	void (*_CallBack)(int value);
	char	temp[32];
	int		c, x, y, w, h, x0, y0, width, tw, val, contflg, delta, timeout, tColor, fColor, bColor;

	wndDrawWindow(wnd);		
	_DrawLowerButtons(BTN_CANCEL, BTN_LEFT_ARROW, BTN_RIGHT_ARROW, BTN_OK, 0);
	_CallBack = CallBack;
	wndGetClientArea(wnd, &rect);
	x = rect.x + 12; y = rect.y + 16;
	width = rect.Width - 24;
	x0 = x + (width >> 1);	// center
	tColor = wnd->BackGroundTextColor;
	sprintf(temp, "%d", minValue);
	lcdPut8x16Digits(x, y, temp, tColor);
	sprintf(temp, "%d", maxValue); tw = strlen(temp)*8;
	lcdPut8x16Digits(x+width-tw, y, temp, tColor);
	val = *curValue;
	sprintf(temp, "%d", val); tw = strlen(temp)*8;
	lcdPut8x16Digits(x0-(tw>>1), y, temp, tColor);	// put center text
	fColor = RGB(220,120,90); bColor = RGB(255,255,255);
	y0 = y;
	y += 16;
	h = 32;
	lcdFillRect(x, y, width, h, RGB(0,0,0));	// boundary bar
	x++; y++; width -= 2; h -= 2;
	lcdFillRect(x, y, width, h, bColor);		// inner bar
	delta = maxValue - minValue;
	w = ((val - minValue) * width + (delta >> 1)) / delta;
	lcdFillRect(x, y, w, h, fColor);

	timeout = GWND_TIMER;
	contflg = 1;
	while(contflg) {
		c = keyGetWithExternalCancel(timeout, menuGetGlobalExit);
		if(c > 0) timeout = GWND_HALF_TIMER;
		switch(c) {
		case 0:
			if(_CallBack) (*_CallBack)(*curValue);
			c = GWND_TIMEOUT; contflg = 0;
			break;
		case KEY_CANCEL:
		case KEY_F1:
			if(_CallBack) (*_CallBack)(*curValue);
			c = GWND_QUIT; contflg = 0;
			break;
		case KEY_OK:
		case KEY_F4:
			*curValue = val; contflg = 0; c = 1;
			break;
		case KEY_UP:
		case KEY_DOWN:
			if(c == KEY_DOWN && val < maxValue || c == KEY_UP && val > minValue) {
				lcdFillRect(x0-(tw>>1), y0, tw, 16, (int)wnd->BackGroundColor);	// erase center text
				if(c == KEY_DOWN) val++; else val--;
				sprintf(temp, "%d", val); tw = strlen(temp)*8;
				lcdPut8x16Digits(x0-(tw>>1), y0, temp, tColor);		// put center text
				c = ((val - minValue) * width + (delta >> 1)) / delta;
				if(c > w) lcdFillRect(x+w, y, c-w, h, fColor);
				else	  lcdFillRect(x+c, y, w-c, h, bColor); 	
				w = c;
				if(_CallBack) (*_CallBack)(val);
			}
			break;
		}
	}
	return c;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputAnalog(GWND *ParentWnd, char *Title, unsigned char *pVal, int Min, int Max, void (*CallBack)(), int Section)
{
	GWND	*wnd, _wnd;
	int		rval, NewVal;

	if(MenuGetAdminConsole()) {
		return WndInputNumber(ParentWnd, Title, NULL, pVal, 1, Min, Max, R_INVALID_DATA, Section);
	} else {
		wnd = &_wnd;
		wndInitialize(wnd, Title, 30, 63, 260, 114);
		wndSetParentWnd(wnd, ParentWnd);
		NewVal = *pVal;
		rval = _WndInputAnalog(ParentWnd, wnd, &NewVal, Min, Max, CallBack);
		if(!MenuGetAdminConsole()) DestroyWindow(wnd);	// maybe not needed
		if(rval <= 0) {
			if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		} else rval = WndSaveByte(ParentWnd, Title, pVal, NewVal, Section);
		return rval;
	}
}
