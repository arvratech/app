
int ExportFile(int sock, unsigned char *data, int *DataSize)
{
/*
	FS_FILE	*f;
	char	temp[20];
	int		rval, size, file;

#ifdef _USER_ID_EX
	memcpy_chop(temp, data+15, 16);
#else
	memcpy_chop(temp, data+11, 16);
#endif
	file = (int)n_atol(temp);
	if(file < 64 || file > 255) return R_PARAMETER_ERROR;
	size = GetXferSize(0);
	flashWriteEnable();
	rval = fsDelete(file);
	f = fsCreate(file);
	if(!f) {
#ifdef BK_DEBUG
		printf("fsCreate(%d) error: %d\n", file, fsErrno);
#endif
		flashWriteDisable();
		return R_R_SYSTEM_ERROR;
	}
	rval = fsWrite(f, GetXferBuffer(0), size);
	if(rval != size) {
#ifdef BK_DEBUG
		printf("fsWrite(%d) error: %d\n", file, fsErrno);
#endif
		fsClose(f);
		fsDelete(file);
		flashWriteDisable();
		return R_R_SYSTEM_ERROR;
	}
	fsClose(f);
	flashWriteDisable();
	LONGtoBYTE(size, data);	
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ExportFile[%d]: %d\n", file, size);
#endif
	if(file == 150) RefreshTopScreen();
*/
	return R_SUCCESS;
}

int ExportHolidays(int sock, unsigned char *data, int *DataSize)
{
	HOLIDAY			*h;
	unsigned char	*buf, *p, holDate[4];
	char	FileName[20];
	int		i, size, holTab, ydays, mm, dd;
	
#ifdef _USER_ID_EX
	memcpy_chop(FileName, data+15, 16);
#else
	memcpy_chop(FileName, data+11, 16);
#endif
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	p = buf = GetXferBuffer(0);
	for(i = 0, h = sche_cfg->holidays;i < 2;i++, h++) {
		holDate[0] = h->year;
		ydays = get_year_days((int)holDate[0], 12, 31);
		for(i = 0;i < ydays;i++) {
			get_mmdd_from_ydays((int)holDate[0], i+1, &mm, &dd);
			holDate[1] = mm; holDate[2] = dd;
			schecfgGetHoliday(sche_cfg, holDate, &holTab);
			if(holTab) p += holidayEncode(p, holDate, holTab);
		}
	}
	size = p - buf;
	SetXferSize(0, size);
	LONGtoBYTE(size, data);
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ExportHolidays: %d\n", size);
#endif
	return R_SUCCESS;
}

int ExportTimeZones(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf, *p, tmzTab[16];
	char	FileName[20];
	int		i, size;

#ifdef _USER_ID_EX
	memcpy_chop(FileName, data+15, 16);
#else
	memcpy_chop(FileName, data+11, 16);
#endif
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	p = buf = GetXferBuffer(0);
  	for(i = 0;i < 32;i++) {
		schecfgGetTimeZone(sche_cfg, i+1, tmzTab);
	 	if(tmzValidateData(i+1, tmzTab)) p += tmzEncode(p, i+1, tmzTab);
	}
	size = p - buf;
	SetXferSize(0, size);
	LONGtoBYTE(size, data);
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ExportTimeZones: %d\n", size);
#endif
	return R_SUCCESS;
}

int ExportSchedules(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf, *p, scheTab[12];
	char	FileName[20];
	int		i, size;

#ifdef _USER_ID_EX
	memcpy_chop(FileName, data+15, 16);
#else
	memcpy_chop(FileName, data+11, 16);
#endif
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	p = buf = GetXferBuffer(0);
 	for(i = 0;i < 32;i++) {
		schecfgGetSchedule(sche_cfg, i+1, scheTab);
		if(scheValidateData(i+1, scheTab)) p += scheEncode(p, i+1, scheTab);
	}
	size = p - buf;
	SetXferSize(0, size);
	LONGtoBYTE(size, data);
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ExportSchedules: %d\n", size);
#endif
	return R_SUCCESS;
}

