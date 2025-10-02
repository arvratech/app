void InitializeDoorCfg(int initflg);

int DecodeDeviceCfg(SYS_CFG *sys_cfg, void *buf)
{
	unsigned char	*p, val;
	
	p = (unsigned char *)buf;
	val = *p++;
	if(val & 0x10) sys_cfg->devOption |= CR_TAMPER;
	else	sys_cfg->devOption &= ~CR_TAMPER;
	sys_cfg->fpSecurityLevel	= *p++;
	sys_cfg->eventOption		= *p++;
	sys_cfg->language			= *p++;
	sys_cfg->fpFormat.exposure	= *p++;
	p++;				// doorConfiguration
	p++;				// otherOption
	sys_cfg->eventOutput		= *p++;
	sys_cfg->eventWiegand		= *p++;
	sys_cfg->backLightTime		= *p++;
	sys_cfg->lcdContrast		= *p++;
	sys_cfg->lcdBrightness		= *p++;
	sys_cfg->userSoundVolume	= *p++;
	p++;				// adminSoundVolume
	sys_cfg->systemSoundVolume	= *p++;
 	return p - (unsigned char *)buf;
}

int DecodeDeviceCfgExtra2(SYS_CFG *sys_cfg, void *buf)
{
	unsigned char	val, *p, temp[4];
	void	*cr, *cf, *fld;
	int		values[2];
	
	p = (unsigned char *)buf;
	p += 6;
	cf = cfCardsGet(CFCARD_SC_BLOCK);
	fld = cfCardCardFieldAtIndex(cf, 0);
	val = *p++;
	values[0] = val >> 2; values[1] = val & 0x03;
	cfCardSetBlockNo(cf, values);
	val = *p++;
	cardFieldSetOffset(fld, (int)val);
	val = *p++;
	cardFieldSetLength(fld, (int)val);
	val = *p++;
	if(val == 3) {			// BCD
		cfCardSetCoding(cf, CF_CODING_BCD);
		cfCardSetCredential(cf, CRED_BCD_DIGITS);
	} else if(val == 1) {	// Digit string
		cfCardSetCoding(cf, CF_CODING_ASCII);
		cfCardSetCredential(cf, CRED_BCD_DIGITS);
	} else {				// Binary
		cfCardSetCoding(cf, CF_CODING_BINARY);
		cfCardSetCredential(cf, CRED_INT32);
	}
	cr = devCredentialReader(NULL); 
	bcd2string(p, 8, temp); p += 4;
	crSetDuressDigits(cr, temp);
	crSetDuressAlarm(cr, val);
	val = *p++;
 	return p - (unsigned char *)buf;
}

