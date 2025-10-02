
static void _MakeDateValue(char *buf, unsigned char *data)
{
	sprintf(buf, "%02d/%02d/%02d", data[0], data[1], data[2]);
}

static void _MakeTimeValue(char *buf, unsigned char *data)
{
	sprintf(buf, "%02d:%02d:%02d", data[0], data[1], data[2]);
}	

static void _MakeDateOrTime(char *buf, unsigned char *cdata, int dateflg)
{
	char	temp[4];
	int		len;
	
	len = strlen(buf);
	if(dateflg) {
		if(len != 8 || buf[2] != '/' || buf[5] != '/') len = 0;
	} else {
		if(len != 8 || buf[2] != ':' || buf[5] != ':') len = 0;
	}
	if(len) {	
		buf[2] = buf[3]; buf[3] = buf[4];
		buf[4] = buf[6]; buf[5] = buf[7];
		buf[6] = 0;
		len = isdigits(buf);
		if(len != 6) len = 0;
	}
	if(len) {
		temp[2] = 0;
		temp[0] = buf[0]; temp[1] = buf[1]; cdata[0] = n_atou(temp);
		temp[0] = buf[2]; temp[1] = buf[3]; cdata[1] = n_atou(temp);
		temp[0] = buf[4]; temp[1] = buf[5]; cdata[2] = n_atou(temp);
	}
	if(!len) {
		cdata[0] = cdata[1] = cdata[2] = 0;
	}
}

void DrawInputDateTimeDesp(int x, int y)
{
	lcdPutIconFile(x, y, 24, BTN_DELETE); x += 26;
	lcdPutIconFile(x, y, 24, BTN_ASTERISK); x += 36;
	lcdPutIconFile(x, y, 24, BTN_BACKSPACE); x += 26;
	lcdPutIconFile(x, y, 24, BTN_SHARP);
}


