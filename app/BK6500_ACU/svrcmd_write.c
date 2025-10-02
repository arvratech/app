void _SetupUnits(SYS_CFG *tmp_cfg);

void WriteObject(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
#ifndef STAND_ALONE
	UNITS_CFG		_UnitsCfg;
#endif
	unsigned char	*p, *data;
	long	nID;
	int		rval, ot, Result, nSize, val, section, UnitID, IOUnitID;

	if(reqnet->DataLength < 4) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;
		return;
	}
	data = reqnet->Data;
	if(tsmsWritingObject(RspTSMs, MAX_RSP_TSM_SZ, tsm, (int)data[0])) {
		Result = R_ACTION_NOT_ALLOWED;
		netCodeError(rspnet, 1, Result);
		tsm->SvcPending = 1;
		return;
	}
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	Result = 0; section = 0;
	PACK3toID(data+1, &nID);
	p = data + 4;
	nSize = reqnet->DataLength - 4;
	ot = data[0];
cprintf("WriteObject: ID=%d %02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], nSize);
	switch(ot) {
	case OT_DEVICE:
		devDecode(&tmp_cfg->Device, p, 1);
		if(!devValidate(&tmp_cfg->Device)) Result = R_PARAMETER_ERROR;
		else	section = 1;
		break;
#ifndef STAND_ALONE
	case OT_UNIT_SETTING:
		syscfgDecodeUnits(tmp_cfg, p);
		if(!syscfgValidateUnits(tmp_cfg)) Result = R_PARAMETER_ERROR;
		else	section = 1;
		break;
#endif
	case OT_NETWORK_PORT:
		nwpDecode(&tmp_cfg->NetworkPort, p);
		if(!nwpValidate(&tmp_cfg->NetworkPort)) Result = R_PARAMETER_ERROR;
		else	section = 1;
		break;
	case OT_ACCESS_DOOR:
		if(!adIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			adDecode(&tmp_cfg->AccessDoors[nID], p);
			if(!adValidate(&tmp_cfg->AccessDoors[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
			UnitID = nID; IOUnitID = -1;
		}
		break;
	case OT_ACCESS_POINT:
		if(!apIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			apDecode(&tmp_cfg->AccessPoints[nID], p);
			if(!apValidate(&tmp_cfg->AccessPoints[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
		}
		break;
#ifndef STAND_ALONE
	case OT_ACCESS_ZONE:
		if(!azValidateID(nID) || !nID) Result = R_PARAMETER_ERROR;
		else {
			rval = azAddEncoded(nID, p);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else p += rval;
		}
		break;
#endif
	case OT_BINARY_INPUT:
		if(!biIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			biDecode(&tmp_cfg->BinaryInputs[nID], p);
			if(!biValidate(&tmp_cfg->BinaryInputs[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
#ifndef STAND_ALONE
			IOUnitID = biGetIOUnit((int)nID);
			if(IOUnitID < 0) UnitID = biGetUnit((int)nID);
			else	UnitID = -1;
#endif
		}
		break;
	case OT_BINARY_OUTPUT:
		if(!boIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			boDecode(&tmp_cfg->BinaryOutputs[nID], p);
			if(!boValidate(&tmp_cfg->BinaryOutputs[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
#ifndef STAND_ALONE
			IOUnitID = boGetIOUnit((int)nID);
			if(IOUnitID < 0) UnitID = boGetUnit((int)nID);
			else	UnitID = -1;
#endif
		}
		break;
	case OT_CREDENTIAL_READER:
		if(!crIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			crDecode(&tmp_cfg->CredentialReaders[nID], p);
			if(!crValidate(&tmp_cfg->CredentialReaders[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
		}
		break;
	case OT_FIRE_ZONE:
		if(nID < 0 || nID >= MAX_UNIT_HALF_SZ) Result = R_OBJECT_NOT_FOUND;
		else {
			fzDecode(&tmp_cfg->FireZones[nID], p);
			if(!fzValidate(&tmp_cfg->FireZones[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
		}
		break;
	case OT_ALARM_ACTION:
		if(!actIsUsable((int)nID)) Result = R_OBJECT_NOT_FOUND;
		else {
			actDecode(&tmp_cfg->AlarmActions[nID], p);
//cprintf("ID=%d AlarmOutput=%d\n", nID, (int)tmp_cfg->AlarmActions[nID].AlarmOutput);
			if(!actValidate(&tmp_cfg->AlarmActions[nID])) Result = R_PARAMETER_ERROR;
			else	section = 1;
		}
		break;
	case OT_OUTPUT_COMMAND:
		if(!ocValidateID(nID) || nID < 2) Result = R_PARAMETER_ERROR;
		else {
			rval = ocAddEncoded(nID, p);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else p += rval;
		}
		break;
	case OT_CALENDAR:
		if(!calValidateID(nID) || nSize < 126) Result = R_PARAMETER_ERROR;
		else {
			rval = calfsAddEncoded(nID, p);
//cprintf("calfsAddEncoded(%d)=%d\n", nID, rval);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else p += rval;
		}
		break;
	case OT_SCHEDULE:
		if(!scheValidateID(nID) || nSize < 254) Result = R_PARAMETER_ERROR;
		else {
			rval = schefsAddEncoded(nID, p);
//cprintf("schefsAddEncoded(%d)=%d\n", nID, rval);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else p += rval;
		}
		break;
	case OT_USER:
		if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			userDecode(sUser, p);
			if(!userValidate(sUser)) Result = R_PARAMETER_ERROR;
			else if(userfsGetCount() >= GetMaxUserSize()) Result = R_OBJECT_IS_FULL;
			else {
				rval = userfsAddEncoded(nID, p);
				if(rval < 0) Result = R_R_SYSTEM_ERROR;
				else if(!rval) Result = R_OBJECT_IS_FULL; 
				else	p += rval;
			}
		}
		break;
	case OT_ACCESS_RIGHTS:
		if(!arValidateID(nID)) Result = R_PARAMETER_ERROR;
		else {
			rval = arfsAddEncoded(nID, p);
//cprintf("arfsAddEncoded(%d)=%d\n", nID, rval);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else	p += rval;
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(!Result && section && syscfgCompare(sys_cfg, tmp_cfg)) {	
		rval = syscfgWrite(tmp_cfg);
		if(rval < 0) Result = R_R_SYSTEM_ERROR;
		else {
			val = sys_cfg->Device.Language;
#ifndef STAND_ALONE
			if(ot == OT_UNIT_SETTING) {
				unitscfgSet(&_UnitsCfg);
				syscfgCopyToUnits(sys_cfg);	
			}
#endif
			syscfgCopy(sys_cfg, tmp_cfg);
			if(val != sys_cfg->Device.Language) {
				ChangeLanguage((int)sys_cfg->Device.Language);
				//if(DesktopMainIsVisible()) DesktopDrawAll();
			}
#ifndef STAND_ALONE
			if(ot == OT_ACCESS_DOOR || ot == OT_BINARY_INPUT || ot == OT_BINARY_OUTPUT) {
				if(IOUnitID >= 0) iounitEncodeConfig(IOUnitID);
				else {
					unitEncodeIouConfig(UnitID);
				}
			} else if(ot == OT_UNIT_SETTING) {
				UpdateViewMain();
				syscfgCopyFromUnits(tmp_cfg);
				_SetupUnits(tmp_cfg);
				_SetupIOUnits(tmp_cfg);				
				//val = (MAX_UNIT_SZ >> 1) - 1;
				//for(rval = 0;rval < val;rval++) unitEncodeIouConfig((rval<<1)+2);
			}
#endif
		}
	}
	if(Result) {
		netCodeError(rspnet, 1, Result);
	} else {
		rspnet->DataLength = 0;
		netCodeResponse(rspnet);
	}
	tsm->SvcPending = 1;
}

void CliDiarm(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet);
int  DoorListControl(long adMask, int Command, char *LoginID);
int  DoorControl(long adID, int Command, char *LoginID);
int  DoorControlAllLockOpen(char *LoginID);
int  DoorControlAllLockOpenRelease(char *LoginID);
int  ResetFireZone(int ID);
int  UnsilenceFireZone(int ID);

void scPrintKey(unsigned char *key, char *msg)
{
	cprintf("%s=[%02x %02x %02x %02x %02x %02x]\n", msg, (int)key[0], (int)key[1], (int)key[2], (int)key[3], (int)key[4], (int)key[5]);
}

void WriteProperty(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CR_CFG	*cr_cfg, *tmp_cr_cfg, _tmp_cr_cfg;
	unsigned char	*p, *data, temp[40];
	unsigned long	FPID, nFPID;
	long	nID;
	int		rval, Result, cnt, nSize, section, pwd, rsp;

	if(reqnet->DataLength < 5) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;		
		return;
	}
	data = reqnet->Data;
	if(tsmsWritingObject(RspTSMs, MAX_RSP_TSM_SZ, tsm, (int)data[0])) {
		Result = R_ACTION_NOT_ALLOWED;
		netCodeError(rspnet, 1, Result);
		tsm->SvcPending = 1;
		return;
	}
	rsp = 1;	
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	Result = 0; section = 0;
	PACK3toID(data+1, &nID);
	p = data + 5;
	nSize = reqnet->DataLength - 5;
cprintf("WriteProperty: ID=%d %02x-%02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], (int)data[4], nSize);
	pwd = 0;
	switch(data[0]) {
	case OT_DEVICE:
		switch(data[4]) {
		case OP_LOCAL_DATE_TIME:
			if(nSize < 6) Result = R_PARAMETER_ERROR;
			else {
				if(!datetime_validate(p)) Result = R_PARAMETER_ERROR;
				else {
					rtcSetDateTime(p);
#ifndef STAND_ALONE
					ClisLocalDateTimeChanged();				
#endif
					gRequest |= G_DATE_TIME_UPDATED;
				}
			}
			break;
		case OP_DEVICE_PASSWORD:
			if(nSize < 4) Result = R_PARAMETER_ERROR;
			else {
				bcd2string(p, 8, tmp_cfg->AdminPIN);
				pwd = 1;			
				section = 1;
			}
			break;
		case OP_MAKER:
			if(nSize < 16) Result = R_PARAMETER_ERROR;
			else {
				memcpy_chop(tmp_cfg->Device.Maker, p, 16);
				section = 1;
			}
			break;
		case OP_MODEL:
			if(nSize < 8) Result = R_PARAMETER_ERROR;
			else {
				memcpy_chop(tmp_cfg->Device.Model, p, 8);
				section = 1;
			}
			break; 		
#ifndef STAND_ALONE
		case OP_PASSBACK:
			sys_cfg->Passback			= *p++;
			sys_cfg->RepeatAccessDelay	= *p++;
			bcd2bin(p, 2, sys_cfg->PassbackResetTime); p += 2;
			break;
#endif
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_ACCESS_DOOR:
		switch(data[4]) {
		case OP_PRESENT_VALUE:
			rval = *p++;
			if(reqnet->DataLength >= 14) memcpy_chop(temp, p, 8);
			else	temp[0] = 0;
//cprintf("WriteProperty: AD nID=0x%x cmd=%d\n", nID, rval);
			if(nID < 0 && rval == 9) Result = DoorControlAllLockOpen((char *)temp);
			else if(nID < 0 && rval == 10) Result = DoorControlAllLockOpenRelease((char *)temp);
			else if(nID >= 0x800000) Result = DoorListControl(nID, rval, (char *)temp);
			else if(adIsEnable(nID)) Result = DoorControl(nID, rval, (char *)temp);
			else	Result = R_PARAMETER_ERROR;
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_BINARY_OUTPUT:
		switch(data[4]) {
		case OP_PRESENT_VALUE:
			rval = *p;
			if(boIsEnable((int)nID)) boSetPresentValue((int)nID, rval);
			else	Result = R_OBJECT_NOT_FOUND;
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_CREDENTIAL_READER:
		cr_cfg = &sys_cfg->CredentialReaders[nID];
		tmp_cr_cfg = &_tmp_cr_cfg;
		memcpy(tmp_cr_cfg, cr_cfg, sizeof(CR_CFG));
		switch(data[4]) {
		case OP_FP_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				crfpDecode(&tmp_cr_cfg->crFP, p);
				if(!crfpValidate(&tmp_cr_cfg->crFP)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_PIN_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				crpinDecode(&tmp_cr_cfg->crPIN, p);
				if(!crpinValidate(&tmp_cr_cfg->crPIN)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_MIFARE_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				crmifareDecode(&tmp_cr_cfg->crMifare, p);
				if(!crmifareValidate(&tmp_cr_cfg->crMifare)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_EM_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				cremDecode(&tmp_cr_cfg->crEM, p);
				if(!cremValidate(&tmp_cr_cfg->crEM)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_WIEGAND_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				crwiegandDecode(&tmp_cr_cfg->crWiegand, p);
				if(!crwiegandValidate(&tmp_cr_cfg->crWiegand)) Result = R_PARAMETER_ERROR;
				else	section = 1;
			}
			break;
		case OP_SERIAL_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				crserialDecode(&tmp_cr_cfg->crSerial, p);
				if(!crserialValidate(&tmp_cr_cfg->crSerial)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_IP_READER:
			if(!crIsEnable(nID)) Result = R_OBJECT_NOT_FOUND;
			else {
				cripDecode(&tmp_cr_cfg->crIP, p);
				if(!cripValidate(&tmp_cr_cfg->crIP)) Result = R_PARAMETER_ERROR;
			}
			break;
		case OP_SMART_CARD_KEY:
		// SmartCard Key: CurrentPrimaryKey(C) + NewPrimaryKey(P) + NewSecondaryKey(S) + KeyOption(O)
		// xxxxCCCCCCxxxxPPPPPPxxxxSSSSSSxO
//scPrintKey(sys_cfg->SCPrimaryKey, "Old PrimaryKey");
//scPrintKey(sys_cfg->SCSecondaryKey, "Old SecondaryKey");
			if(nSize < 32) Result = R_PARAMETER_ERROR;
			else {
				rval = sckeyDecrypt(temp, p, (int)sys_cfg->DeviceID);
//scPrintKey(temp, "Current PrimaryKey");
//scPrintKey(temp+6, "New PrimaryKey");
//scPrintKey(temp+12, "New SecondaryKey");
				if(sckeyValidate(sys_cfg->SCPrimaryKey) && n_memcmp(sys_cfg->SCPrimaryKey, temp, 6)) Result = R_ACTION_NOT_ALLOWED;
				else {
					//sckeySetValidateKey(temp+6);
					rval = CruSaveSCKey(temp+6);
					if(rval) Result = R_R_SYSTEM_ERROR;
				}
			}
//cprintf("New SCKeyOption=%02x\n", sys_cfg->SCKeyOption);
			break;
#ifndef STAND_ALONE
		case OP_INTRUSION_PRESENT_VALUE:
			if(nSize < 1 || (*p)) Result = R_PARAMETER_ERROR;
			else {
				CrDiarm(nID, tsm, reqnet, rspnet); rsp = 0;
			}
#endif
			break;
		case OP_CAMERA:
			if(nSize < 1) Result = R_PARAMETER_ERROR;
			else {
				CrWriteProperty(nID, tsm, reqnet, rspnet, (int)data[4]); rsp = 0;
			}
			break;
		case OP_BLE_CONF:
			if(nSize < 1) Result = R_PARAMETER_ERROR;
			else {
				CrWriteProperty(nID, tsm, reqnet, rspnet, (int)data[4]); rsp = 0;
			}
			break;
		case OP_NETWORK_PORT:
			if(nSize < 1) Result = R_PARAMETER_ERROR;
			else {
				CrWriteProperty(nID, tsm, reqnet, rspnet, (int)data[4]); rsp = 0;
			}
			break;
		case OP_DEVICE:
			if(nSize < 1) Result = R_PARAMETER_ERROR;
			else {
				CrWriteProperty(nID, tsm, reqnet, rspnet, (int)data[4]); rsp = 0;
			}
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_FIRE_ZONE:
		switch(data[4]) {
		case OP_PRESENT_VALUE:
			if(nID >= 0 && !fzGetEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
			else if(p[0]) Result = R_PARAMETER_ERROR;
			else	Result = ResetFireZone(nID);
			break;
		case OP_SILENCED:
			if(!fzGetEnable((int)nID)) Result = R_OBJECT_NOT_FOUND;
			else if(p[0]) Result = R_PARAMETER_ERROR;
			else	UnsilenceFireZone(nID);
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_USER:
		switch(data[4]) {
		case OP_FP_AUTHENTICATION_FACTOR:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else if(nSize != 804 && nSize != 1608) Result = R_PARAMETER_ERROR;
			else {
				userID2FPID(nID, &FPID);
				userDecodeFPID(&nFPID, p);
//cprintf("nID=%d FPID=%x %x %02x-%02x-%02x-%02x\n", nID, FPID, nFPID, (int)p[0], (int)p[1], (int)p[2], (int)p[3]);
				if(nFPID != FPID && nFPID != FPID+1) {
					Result = R_PARAMETER_ERROR; break;
				}
				cnt = userfsGetFPTemplateCount();
				cnt++;
				if(nSize >= 1608) {
					userDecodeFPID(&nFPID, p+804);
					if(nFPID != FPID && nFPID != FPID+1) {
						Result = R_PARAMETER_ERROR; break;
					}
					cnt++;
				}
				//if(sys_cfg->FPReader.FPIdentify) rval = GetMaxFPIdentifySize();
				//else	rval = GetMaxFPTemplateSize();
				rval = GetMaxFPIdentifySize();
				if(cnt > rval) Result = R_OBJECT_IS_FULL;
				else if(nSize >= 804) {
//for(i=0;i<16;i++) cprintf("%02x-", (int)p[i]); cprintf("\n");
					HFP_DecryptTamplate(p+4); HFP_DecryptTamplate(p+404);
					rval = userfsAddEncodedFPTemplate(p);
//cprintf("1st userfsAddEncodedFPTemplate=%d\n", rval);
//for(i=0;i<16;i++) cprintf("%02x-", (int)p[i]); cprintf("\n");
					if(rval > 0 && nSize >= 1608) {
						HFP_DecryptTamplate(p+808); HFP_DecryptTamplate(p+1208);
						rval = userfsAddEncodedFPTemplate(p+804);
//cprintf("2nd userfsAddEncodedFPTemplate=%d\n", rval);
//for(i=0;i<16;i++) cprintf("%02x-", (int)p[i+804]); cprintf("\n");
					}
					if(rval < 0) Result = R_R_SYSTEM_ERROR;					
				}
			}
			break;
		case OP_USER_EX:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				userDecodeEx(sUser, p);
				if(!userValidateEx(sUser)) Result = R_PARAMETER_ERROR;
				else {
					rval = userfsAddExEncoded(nID, p);
					if(rval < 0) Result = R_R_SYSTEM_ERROR;
					else if(!rval) Result = R_OBJECT_IS_FULL; 
					else	p += rval;
				}
			}
			break;
		case OP_USER_PHOTO:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				userDecodePhoto(sUser, p);
				if(!userValidatePhoto(sUser)) Result = R_PARAMETER_ERROR;
				else {
					rval = userEncodePhoto(sUser, p);
					if(rval < 0) Result = R_R_SYSTEM_ERROR;
					else if(!rval) Result = R_OBJECT_IS_FULL; 
					else	p += rval;
				}
			}
			break;
		case OP_USER_ACCESS_RIGHTS:
			if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else if(userfsExist(nID) <= 0) Result = R_OBJECT_NOT_FOUND;
			else {
				userDecodeAccessRights(sUser, p);
				if(!userValidateAccessRights(sUser)) Result = R_PARAMETER_ERROR;
				else {
					rval = userfsAddAccessRightsEncoded(nID, p);
					if(rval < 0) Result = R_R_SYSTEM_ERROR;
					else if(!rval) Result = R_OBJECT_IS_FULL; 
					else	p += rval;
				}
			}
			break;
#ifndef STAND_ALONE
		case OP_RESET_PASSBACK:
			wdtResetLong();
			if(nID < 0)	rval = userfsRemoveAllTransaction();
			else if(!userValidateID(nID)) Result = R_PARAMETER_ERROR;
			else	rval = userfsRemoveTransaction(nID);
			wdtReset();
			break;	
#endif
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
//cprintf("Result=%d Section=%d\n", Result, section);
	if(!Result && data[0] == OT_CREDENTIAL_READER) {
		if(n_memcmp(cr_cfg, tmp_cr_cfg, sizeof(CR_CFG))) {
			rval = CruSaveCfg(nID, tmp_cr_cfg);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
		}
	} else if(!Result && section && syscfgCompare(sys_cfg, tmp_cfg)) {
		rval = syscfgWrite(tmp_cfg);
		if(rval < 0) Result = R_R_SYSTEM_ERROR;
		else {
			syscfgCopy(sys_cfg, tmp_cfg);
#ifndef STAND_ALONE	
			if(pwd) ClisAdminPINChanged();
#endif
		}
	}
	if(rsp) {
		if(Result) {
			netCodeError(rspnet, 1, Result);
		} else {
			rspnet->DataLength = 0;
			netCodeResponse(rspnet);
		}
		tsm->SvcPending = 1;
	}
}

int ResetFireZone(int ID)
{
	unsigned char	ctm[8];
	int		i, rval, cnt, lcnt;

	cnt = 0;
	rtcGetDateTime(ctm);
	if(ID < 0) {
		for(i = 0;i < MAX_UNIT_HALF_SZ;i++) {
			if(fzGetEnable(i)) {
				lcnt = 0;
				if(fzGetPresentValue(i)) {
					EventAdd(OT_FIRE_ZONE, i, E_FIRE_NORMAL, ctm, NULL);
					fzSetPresentValue(i, 0, ctm);
					lcnt++;
cprintf("%ld FireZone-%d: Normal\n", DS_TIMER, i);
				}
				if(!fzGetSilenced(i)) {
					fzSetSilenced(i, 1, ctm);
					lcnt++;
				}
				if(lcnt) {
					fzNotifyStatus(i);
					cnt++;
				}
			}
		}
	} else {
		if(fzGetPresentValue(ID)) {
			EventAdd(OT_FIRE_ZONE, ID, E_FIRE_NORMAL, ctm, NULL);
			fzSetPresentValue(ID, 0, ctm);
			cnt++;
cprintf("%ld FireZone-%d: Normal\n", DS_TIMER, ID);
		}
		if(!fzGetSilenced(ID)) {
			fzSetSilenced(ID, 1, ctm);
			cnt++;
		}
		if(cnt) fzNotifyStatus(ID);
	}
	if(cnt) rval = R_SUCCESS;			
	else	rval = R_ACTION_NOT_ALLOWED;
	return rval;
}

int UnsilenceFireZone(int ID)
{
	unsigned char	ctm[8];
	int		rval;

cprintf("UnsilenceFileZone=%d...\n", ID);
	if(fzGetSilenced(ID)) {
		rtcGetDateTime(ctm);
		fzSetSilenced(ID, 0, ctm);
		fzNotifyStatus(ID);
		rval = R_SUCCESS;			
	} else rval = R_ACTION_NOT_ALLOWED;
	return rval;
}

int DoorControl(long adID, int Command, char *LoginID)
{
	ACCESS_DOOR		*ad;
	unsigned char	ctm[8];
	unsigned char	EventData[12];
	int		rval, val, PresentValue, NewPV, EventID;

//cprintf("DoorControl: ID=%d cmd=%d enable=%d\n", adID, Command, adIsEnable(adID));
	if(!adIsEnable(adID)) return R_OBJECT_NOT_FOUND;
	ad = adsGetAt(adID);
	rtcGetDateTime(ctm);
	PresentValue = adGetPresentValue(adID);
	switch(Command) {
	case 1:			// Momentary Unlock
		if(PresentValue == S_NORMAL_LOCKED || PresentValue == S_NORMAL_UNLOCKED) {
			NewPV = S_NORMAL_UNLOCKED;
			if(PresentValue == S_NORMAL_LOCKED) EventID = E_NORMAL_UNLOCKED_MOMEMTARY_UNLOCK;
			else	EventID = 0;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 2:			// Forced Lock
		if(PresentValue == S_NORMAL_LOCKED || PresentValue == S_NORMAL_UNLOCKED) {
			NewPV   = S_FORCED_LOCKED;
			EventID = E_FORCED_LOCKED;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 3:			// Forced Lock Release
		if(PresentValue == S_FORCED_LOCKED) {
			NewPV   = S_NORMAL_LOCKED;
			EventID = E_NORMAL_LOCKED_FORCED_LOCK_RELEASE;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 4:			// Forced Unlock 
		if(PresentValue == S_NORMAL_LOCKED || PresentValue == S_NORMAL_UNLOCKED) {
			NewPV   = S_FORCED_UNLOCKED;
			EventID = E_FORCED_UNLOCKED;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 5:			// Forced Unlock Release
		if(PresentValue == S_FORCED_UNLOCKED) {
			NewPV   = S_NORMAL_LOCKED;
			EventID = E_NORMAL_LOCKED_FORCED_UNLOCK_RELEASE;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 6:			// Clear
		if(PresentValue >= S_NORMAL_UNLOCKED && PresentValue <= S_SCHEDULED_UNLOCKED) {
			NewPV   = S_NORMAL_LOCKED;
			EventID =  E_NORMAL_LOCKED_CLEAR;
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 7:			// Lock Down
		if(PresentValue <= S_SCHEDULED_UNLOCKED) {
			ad->PrevPresentValue = PresentValue;
			NewPV   = S_LOCK_DOWNED;
			EventID = E_LOCK_DOWNED;
			rval = R_SUCCESS;	
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 8:			// Lock Down Release
		if(PresentValue == S_LOCK_DOWNED) {
			val = adCheckLockSchedule(adID, ctm);
			if(val > 1) {
				NewPV   = S_SCHEDULED_UNLOCKED;
				EventID = E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE;
			} else if(val == 1) {
				NewPV   = S_SCHEDULED_LOCKED;
				EventID = E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE;
			} else if(ad->PrevPresentValue == S_FORCED_LOCKED) {
				NewPV   = S_FORCED_LOCKED;
				EventID = E_FORCED_LOCKED_LOCK_DOWN_RELEASE;
			} else if(ad->PrevPresentValue == S_FORCED_UNLOCKED) {
				NewPV   = S_FORCED_UNLOCKED;
				EventID = E_FORCED_UNLOCKED_LOCK_DOWN_RELEASE;
			} else {
				NewPV   = S_NORMAL_LOCKED;
				EventID = E_NORMAL_LOCKED_LOCK_DOWN_RELEASE;
			}
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 9:			// Lock Open
		if(PresentValue <= S_SCHEDULED_UNLOCKED) {
			ad->PrevPresentValue = PresentValue;
			NewPV   = S_LOCK_OPENED;
			EventID = E_LOCK_OPENED;
			rval = R_SUCCESS;			
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	case 10:		// Lock Open Release
		if(PresentValue == S_LOCK_OPENED) {
			val = adCheckLockSchedule(adID, ctm);
			if(val > 1) {
				NewPV   = S_SCHEDULED_UNLOCKED;
				EventID = E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE;
			} else if(val == 1) {
				NewPV   = S_SCHEDULED_LOCKED;
				EventID = E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE;
			} else if(ad->PrevPresentValue == S_FORCED_LOCKED) {
				NewPV   = S_FORCED_LOCKED;
				EventID = E_FORCED_LOCKED_LOCK_OPEN_RELEASE;
			} else if(ad->PrevPresentValue == S_FORCED_UNLOCKED) {
				NewPV   = S_FORCED_UNLOCKED;
				EventID = E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE;
			} else { 
				NewPV   = S_NORMAL_LOCKED;
				EventID = E_NORMAL_LOCKED_LOCK_OPEN_RELEASE;
			}
			rval = R_SUCCESS;
		} else	rval = R_ACTION_NOT_ALLOWED;
		break;
	default:	rval = R_PARAMETER_ERROR;
	}
	if(rval == R_SUCCESS) {
		DoorSetByCommand(adID, NewPV);
		if(EventID > 0) {
			str2card(LoginID, EventData);
			EventAdd(OT_ACCESS_DOOR, adID, EventID, ctm, EventData);
		}
	}
	return rval;
}

int DoorListControl(long adMask, int Command, char *LoginID)
{
	unsigned char	msk[4];
	int		i, rval;

	msk[0] = (adMask >> 16) & 0xff; msk[1] = (adMask >> 8) & 0xff; msk[2] = adMask & 0xff;
	for(i = 0;i < MAX_AD_SZ;i++) {
		rval = BitMaskGet(msk, i+1);
		if(rval) DoorControl(i, Command, LoginID);
	}
	return 0;
}	

int DoorControlAllLockOpen(char *LoginID)
{
	ACCESS_DOOR		*ad;
	unsigned char	ctm[8];
	unsigned char	EventData[12];
	int		i, PresentValue, cnt;

	cnt = 0;
	for(i = 0;i < MAX_AD_SZ;i++) {
		if(adIsEnable(i)) {
			ad = adsGetAt(i);
			PresentValue = adGetPresentValue(i);
			if(PresentValue <= S_SCHEDULED_UNLOCKED) {
cprintf("DoorControlAllLockOpen: adID=%d PV=%d\n", i, PresentValue);
				ad->PrevPresentValue = PresentValue;
				DoorSetByCommand(i, S_LOCK_OPENED);
				cnt++;
			}
		}
	}
	if(cnt) {
		str2card(LoginID, EventData);
		rtcGetDateTime(ctm);
		EventAdd(OT_ACCESS_DOOR, -1, E_LOCK_OPENED, ctm, EventData);
		cnt = R_SUCCESS;			
	} else {
		cnt = R_ACTION_NOT_ALLOWED;
	}
	return cnt;
}

int DoorControlAllLockOpenRelease(char *LoginID)
{
	ACCESS_DOOR		*ad;
	unsigned char	ctm[8];
	unsigned char	EventData[12];
	int		i, val, PresentValue, NewPV, EventID, cnt;

	rtcGetDateTime(ctm);
	cnt = 0;
	for(i = 0;i < MAX_AD_SZ;i++) {
		if(adIsEnable(i)) {
			ad = adsGetAt(i);
			PresentValue = adGetPresentValue(i);
			if(PresentValue == S_LOCK_OPENED) {
cprintf("DoorControlAllLockOpenRelease: adID=%d\n", i);
				val = adCheckLockSchedule(i, ctm);
				if(val > 1) {
					NewPV   = S_SCHEDULED_UNLOCKED;
					EventID = E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE;
				} else if(val == 1) {
					NewPV   = S_SCHEDULED_LOCKED;
					EventID = E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE;
				} else if(ad->PrevPresentValue == S_FORCED_LOCKED) {
					NewPV   = S_FORCED_LOCKED;
					EventID = E_FORCED_LOCKED_LOCK_OPEN_RELEASE;
				} else if(ad->PrevPresentValue == S_FORCED_UNLOCKED) {
					NewPV   = S_FORCED_UNLOCKED;
					EventID = E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE;
				} else { 
					NewPV   = S_NORMAL_LOCKED;
					EventID = E_NORMAL_LOCKED_LOCK_OPEN_RELEASE;
				}
				DoorSetByCommand(i, NewPV);
				cnt++;	
			}
		}
	}
	if(cnt) {
		str2card(LoginID, EventData);
		EventAdd(OT_ACCESS_DOOR, -1, EventID, ctm, EventData);
		cnt = R_SUCCESS;
	} else {
		cnt = R_ACTION_NOT_ALLOWED;
	}
	return cnt;
}