int StoreTerminal(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	int		size, val, val2;
	
	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreTerminal: DataSize=%d\n", size);
#endif
	if(size < 18) return R_PARAMETER_ERROR;	// 2+16 => 2+31
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	p = data + 3;
	p += DecodeDeviceCfg(tmp_cfg, p);
	tmp_cfg->lcdContrast		= sys_cfg->lcdContrast;
	tmp_cfg->lcdBrightness		= sys_cfg->lcdBrightness;
	tmp_cfg->userSoundVolume	= sys_cfg->userSoundVolume;
//	tmp_cfg->AdminSoundVolume	= sys_cfg->AdminSoundVolume;
	tmp_cfg->systemSoundVolume	= sys_cfg->systemSoundVolume;
	if(size >= 33) p += DecodeDeviceCfgExtra2(tmp_cfg, p);	
//	if(!ValidateTerminal(tmp_cfg)) return R_PARAMETER_ERROR;
	if(syscfgCompare(sys_cfg, tmp_cfg) && syscfgWrite(tmp_cfg) < 0) return R_R_SYSTEM_ERROR;
	val = sys_cfg->language;	
	syscfgCopy(sys_cfg, tmp_cfg);
//	slvmWriteScInfo();
	if(val != sys_cfg->language) {
		ChangeLanguage((int)sys_cfg->language);
//		RefreshTopScreen();
	}
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreOperation(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	int		size, OldOpMode, OldAcuCommID;

	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreOperation: DataSize=%d\n", size);
#endif
	if(size < 32) return R_PARAMETER_ERROR;
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	p = data + 3;
	p += DecodeOperation(tmp_cfg, p, 0);
	size = size - (p - data);
#ifdef MEALS_NUMBER
	if(size >= 40) DecodeExtraOperation(tmp_cfg, p, 0);
#endif
	if(!ValidateOperation(tmp_cfg)) return R_PARAMETER_ERROR;
	if(syscfgCompare(sys_cfg, tmp_cfg) && syscfgWrite(tmp_cfg) < 0) return R_R_SYSTEM_ERROR;
	OldOpMode		= sys_cfg->OperationMode;
	OldAcuCommID	= sys_cfg->AcuCommID;
	syscfgCopy(sys_cfg, tmp_cfg);	
	if(OldOpMode != sys_cfg->OperationMode || OldAcuCommID != sys_cfg->AcuCommID) {
		if(OldOpMode >= 3 && sys_cfg->OperationMode <= 2) {

		}
	}
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreAccessDoor(int sock, unsigned char *data, int *DataSize)
{
	unsigned short	sval;
	unsigned char	*p;
	void	*ad, *bi, *bo, *aa;
	BOOL	bVal;
	int		id, val, size;
	
	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreAccessDoor: DataSize=%d\n", size);
#endif
	if(size < 21) return R_PARAMETER_ERROR;	// 2+19
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	val = *(data+3);
	if(val != 1) return R_OBJECT_NOT_FOUND;
	ad = adsGet(0);
	p = data + 4;
	val = *p++; adSetDoorPulseTime(ad, val / 10);
	val = *p++; adSetUnlockScheID(ad, val);
	val = *p++; adSetLockScheID(ad, val);
	id = IO2BI(adIoSet(ad));
	val = *p++;
	bi = bisGet(id-1);
	if(val) biCreate(bi, BI_TYPE_DOOR); else biDelete(bi);
	val = *p++;
	bi = bisGet(id);
	if(val) biCreate(bi, BI_TYPE_DOOR); else biDelete(bi);
	val = *p++;
	val = *p++; adSetTimeDelay(ad, val / 10); adSetTimeDelayNormal(ad, val / 10);
	BYTEtoSHORT(p, &sval); p += 2; adSetDoorOpenTooLongTime(ad, sval / 10);
	aa = aasGet(0);
	val = *p++; aaSetEnableAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG, val);
	val = *p++; //adSetDoorOpenTooLongOC(ad, val);
	val = *p++; aaSetOnTimeAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG, val);	// OnTime
	val = *p++; aaSetOffTimeAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG, val);	// OffTime
	val = *p++; aaSetEnableAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN, val);
	val = *p++; // adSetForcedOpenOC(ad, val);
	val = *p++; aaSetOnTimeAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN, val);		// OnTime
	val = *p++; aaSetOffTimeAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN, val);	// OffTime
	if(aaEnableAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG) || aaEnableAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN)) val = 1;
	else	val = 0;
	adSetEventDetectionEnable(ad, val);
//	if(!adValidate(&sys_cfg->accessDoors[adID])) return R_PARAMETER_ERROR;
	if(syscfgCompare(sys_cfg, tmp_cfg) && syscfgWrite(sys_cfg) < 0) {
		syscfgCopy(sys_cfg, tmp_cfg);
		return R_R_SYSTEM_ERROR;
	}