static void _DrawInputDateTimeDesp(GWND *wnd)
{
	RECT	rect;
	int		x, y;

	wndGetClientArea(wnd, &rect);
	x = rect.x + 60; y = rect.y + 60;
	DrawInputDateTimeDesp(x, y);
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK
int WndInputDate(GWND *ParentWnd, char *Title, unsigned char *cdate)
{
	char	*p, buf[64], temp[64];
	int		rval;
	
	rtcGetDate(cdate);
	if(MenuGetAdminConsole()) {
		p = temp; strcpy(p, "Current Date"); p += strlen(p);; *p++ = ':'; *p++ = ' '; _MakeDateValue(p, cdate);
		cPutTextLine(temp); if(menu_errno) return GWND_ERROR;
		strcpy(temp, xmsgs[M_YYMMDD]); p = temp + strlen(temp); *p++ = ' '; *p = 0;
		buf[0] = 0;
		rval = menucInputString(temp, buf, 8);
		if(rval == 1) rval = 0;
		if(rval > 0) _MakeDateOrTime(buf, cdate, 1);
	} else {	
		wndSetParentWnd(wndInput, ParentWnd);
		wndSetSize(wndInput, 60, 64, 200, 112);
		wndSetWindowText(wndInput, Title);
		labelSetSize(lblInput, 6, 8, 150, 24); 
		dateSetSize(dateInput, 24, 32, 150, 26);
		dateSetFont(dateInput, GWND_NORMAL_FONT);
		dateModifyStyle(dateInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
		dateSetType(dateInput, DT_DATE_ONLY);
		labelSetWindowText(lblInput, GetPromptMsg(M_YYMMDD)); 
		memcpy(dateInput->Data, cdate, 3);
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		dateDrawWindow(dateInput);
		_DrawInputDateTimeDesp(wndInput);
		rval = dateProcessWindow(dateInput, 0);
		memcpy(cdate, dateInput->Data, 3);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	rval = date_validate(cdate);
	if(!rval) {
		ResultMsg(ParentWnd, Title, R_INVALID_DATE);
		rval = 1;
	} else {
		rval = 2;
	}
	return rval;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK
int WndInputTime(GWND *ParentWnd, char *Title, unsigned char *ctime)
{
	char	*p, buf[64], temp[64];
	int		rval;
	
	rtcGetTime(ctime);
	if(MenuGetAdminConsole()) {
		p = temp; strcpy(p, "Current Time"); p += strlen(p); *p++ = ':'; *p++ = ' '; _MakeTimeValue(p, ctime);
		cPutTextLine(temp); if(menu_errno) return GWND_ERROR;
		strcpy(temp, xmsgs[M_HHMMSS]); p = temp + strlen(temp); *p++ = ' '; *p = 0;
		buf[0] = 0;	
		rval = menucInputString(temp, buf, 8);
		if(rval == 1) rval = 0;
		if(rval > 1) _MakeDateOrTime(buf, ctime, 0);
	} else {
		wndSetParentWnd(wndInput, ParentWnd);		
		wndSetSize(wndInput, 60, 64, 200, 112);
		wndSetWindowText(wndInput, Title);
		labelSetSize(lblInput, 6, 8, 150, 24); 
		dateSetSize(dateInput, 24, 32, 150, 26);	
		dateSetFont(dateInput, GWND_NORMAL_FONT);
		dateModifyStyle(dateInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
		dateSetType(dateInput, DT_TIME_ONLY);		
		labelSetWindowText(lblInput, GetPromptMsg(M_HHMMSS)); 
		memcpy(dateInput->Data, ctime, 3);
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		dateDrawWindow(dateInput);
		_DrawInputDateTimeDesp(wndInput);
		rval = dateProcessWindow(dateInput, 0);
		memcpy(ctime, dateInput->Data, 3);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	rval = time_validate(ctime);
	if(!rval) {
		ResultMsg(ParentWnd, Title, R_INVALID_TIME);
		rval = 1;
	} else {
		rval = 2;
	}
	return rval;
}

static void _MakeTimeSection(char *buf, unsigned char *tms)
{
	char	temp[4];
	int		len;
	
	len = strlen(buf);
	if(len != 11 || buf[2] != ':' || buf[5] != '-' || buf[8] != ':') len = 0;
	if(len) {
		buf[2] = buf[3]; buf[3] = buf[4];
		buf[4] = buf[6]; buf[5] = buf[7];
		buf[6] = buf[9]; buf[7] = buf[10];
		buf[8] = 0;
		len = isdigits(buf);
		if(len != 8) len = 0;
	}
	if(len) {
		temp[2] = 0;
		temp[0] = buf[0]; temp[1] = buf[1]; tms[0] = n_atou(temp);
		temp[0] = buf[2]; temp[1] = buf[3]; tms[1] = n_atou(temp);
		temp[0] = buf[4]; temp[1] = buf[5]; tms[2] = n_atou(temp);
		temp[0] = buf[6]; temp[1] = buf[7]; tms[3] = n_atou(temp);
	} else {
		tms[0] = tms[1] = tms[2] = tms[3] = 0;
	}
}

void TimeSection2Str(unsigned char *tms, char *buf)
{
	utoa02((unsigned int)tms[0], buf);   buf[2] = ':';
	utoa02((unsigned int)tms[1], buf+3); buf[5] = '-'; 
	utoa02((unsigned int)tms[2], buf+6); buf[8] = ':';
	utoa02((unsigned int)tms[3], buf+9); buf[11] = ' '; buf[12] = 0;
}

// Return	-1:Timeout, System error  0:Cancelled  1:Invalid  2:OK(newVal=OldVal)  3:OK(newVal!=OldVal)
int WndInputTimeSection(GWND *ParentWnd, char *Title, unsigned char *tms, int Section)
{
	char	*p, buf[32], temp[32];
	unsigned char	NewTms[6];
	int		rval;
	
	if(MenuGetAdminConsole()) {
		sprintf(temp, "%02d:%02d-%02d:%02d", (int)tms[0], (int)tms[1], (int)tms[2], (int)tms[3]);
		cPutTextLine(temp); if(menu_errno) return GWND_ERROR;
		strcpy(temp, "HH:MM-HH:MM"); p = temp + strlen(temp); *p++ = ' '; *p = 0;
		buf[0] = 0;
		rval = menucInputString(temp, buf, 11);
		if(rval == 1) rval = 0;
		if(rval > 0) _MakeTimeSection(buf, NewTms);
	} else {
		wndSetParentWnd(wndInput, ParentWnd);		
		wndSetSize(wndInput, 60, 64, 200, 112);
		wndSetWindowText(wndInput, Title);
		labelSetSize(lblInput, 6, 8, 150, 24); 
		dateSetSize(dateInput, 24, 32, 150, 26);
		dateSetFont(dateInput, GWND_NORMAL_FONT);
		dateModifyStyle(dateInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
		dateSetType(dateInput, DT_TIME_SECTION);		
		labelSetWindowText(lblInput, "HH:MM-HH:MM"); 
		memcpy(dateInput->Data, tms, 4);
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		dateDrawWindow(dateInput);
		_DrawInputDateTimeDesp(wndInput);
		rval = dateProcessWindow(dateInput, 0);
		memcpy(NewTms, dateInput->Data, 4);
	}
	if(rval <= 0) {
		if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
		return rval;
	}
	if(tmsIsNull(NewTms) || tmsValidateEx(NewTms)) rval = 2;
	else	rval = 1;
	if(rval == 1) ResultMsg(ParentWnd, Title, R_INVALID_DATA);
	else rval = WndSaveByteData(ParentWnd, Title, tms, 4, NewTms, Section);
	return rval;
}