int ExportUsers(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	unsigned long	position;
	char	UserKey[16], FileName[20];
	int		size;

	memcpy_chop(FileName, data+11, 16);
	size = *DataSize;	
	if(strlen(FileName) != 1 || FileName[0] != '0' || size < 31) return R_PARAMETER_ERROR;
	BYTEtoLONG(data+27, &position); 
	memcpy_chop(UserKey, data+3, MAX_USERID_SZ);	
	size = GetXferBufferSize(0);
	buf = GetXferBuffer(0);
	fsSeekUser(position);
	size = fsReadBulkUser(buf, size);
	if(size < 0) {
		return R_R_SYSTEM_ERROR;	
	}
	SetXferSize(0, size);
	LONGtoBYTE(size, data);
	if(fsEofUser()) position = 0L;
	else	position = fsTellUser();
	LONGtoBYTE(position, data+4);
	*DataSize = 8;
#ifdef BK_DEBUG
	printf("ExportUsers: Size=%d Position=%d\n", size, position);
#endif
	return R_SUCCESS;
}

int ExportFPTemplates(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	unsigned long	position;
	char	UserKey[16], FileName[20];
	int		size;

	memcpy_chop(FileName, data+11, 16);
	size = *DataSize;
	if(strlen(FileName) != 1 || FileName[0] != '0' || size < 31) return R_PARAMETER_ERROR;
	BYTEtoLONG(data+27, (unsigned long *)&position); 
	memcpy_chop(UserKey, data+3, MAX_USERID_SZ);	
	memset(data, 0, 8);
	*DataSize = 8;
#ifdef BK_DEBUG
	printf("ExportFPTemplates: Size=0 Position=0\n");
#endif
	return R_SUCCESS;
}

int ExportUserExs(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	unsigned long	position;
	char	UserKey[16], FileName[20];
	int		size;

	memcpy_chop(FileName, data+11, 16);
	size = *DataSize;	
	if(strlen(FileName) != 1 || FileName[0] != '0' || size < 31) return R_PARAMETER_ERROR;
	BYTEtoLONG(data+27, &position); 
	memcpy_chop(UserKey, data+3, MAX_USERID_SZ);	
	size = GetXferBufferSize(0);
	buf = GetXferBuffer(0);
	fsSeekUserEx(position);
	size = fsReadBulkUserEx(buf, size);
	if(size < 0) {
		return R_R_SYSTEM_ERROR;	
	}
	SetXferSize(0, size);
	LONGtoBYTE(size, data);
	if(fsEofUserEx()) position = 0L;
	else	position = fsTellUserEx();
	LONGtoBYTE(position, data+4);
	*DataSize = 8;
#ifdef BK_DEBUG
	printf("ExportUserExs: Size=%d Position=%d\n", size, position);
#endif
	return R_SUCCESS;
}

int ExportUserPhotos(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	unsigned long	position;
	char	UserKey[16], FileName[20];
	int		size;

	memcpy_chop(FileName, data+11, 16);
	size = *DataSize;	
	if(strlen(FileName) != 1 || FileName[0] != '0' || size < 31) return R_PARAMETER_ERROR;
	BYTEtoLONG(data+27, &position); 
	memcpy_chop(UserKey, data+3, MAX_USERID_SZ);	
	memset(data, 0, 8);
	*DataSize = 8;
#ifdef BK_DEBUG
	printf("ExportUserPhotos: Size=0 Position=0\n");
#endif
	return R_SUCCESS;
}

int ImportFile(int sock, unsigned char *data, int *DataSize)
{
/*
	FS_FILE	*f;
	char	temp[20];
	int		rval, file;

	memcpy_chop(temp, data+3, 16);
	file = (int)n_atol(temp);
	if(file < 64 || file > 255) return R_PARAMETER_ERROR;
	f = fsOpen(file, FS_READ);
	if(!f) return R_R_SYSTEM_ERROR;
	rval = fsRead(f, GetXferBuffer(0), GetXferSize(0));
	SetXferSize(0, rval);
	LONGtoBYTE(rval, data);
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ImportFile(%d): %d\n", file, rval);
#endif
*/
	return R_SUCCESS;
}