//	slvmWriteLocalReg();
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreAccessPoint(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	void	*cr, *aa;
	BOOL	bVal;
	int		size, apID, val, n;
	
	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreAccessPoint: DataSize=%d\n", size);
#endif
	if(size < 10) return R_PARAMETER_ERROR;	// 2+8
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	val = *(data+3);
	if(val != 1) return R_OBJECT_NOT_FOUND;
	cr = devCredentialReader(NULL); 
	p = data + 4;
	val = *p++; crSetAuthorizationMode(cr, val - 1);
	val = *p++;
	n = val & 0x03;
	if(n < 1 || n > 3) n = -1;
	else	n--;
//	crSetEnableScReader(cr, enable);
//	if(n >= 0) crSetScCredential(n); 
	n = (val >> 2) & 0x03;
	crSetScFormat(cr, n);
	n = (val >> 4) & 0x03;
	if(n) bVal = TRUE; else bVal = FALSE;
	crSetEnableEmReader(cr, bVal);
	val = *p++;
	if(val & 1) bVal = TRUE; else bVal = FALSE;
	crSetEnableWiegandReader(cr, bVal);
	aa = aasGet(0);
	*p++;	// fpReader	
	val = *p++; if(val) val = 1; else val = 0; aaSetEnableAtIndex(aa, AC_INDEX_UNREGISTERED, val);
	val = *p++; if(val) val = 1; else val = 0; aaSetEnableAtIndex(aa, AC_INDEX_EXPIRED, val);
//	if(!apValidate(&sys_cfg->accessPoints[apID])) return R_PARAMETER_ERROR;
	if(syscfgCompare(sys_cfg, tmp_cfg) && syscfgWrite(sys_cfg) < 0) {
		syscfgCopy(sys_cfg, tmp_cfg);
		return R_R_SYSTEM_ERROR;
	}
//	slvmWriteLocalReg();
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreHoliday(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	holDate[4], curDate[4];
	int		size, holTab;

	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreHoliday: DataSize=%d\n", size);
#endif
	if(size != 7) return R_PARAMETER_ERROR;
	holidayDecode(data+2, holDate, &holTab);
	rtcGetDate(curDate);
	if(!holidayValidate(holDate, holTab, curDate)) return R_PARAMETER_ERROR;
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgSetHoliday(tmpsche_cfg, holDate, holTab);
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreTimeZone(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	tmzTab[16];
	int		size, tmzID, section;
	
	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreTimeZone: DataSize=%d\n", size);
#endif
	if(size != 20) return R_PARAMETER_ERROR;
	tmzDecode(data+2, &tmzID, tmzTab);
	if(!tmzValidateData(tmzID, tmzTab)) return R_PARAMETER_ERROR;
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgSetTimeZone(tmpsche_cfg, tmzID, tmzTab); 
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
	*DataSize = 0;
	return R_SUCCESS;
}

int StoreSchedule(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	scheTab[16];
	int		size, scheID;

	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreSchedule: DataSize=%d\n", size);
#endif
	if(size != 15) return R_PARAMETER_ERROR;
	scheDecode(data+2, &scheID, scheTab);
	if(!scheValidateData(scheID, scheTab)) return R_PARAMETER_ERROR;
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgSetSchedule(tmpsche_cfg, scheID, scheTab); 
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
	*DataSize = 0;
	return R_SUCCESS;
}

#ifdef BIO_FP

int StoreUser(int sock, unsigned char *data, int *DataSize)
{
	int		rval, cnt, size;

	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreUser: DataSize=%d\n", size);
#endif
	sUser = &_sUser;
	if(size < 34) return R_PARAMETER_ERROR; 
	sUser->id = userDecodeID(data+3);
	userDecode(sUser, data+2);
	if(!userValidate(sUser)) return R_PARAMETER_ERROR;
	rval = fsAddEncodedUser(data+2, cnt);
	if(rval < 0) rval = R_R_SYSTEM_ERROR;
	else if(rval == 0) rval = R_OBJECT_IS_FULL; 
	else {
		if(rval & 0x0f) UpdateTrpls();
		*DataSize = 0;
		rval = R_SUCCESS;
	}
	return rval;
}

#else

int StoreUser(int sock, unsigned char *data, int *DataSize)
{
	unsigned long	FPID, nFPID;
	unsigned char	*p;
	int		rval, size, cnt;

	size = *DataSize;
#ifdef BK_DEBUG
	printf("StoreUser: DataSize=%d...\n", size);
#endif
	sUser = &_sUser;
	if(size < 34) return R_PARAMETER_ERROR; 
	p = data + 2; size -= 2;
	sUser->id = userDecodeID(p+1);
	userDecode(sUser, p);
	if(!userValidateID(sUser->id) || !userValidate(sUser)) return R_PARAMETER_ERROR;
	p += 32; size -= 32;
	p += 3;	size -= 3;		// fill for byte alignment
	rval = fsAddEncodedUser(data+2);
	if(rval < 0) rval = R_R_SYSTEM_ERROR;
	else if(rval == 0) rval = R_OBJECT_IS_FULL; 
	else {	
		*DataSize = 0; rval = R_SUCCESS;
	}
#ifdef BK_DEBUG
	printf("StoreUser: Result=0x%02x\n", rval);
#endif
	return rval;
}

#endif

int StoreUserEx(int sock, unsigned char *data, int *DataSize)
{
	int		rval, size, photo;

	size = *DataSize;
#ifdef BK_DEBUG
printf("StoreUserEx: DataSize=%d\n", size);
#endif
	sUser = &_sUser;
	if(size != 34 && size != 2077) return R_PARAMETER_ERROR;
	if(size > 34) photo = 1; else	photo = 0;
	sUser->jpegBuffer = GetXferBuffer(0);
	sUser->id = userDecodeID(data+3);
	userDecodeEx(sUser, photo, data+2);	
//printf("[%u] [%s] [%s]\n", sUser->id, sUser->userName, sUser->userExternalID);
//printf("Photo QL=%d W=%d H=%d\n", (int)sUser->quantityLevel, (int)sUser->width, (int)sUser->height);
	if(!userValidateID(sUser->id) || !userValidateEx(sUser) && photo && !userValidatePhoto(sUser)) return R_PARAMETER_ERROR;
	rval = fsGetUser(sUser, 0);	
	if(rval < 0) rval = R_R_SYSTEM_ERROR;
	else if(rval == 0) rval = R_OBJECT_NOT_FOUND;
	else {
		rval = fsAddUserEx(sUser);
		if(rval < 0) rval = R_R_SYSTEM_ERROR;
		else if(rval == 0) rval = R_OBJECT_IS_FULL; 
		else {
			*DataSize = 0;
			rval = R_SUCCESS;
		}
	}
	return rval;
}

int EncodeDeviceCfg(SYS_CFG *cfg, void *buf)
{
	unsigned char	val, *p, data[8];
	int		values[2];
	void	*cr, *cf, *fld;

	p = (unsigned char *)buf;
	if(sys_cfg->devOption & CR_TAMPER) val = 0x10;
	else	val = 0x00;
	*p++ = val;
	*p++ = cfg->fpSecurityLevel;
	*p++ = cfg->eventOption;
	*p++ = cfg->language;
	*p++ = cfg->fpFormat.exposure;
	*p++ = 1;		// DoorConfiguration
	*p++ = cfg->otherOption;	
	*p++ = cfg->eventOutput;
	*p++ = cfg->eventWiegand;
	*p++ = cfg->backLightTime;
	*p++ = cfg->lcdContrast;
	*p++ = cfg->lcdBrightness;
	*p++ = cfg->userSoundVolume;
	*p++ = cfg->systemSoundVolume;
	*p++ = cfg->systemSoundVolume;

	*p++ = 3;			// networkType
	*p++ = 2;			// serialSpeeed
	*p++ = sys_cfg->networkOption;

	*p++ = 0;			// sys_cfg->ZoneSetting;
	*p++ = 0;			// sys_cfg->EntryPoint;
	*p++ = 0;			// sys_cfg->PassbackMode[0];
	*p++ = 0;			// sys_cfg->PassbackMode[1];
	*p++ = 0; *p++ = 0;	// sys_cfg->PassbackTimeout
	cf = cfCardsGet(CFCARD_SC_BLOCK);
	fld = cfCardCardFieldAtIndex(cf, 0);
	cfCardGetBlockNo(cf, values);
	*p++ = (values[0] << 2) + values[1];
	*p++ = cardFieldOffset(fld);
	*p++ = cardFieldLength(fld);
	val = cfCardCoding(cf);
	if(val == CF_CODING_BCD && cfCardCredential(cf) == CRED_BCD_DIGITS) val = 3;	// BCD
	else if(val == CF_CODING_ASCII && cfCardCredential(cf) == CRED_BCD_DIGITS) val = 1;	// Digit string
	else	val = 2;			// Binary
	*p++ = val;
	cr = devCredentialReader(NULL); 
	string2bcd(crDuressDigits(cr), 8, p); p += 4;
	*p++ = crDuressAlarm(cr);
	//if(sys_cfg->TCPIPOption & USE_DHCP) sockGetIfConfig(p, NULL, NULL);
	//else	memcpy(p, sys_cfg->IPAddress, 4);
	memcpy(p, sys_cfg->ipAddress, 4);
	p += 4;
	int2bcd(5000, 4, p); p += 2;
	rtcGetDateTime(data);
	bin2bcd(data, 6, p); p += 6;
 	return p - (unsigned char *)buf;
}

int GetTerminal(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	
printf("GetTerminal...%d\n", *DataSize);
	if(*DataSize != 3) return R_PARAMETER_ERROR;
	data[0] = data[2];
	p = data + 1;
	p += EncodeDeviceCfg(sys_cfg, p);
	p += EncodeDevStatus(p);
	*DataSize = p - data;
//#ifdef BK_DEBUG
	printf("GetTerminal: DataSize=%d\n", *DataSize);
//#endif
	return R_SUCCESS;
}

int GetOperation(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	
	if(*DataSize != 3) return R_PARAMETER_ERROR;
	data[0] = data[2];
	p = data + 1;
	p += EncodeOperation(sys_cfg, p, 0);
#ifdef MEALS_NUMBER
	p += EncodeExtraOperation(sys_cfg, p);
#endif
	*DataSize = p - data;
#ifdef BK_DEBUG
	printf("\nGetOperation: DataSize=%d\n", *DataSize);
#endif
	return R_SUCCESS;
}

int GetAccessDoor(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	unsigned short	sval;
	void	*ad, *bi, *bo, *aa;
	int		id, val, val2;
	
	if(*DataSize != 4) return R_PARAMETER_ERROR;
	val = data[3];
	ad = adsGet(0);
printf("GetAccessDoor...%d ID=%d\n", *DataSize, val-1);
	if(val != 1 || !ad || !adIsNonNull(ad)) return R_OBJECT_NOT_FOUND;
	data[0] = data[2];
	data[1] = val;
	p = data + 2;
	*p++ = adDoorPulseTime(ad) * 10;
	*p++ = adUnlockScheID(ad);
	*p++ = adLockScheID(ad);
	id = IO2BI(adIoSet(ad));
	bi = bisGet(id+1);
	if(biIsDoorType(bi)) val = 1; else val = 0;
	*p++ = val;
	*p++ = val2;
	*p++ = 0;
	*p++ = adTimeDelay(ad) * 10;
	sval = adDoorOpenTooLongTime(ad) * 10; SHORTtoBYTE(sval, p); p += 2;
	aa = aasGet(0);
	*p++ = aaEnableAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG);
	*p++  = 0;	// OpenTooLong Alarm Output
	*p++ = aaOnTimeAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG);
	*p++ = aaOffTimeAtIndex(aa, AC_INDEX_DOOR_OPEN_TOO_LONG);
	*p++ = aaEnableAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN);
	*p++  = 0;	// ForcedOpen Alarm Output
	*p++ = aaOnTimeAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN);
	*p++ = aaOffTimeAtIndex(aa, AC_INDEX_DOOR_FORCED_OPEN);
