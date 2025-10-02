int crAuthPolicy(CREDENTIAL_READER *cr, int authPolicy)
{
	int		mode;

	mode = authPolicy;
	if(!crIsEnableFPReader(cr)) mode &= ~(USER_FP | USER_FP_PIN | USER_CARD_FP | USER_CARD_FP_PIN);
	if(!crIsEnableCardReader(cr)) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN);
	if(!crIsEnablePINReader(cr)) mode &= ~(USER_PIN | USER_CARD_PIN | USER_CARD_FP_PIN | USER_FP_PIN);
	if(mode & USER_CARD_FP_PIN) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN);
	else if(mode & (USER_CARD_FP | USER_CARD_PIN)) mode &= ~USER_CARD;
	return mode;
}

int _MaskAuthPolicy(int authPolicy, int authDone, int maskPolicy)
{
	int		ap;

	ap = authPolicy;
	if(!authDone) {
		if(maskPolicy == USER_CARD) ap &= USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN;
		else if(maskPolicy == USER_FP) ap &= USER_FP | USER_FP_PIN;
		else if(maskPolicy == USER_PIN) ap &= USER_PIN;
	} else if(authDone == USER_CARD) {
		if(maskPolicy == USER_FP) ap &= USER_CARD_FP | USER_CARD_FP_PIN;
		else if(maskPolicy == USER_PIN) ap &= USER_CARD_PIN;
		else	ap = 0;
	} else if(authDone == USER_FP) {
		if(maskPolicy == USER_PIN) ap &= USER_FP_PIN;
		else	ap = 0;
	} else if(authDone == (USER_CARD | USER_FP)) {
		if(maskPolicy == USER_PIN) ap &= USER_CARD_FP_PIN;
		else	ap = 0;
	} else	ap = 0;
	return ap;
}

int _GetCaptureMode(int authMode, int authDone)
{
	int		mode;

	mode = 0;
	if(!authDone) {
		if(authMode & (USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN)) mode |= USER_CARD;
		if(authMode & (USER_FP | USER_FP_PIN)) mode |= USER_FP;
		if(authMode & USER_PIN) mode |= USER_PIN;
	} else if(authDone == USER_CARD) {
		if(authMode & (USER_CARD_FP | USER_CARD_FP_PIN)) mode |= USER_FP;
		if(authMode & USER_CARD_PIN) mode |= USER_PIN;
	} else if(authDone == USER_FP) {
		if(authMode & USER_FP_PIN) mode |= USER_PIN;
	} else if(authDone == (USER_CARD | USER_FP)) {
		if(authMode & USER_CARD_FP_PIN) mode |= USER_PIN;
	}
	return mode;
}

int _GetAuthModeCardOnly(int ID, FP_USER *user)
{
	int		mode;
	void	*cr;
	
	cr = crsGet(0);
	if(!crIsEnableCardReader(cr)) mode = 0;
	else {
		mode = user->accessMode & (USER_CARD | USER_CARD_FP | USER_CARD_PIN | USER_CARD_FP_PIN);
		if(!crIsEnableFpReader(cr)) mode &= ~(USER_CARD_FP | USER_CARD_FP_PIN);
		if(mode & USER_CARD_FP_PIN) mode &= ~(USER_CARD | USER_CARD_FP | USER_CARD_PIN);
		else if(mode & USER_CARD_FP || mode & USER_CARD_PIN) mode &= ~USER_CARD;
	}
	return mode;
}

int _GetAccessGranted(int authDone)
{
	int		accessEvent;

	if(authDone == USER_CARD) accessEvent = E_ACCESS_GRANTED_CARD;
	else if(authDone == USER_FP) accessEvent = E_ACCESS_GRANTED_FP;
	else if(authDone == USER_PIN) accessEvent = E_ACCESS_GRANTED_PIN;
	else if(authDone == (USER_CARD | USER_FP)) accessEvent = E_ACCESS_GRANTED_CARD_FP;
	else if(authDone == (USER_CARD | USER_PIN)) accessEvent = E_ACCESS_GRANTED_CARD_PIN;
	else if(authDone == (USER_FP | USER_PIN)) accessEvent = E_ACCESS_GRANTED_FP_PIN;
	else	accessEvent = E_ACCESS_GRANTED_CARD_FP_PIN;
	return accessEvent;
}

