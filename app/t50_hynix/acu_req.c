static void _SvrRequest(int command, int dataSize)
{
	unsigned char	*p;

	p = SlvnetConfReqBuf();
	*p = reqCmd = (unsigned char)command;
	SlvnetSetConfReqTxSize(dataSize+1, 0);
}

void acuReqReadPassword(void)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_DEVICE;
	IDtoPACK3(0, p); p += 3;
	*p++ = reqOp = OP_DEVICE_PASSWORD;
	_SvrRequest(P_READ_PROPERTY, p - p0); 
}

void acuReqReadUser(FP_USER *user)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(user->id, p); p += 3;
	_SvrRequest(P_READ_OBJECT, p - p0); 
}

int acuCnfReadUser(FP_USER *user)
{
	unsigned char	*p;
	int		result, dataSize;

	p = SlvnetConfReqBuf();
	result = *p++;
	if(!result) {
		dataSize = SlvnetConfReqRxSize();
		userDecode(user, p);
	}
	return result;
}

void acuReqFindUser(long nID)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(nID, p); p += 3;
	_SvrRequest(P_FIND_OBJECT, p - p0); 
}

void acuReqFindUserFPTemplate(FP_USER *user)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(user->id, p); p += 3;
	*p++ = reqOp = OP_FP_AUTHENTICATION_FACTOR;
	_SvrRequest(P_FIND_PROPERTY, p - p0); 
}

int acuCnfFindUserFPTemplate(FP_USER *user)
{
	unsigned char	*p;
	int		result, dataSize;

	p = SlvnetConfReqBuf();
	result = *p++;
	if(!result) {
		dataSize = SlvnetConfReqRxSize();
		if(*p) userSetFPData1(user);
		p++;
		if(*p) userSetFPData2(user);
	}
	return result;
}

void acuReqFindUserCardData(char *data)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(0, p); p += 3;
	*p++ = reqOp = OP_SECOND_CARD_AUTHENTICATION_FACTOR;
	memcpy(p, data, 9); p += 9;
	_SvrRequest(P_FIND_PROPERTY, p - p0); 
}

void acuReqDeleteUser(long nID)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(nID, p); p += 3;
	_SvrRequest(P_DELETE_OBJECT, p - p0); 
}

void acuReqGetNewUserID(long ID, long minID, long maxID)
{
	unsigned char	*p, *p0;
	int		result, dataSize;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(ID, p); p += 3;
	*p++ = reqOp = 0x30;
	IDtoPACK3(minID, p); p += 3;
	IDtoPACK3(maxID, p); p += 3;
	_SvrRequest(P_GET_PROPERTY, p - p0); 
}

int acuCnfGetNewUserID(long *pID)
{
	unsigned char	*p;
	int		result, dataSize;

	p = SlvnetConfReqBuf();
	result = *p++;
	if(!result) {
		dataSize = SlvnetConfReqRxSize();
		PACK3toID(p, pID);
	}
	return result;
}

void acuReqGetUserCount(int *pCount)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(0, p); p += 3;
	*p++ = reqOp = 0x31;
	_SvrRequest(P_GET_PROPERTY, p - p0); 
}

int acuCnfGetUserCount(int *pCount)
{
	unsigned char	*p;
	long	lVal;
	int		result, dataSize;

	p = SlvnetConfReqBuf();
	result = *p++;
	if(!result) {
		dataSize = SlvnetConfReqRxSize();
		BYTEtoLONG(p, &lVal); *pCount = (int)lVal;
	}
	return result;
}

void acuReqWriteUser(FP_USER *user)
{
	unsigned char	*p, *p0;
	int		result;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(user->id, p); p += 3;
	p += userEncode(user, p);
	_SvrRequest(P_WRITE_OBJECT, p - p0); 
}

int acuCnfWriteUser(void)
{
	unsigned char	*p;
	int		result;

	p = SlvnetConfReqBuf();
	result = *p++;
	return result;
}

void acuReqWriteUserFPTemplate(long nID, unsigned long fpId, unsigned char *FPTemplate)
{
	unsigned char	*p, *p0, *s;
	unsigned short	len;
	int		result, i;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(nID, p); p += 3;
	*p++ = reqOp = OP_FP_AUTHENTICATION_FACTOR;
	userEncodeFpid(fpId, p); p += 4;
	s = FPTemplate + 399;
	for(i = 0;i < 400;i++, s--) if(*s) break;
	len = 400 - i;
	SHORTtoBYTE(len, p); p += 2;
	memcpy(p, FPTemplate, (int)len); p += len;
	s = FPTemplate + 899;
	for(i = 0;i < 400;i++, s--) if(*s) break;
	len = 400 - i;
	SHORTtoBYTE(len, p); p += 2;
	memcpy(p, FPTemplate+400, (int)len); p += len;
	_SvrRequest(P_WRITE_PROPERTY, p - p0); 
}