//bo = adRequestDoorLock(ad);
//bo++;
//if(boIsDoorType(bo)) val = 1; else val = 0;
	p += adEncodeStatus(ad, p);
	*DataSize = p - data;
//#ifdef BK_DEBUG
	printf("GetAccessDoor(ID=%d): DataSize=%d\n", adId(ad), *DataSize);
//#endif
	return R_SUCCESS;
}

int GetAccessPoint(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	void	*cr, *ac;
	int		val, n;

	if(*DataSize != 4) return R_PARAMETER_ERROR;
	val = data[3];
	if(val != 1) return R_OBJECT_NOT_FOUND;
	data[0] = data[2];
	data[1] = val;
	cr = devCredentialReader(NULL); 
	p = data + 2;
	val = crAuthorizationMode(cr) + 1; *p++ = val;
	if(crIsEnableScReader(cr)) n = crScFormat(cr) + 1; else n = 0;
	val = n;
//	n = crScFormat();
	val |= n << 2;
	if(crIsEnableEmReader(cr)) n = 1; else n = 0;
	val |= n << 4;
	*p++ = val;
	if(crIsEnableWiegandReader(cr)) val = 0x01; else val = 0;
	*p++ = val;
	*p++ = 0;	// fpReader	
	ac = aasGet(0);
	*p++ = aaEnableAtIndex(ac, AC_INDEX_UNREGISTERED);
	*p++ = aaEnableAtIndex(ac, AC_INDEX_EXPIRED);
	p += crEncodeStatus(cr, p);
	*DataSize = p - data;
#ifdef BK_DEBUG
	printf("GetAccessPoint(ID=%d): DataSize=%d\n", id, *DataSize);
#endif
	return R_SUCCESS;
}

