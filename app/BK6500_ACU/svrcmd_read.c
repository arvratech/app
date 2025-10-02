void ReadObject(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*p, *data;
	long	nID;
	int		rval, Result;

	if(reqnet->DataLength < 4) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;
		return;
	}
	data = reqnet->Data;
	Result = 0;
	PACK3toID(data+1, &nID);
	p = rspnet->Data; p += 4;
cprintf("ReadObject: ID=%d %02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], reqnet->DataLength-4);
	switch(data[0]) {
	case OT_DEVICE:
		p += devEncode(&sys_cfg->Device, p, 0);
		break;
#ifndef STAND_ALONE
	case OT_UNIT_SETTING:
		p += syscfgEncodeUnits(sys_cfg, p);
		break;
#endif
	case OT_NETWORK_PORT:
		p += nwpEncode(&sys_cfg->NetworkPort, p);
		memcpy(p, syscfgGetMACAddress(), 6); p += 6;
		break;
	case OT_ACCESS_DOOR:
		if(!adIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += adEncode(&sys_cfg->AccessDoors[nID], p);
		break;
	case OT_ACCESS_POINT:
		if(!apIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += apEncode(&sys_cfg->AccessPoints[nID], p);
		break;
#ifndef STAND_ALONE
	case OT_ACCESS_ZONE:
		if(!azValidateID(nID) || !nID) Result = R_PARAMETER_ERROR;
		else {
			rval = azGetEncoded(nID, p);
			if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
#endif
	case OT_BINARY_INPUT:
		if(!biIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += biEncode(&sys_cfg->BinaryInputs[nID], p);
		break;
	case OT_BINARY_OUTPUT:
		if(!boIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += boEncode(&sys_cfg->BinaryOutputs[nID], p);
		break;
	case OT_CREDENTIAL_READER:
		if(!crIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += crEncode(&sys_cfg->CredentialReaders[nID], p);
		break;
	case OT_FIRE_ZONE:
		if(!fzGetEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += fzEncode(&sys_cfg->FireZones[nID], p);
		break;
	case OT_ALARM_ACTION:
		if(!actIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += actEncode(&sys_cfg->AlarmActions[nID], p);
		break;
	case OT_OUTPUT_COMMAND:
		if(!ocValidateID(nID) || nID < 2) Result = R_PARAMETER_ERROR;
		else {
			rval = ocGetEncoded(nID, p);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	case OT_CALENDAR:
		if(!calValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			rval = calfsGetEncoded(nID, p);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	case OT_SCHEDULE:
		if(!scheValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			rval = schefsGetEncoded(nID, p);
//cprintf("schefsGetEncoded(%d)=%d\n", nID, rval);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	case OT_USER:
		if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			rval = userfsGetEncoded(nID, p);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	case OT_ACCESS_RIGHTS:
		if(!arValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			rval = arfsGetEncoded(nID, p);
//cprintf("arfsGetEncoded(%d)=%d\n", nID, rval);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else if(!rval) Result = R_OBJECT_NOT_FOUND;
			else	p += rval;
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(Result) {
		netCodeError(rspnet, 1, Result);
	} else {
		memcpy(rspnet->Data, reqnet->Data, 4);
		rspnet->DataLength = p - rspnet->Data;
		netCodeResponse(rspnet);
	}
	tsm->SvcPending = 1;	
}

int _EncodeUnitFirmwareStatus(unsigned char *buf);

void ReadProperty(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*p, *data;
	unsigned long	FPID;
	long	nID;
	int		rval, Result, i, rsp;

	if(reqnet->DataLength < 4) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;
		return;
	}
	rsp = 1;	
	data = reqnet->Data;
	Result = 0;
	PACK3toID(data+1, &nID);
	p = rspnet->Data; p += 5;
cprintf("ReadProperty: ID=%d %02x-%02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], reqnet->DataLength-5);
	switch(data[0]) {
	case OT_DEVICE:
		switch(data[4]) {
		case OP_LOCAL_DATE_TIME:
			rtcGetDateTime(p); p += 6;
			break;
		case OP_DEVICE_PASSWORD:
			string2bcd(sys_cfg->AdminPIN, 8, p); p += 4;
			break;
		case OP_MAKER:
			 memcpy_pad(p, sys_cfg->Device.Maker, 16); p += 16;
			break;
		case OP_MODEL:
			memcpy_pad(p, sys_cfg->Device.Model, 8); p += 8;
			break;
#ifndef STAND_ALONE
		case OP_PASSBACK:
			*p++ = sys_cfg->Passback;
			*p++ = sys_cfg->RepeatAccessDelay;
			bin2bcd(sys_cfg->PassbackResetTime, 2, p); p += 2;
			break;
		case OP_UNIT_FIRMWARE_STATUS:
			p += _EncodeUnitFirmwareStatus(p);
			break;
#endif
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_CREDENTIAL_READER:
		switch(data[4]) {
		case OP_FP_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += crfpEncode(&sys_cfg->CredentialReaders[nID].crFP, p);
			break;
		case OP_PIN_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += crpinEncode(&sys_cfg->CredentialReaders[nID].crPIN, p);
			break;
		case OP_MIFARE_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += crmifareEncode(&sys_cfg->CredentialReaders[nID].crMifare, p);
			break;
		case OP_EM_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += cremEncode(&sys_cfg->CredentialReaders[nID].crEM, p);
			break;
		case OP_WIEGAND_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += crwiegandEncode(&sys_cfg->CredentialReaders[nID].crWiegand, p);
			break;
		case OP_SERIAL_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			p += crserialEncode(&sys_cfg->CredentialReaders[nID].crSerial, p);
			break;
		case OP_IP_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	p += cripEncode(&sys_cfg->CredentialReaders[nID].crIP, p);
			break;
#ifndef STAND_ALONE
		case OP_IP_ADDRESS:
			if(!apIsEnable(nID) || unitGetType(nID) != 2 || crGetType(nID) == 0) Result = R_OBJECT_NOT_FOUND;
			else	{ SlaveSinGetIPAddress(nID, p); p += 4; }
			break;
#endif
		case OP_CAMERA:
		case OP_BLE_CONF:
		case OP_NETWORK_PORT:
		case OP_DEVICE:
			CrReadProperty(nID, tsm, reqnet, rspnet, (int)data[4]); rsp = 0;
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_FIRE_ZONE:
		switch(data[4]) {
		case OP_PRESENT_VALUE:
			if(!fzGetEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else	*p++ = fzGetPresentValue(nID);
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;

	case OT_USER:
		switch(data[4]) {
		case OP_FP_AUTHENTICATION_FACTOR:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				userID2FPID(nID, &FPID);
				rval = userfsGetEncodedFPTemplate(FPID, p);
//cprintf("1st userfsGetEncodedFPTemplate=%d\n", rval);
//if(rval > 0) { for(i=0;i<16;i++) cprintf("%02x-", (int)p[i]); cprintf("\n"); }
				if(rval > 0) {
					HFP_EncryptTamplate(p+4, 400, 2); HFP_EncryptTamplate(p+404, 400, 2);
					p += 804;
				}
				FPID++;
				rval = userfsGetEncodedFPTemplate(FPID, p);
//cprintf("2nd userfsGetEncodedFPTemplate=%d\n", rval);
//if(rval > 0) { for(i=0;i<16;i++) cprintf("%02x-", (int)p[i]); cprintf("\n"); }
				if(rval > 0) {
					HFP_EncryptTamplate(p+4, 400, 2); HFP_EncryptTamplate(p+404, 400, 2);
					p += 804;
				} else if(rval < 0) Result = R_R_SYSTEM_ERROR;
			}
			break;
		case OP_USER_EX:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				rval = userfsGetExEncoded(nID, p);
				if(rval < 0) Result = R_R_SYSTEM_ERROR;
				else	p += rval;
			}
			break;
		case OP_USER_PHOTO:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				//rval = userfsGetPhoto(sUser);
				//if(rval < 0) Result = R_R_SYSTEM_ERROR;
				//else if(rval > 0) p += userEncodePhoto(sUser, p);
				rval = userfsGetPhotoEncoded(nID, p);
				if(rval < 0) Result = R_R_SYSTEM_ERROR;
				else	p += rval;
			}
			break;
		case OP_USER_ACCESS_RIGHTS:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				rval = userfsGetAccessRightsEncoded(nID, p);
				if(rval < 0) Result = R_R_SYSTEM_ERROR;
				else	p += rval;
			}
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(rsp) {
		if(Result) {
			netCodeError(rspnet, 1, Result);
		} else {
			memcpy(rspnet->Data, reqnet->Data, 5);
			rspnet->DataLength = p - rspnet->Data;
			netCodeResponse(rspnet);
		}
		tsm->SvcPending = 1;
	}
}

#ifndef STAND_ALONE

int _EncodeUnitFirmwareStatus(unsigned char *buf)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*p, val;
	int		i;
	
	p = buf;
	if(CliGetXferState()) val = 1; else val = 0;
	*p++ = val;
	if(val) memset(p, 0, 6); else CliGetXferLastTime(p);
	p += 6;
	if(CliGetXferState() || *(p-6)) {
		cr = crsGetAt(0);
		for(i = 0;i < MAX_UNIT_SZ;i++, cr++)
			if(unitGetType(i) == 2 && cr->XferState) {
				*p++ = i;
				if(i == CliGetXferID()) {
					*p++ = 1;								// Processing
					*p++ = 100 * CliGetXferCount() / CliGetXferSize();
				} else {
					*p++ = cr->XferState;
					if(!crGetReliability(i)) val = 1; else val = 0;
					*p++ = val;
				}
			}
	}
	return p - buf;
}

#endif

void ReadStatus(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*p, *data;
	long	nID;
	int		Result;

	if(reqnet->DataLength < 4) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;
		return;
	}
	data = reqnet->Data;
	Result = 0;	
	PACK3toID(data+1, &nID);
	p = rspnet->Data; p += 4;
cprintf("ReadStatus: ID=%d %02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], reqnet->DataLength-4);
	switch(data[0]) {
#ifndef STAND_ALONE
	case OT_UNIT_SETTING:
		p += unitsEncodeStatusAll(p);
		break;
#endif
	case OT_ACCESS_DOOR:
		if(!adIsEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += adEncodeStatus(nID, p);
		break;
	case OT_BINARY_INPUT:
		if(!biIsEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += biEncodeStatus(nID, p);
		break;
	case OT_BINARY_OUTPUT:
		if(!boIsEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += boEncodeStatus(nID, p);
		break;
	case OT_CREDENTIAL_READER:
		if(!crIsEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += crEncodeStatus(nID, p);
		break;
	case OT_FIRE_ZONE:
		if(!fzGetEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else	p += fzEncodeStatus(nID, p);
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(Result) {
		netCodeError(rspnet, 1, Result);
	} else {
		memcpy(rspnet->Data, reqnet->Data, 4);
		rspnet->DataLength = p - rspnet->Data;
		netCodeResponse(rspnet);
	}
	tsm->SvcPending = 1;
}
