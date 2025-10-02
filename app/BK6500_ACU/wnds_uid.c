
// Return	-2=Error  -1=Timeout  0=Cancelled  1=Invalid  2=OK
int WndEnterID(GWND *ParentWnd, char *Title, long *pID, int NewUser, int TryCount)
{
	long	nID;
	int		rval, i, no, found;

	_DrawLowerButtons(BTN_CANCEL, 0, 0, BTN_OK, 0);
	wndSetParentWnd(wndInput, ParentWnd);
	wndSetSize(wndInput, 60, 63, 200, 114); 
	wndSetWindowText(wndInput, Title);
	labelSetSize(lblInput, 6, 8, 150, 24); 	
	editSetSize(edtInput, 24, 32, 150, 26); 
	editSetFont(edtInput, GWND_NORMAL_FONT);
	editLimitText(edtInput, 8);
	editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	if(TryCount) no = M_VERIFY_USER_ID;
	else	no = M_ENTER_USER_ID;
	labelSetWindowText(lblInput, GetPromptMsg(no)); 
	for(i = 0;i < 2;i++) {
		edtInput->Text[0] = 0;
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		editDrawWindow(edtInput);
		_DrawInputDigitDesp(wndInput);
		rval = editProcessWindow(edtInput, 0);
		no = 0;
		if(rval > 0) {
			nID = n_check_atou(edtInput->Text);
			if(userValidateID(nID)) rval = 2;
			else {
				rval = 1;
				no = R_INVALID_USER_ID;
			}
		}
		if(rval > 1) {		
			found = userfsExist(nID);
			if(found < 0) rval = -2;
			else if(found && NewUser) no = R_USER_ID_OVERLAPPED;
			else if(!found && !NewUser) no = R_USER_NOT_FOUND;
		}
		if(no) {
			rval = 1;
			ResultMsg(ParentWnd, Title, no); if(menu_errno) return GWND_ERROR;
		} else	break;
	}
	if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
	else if(rval < -1) ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
	else if(rval > 1) *pID = nID;
	return rval;
}

// Return	-2=Error  -1=Timeout  0=Cancelled  1=Invalid  2=OK
int WndEnterAutoNewID(GWND *ParentWnd, char *Title, long *pID)
{
	long	nID;
	int		rval, i, no, found;

	_DrawLowerButtons(BTN_CANCEL, 0, 0, BTN_OK, 0);
	wndSetParentWnd(wndInput, ParentWnd);
	wndSetSize(wndInput, 60, 63, 200, 114); 
	wndSetWindowText(wndInput, Title);
	labelSetSize(lblInput, 6, 8, 150, 24); 	
	editSetSize(edtInput, 24, 32, 150, 26); 
	editSetFont(edtInput, GWND_NORMAL_FONT);
	editLimitText(edtInput, 8);
	editModifyStyle(edtInput, ES_PASSWORD|ES_READONLY|ES_WILDDIGIT, ES_NUMBER);
	no = M_ENTER_USER_ID;
	labelSetWindowText(lblInput, GetPromptMsg(no)); 
	for(i = 0;i < 2;i++) {
		edtInput->Text[0] = 0;
		wndDrawWindow(wndInput);
		labelDrawWindow(lblInput);
		editDrawWindow(edtInput);
		_DrawInputDigitDesp(wndInput);
		rval = editProcessWindow(edtInput, 0);
		no = 0;
		if(rval == 1) {
			nID = *pID;
			if(nID > 0L) rval = userfsExist(nID);
			else	rval = 1;
			if(rval > 0) {
				nID++;
				ResultMsgNoWait(NULL, Title, R_PROCESSING);	
				nID = userGetNewID(nID, 1L, 0xffffff-1);
			}
			if(!nID) no = R_USER_ARE_FULL;
			else {
				rval = 2;
				sprintf(edtInput->Text, "%ld", nID);
			}		
		} else if(rval > 1) {
			nID = n_check_atou(edtInput->Text);			
			if(userValidateID(nID)) rval = 2;
			else {
				rval = 1;
				no = R_INVALID_USER_ID;
			}
			if(rval > 1) {
				found = userfsExist(nID);
				if(found < 0) rval = -2;
				else if(found) no = R_USER_ID_OVERLAPPED;
			}
		}
		if(no) {
			rval = 1;
			ResultMsg(ParentWnd, Title, no); if(menu_errno) return GWND_ERROR;
		} else	break;
	}
	if(rval == GWND_TIMEOUT) ResultMsg(ParentWnd, Title, R_INPUT_TIME_OVER);
	else if(rval < -1) ResultMsg(ParentWnd, Title, R_MSG_SYSTEM_ERROR);
	else if(rval > 1) *pID = nID;
	return rval;
}