int GetHoliday(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	holDate[4], curDate[4];
	int		holTab;

	if(*DataSize != 6) return R_PARAMETER_ERROR;
	bcd2bin(data+3, 3, holDate);
	rtcGetDate(curDate);
	if(!holidayValidate(holDate, 0x0f, curDate)) return R_PARAMETER_ERROR;
	schecfgGetHoliday(sche_cfg, holDate, &holTab);
	*DataSize = holidayEncode(data, holDate, holTab);
#ifdef BK_DEBUG
	printf("GetHolidays: DataSize=%d\n", *DataSize);
#endif
	return R_SUCCESS;
}

int GetTimeZone(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	tmzTab[16];
	int		tmzID;
	
	if(*DataSize != 4) return R_PARAMETER_ERROR;
	tmzID = data[3];
	if(!tmzValidateData(tmzID, NULL)) return R_PARAMETER_ERROR;
	schecfgGetTimeZone(sche_cfg, tmzID, tmzTab);
	if(!tmzValidateData(tmzID, tmzTab)) return R_OBJECT_NOT_FOUND;
	*DataSize = tmzEncode(data, tmzID, tmzTab);
#ifdef BK_DEBUG
	printf("GetTimeZones: DataSize=%d\n", *DataSize);
#endif
	return R_SUCCESS;
}

