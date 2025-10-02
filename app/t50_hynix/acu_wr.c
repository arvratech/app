int nwpEncode2(void *buf)
{
	unsigned char	*p, ip[4], sn[4], gw[4];
	int		rval;

	p = (unsigned char *)buf;
/*
	if(syscfgNetworkType(NULL)) {
		*p++ = 1;		// NetworkType
		*p++ = 0;		// SerialPort;
		*p++ = 3;		// SerialSpeed;
		memset(p, 0, 19); p += 19;
	} else {
*/
		*p++ = 3;		// NetworkType
		*p++ = 0;		// SerialPort;
		*p++ = 0;		// SerialSpeed;
		if(syscfgUseDhcp(NULL)) {
			*p++ = 0x01;	// Option
			rval = niGetIpAddress(ip, sn, NULL);
			if(rval < 0) {
				ipSetNull(ip); ipSetNull(sn);
			}
			niGetDefaultGateway(gw);
			memcpy(p, ip, 4); p += 4;
			memcpy(p, sn, 4); p += 4;
			memcpy(p, gw, 4); p += 4;
		} else {
			*p++ = 0x00;	// Option
			memcpy(p, syscfgIpAddress(NULL), 4); p += 4;
			memcpy(p, syscfgSubnetMask(NULL), 4); p += 4;
			memcpy(p, syscfgGatewayIpAddress(NULL), 4); p += 4;
		}
		memcpy(p, niMacAddress(), 6); p += 6;
//	}
	return p - (unsigned char *)buf;
}

int devEncode2(void *buf)
{
	unsigned char	*p, val;
	char	temp[8];

	p = (unsigned char *)buf;
	memcpy_pad(p, syscfgFirmwareVersionName(NULL), 16); p += 16;
	memcpy_pad(p, devMakerName(NULL), 16); p += 16;		
	memcpy_pad(p, devModelName(NULL),  8); p += 8;
	*p++ = sys_cfg->language;
	val = sys_cfg->devOption;
	if(sys_cfg->dateNotation & CR_TAMPER) {
		val &= ~CR_TAMPER; val |= 0x01;
	} else {
		val &= 0xfe;
	}
	*p++ = val;
	if(sys_cfg->dateNotation & DISPLAY_DATETIME) val = 1;
	else	val = 0;
	*p++ = val;		// DateNotation
	if(sys_cfg->dateNotation & TIME_NOTATION) val = 1;
	else	val = 0;
	*p++ = val;		// TimeNotation;
	*p++ = 0;		// EventOption;
	*p++ = 0;		// EventOutput;
	*p++ = 0;		// EventWiegand;
	*p++ = 0;		// BackLightTime;
	*p++ = sys_cfg->lcdContrast;
	*p++ = sys_cfg->lcdBrightness;
	*p++ = sys_cfg->userSoundVolume;
	*p++ = sys_cfg->systemSoundVolume;
	*p++ = sys_cfg->systemSoundVolume;
    getKernelLocalVersion(temp);
    *p++ = temp[0] - '0'; *p++ = n_atoi(temp+2);
    memset(p, 0xff, 2); p += 2;
//printf("enc: contrast=%d bright=%d volume=%d %d\n", (int)cfg->lcdContrast, (int)cfg->lcdBrightness, (int)cfg->userSoundVolume, (int)sys_cfg->systemSoundVolume);
    *p++ = sys_cfg->sleepMode;
    SHORTtoBYTE(sys_cfg->sleepWaitTime, p); p += 2;
    *p++ = sys_cfg->mediaOption;
    *p++ = sys_cfg->systemSoundVolume;
    *p++ = sys_cfg->mediaSoundVolume;
    *p++ = sys_cfg->callSoundVolume;
	return p - (unsigned char *)buf;
}

int devDecode2(SYS_CFG *cfg, void *buf)
{
	unsigned char	*p, val;
	
	p = (unsigned char *)buf;
	p += 40;
	cfg->language			= *p++;
	val = *p++;
	if(val & 0x01) {
		val |= CR_TAMPER; val &= 0xfe;
	} else {
		val &= ~CR_TAMPER;
	}
	cfg->devOption = val;
	val = *p++;		// DateNotation
	if(val) cfg->dateNotation |= DISPLAY_DATETIME;
	else	cfg->dateNotation &= ~DISPLAY_DATETIME;
	val = *p++;		// TimeNotation
	if(val) cfg->dateNotation |= TIME_NOTATION;
	else	cfg->dateNotation &= ~TIME_NOTATION;
	p++;
	p++;
	p++;
	p++;
	cfg->lcdContrast		= *p++;
	cfg->lcdBrightness		= *p++;
	cfg->userSoundVolume	= *p++;
	cfg->systemSoundVolume	= *p++;
	cfg->systemSoundVolume	= *p++;
	p += 4;
    cfg->sleepMode			= *p++;
    BYTEtoSHORT(p, &cfg->sleepWaitTime); p += 2; if(!cfg->sleepWaitTime) cfg->sleepWaitTime = 600;
    cfg->mediaOption        = *p++;
    cfg->systemSoundVolume  = *p++;
    cfg->mediaSoundVolume   = *p++;
    cfg->callSoundVolume    = *p++;
//printf("sleep: %d %d\n", (int)cfg->sleepMode, (int)cfg->sleepWaitTime);
//printf("volume: %d %d %d\n", (int)cfg->systemSoundVolume, (int)cfg->mediaSoundVolume, (int)cfg->callSoundVolume);
	return p - (unsigned char *)buf;
}

