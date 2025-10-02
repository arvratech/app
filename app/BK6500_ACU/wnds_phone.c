// Return	-2=Error  -1=Timeout  0=Cancelled  1=Invalid  2=OK
int WndEnterPhoneNo(GWND *ParentWnd, char *Title, char *PhoneNo, int NewPhone, int TryCount)
{
	RECT	rect;
	int		rval, i, no, found;

	_DrawLowerButtons(BTN_CANCEL, 0, 0, BTN_OK, 0);
	wndSetParentWnd(wndInput, ParentWnd);
	wndSetSize(wndInput, 35, 63, 250, 114); 
	wndSetWindowText(wndInput, Title);
	labelSetSize(lblInput, 6,  8, 230, 24);
	editSetSize(edtInput, 16, 32, 205, 26);
	editSetFont(edtInput, GWND_NORMAL_FONT);
	editLimitText(edtInput, MAX_PHONENO_SZ);
	editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	labelSetWindowText(lblInput, GetPromptMsg(M_DIAL_EXT_NUMBER)); 
	for(i = 0;i < 2;i++) {
		edtInput->Text[0] = 0;
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		editDrawWindow(edtInput);
		//_DrawInputDigitDesp(wndInput);
		wndGetClientArea(wndInput, &rect);
		DrawInputDigitDesp(rect.x+140, rect.y+60);
		rval = editProcessWindow(edtInput, 0);
		no = 0;
		if(rval > 0) {
			if(pnoValidatePhoneNo(edtInput->Text)) rval = 2;
			else {
				rval = 1; no = R_INVALID_EXT_NUMBER;
			}
		}
		if(rval > 1) {
			found = pnofsExistPhoneNo(edtInput->Text);
			if(found < 0) rval = -2;
			else if(found && NewPhone) no = R_OVERLAPPED;
			else if(!found && !NewPhone) no = R_NOT_FOUND;
		}
		if(no) {
			rval = 1;
			ResultMsg(ParentWnd, Title, no); if(menu_errno) return GWND_ERROR;
		} else	break;
	}
	if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
	else if(rval < -1) ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
	else if(rval > 1) strcpy(PhoneNo, edtInput->Text);
	return rval;
}

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Invalid  2=Normal PhoneNo  3=Wild PhoneNo
int WndEnterWildPhoneNo(GWND *ParentWnd, char *Title, char *PhoneNo, int NewPhone, int TryCount)
{
	RECT	rect;
	int		rval, i, no, found;

	_DrawLowerButtons(BTN_CANCEL, 0, 0, BTN_OK, 0);
	wndSetParentWnd(wndInput, ParentWnd);
	wndSetSize(wndInput, 35, 63, 250, 114); 
	wndSetWindowText(wndInput, Title);
	labelSetSize(lblInput, 6,  8, 230, 24);
	editSetSize(edtInput, 16, 32, 205, 26);
	editSetFont(edtInput, GWND_NORMAL_FONT);
	editLimitText(edtInput, MAX_PHONENO_SZ);
	editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY, ES_NUMBER|ES_WILDDIGIT);
	labelSetWindowText(lblInput, GetPromptMsg(M_DIAL_EXT_NUMBER)); 
	for(i = 0;i < 2;i++) {
		edtInput->Text[0] = 0;
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		editDrawWindow(edtInput);
		//_DrawInputDigitDesp(wndInput);
		wndGetClientArea(wndInput, &rect);
		DrawInputDigitDesp(rect.x+140, rect.y+60);
		rval = editProcessWindow(edtInput, 0);
		no = 0;
		if(rval > 1) {
			rval = iswilddigits(edtInput->Text);
			if(rval > 0) {
				rval = iswilds(edtInput->Text);
				if(rval == 1) rval = 3;
				else if(rval == 0 && pnoValidatePhoneNo(edtInput->Text)) rval = 2;
				else rval = 1;	
			} else rval = 1;
			if(rval == 3) break;
		}
		if(rval > 1) {
			found = pnofsExistPhoneNo(edtInput->Text);
			if(found < 0) rval = -2;
			else if(found && NewPhone) no = R_OVERLAPPED;
			else if(!found && !NewPhone) no = R_NOT_FOUND;
		}
		if(rval == 1) no = R_INVALID_EXT_NUMBER;
		if(no) {
			rval = 1; ResultMsg(ParentWnd, Title, no); if(menu_errno) return GWND_ERROR;
		} else	break;
	}
	if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
	else if(rval < -1) ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
	else if(rval > 1) strcpy(PhoneNo, edtInput->Text);
	return rval;
}