int GetSchedule(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	scheTab[16];
	int		scheID;
	
	if(*DataSize != 4) return R_PARAMETER_ERROR;
	scheID = data[3];
	if(!scheValidateData(scheID, NULL) < 0) return R_PARAMETER_ERROR;
	schecfgGetSchedule(sche_cfg, scheID, scheTab);
	if(!scheValidateData(scheID, scheTab)) return R_OBJECT_NOT_FOUND;
	*DataSize = scheEncode(data, scheID, scheTab);
#ifdef BK_DEBUG
	printf("GetSchedules: DataSize=%d\n", *DataSize);
#endif
	return R_SUCCESS;
}

int GetUser(int sock, unsigned char *data, int *DataSize)
{
	unsigned long	FPID;
	unsigned char	*p, dUserID[12];
	long	userId;
	int		rval, c;
	
	if(*DataSize != MAX_USERID_HALF_SZ+3) return R_PARAMETER_ERROR;
	userId = userDecodeID(data+3);
	if(!userValidateID(userId)) return R_PARAMETER_ERROR;
	c = data[2];
	p = data;
	userEncodeId(userId, dUserID);
	rval = fsGetEncodedUser(dUserID, p);
	if(rval > 0) {
		*p = c; p += 32;
	}
	if(rval < 0) rval = R_R_SYSTEM_ERROR;
	else if(rval == 0) rval = R_OBJECT_NOT_FOUND;
	else {
		*DataSize = p - data;
		rval = R_SUCCESS;
	}
#ifdef BK_DEBUG
	printf("GetUser: Result=0x%02x DataSize=%d\n", rval, *DataSize);
#endif
	return rval;
}

int GetUserEx(int sock, unsigned char *data, int *DataSize)
{
	int		rval, c, photo;

	if(*DataSize != MAX_USERID_HALF_SZ+3) return R_PARAMETER_ERROR;
	sUser = &_sUser;
	sUser->id = userDecodeID(data+3);
	if(!userValidateID(sUser->id)) return R_PARAMETER_ERROR;
	c = data[2];
	sUser->jpegBuffer = GetXferBuffer(0);
	rval = fsGetUserEx(sUser);
	if(rval < 0) return R_R_SYSTEM_ERROR;
	else if(rval == 0) return R_OBJECT_NOT_FOUND;
	else {
		if(sUser->width && sUser->height) photo = 1; else photo = 0;		
		userEncodeEx(sUser, photo, data);
		*data = c;
		if(photo) *DataSize = 2075;
		else	*DataSize = 32;
#ifdef BK_DEBUG
	printf("GetUserEx: DataSize=%d\n", *DataSize);
#endif
		return R_SUCCESS;
	}
}

int GetExTerminal(int sock, unsigned char *data, int *DataSize)
{
	void		*cf;
	unsigned char	*p, temp[24], temp2[16];
	int		i, result, AttrType;
		
	data[0] = data[2];
	data[1] = data[3];
	result = R_SUCCESS;
	AttrType = data[3];
	p = data + 2;
	switch(AttrType) {
	case 0x46:
		cf = cfCardsGet(CFCARD_SC_BLOCK);
		memcpy(p+34, cfCardMifareKey(cf), 6);
		i = scEncodeKey(p+34, p); p += i;
		break;
	case 0x55:
		sprintf((char *)temp, "%s_ver%d.%d.%d", devModelName(NULL), syscfgFirmwareVersionName(NULL));
		memcpy_pad(p, temp, 20); p += 20;
		break;
#ifndef _ACU	
	case 0x56:
		*p++ = sys_cfg->language;
		break;
#endif
#if !defined(BK4300) && !defined(_ACU)
	case 0x70:
//		if(sys_cfg->networkType != NETWORK_SERIAL) result = R_PARAMETER_ERROR;
//		else	p += taEncodeTexts(sys_cfg, p);
		break;
#endif
	default:	result = R_PARAMETER_ERROR;
	}
	if(result == R_SUCCESS) *DataSize = p - data;
	return result;
}

int _SaveDynamicIpAddress(int newVal, int InMenu)
{
	unsigned char	ipAddr[4];
	int		rval, oldVal;

	oldVal = sys_cfg->networkOption;
	if(oldVal != newVal) {
		memcpy(ipAddr, sys_cfg->ipAddress, 4);
		sys_cfg->networkOption = newVal;
		if(newVal & 0x01) memset(sys_cfg->ipAddress, 0, 4);
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) {
			rval = 1;
		} else {
			sys_cfg->networkOption = oldVal;
			memcpy(sys_cfg->ipAddress, ipAddr, 4);
			rval = -1;
		}
	} else	rval = 0;
	return rval;
}