void acuWriteObject(unsigned char *rxData, int dataSize)
{
	unsigned char	*s, *d;
	long	nID;
	int		result, nSize, ot, rval, val;

	d = SlvnetConfRspBuf();
	if(dataSize < 4) {
		d[0] = R_PARAMETER_ERROR;
		SlvnetSetConfRspTxSize(1, 0);
		return;
	}
	tmp_syscfg = &_tmp_syscfg;
	syscfgCopy(tmp_syscfg, sys_cfg);
	tmp_tacfg = &_tmp_tacfg;
	tacfgCopy(tmp_tacfg, ta_cfg);
	PACK3toID(rxData+1, &nID);
	s = rxData + 4;
	nSize = dataSize - 4;
printf("Rx Req-WriteObject: ot=%02x id=%d data=%d\n", (int)rxData[0], nID, nSize);
	result = 0;
	ot = rxData[0];
	switch(ot) {
	case 0x70:
		s += acuAdDecode(s); s += acuBioDecode(s);
		_PostDoorChanged();
		break;
	case 0x71:
		if(tamperIsEnable(NULL)) rval = 1; else rval = 0;
		if(intrusionIsEnable(NULL)) rval |= 0x02;
		acuCrDecodeAll(s);
		if(tamperIsEnable(NULL)) val = 1; else val = 0;
		if(intrusionIsEnable(NULL)) val |= 0x02;
		rval ^= val;
		if(rval & 0x01) tamperClearPresentValue(NULL);
		if(rval & 0x02) intrusionDisarm(NULL);
		break;
	case 0x72:
		acuCrDecodeScKey(s);
		break;
	case 0x73:
		acuTaDecode(s);
		break;
	default:   result = R_PARAMETER_ERROR;
	}
printf("Tx Rsp-%d\n", result);
	if(!result && ot == 0x73) {
		if(tacfgCompare(ta_cfg, tmp_tacfg)) {
			rval = tacfgWrite(ta_cfg);
			if(rval < 0) {
				result = R_R_SYSTEM_ERROR;
				tacfgCopy(ta_cfg, tmp_tacfg);
			} else {
//		DesktopTAConfigChanged();
			}
		}
	} else if(!result) {
		if(syscfgCompare(sys_cfg, tmp_syscfg)) {	
			rval = syscfgWrite(sys_cfg);
			if(rval < 0) {
				result = R_R_SYSTEM_ERROR;
				syscfgCopy(sys_cfg, tmp_syscfg);
			} else {
				//slvmUpdateDevStatus();
				slvmWriteLocalIoConfig();
				usleep(500);
				slvmWriteReaderConfig();
				//usleep(500);
				//slvmWriteCardFormats();
//				if(ot == 0x70) DesktopDrawDoor();
			}
		}
	}
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
}