int _GetAccessDenied(int authMode, int authDone)
{
	int		accessEvent;

	if(authDone == USER_CARD) accessEvent = E_ACCESS_DENIED_BAD_CARD;
	else if(authDone == USER_FP) accessEvent = E_ACCESS_DENIED_BAD_FP;
	else if(authDone == USER_PIN) accessEvent = E_ACCESS_DENIED_BAD_PIN;
	else if(authDone == (USER_CARD | USER_FP)) {
		if(authMode & USER_CARD_FP_PIN) accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP_PIN;
		else if((authMode & (USER_CARD_FP | USER_CARD_PIN)) == USER_CARD_FP) accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP;
		else	accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP_BAD_PIN;
	} else if(authDone == (USER_CARD | USER_PIN)) {
		if((authMode & (USER_CARD_FP | USER_CARD_PIN)) == USER_CARD_PIN) accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_PIN;
		else	accessEvent = E_ACCESS_DENIED_GOOD_CARD_BAD_FP_BAD_PIN;
	} else if(authDone == (USER_FP | USER_PIN)) accessEvent = E_ACCESS_DENIED_BAD_PIN;
	else	accessEvent = E_ACCESS_DENIED_GOOD_CARD_FP_BAD_PIN;
	return accessEvent;
}

int _CheckCardStatus(FP_USER *user)
{
	int		EventID;
	
	if(user->cardStatus != 1) EventID = E_ACCESS_DENIED_LOST_CARD;
	else	EventID = 0;
	return EventID;
}

// Return	0=OK  >0=Event ID  
int _CheckAccess(CREDENTIAL_READER *cr, FP_USER *user)
{
	int		EventID, AuthMode;
	
	AuthMode = crAuthorizationMode(cr);
	if(date_validate(user->activateDate) && date_comp(cr->accessTime, user->activateDate) < 0) EventID = E_ACCESS_DENIED_USER_NOT_ACTIVATED; 
	else if(date_validate(user->expireDate) && date_comp(cr->accessTime, user->expireDate) > 0) EventID = E_ACCESS_DENIED_USER_EXPIRED;
	else if(AuthMode != 1 && userValidateAccessRights(user) && !userBoundAccessRights(user, cr->accessTime)) EventID = E_ACCESS_DENIED_ACCESS_RIGHTS;
	else	EventID = 0;
	return EventID;
}

// Return	0=OK  E_ACCESS_DENIED_ACCESS_CONTROL=Fail 
int _CheckLockState(CREDENTIAL_READER *cr)
{
	void	*ad;
	int		priority, EventID;

	ad = adsGet(crId(cr));
	priority = adPriority(ad);
	if(priority < 8) EventID = E_ACCESS_DENIED_ACCESS_CONTROL;	// modified 2013.3.19
	else	EventID = 0;
	return EventID;
}

void _AccessUnregistered(CREDENTIAL_READER *cr, int cardauth)
{
	int		EventID, output;

	if(cardauth) EventID = E_ACCESS_DENIED_UNREGISTERED_CARD;
	else		 EventID = E_ACCESS_DENIED_UNREGISTERED_FP;	
	cr->accessEvent = EventID;
	EventAddRaw(EventID, cr->accessTime, cr->data);
//	output = apGetUnregisteredAlarm(0);
//	if(output) modAlarmCommand(2);
}

void _AccessDenied(CREDENTIAL_READER *cr)
{
	char	data[32];
	int		item, output;

printf("AccessDenied: %d\n", (int)cr->accessEvent);
	if(cr->accessEvent == E_ACCESS_DENIED_UNREGISTERED_CARD) {
		EventAddRaw((int)cr->accessEvent, cr->accessTime, cr->data);
	} else {
		item = cr->funcKey - 1; if(item < 0) item = 0;	
		if(taGetItemSize() > 10) utoa02(item, data);
		else {
			data[0] = '5';		// ID=1 => '5'
			data[1] = item + '0';
		}
		strcpy(data+2, cr->userID);
//	output = apGetExpiredAlarm(0);
//	if(output) modAlarmCommand(2);
		EventAdd((int)cr->accessEvent, cr->accessTime, data);
	}
}

void _AccessGranted(CREDENTIAL_READER *cr)
{
	void	*ad;
	char	*p, data[32];
	int		item, val, idx;
	unsigned long	lval;

printf("AccessGranted: %d\n", (int)cr->accessEvent);
	ad = adsGet(0);
	AdSetPv(ad, PV_PULSE_UNLOCK, 8);
/*
	if(ap->duress) {
		if(user) EventAdd(E_ACCESS_DURESS, cr->accessTime, user->userID);
		else	 EventAddRaw(E_ACCESS_DURESS, cr->accessTime, ap->data);
		val = sys_cfg->accessPoints[0].duressAlarm;
		if(val) modAlarmCommand(2);
//cprintf("DURESS, output=%d\n", output);
	}
*/
	if(cr->accessEvent == E_ACCESS_GRANTED_ALL) {
		EventAddRaw((int)cr->accessEvent, cr->accessTime, cr->data);
	} else {
		item = cr->funcKey - 1; if(item < 0) item = 0;	
		if(taGetItemSize() > 10) utoa02(item, data);
		else {
			data[0] = '5';		// ID=1 => '5'
			data[1] = item + '0';
		}
		strcpy(data+2, cr->userID);
	}
	if(cr->accessEvent != E_ACCESS_GRANTED_ALL) EventAdd((int)cr->accessEvent, cr->accessTime, data);
}