void acuReqWriteBinaryInput(BINARY_INPUT *bi)
{
	unsigned char	*p, *p0;
	int		result, val;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_BINARY_INPUT;
	IDtoPACK3(biId(bi), p); p += 3;
	if(biPolarity(bi)) val = 0x01; else val = 0x00;
	if(biAlarmInhibit(bi)) val |= 0x02;
	*p++ = val;
	*p++ = biTimeDelay(bi);
	*p++ = 0xff;	//	bi_cfg->AlarmInhibitScheID;	
	_SvrRequest(P_WRITE_OBJECT, p - p0); 
}

void acuReqWriteBinaryOutput(BINARY_OUTPUT *bo)
{
	unsigned char	*p, *p0;
	int		result, val;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_BINARY_OUTPUT;
	IDtoPACK3(boId(bo), p); p += 3;
	if(boPolarity(bo)) val = 0x01; else val = 0x00;
	*p++ = val;
	_SvrRequest(P_WRITE_OBJECT, p - p0); 
}

void acuReqWriteAccessDoor(ACCESS_DOOR *ad)
{
	void	*bi, *bo;
	unsigned char	*p, *p0;
	int		result, val;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_ACCESS_DOOR;
//	IDtoPACK3(adId(ad), p); p += 3;
	IDtoPACK3(0, p); p += 3;
	if(adIsNonNull(ad)) val = 0x01;
	else	val = 0x00;
	bi = bisGet(0);
	if(biType(bi) == BI_TYPE_DOOR) val |= 0x02;
	bi = bisGet(1);
	if(biType(bi) == BI_TYPE_DOOR) val |= 0x04;
	bo = bosGet(1);
	if(boType(bo) == BO_TYPE_DOOR) val |= 0x08;
	*p++ = val;
	if(adLockType(ad)) val = 0x01; else val = 0x00;
	*p++ = val;
	*p++ = adDoorPulseTime(ad);
	*p++ = adTimeDelay(ad);
	*p++ = adTimeDelayNormal(ad);
	_SvrRequest(P_WRITE_OBJECT, p - p0); 
}

void acuReqWriteCredentialReader(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		result, val;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_CREDENTIAL_READER;
	IDtoPACK3(0, p); p += 3;
	if(devEnableTamper(NULL)) val = 0x00; else val = 0x01;
	if(devEnableIntrusion(NULL)) val |= 0x02;
	*p++ = val;
	_SvrRequest(P_WRITE_OBJECT, p - p0); 
}

void acuReqDeleteUserFPTemplate(long nID, unsigned long fpId)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = reqOt = OT_USER;
	IDtoPACK3(nID, p); p += 3;
	*p++ = reqOp = OP_FP_AUTHENTICATION_FACTOR;
	userEncodeFpid(fpId, p); p += 4;
	_SvrRequest(P_DELETE_PROPERTY, p - p0); 
}	

/*
void acuSvrWriteCredentialReader(CR_CFG *cr_cfg, int OP_ID)
{
	unsigned char	*p, *p0;

	p0 = p = SlvnetConfReqBuf() + 1;
	*p++ = OT_CREDENTIAL_READER;
	IDtoPACK3((long)sys_cfg->DeviceID, p); p += 3;
	*p++ = OP_ID;
	*p++ = cr_cfg->Version;
	switch(OP_ID) {
	case 0:
		p += crEncode(cr_cfg, p); break;	
	case OP_FP_READER:
		p += crfpEncode(&cr_cfg->crFP, p); break;
	case OP_PIN_READER:
		p += crpinEncode(&cr_cfg->crPIN, p); break;
	case OP_MIFARE_READER:
		p += crmifareEncode(&cr_cfg->crMifare, p); break;
	case OP_EM_READER:
		p += cremEncode(&cr_cfg->crEM, p); break;
	case OP_WIEGAND_READER:
		p += crwiegandEncode(&cr_cfg->crWiegand, p); break;
	case OP_SERIAL_READER:
		p += crserialEncode(&cr_cfg->crSerial, p); break;
	case OP_IP_READER:
		p += cripEncode(&cr_cfg->crIP, p); break;
	}		
	_SvrRequest(P_WRITE_PROPERTY, p - p0); 
}

int SvrWriteCredentialReader(CR_CFG *cr_cfg, int OP_ID)
{
	unsigned char	*p;
	int		result;

	p = SlvnetConfReqBuf();
	result = *p++;
	return result;
}
*/