void acuWriteProperty(unsigned char *rxData, int dataSize)
{
	unsigned char	*s, *d;
	long	nID;
	int		result, nSize, rval, len, section;

	d = SlvnetConfRspBuf();
	if(dataSize < 4) {
		d[0] = R_PARAMETER_ERROR;
		SlvnetSetConfRspTxSize(1, 0);
		return;
	}
	tmp_syscfg = &_tmp_syscfg;
	syscfgCopy(tmp_syscfg, sys_cfg);
	PACK3toID(rxData+1, &nID);
	s = rxData + 5;
	nSize = dataSize - 5;
printf("Rx Req-WriteProperty: ot=%02x id=%d op=%02x data=%d\n", (int)rxData[0], nID, (int)rxData[4], nSize);
	result = 0;
	switch(rxData[0]) {
	case OT_DEVICE:
		switch(rxData[4]) {
		case OP_LOCAL_DATE_TIME:
			if(nSize < 6) result = R_PARAMETER_ERROR;
			else {
				if(!datetime_validate(s)) result = R_PARAMETER_ERROR;
				else {
printf("LOCAL_DATE_TIME...\n");
					rtcSetDateTime(s);
					//gRequest |= G_DATE_TIME_UPDATED;
				}
			}
			break;
		case OP_DEVICE_PASSWORD:
			if(nSize < 4) result = R_PARAMETER_ERROR;
			else {
				bcd2string(s, 8, tmp_syscfg->adminPassword);
			}
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;
	case OT_CREDENTIAL_READER:
		switch(rxData[4]) {
		case OP_INTRUSION_PRESENT_VALUE:
			if(nSize < 1) result = R_PARAMETER_ERROR;
			else	intrusionDisarm(NULL);
			break;
		case OP_CAMERA:
			len = camDecode2(&tmp_syscfg->camera, s);
			break;
		case OP_BLE_CONF:
			len = bleDecode(&tmp_syscfg->ble, s);
			break;
		case OP_NETWORK_PORT:
			break;
		case OP_DEVICE:
			devDecode2(tmp_syscfg, s);
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;	
	default:	result = R_PARAMETER_ERROR;
	}
	if(!result && syscfgCompare(sys_cfg, tmp_syscfg)) {	
		rval = syscfgWrite(tmp_syscfg);
		if(rval < 0) result = R_R_SYSTEM_ERROR;
		else {
			syscfgCopy(sys_cfg, tmp_syscfg);
			blemWrite();
		}
	}
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
}

void acuReadProperty(unsigned char *rxData, int dataSize)
{
	unsigned char	*s, *d;
	long	nID;
	int		result, nSize, rval, len;

	d = SlvnetConfRspBuf();
	if(dataSize < 4) {
		d[0] = R_PARAMETER_ERROR;
		SlvnetSetConfRspTxSize(1, 0);
		return;
	}
	PACK3toID(rxData+1, &nID);
	s = rxData + 5;
	nSize = dataSize - 5;
printf("Rx Req-ReadProperty: ot=%02x id=%d op=%02x data=%d\n", (int)rxData[0], nID, (int)rxData[4], nSize);
	result = 0;
	len = 0;
	switch(rxData[0]) {
	case OT_CREDENTIAL_READER:
		switch(rxData[4]) {
		case OP_CAMERA:
			len = camEncode2(&sys_cfg->camera, d+1);
			//blemWrite();
			break;
		case OP_BLE_CONF:
			len = bleEncode(&sys_cfg->ble, d+1);
			break;
		case OP_NETWORK_PORT:
			len = nwpEncode2(d+1);
			break;
		case OP_DEVICE:
			len = devEncode2(d+1);
			break;
		default:	result = R_PARAMETER_ERROR;
		}
		break;	
	default:	result = R_PARAMETER_ERROR;
	}
	d[0] = result;
	if(result) len = 1; else len++;	
	SlvnetSetConfRspTxSize(len, 0);
}

void _AdmSysReinitializeDevice(void);

void acuReinitializeCR(unsigned char *rxData, int dataSize)
{
	unsigned char	*s, *d;
	long	nID;
	int		result;

printf("ReinitlizeCR...\n");
	d = SlvnetConfRspBuf();
	if(dataSize < 1) {
		d[0] = R_PARAMETER_ERROR;
		SlvnetSetConfRspTxSize(1, 0);
		return;
	}
	nID = rxData[0];
	result = 0;
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	_AdmSysReinitializeDevice();
}

static void _AcuDownloading1(int status)
{
	if(status == 2) _AdmSysReinitializeDevice();
}
		
static void _AcuDownloading2(int status)
{
	if(status == 2) UpdateWallpaper();
}

static void _AcuDownloading3(int status)
{
	unsigned char	buf[12];

	if(status == 2) {
		buf[0] = GM_FILE_CHANGED; memset(buf+1, 0, 9);
		appPostMessage(buf);
	}
}

void *MainLoop(void);

void acuDownloadFirmwareCR(unsigned char *rxData, int dataSize)
{
	char	local[80], remote[80];
	unsigned char	*d;
	unsigned long   size;
	int		rval, result;

printf("DwonloadFirmwareCR...\n");
	if(dataSize < 1) result = R_PARAMETER_ERROR;
	else	result = 0;
	d = SlvnetConfRspBuf();
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	if(!result) {
		sprintf(local, "%s.out", devFirmwareName(NULL));
		strcpy(remote, devFirmwareName(NULL));
        fileSize(remote, &size);
		rval = tftpcGet(local, remote, NULL, size, _AcuDownloading1);
		if(rval < 0) result = R_R_SYSTEM_ERROR;
	}
}

void acuDownloadWallPaperCR(unsigned char *rxData, int dataSize)
{
	char	local[80], remote[80], target[80];
	unsigned char	*d;
	unsigned long   size;
	int		rval, result;
printf("DownloadWallPaperCR...\n");

	if(dataSize < 1) result = R_PARAMETER_ERROR;
	else	result = 0;
	d = SlvnetConfRspBuf();
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	if(!result) {
		sprintf(remote, "wallpaper/w%03d%02d.bmp", SlvnetMasterDevId(), devId(NULL));
		getTmpRandFileName(local);
		sprintf(target, "wallpaper/%s", _bgscreenName);
        fileSize(target, &size);
		rval = tftpcGet(local, remote, target, size, _AcuDownloading2);
		if(rval < 0) result = R_R_SYSTEM_ERROR;
	}
}

void acuDownloadKernelCR(unsigned char *rxData, int dataSize)
{
	char	local[80], remote[80], target[80];
	unsigned char	*d;
	unsigned long   size;
	int		rval, result;

printf("DownloadKernelCR...\n");
	if(dataSize < 1) result = R_PARAMETER_ERROR;
	else	result = 0;
	d = SlvnetConfRspBuf();
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	if(!result) {
		strcpy(remote, _kernelName);
		getTmpRandFileName(local);
		strcpy(target, "/boot/uImage");
        fileSize(target, &size);
		rval = tftpcGet(local, remote, target, size, _AcuDownloading1);
		if(rval < 0) result = R_R_SYSTEM_ERROR;
	}
}

void acuDownloadVideoCR(unsigned char *rxData, int dataSize)
{
	char	local[80], remote[80];
	unsigned char	*d;
	unsigned long   size;
	int		rval, result;

printf("DwonloadVideoCR...\n");
	if(dataSize < 1) result = R_PARAMETER_ERROR;
	else	result = 0;
	d = SlvnetConfRspBuf();
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	if(!result) {
		sprintf(local, "video/%s_", _bgvideoName);
		sprintf(remote, "video/%s", _bgvideoName);
        fileSize(remote, &size);
		rval = tftpcGet(local, remote, NULL, size, _AcuDownloading3);
		if(rval < 0) result = R_R_SYSTEM_ERROR;
	}
}

void acuDownloadPollCR(unsigned char *rxData, int dataSize)
{
	unsigned char	*d;
	unsigned long	len;
	int		size;

	d = SlvnetConfRspBuf();
	if(dataSize < 1) {
		d[0] = R_PARAMETER_ERROR;
		size = 1;
	} else {
		d[0] = 0;
		d[1] = tftpcStatus();
		if(d[1] == 1 && d[1] == 2) {
			len = tftpcDownSize();
		} else	len = 0;
		LONGtoBYTE(len, d+2);
		size = 6;
	}
	SlvnetSetConfRspTxSize(size, 0);
}

void RunScriptFileDo(uv_work_t *work)
{
printf("RunScriptFile...\n");
	runScriptFile((char *)work->data);
}

void RunScriptFileDone(uv_work_t *work, int status)
{
}

uv_work_t *GetMainWork(void);
char	scriptFile[8];

void acuRunScriptFile(unsigned char *rxData, int dataSize)
{
	uv_work_t	*work;
	FILE	*fp;
	unsigned char	*d, *s;
	long	nID;
	int		c, result, count;

	d = SlvnetConfRspBuf();
	if(dataSize < 1) {
		d[0] = R_PARAMETER_ERROR;
		SlvnetSetConfRspTxSize(1, 0);
		return;
	}
	nID = rxData[0];
	count = dataSize - 1;
	if(count < 1) result = R_PARAMETER_ERROR;
	else	result = 0;
	d[0] = result;
	SlvnetSetConfRspTxSize(1, 0);
	if(result) return;
	strcpy(scriptFile, "temp.sh");
	fp = fopen(scriptFile, "w");
	if(!fp) return;		
	s = rxData + 1; d = rxData;
	while(count--) {
		c = *s++;
		if(c != '\r') *d++ = c;
	}
	count = d - rxData;
	fwrite(rxData, 1, count, fp);
	fclose(fp);
	d[0] = 0;
	work = GetMainWork();
	work->data = scriptFile;
	uv_queue_work(MainLoop(), work, RunScriptFileDo, RunScriptFileDone);
}

void acuReadEvent(unsigned char *rxData, int dataSize)
{
	unsigned char	*d;
	int		rval, result;

	d = SlvnetConfRspBuf();
	result = rxData[0];
	/*
	if(sys_cfg->slvnet) {
		rval = 0;
	} else {	
		if(!Result) EventClear();
		*d++ = 0; 
		rval = EventRead(d, 512);
printf("Result=%d EventSize=%d\n", Result, rval);
	}
	*/
	rval = 0;	// 2018.8.27
	SlvnetSetConfRspTxSize(rval+1, 0);
}