static int _SaveConfig(unsigned char *pVal, int NewVal)
{
	int		rval, OldVal;
		
	OldVal = *pVal;
	if(OldVal != NewVal) {
		*pVal = (unsigned char)NewVal;
		rval = syscfgWrite(sys_cfg);
		if(rval == 0) rval = R_SUCCESS;
		else {
			*pVal = (unsigned char)OldVal;
			rval = R_R_SYSTEM_ERROR;;
		}
	} else	rval = R_SUCCESS;
	return rval;
}

int SetTerminal(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p, *p1, buf[64];
	int		i, rval, size, oldVal, newVal, result;
	
	size = *DataSize - 4;
	p = data + 4;
	switch(data[3]) {
	case 0x40:	// Terminal ID
		if(size != 2) result = R_PARAMETER_ERROR;
		else {
			oldVal = devId(NULL);
			newVal = bcd2int(p, 4);
			if(newVal < 1 || newVal > 999) result = R_PARAMETER_ERROR;
			else {
				if(oldVal != newVal) {
					devSetId(NULL, newVal);
					rval = syscfgWrite(sys_cfg);
					if(rval == 0) {
						slvmWriteDevice();
						result = R_SUCCESS;
					} else {
						devSetId(NULL, oldVal);
						result = R_R_SYSTEM_ERROR;
					}
				} else	result = R_SUCCESS;
			}
		}
		break;
#ifndef _ACU
	case 0x41:	// Network Type
		/*
		if(size != 1) result = R_PARAMETER_ERROR;
		else {
			NewVal = *p;
			if(NewVal < 1 || NewVal > 3) result = R_PARAMETER_ERROR;
			else	result = _SaveConfig(&sys_cfg->NetworkType, NewVal, 1);
		}
		*/
		result = 0;
		break;
#endif
	case 0x43:	// IP Address
		if(size != 4) result = R_PARAMETER_ERROR;
		else {
			memcpy(buf, sys_cfg->ipAddress, 4);
			memcpy(buf+8, p, 4);
			if(!validateIpAddress(buf+8)) result = R_PARAMETER_ERROR;
			else {
				if(memcmp(buf, buf+8, 4)) {
					memcpy(sys_cfg->ipAddress, buf+8, 4);
					rval = syscfgWrite(sys_cfg);
					if(rval == 0) {
						gRequest |= G_TCP_IP_CHANGED;
						result = R_SUCCESS;
					} else {
						memcpy(sys_cfg->ipAddress, buf, 4);
						result = R_R_SYSTEM_ERROR;
					}
				} else	result = R_SUCCESS;
			}
		}
		break;
	case 0x44:	// Date & Time
		if(size != 6) result = R_PARAMETER_ERROR;
		else {
			bcd2bin(p, 6, buf);
			if(!datetime_validate(buf)) result = R_PARAMETER_ERROR;
			else {
				rtcSetDateTime(buf);
				gRequest |= G_DATE_TIME_UPDATED;
				result = R_SUCCESS;
			}
		}
		break;
/*
	case 0x46:	// SmartCard Key: CurrentPrimaryKey(C) + NewPrimaryKey(P) + NewSecondaryKey(S) + KeyLegth(L) + KeyOption(O)
				// XXXXCCCCCCXXXXPPPPPPXXXXSSSSSSXLXO
//scPrintKey(sys_cfg->SCPrimaryKey, "Old PrimaryKey");
//scPrintKey(sys_cfg->SCSecondaryKey, "Old SecondaryKey");
		if(size != 34) result = R_PARAMETER_ERROR;
		else {
			i = scDecodeKey(p, buf);
			if(i != 6) result = R_PARAMETER_ERROR;
			else {
//scPrintKey(buf, "Current PrimaryKey");
//scPrintKey(buf+6, "New PrimaryKey");
//scPrintKey(buf+12, "New SecondaryKey");
				p1 = sys_cfg->smartCardInfo.mifareKey;
				if(scValidateKey(p1) && memcmp(p1, buf, 6)) result = R_ACTION_NOT_ALLOWED;
				else {
					scGetKey(sys_cfg, p);
					scSetValidateKey(sys_cfg, buf+6);
					if(scCompareKey(sys_cfg, p)) {
						rval = syscfgWrite(sys_cfg, 1);
						if(rval == 0) {
							//gRequest |= G_CARD_CLOSE_REQUESTED | G_CARD_OPEN_REQUESTED;
							slvmWriteScInfo();
							result = R_SUCCESS;
						} else {
							scSetKey(sys_cfg, p);
							result = R_R_SYSTEM_ERROR;
						}
					} else	result = R_SUCCESS;
				}
			}
		}
//scPrintKey(sys_cfg->smartCardInfo.mifareKey, "PrimaryKey");
		break;
*/
	case 0x47:	// DynamicIPAddress
		if(size != 1 || (*p) > 1) result = R_PARAMETER_ERROR;
		else {
			newVal = sys_cfg->networkOption;
			if(*p) newVal |= 0x01; else newVal &= 0xfe; 
			rval = _SaveDynamicIpAddress(newVal, 0);
			if(rval >= 0) result = R_SUCCESS; else result = R_R_SYSTEM_ERROR;
		}
		break;
	default:	result = R_PARAMETER_ERROR;
	}
	*DataSize = 0;
	return result;
}

