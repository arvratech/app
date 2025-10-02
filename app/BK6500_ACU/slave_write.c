int SlaveWriteObject(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	FP_USER		*user, _user;
	BINARY_INPUT	*bi;
	BINARY_OUTPUT	*bo;
	ACCESS_DOOR		*ad;
	CREDENTIAL_READER	*cr;
	unsigned char	*p;
	long	nID;
	int		rval, result;

	if(*dataSize < 4) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("WriteObject: OT=%02x ID=%d\n", (int)rxData[0], nID);
	result = 0;
	switch(rxData[0]) {
	case OT_USER:
		if(!userValidateID(nID) || *dataSize < 32) result = R_PARAMETER_ERROR;
		else {
			user = &_user;
			userDecode(user, rxData+4);
			if(!userValidate(user)) result = R_PARAMETER_ERROR;
			else if(userfsGetCount() >= GetMaxUserSize()) result = R_OBJECT_IS_FULL;
			else {
				rval = userfsAddEncoded(nID, rxData+4);
				if(rval < 0) result = R_R_SYSTEM_ERROR;
				else if(!rval) result = R_OBJECT_IS_FULL;
			}
		}
		break;
	case OT_BINARY_INPUT:
		if(*dataSize < 3) result = R_PARAMETER_ERROR;
		else if(nID > 4) result = R_PARAMETER_ERROR;
		else {
			nID = unitGetBinaryInput(unitID) + nID;
			bi = bisGetAt(nID);
			biDecode(bi->cfg, rxData+4);
			rval = syscfgWrite(sys_cfg);
		}
		break;
	case OT_BINARY_OUTPUT:
		if(*dataSize < 1) result = R_PARAMETER_ERROR;
		else if(nID > 2) result = R_PARAMETER_ERROR;
		else {
			nID = unitGetBinaryOutput(unitID) + nID;
			bo = bosGetAt(nID);
			boDecode(bo->cfg, rxData+4);
			rval = syscfgWrite(sys_cfg);
		}
		break;
	case OT_ACCESS_DOOR:
		if(*dataSize < 5) result = R_PARAMETER_ERROR;
		else if(nID > 0) result = R_PARAMETER_ERROR;
		else {
			ad = adsGetAt(unitID);
			adDecode2(ad->cfg, rxData+4);
			rval = syscfgWrite(sys_cfg);
		}
		break;
	case OT_CREDENTIAL_READER:
		if(*dataSize < 1) result = R_PARAMETER_ERROR;
		else if(nID > 0) result = R_PARAMETER_ERROR;
		else {
			cr = crsGetAt(unitID);
			crDecode2(cr->cfg, rxData+4);
			rval = syscfgWrite(sys_cfg);
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result) *dataSize = p - txData;
	return result;
}

int SlaveWriteProperty(int unitID, unsigned char *txData, unsigned char *rxData, int *dataSize)
{
	CR_CFG	*cr_cfg, *tmp_cfg, _tmp_cfg;
	unsigned char	*s, *d, *p, temp[800];
	long	nID;
	unsigned long	FPID, nFPID;
	unsigned short	len1, len2;
	int		rval, cnt, result;

	if(*dataSize < 5) return R_PARAMETER_ERROR;
	PACK3toID(rxData+1, &nID);
	p = txData;
//cprintf("WriteProperty: OT=%02x ID=%d OP=%02x\n", (int)rxData[0], nID, (int)rxData[4]);
	result = 0; s = rxData + 5;
	switch(rxData[0]) {
	case OT_USER:
		switch(rxData[4]) {
		case OP_FP_AUTHENTICATION_FACTOR:
			if(!userValidateID(nID) || *dataSize < 13) result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) result = R_OBJECT_NOT_FOUND;
			else {
				userID2FPID(nID, &FPID);
				userDecodeFPID(&nFPID, s); s += 4;
//cprintf("nID=%d FPID=%x %x %02x-%02x-%02x-%02x\n", nID, FPID, nFPID, (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
				if(nFPID != FPID && nFPID != FPID+1) {
					result = R_PARAMETER_ERROR; break;
				}
				rval = sfpGetTemplate(FPID, temp);
				if(rval <= 0) {
					cnt = userfsGetFPTemplateCount();
					if(cnt >= GetMaxFPIdentifySize()) result = R_OBJECT_IS_FULL;
				}
				if(!result) { 
					cnt = *dataSize - 9;
					d = s;
					BYTEtoSHORT(s, &len1); s += 2;
					if(cnt < len1 + 4) result = R_PARAMETER_ERROR;
					else {
						BYTEtoSHORT(s+len1, &len2);
						if(cnt != len1 + len2 + 4) result = R_PARAMETER_ERROR;
					}	
					if(!result) {
						memcpy(d, s, len1); d += len1; s += len1 + 2;
						memcpy(temp, s, len2);
						cnt = 400 - len1;
						if(cnt > 0) {
							memcpy(d, 0, cnt); d += cnt;
						}
						memcpy(d, temp, len2); d += len2;
						cnt = 400 - len2;
						if(cnt > 0) memcpy(d, 0, cnt);
						s = rxData + 5;
					}
				}
				if(!result) {
					rval = userfsAddEncodedFPTemplate(s);
					if(rval < 0) result = R_R_SYSTEM_ERROR;					
				}
			}
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	case OT_CREDENTIAL_READER:
		cr_cfg = &sys_cfg->CredentialReaders[nID];
		tmp_cfg = &_tmp_cfg;
		memcpy(tmp_cfg, cr_cfg, sizeof(CR_CFG));
		tmp_cfg->Version = *s++;
		if(!crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
		else if(unitGetType(nID) != 2 && cr_cfg->Version != tmp_cfg->Version) result = R_ACTION_NOT_ALLOWED;
		else {
			switch(rxData[4]) {
			case 0:
				crDecode(tmp_cfg, s);
				if(!crValidate(tmp_cfg)) result = R_PARAMETER_ERROR;
				tmp_cfg->Type = cr_cfg->Type;
				break;
			case OP_FP_READER:
				crfpDecode(&tmp_cfg->crFP, s);
				if(!crfpValidate(&tmp_cfg->crFP)) result = R_PARAMETER_ERROR;
				break;
			case OP_PIN_READER:
				crpinDecode(&tmp_cfg->crPIN, s);
				if(!crpinValidate(&tmp_cfg->crPIN)) result = R_PARAMETER_ERROR;
				break;
			case OP_MIFARE_READER:
				crmifareDecode(&tmp_cfg->crMifare, s);
				if(!crmifareValidate(&tmp_cfg->crMifare)) result = R_PARAMETER_ERROR;
				break;
			case OP_EM_READER:
				cremDecode(&tmp_cfg->crEM, s);
				if(!cremValidate(&tmp_cfg->crEM)) result = R_PARAMETER_ERROR;
				break;
			case OP_WIEGAND_READER:
				crwiegandDecode(&tmp_cfg->crWiegand, s);
				if(!crwiegandValidate(&tmp_cfg->crWiegand)) result = R_PARAMETER_ERROR;
				break;
			case OP_SERIAL_READER:
				crserialDecode(&tmp_cfg->crSerial, s);
				if(!crserialValidate(&tmp_cfg->crSerial)) result = R_PARAMETER_ERROR;
				break;
			case OP_IP_READER:
				cripDecode(&tmp_cfg->crIP, s);
				if(!cripValidate(&tmp_cfg->crIP)) result = R_PARAMETER_ERROR;
				break;
			default:	result = R_PARAMETER_ERROR;
			}
		}
		if(!result) {
			rval = CruSaveCfg(nID, tmp_cfg);
			if(rval < 0) result = R_R_SYSTEM_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
//cprintf("WriteProperty: result=%d\n", result);
	if(!result) *dataSize = p - txData;
	return result;
}