int ImportHolidays(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf, *p, holDate[4], curDate[4];
	char	FileName[20];
	int		size, count, holTab;
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	rtcGetDate(curDate);
	schecfgClearHoliday(tmpsche_cfg, NULL, curDate);
	p = buf = GetXferBuffer(0); size = GetXferSize(0);
#ifdef BK_DEBUG
	printf("ImportHoliday: Size=%d...\n", size);
#endif
	while(size > 0) {
		if(*p != data[2]) return R_OBJECT_NOT_FOUND;
		count = holidayDecode(p, holDate, &holTab);
		if(!holidayValidate(holDate, holTab, curDate)) return R_OBJECT_NOT_FOUND;
		schecfgSetHoliday(tmpsche_cfg, holDate, holTab);
		size -= count; p += count;
	}
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
//syscfgPrint(sys_cfg, section);
	size = GetXferSize(0);
	LONGtoBYTE(size, data);	
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportTimeZones(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf, *p, tmzTab[16];
	char	FileName[20];
	int		size, count, tmzID;
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgClearTimeZone(tmpsche_cfg, 0);
	p = buf = GetXferBuffer(0); size = GetXferSize(0);
#ifdef BK_DEBUG
	printf("ImportTimeZone: Size=%d...\n", size);
#endif
	while(size > 0) {
		if(*p != data[2]) return R_OBJECT_NOT_FOUND;
		count = tmzDecode(p, &tmzID, tmzTab);
		if(!tmzValidateData(tmzID, tmzTab)) return R_OBJECT_NOT_FOUND;
		schecfgSetTimeZone(tmpsche_cfg, tmzID, tmzTab);
		size -= count; p += count;
	}
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
//syscfgPrint(sys_cfg, section);
	size = GetXferSize(0);
	LONGtoBYTE(size, data);	
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportSchedules(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf, *p, scheTab[12];
	char	FileName[20];
	int		size, count, scheID;
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	tmpsche_cfg = &_tmpsche_cfg;
	memcpy(tmpsche_cfg, sche_cfg, sizeof(SCHE_CFG));
	schecfgClearSchedule(tmpsche_cfg, 0);
	p = buf = GetXferBuffer(0); size = GetXferSize(0);
#ifdef BK_DEBUG
	printf("ImportSchedule: Size=%d...\n", size);
#endif
	while(size > 0) {
		if(*p != data[2]) return R_OBJECT_NOT_FOUND;
		count = scheDecode(p, &scheID, scheTab);
		if(!scheValidateData(scheID, scheTab)) return R_OBJECT_NOT_FOUND;
		schecfgSetSchedule(tmpsche_cfg, scheID, scheTab);
		size -= count; p += count;
	}
	if(schecfgCompare(sche_cfg, tmpsche_cfg) && schecfgWrite(tmpsche_cfg) < 0) return R_R_SYSTEM_ERROR;
	schecfgCopy(sche_cfg, tmpsche_cfg);
//syscfgPrint(sys_cfg, section);
	size = GetXferSize(0);
	LONGtoBYTE(size, data);	
	*DataSize = 4;
	return R_SUCCESS;
}

#ifdef _ACU
extern unsigned char gAcuState, gFuncState;
#endif

int ImportUsers(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	char	FileName[20];
	int		rval, size, count;
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	buf = GetXferBuffer(0);  size = GetXferSize(0);
#ifdef BK_DEBUG
	printf("ImportUsers: Size=%d...\n", size);
#endif
#ifdef _ACU
	if(gFuncState < 0x30 && gAcuState != 7) return R_ACTION_NOT_ALLOWED;	
#endif
	count = fsGetUserCount() + size / USER_RECORD_SZ;
	if(count >= GetMaxUserSize()) return R_OBJECT_IS_FULL;
	rval = fsAddBulkUser(buf, size);
	if(rval < 0) return R_R_SYSTEM_ERROR;
	LONGtoBYTE(size, data);	
	*DataSize = 4;
#ifdef BIO_FP
	UpdateTrpls();
#endif
	return R_SUCCESS;
}

int ImportFPTemplates(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	char	FileName[20];

	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
#ifdef BK_DEBUG
	printf("ImportFPTemplates: Size=%d\n", size);		// 1023492/804=1273
#endif
	memset(data, 0, 4);
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportUserExs(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	char	FileName[20];
	int		rval, size, count;
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	buf = GetXferBuffer(0);  size = GetXferSize(0);
#ifdef BK_DEBUG
	printf("ImportUserExs: Size=%d...\n", size);
#endif
#ifdef _ACU
	if(gFuncState < 0x30 && gAcuState != 7) return R_ACTION_NOT_ALLOWED;	
#endif
	count = fsGetUserExCount() + size / USER_RECORD_SZ;
	if(count >= GetMaxUserSize()) return R_OBJECT_IS_FULL;
	rval = fsAddBulkUserEx(buf, size);
	if(rval < 0) return R_R_SYSTEM_ERROR;
	LONGtoBYTE(size, data);	
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportUserPhotos(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	char	FileName[20];
	
	memcpy_chop(FileName, data+3, 16);
	if(strlen(FileName) != 1 || FileName[0] != '0') return R_PARAMETER_ERROR; 
	memset(data, 0, 4);
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportFPImage(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	long	userId;
	int		i, rval, size;
 
	buf = GetXferBuffer(0); size = GetXferSize(0); 
//printf("UserPhoto: %d [%02x", size, (int)buf[0]); for(i = 1;i < 6;i++) printf("-%02x", (int)buf[i]); printf("]\n");
	if(size < 1000 || buf[0] != 'u' || buf[1] != 'p') return R_PARAMETER_ERROR;
	userId = userDecodeID(buf+2);
	rval = fsPutUserPhoto(userId, buf+8, size-8);
#ifdef BK_DEBUG
	printf("ImportFPImge(UserPhoto): %d\n", size);
#endif
	LONGtoBYTE(size, data);
	*DataSize = 4;
	return R_SUCCESS;
}

int ImportFirmware(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*buf;
	int		rval, size;

	buf = GetXferBuffer(0); size = GetXferSize(0);
	if(size <= 0) return R_PARAMETER_ERROR;
	rval = WriteProgram(buf, size >> 2);
	if(rval < 1) { 
		//taskDelay(3);
		rval = WriteProgram(buf, size >> 2);
	}
	if(rval < 1) return R_R_SYSTEM_ERROR;
	LONGtoBYTE(size, data);	
	*DataSize = 4;
#ifdef BK_DEBUG
	printf("ImportFrirmware: %d\n", size);
#endif
	return R_SUCCESS;
}	

int GetTerminalStatus(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p;
	
	p = data;
	p += EncodeDevStatus(p);
	*DataSize = p - data;
#ifdef BK_DEBUG
	printf("GetTerminalSatatus: %d\n", *DataSize);
#endif
	return R_SUCCESS;
}

int GetAccessDoorStatus(int sock, unsigned char *data, int *DataSize)
{
	void	*ad;
	unsigned char	*p;
	
	p = data;
	ad = adsGet(0);
	p += adEncodeStatus(ad, p);
	ad = adsGet(0);
	if(ad) p += adEncodeStatus(ad, p);
	else   *p++ = 0x00;
	*DataSize = p - data;
#ifdef BK_DEBUG
	printf("GetAccessDoorSatatus: %d\n", *DataSize);
#endif
	return R_SUCCESS;
}

#ifdef _ACU

int AccessControl(int sock, unsigned char *data, int *DataSize)
{
	ACCESS_DOOR		*ad;
	unsigned char	ctm[8];
	int		rval, PresentValue, EventID, ID;
	
	ID = data[1];
ID = 1;
//	if(ID < 1 || ID > 2 || (doorcfg != 3 && ID > 1)) return R_PARAMETER_ERROR; 
	ad = &_gAccessDoors[ID-1];
	rtcGetDateTime(ctm);
	PresentValue = adPresentValue(ad);
	switch(data[0]) {
	case 1:			// Momentary Unlock
		DoorSetByCommand(ad, S_NORMAL_UNLOCKED);
		EventID = E_NORMAL_UNLOCKED_MOMEMTARY_UNLOCK;
		rval = R_SUCCESS;
		break;
	default:	rval = R_PARAMETER_ERROR;
	}
	//if(rval == R_SUCCESS && EventID > 0) EventAdd(EventID, ctm, NULL);
	*DataSize = 0;
	return rval;
}

#else

int AccessControl(int sock, unsigned char *data, int *DataSize)
{
	void	*ad;
	unsigned char	ctm[8];
	int		rval, val, pv, priority, evtID;
	
//	ID = data[1];
	ad = adsGet(0);
	//doorcfg = GetDoorCfg();
	//if(ID < 1 || ID > 2 || (doorcfg != 3 && ID > 1)) return R_PARAMETER_ERROR; 
	rtcGetDateTime(ctm);
	pv = adPresentValue(ad); priority = adPriority(ad);
	evtID = 0;
	switch(data[0]) {
	case 1:			// Momentary Unlock
		rval = AdSetPv(ad, PV_PULSE_UNLOCK, 8);
		if(rval) evtID = E_NORMAL_UNLOCKED_MOMEMTARY_UNLOCK;
		break;
	case 2:			// Forced Lock
		if(priority >= 6) {
			rval = AdSetPv(ad, PV_LOCK, 6);
			evtID = E_FORCED_LOCKED;
		}
		break;
	case 3:			// Forced Lock Release
		if(pv == PV_LOCK && priority == 6) {
			//rval = AdSetPv(ad, PV_NULL, priority);
			rval = AdNullifyPv(ad, priority);
			evtID = E_NORMAL_LOCKED_FORCED_LOCK_RELEASE;
		}
		break;
	case 4:			// Forced Unlock 
		if(priority >= 6) {
			rval = AdSetPv(ad, PV_UNLOCK, 6);
			evtID = E_FORCED_UNLOCKED;
		}
		break;
	case 5:			// Forced Unlock Release
		if(pv == PV_UNLOCK && priority == 6) {
			//rval = AdSetPv(ad, PV_NULL, priority);
			rval = AdNullifyPv(ad, priority);
			evtID = E_NORMAL_LOCKED_FORCED_UNLOCK_RELEASE;
		}
		break;
	case 6:			// Clear
		if(priority >= 6) {
			//rval = AdSetPv(ad, PV_NULL, priority);
			rval = AdNullifyPv(ad, priority);
			evtID =  E_NORMAL_LOCKED_CLEAR;
		}
		break;
	case 7:			// Lock Down
		if(priority >= 5) {
			rval = AdSetPv(ad, PV_LOCK, 5);
			evtID = E_LOCK_DOWNED;
		}
		break;
	case 8:			// Lock Down Release
		if(pv == PV_LOCK && priority <= 4) {
			//rval = AdSetPv(ad, PV_NULL, 4);
			rval = AdNullifyPv(ad, 4);
			pv = adPresentValue(ad); priority = adPriority(ad);
			if(pv == PV_UNLOCK && priority == 5) {
				evtID = E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE;
			} else if(pv == PV_LOCK && priority == 5) {
				evtID = E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE;
			} else if(pv == PV_LOCK && priority == 6) {
				evtID = E_FORCED_LOCKED_LOCK_DOWN_RELEASE;
			} else if(pv == PV_UNLOCK && priority == 6) {
				evtID = E_FORCED_UNLOCKED_LOCK_DOWN_RELEASE;
			} else {
				evtID = E_NORMAL_LOCKED_LOCK_DOWN_RELEASE;
			}
		}
		break;
	case 9:			// Lock Open
		if(priority >= 5) {
			rval = AdSetPv(ad, PV_UNLOCK, 4);
			evtID = E_LOCK_OPENED;
		}
		break;
	case 10:		// Lock Open Release
		if(pv == PV_UNLOCK && priority <= 4) {
			//rval = AdSetPv(ad, PV_NULL, 4);
			rval = AdNullifyPv(ad, 4);
			pv = adPresentValue(ad); priority = adPriority(ad);
			if(pv == PV_UNLOCK && priority == 5) {
				evtID = E_SCHEDULED_UNLOCKED_LOCK_OPEN_RELEASE;
			} else if(pv == PV_LOCK && priority == 5) {
				evtID = E_SCHEDULED_LOCKED_LOCK_OPEN_RELEASE;
			} else if(pv == PV_LOCK && priority == 5) {
				evtID = E_FORCED_LOCKED_LOCK_OPEN_RELEASE;
			} else if(pv == PV_UNLOCK && priority == 6) {
				evtID = E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE;
			} else {
				evtID = E_NORMAL_LOCKED_LOCK_OPEN_RELEASE;
			}
		}
		break;
	default:	rval = 999;
	}
	if(evtID == 999) {
		rval = R_PARAMETER_ERROR;
	} else if(evtID > 0) {
		EventAdd(evtID, ctm, NULL);
		rval = 0;
	} else {
		rval = R_ACTION_NOT_ALLOWED;
	}
	*DataSize = 0;
	return rval;
}

int SetLockOpen(void)
{
	void	*ad;
	unsigned char	ctm[8];
	int		pv, priority;
	
	ad = adsGet(0);
	pv = adPresentValue(ad); priority = adPriority(ad);
	if(priority <= 4) return -1;
	rtcGetDateTime(ctm);
	AdSetPv(ad, PV_UNLOCK, 4);
	EventAdd(E_LOCK_OPENED, ctm, NULL);
	return 0;
}

int ReleaseLockOpen(void)
{
	void	*ad;
	unsigned char	ctm[8];
	int		val, pv, priority, evtID;

	ad = adsGet(0);
	pv = adPresentValue(ad); priority = adPriority(ad);
	if(pv != PV_UNLOCK || priority > 4)  return -1;
	rtcGetDateTime(ctm);
	pv = adPresentValue(ad); priority = adPriority(ad);
	if(pv == PV_UNLOCK && priority == 5) {
		evtID = E_SCHEDULED_UNLOCKED_LOCK_OPEN_RELEASE;
	} else if(pv == PV_LOCK && priority == 5) {
		evtID = E_SCHEDULED_LOCKED_LOCK_OPEN_RELEASE;
	} else if(pv == PV_LOCK && priority == 6) {
		evtID = E_FORCED_LOCKED_LOCK_OPEN_RELEASE;
	} else if(pv == PV_UNLOCK && priority == 6) {
		evtID = E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE;
	} else {
		evtID = E_NORMAL_LOCKED_LOCK_OPEN_RELEASE;
	}
//	AdSetPv(ad, PV_NULL, 4);
	rval = AdNullifyPv(ad, 4);
	EventAdd(evtID, ctm, NULL);
	return 0;
}

#endif

int OutputControl(int sock, unsigned char *data, int *DataSize)
{
	int		val, ID;

	ID = data[1];
	if(ID < 1 || ID > 3) return R_PARAMETER_ERROR; 
	if(data[0]) val = 1; else val = 0;
//	modCommandOutput(ID, val);
	*DataSize = 0;
	return 0;	
}

#include "gactivity.h"
#include "gapp.h"
#include "appact.h"

extern int	AuthResult, AuthMentID, AuthMealsCount;
extern char	AuthMsg[100];

#ifdef _SVR_AUTH2	// 간소 버젼

int AuthenticationResult(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p, msg[12];
	char	*s, *d, temp[52];
	int		c;
	
printf("AuthenticationResult: DataSize=%d\n", *DataSize);	
	if(*DataSize < 50) return R_PARAMETER_ERROR;
	p = data;
	c = *p++;
	if(c < 2) AuthResult = c;
	AuthMentID = *p++;
	memcpy_chop(AuthMsg, p, 48);
	*DataSize = 0;
	msg[0] = GM_CONF_CNF; msg[0] = 0; memset(msg+2, 0, 8);
	appPostMessage(msg);
	return 0;
}

#else

int AuthenticationResult(int sock, unsigned char *data, int *DataSize)
{
	unsigned char	*p, msg[12];
	char	*s, *d;
	int		c, len;
	
printf("AuthenticationResult: DataSize=%d\n", *DataSize);	
	len = *DataSize;
	if(len < 6 || len >= 106) return R_PARAMETER_ERROR;
	p = data;
	c = *p++;
	if(c > 1) c = 1; AuthResult = c;
	AuthMentID = *p++;
	BYTEtoLONG(p, (unsigned long *)&AuthMealsCount); p += 4; 
	memcpy_chop(AuthMsg, p, len-6);
	*DataSize = 0;
printf("AuthResult=%d postMessage: %d\n", AuthResult, GM_CONF_CNF);	
	msg[0] = GM_CONF_CNF; msg[1] = AuthResult; memset(msg+2, 0, 8);
	appPostMessage(msg);
	return 0;
}

#endif
