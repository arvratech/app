int SlaveGetProperty(int ynitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	unsigned char	*p;
	long	nID, minID, maxID;
	int		result;

	if(*dataSize < 5) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("GetProperty: OT=%02x ID=%d OP=%02x\n", (int)rxData[0], nID, (int)rxData[4]);
	result = 0;
	switch(rxData[0]) {
	case OT_USER:
		switch(rxData[4]) {
		case 0x30:		// NewUserID
			PACK3toID(rxData+5, &minID);
			PACK3toID(rxData+8, &maxID);
			nID = userGetNewID(nID, minID, maxID);
			IDtoPACK3(nID, p); p += 3;
			break;
		case 0x31:		// UserCount
			minID = userfsGetCount();
			if(minID < 0) result = R_R_SYSTEM_ERROR;
			else {
				LONGtoBYTE(minID, p); p += 4;
			}
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}


int SlaveFindObject(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	unsigned char	*p;
	long	nID;
	int		rval, result;

	if(*dataSize < 4) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("FindObject: OT=%02x ID=%d\n", (int)rxData[0], nID);
	result = 0;
	switch(rxData[0]) {
	case OT_USER:
		if(!userValidateID(nID)) result = R_PARAMETER_ERROR;
		else {
			rval = userfsExist(nID);
			if(rval < 0) result = R_R_SYSTEM_ERROR;
			else if(!rval) result = R_OBJECT_NOT_FOUND;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveFindProperty(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	unsigned char	*p, temp[1024];
	FP_USER		*user, _user;
	long	nID;
	unsigned long	FPID;
	int		rval, rval2, result;

	if(*dataSize < 5) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("FindProperty: OT=%02x ID=%d OP=%02x\n", (int)rxData[0], nID, (int)rxData[4]);
	result = 0;
	switch(rxData[0]) {
	case OT_USER:
		switch(rxData[4]) {
		case OP_FP_AUTHENTICATION_FACTOR:
			if(!userValidateID(nID)) result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) result = R_OBJECT_NOT_FOUND;
			else {
				userID2FPID(nID, &FPID);
				rval = userfsGetEncodedFPTemplate(FPID, temp);
				FPID++;
				rval2 = userfsGetEncodedFPTemplate(FPID, temp);
				if(rval < 0 || rval2 < 0) result = R_R_SYSTEM_ERROR;
				else {
					if(rval > 0) *p++ = 1; else *p++ = 0;
					if(rval2 > 0) *p++ = 1; else *p++ = 0;
				}
			}
			break;
		case OP_SECOND_CARD_AUTHENTICATION_FACTOR:
			user = &_user;
			userfsSeek(0L);
			rval = userfsGetCardData(user, rxData+5);
			if(rval < 0) result = R_R_SYSTEM_ERROR;
			else if(!rval)  result = R_OBJECT_NOT_FOUND;
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveReadObject(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	CR_CFG	*cr_cfg;
	unsigned char	*p;
	long	nID;
	int		rval, result;

	if(*dataSize < 4) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("ReadObject: OT=%02x ID=%d\n", (int)rxData[0], nID);
	result = 0;
	switch(rxData[0]) {
	case OT_CREDENTIAL_READER:
		cr_cfg = &sys_cfg->CredentialReaders[unitID];
		*p++ = cr_cfg->Version;
		p += crEncodeAll(cr_cfg, p);
		*p++ = sys_cfg->TAOption;
		*p++ = sys_cfg->FuncKeyTimer;
		p += taEncodeItems(sys_cfg, p);	
		break;
	case OT_USER:
		if(!userValidateID(nID)) result = R_PARAMETER_ERROR;
		else {
			rval = userfsGetEncoded(nID, p);
			if(rval < 0) result = R_R_SYSTEM_ERROR;
			else if(!rval) result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveReadProperty(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	unsigned char	*p;
	long	nID;
	int		result;

	if(*dataSize < 5) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("ReadProperty: OT=%02x ID=%d OP=%02x\n", (int)rxData[0], nID, (int)rxData[4]);
	result = 0;
	switch(rxData[0]) {
	case OT_DEVICE:
		switch(rxData[4]) {
		case OP_LOCAL_DATE_TIME:
			*p++ = gLocalDateTimeVersion;
			rtcGetDateTime(p); p += 6;
			break;
		case OP_DEVICE_PASSWORD:
			string2bcd(sys_cfg->AdminPIN, 8, p); p += 4;
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	case OT_CREDENTIAL_READER:
		switch(rxData[4]) {
		case OP_SMART_CARD_KEY:
			*p++ = sys_cfg->SCKeyVersion;
			p += sckeyEncode(sys_cfg->SCPrimaryKey, p);
//sckeyPrint(sys_cfg->SCPrimaryKey, "PrimaryKey");
//sckeyPrint(sys_cfg->SCSecondaryKey, "SecondaryKey");
//cprintf("SCKeyOption=%02x\n", sys_cfg->SCKeyOption);
			break;
		case OP_TA_TEXTS:
			*p++ = sys_cfg->TAVersion;
			p += taEncodeTexts(sys_cfg, p);		
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveDeleteObject(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	unsigned char	*p;
	long	nID;
	int		rval, result;

	if(*dataSize < 4) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("DeleteObject: OT=%02x ID=%d\n", (int)rxData[0], nID);
	result = 0;
	switch(rxData[0]) {
	case OT_USER:
		wdtResetLong();
		if(nID < 0)	rval = userfsRemoveAll();
		else if(!userValidateID(nID)) result = R_PARAMETER_ERROR;
		else	rval = userfsRemove(nID);
		wdtReset();
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveEventNotification(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	if(*dataSize < 16) return R_PARAMETER_ERROR;
	evtAddRaw(rxData, *dataSize);
cprintf("EventNotification: %d result=0\n", *dataSize);
	*dataSize = 0;
	return 0;
}