int DeleteHoliday(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p, holDate[4], curDate[4];
	
	p = data + 3;
	if(*p == '*') {
		holDate[0] = holDate[1] = holDate[2] = holDate[3] = 0;
		rtcGetDate(curDate);
	} else {
		holDate[0] = memtoi(p, 2); p += 2;
		holDate[1] = memtoi(p, 2); p += 2;
		holDate[2] = memtoi(p, 2); p += 2;
		if(sche_cfg->holidays[0].year != holDate[0] && sche_cfg->holidays[1].year != holDate[0] || !date_validate(holDate)) return R_PARAMETER_ERROR;
	}
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgClearHoliday(tmpsche_cfg, holDate, curDate);
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
	*DataSize = 0;
	return R_SUCCESS;
}

int DeleteTimeZone(int sock, unsigned char *data, int *DataSize)
{
	char	temp[12];
	int		tmzID;
	
	if(data[3] == '*') tmzID = 0;
	else {
		memcpy_chop(temp, data+3, 8);
		tmzID = n_atou(temp);
		if(!tmzValidateData(tmzID, NULL)) return R_PARAMETER_ERROR;
	}
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgClearTimeZone(tmpsche_cfg, tmzID);
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
 	*DataSize = 0;
	return R_SUCCESS;
}

int DeleteSchedule(int sock, unsigned char *data, int *DataSize)
{
	char	temp[12];
	int		scheID;
	
	if(data[3] == '*') scheID = 0;
	else {
		memcpy_chop(temp, data+3, 8);
		scheID = n_atou(temp);
		if(!scheValidateData(scheID, NULL)) return R_PARAMETER_ERROR;
	}
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgClearSchedule(tmpsche_cfg, scheID);
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
 	*DataSize = 0;
	return R_SUCCESS;
}

int DeleteUser(int sock, unsigned char *data, int *DataSize)
{
	char	idstr[12];
	long	id;
	int		rval, i, c, wild, cnt;
	
	sUser = &_sUser;
	memcpy_chop(idstr, data+3, MAX_USERID_SZ);
	for(wild = cnt = i = 0; ;i++) {
		c = idstr[i];
		if(!c) break;
		if(c == '*' && !wild) wild = 1;
		else if(c >= '0' && c <= '9') cnt++;
		else	return R_PARAMETER_ERROR;
	}
	id = n_atol(idstr);
	if(!wild && (cnt == 0 || !userValidateID(id))) return R_PARAMETER_ERROR;
	if(!wild && cnt > 0) {
		rval = fsRemoveUser(sUser);
	} else if(wild && cnt > 0) {
		//rval = fsFindRemoveUser(sUser->id, GetXferBuffer(0));
		rval = 0;
	} else {
		rval = fsRemoveAllUser();
	}
	if(rval < 0) return R_R_SYSTEM_ERROR;
	else {
#ifdef BIO_FP
		UpdateTrpls();
#endif
		*DataSize = 0;
		return R_SUCCESS;
	}
}

int DeleteUserEx(int sock, unsigned char *data, int *DataSize)
{
	char	idstr[12];
	int		rval, i, c, wild, cnt;
	
	sUser = &_sUser;
	memcpy_chop(idstr, data+3, MAX_USERID_SZ);
	for(wild = cnt = i = 0; ;i++) {
		c = idstr[i];
		if(!c) break;
		if(c == '*' && !wild) wild = 1;
		else if(c >= '0' && c <= '9') cnt++;
		else	return R_PARAMETER_ERROR;
	}
	if(!wild) return R_PARAMETER_ERROR;
	rval = fsRemoveAllUserEx();
	if(rval < 0) return R_R_SYSTEM_ERROR;
	else {
		*DataSize = 0;
		return R_SUCCESS;
	}
}

