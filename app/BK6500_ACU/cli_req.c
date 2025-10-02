static int _EncodeWriteObject(unsigned char *buf, int ObjectType, int ObjectID)
{
	unsigned char	*p;

	p = buf;
	*p++ = P_WRITE_OBJECT;
	*p++ = (unsigned char)ObjectType;
	IDtoPACK3(ObjectID, p); p += 3;
	return p - buf;
}

static int _EncodeReadProperty(unsigned char *buf, int ObjectType, int ObjectID, int PropertyID)
{
	unsigned char	*p;

	p = buf;
	*p++ = P_READ_PROPERTY;
	*p++ = (unsigned char)ObjectType;
	IDtoPACK3(ObjectID, p); p += 3;
	if(PropertyID < 0) *p++ = 0xff;
	else	*p++ = (unsigned char)PropertyID;
	return p - buf;
}

static int _EncodeWriteProperty(unsigned char *buf, int ObjectType, int ObjectID, int PropertyID)
{
	unsigned char	*p;

	p = buf;
	*p++ = P_WRITE_PROPERTY;
	*p++ = (unsigned char)ObjectType;
	IDtoPACK3(ObjectID, p); p += 3;
	if(PropertyID < 0) *p++ = 0xff;
	else	*p++ = (unsigned char)PropertyID;
	return p - buf;
}

void CliWriteLocalDateTime(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteProperty(p, OT_DEVICE, 0, OP_LOCAL_DATE_TIME);
	rtcGetDateTime(p); p += 6;
cprintf("%lu slvnets(%d): Tx Req-LocalDateTime: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliWriteIOConfig(CREDENTIAL_READER *cr)
{
	ACCESS_DOOR_CFG		*ad_cfg;
	BINARY_INPUT_CFG	*bi_cfg;
	BINARY_OUTPUT_CFG	*bo_cfg;
	unsigned char	*p, *p0, val, msk;
	int		i, ID, objID;	

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteObject(p, 0x70, 0);
	ad_cfg = &sys_cfg->AccessDoors[ID];
	if(adIsEnable(ID)) val = 0x80; else val = 0x00;
	if(ad_cfg->RequestToExitEnable) val |= 0x40;
	if(ad_cfg->DoorStatusEnable) val |= 0x20;
	if(ad_cfg->AuxLockEnable) val |= 0x10;
	if(ad_cfg->LockType) val |= 0x08;
	*p++ = val;
	*p++ = ad_cfg->DoorPulseTime;
	*p++ = ad_cfg->DoorPulseTime;
	*p++ = ad_cfg->DoorOpenTooLongTime;
	*p++ = ad_cfg->TimeDelay;
	*p++ = ad_cfg->TimeDelayNormal;

	objID = unitGetBinaryInput(ID);
	bi_cfg = &sys_cfg->BinaryInputs[objID];
	objID = unitGetBinaryOutput(ID);
	bo_cfg = &sys_cfg->BinaryOutputs[objID];
	val = 0; msk = 0x80;
	for(i = 0;i < 4;i++, bi_cfg++, msk >>= 1)
		if(bi_cfg->Polarity) val |= msk;
	for(i = 0;i < 2;i++, bo_cfg++, msk >>= 1)
		if(bo_cfg->Polarity) val |= msk;
	*p++ = val;
	objID = unitGetBinaryInput(ID);
	bi_cfg = &sys_cfg->BinaryInputs[objID];
	for(i = 0;i < 4;i++, bi_cfg++) {
		*p++ = bi_cfg->TimeDelay;
		*p++ = bi_cfg->TimeDelayNormal;
	}
cprintf("%lu slvnets(%d): Tx Req-IOConfig: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliWriteCRConfig(CREDENTIAL_READER *cr)
{
	CR_CFG		*cr_cfg;
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteObject(p, 0x71, 0);
	cr_cfg = &sys_cfg->CredentialReaders[ID];
	p += crEncodeAll(cr_cfg, p);
cprintf("%lu slvnets(%d): Tx Req-CRConfig: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliWriteSCKey(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteObject(p, 0x72, 0);
	p += sckeyEncode(sys_cfg->SCPrimaryKey, p);
cprintf("%lu slvnets(%d): Tx Req-SCKey: size=%d\n", DS_TIMER, ID, p - p0);
scPrintKey(p-13, "PrimaryKey");
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}
	
void CliWriteTAConfig(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteObject(p, 0x73, 0);
	*p++ = sys_cfg->TAOption;
	*p++ = sys_cfg->FuncKeyTimer;
	p += taEncodeItems(sys_cfg, p);
cprintf("%lu slvnets(%d): Tx Req-TAConfig: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliWriteAdminPIN(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteProperty(p, OT_DEVICE, 0, OP_DEVICE_PASSWORD);
	string2bcd(sys_cfg->AdminPIN, 8, p); p += 4;
cprintf("%lu slvnets(%d): Tx Req-AdminPIN: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliReadProperty(CREDENTIAL_READER *cr, int PropertyID)
{
	NET_TSM		*tsm;
	NET			*rspnet;
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeReadProperty(p, OT_CREDENTIAL_READER, 0, PropertyID);
	tsm = (NET_TSM *)cr->nettsm;
	rspnet = (NET *)tsm->SvcNet;
	memcpy(p, rspnet->Data, rspnet->DataLength); p += rspnet->DataLength;
cprintf("%lu slvnets(%d): Tx Req-CliReadProperty: op=%02x size=%d\n", DS_TIMER, ID, PropertyID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliWriteProperty(CREDENTIAL_READER *cr, int PropertyID)
{
	NET_TSM		*tsm;
	NET			*rspnet;
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	p += _EncodeWriteProperty(p, OT_CREDENTIAL_READER, 0, PropertyID);
	tsm = (NET_TSM *)cr->nettsm;
	rspnet = (NET *)tsm->SvcNet;
	memcpy(p, rspnet->Data, rspnet->DataLength); p += rspnet->DataLength;
cprintf("%lu slvnets(%d): Tx Req-CliWriteProperty: op=%02x size=%d\n", DS_TIMER, ID, PropertyID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliReinitialize(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_REINITIALIZE_CR;
	*p++ = (unsigned char)ID;
cprintf("%lu slvnets(%d): Tx Req-ReinitializeCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliRunScript(CREDENTIAL_READER *cr, unsigned char *script, int len)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_RUN_SCRIPT;
	*p++ = (unsigned char)ID;
	memcpy(p, script, len); p += len;
cprintf("%lu slvnets(%d): Tx Req-RunScript: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;	
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliDownloadFirmware(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_DOWNLOAD_FIRMWARE_CR;
	*p++ = (unsigned char)ID;
cprintf("%lu slvnets(%d): Tx Req-DownloadFirmwareCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliDownloadWallPaper(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_DOWNLOAD_WALLPAPER_CR;
	*p++ = (unsigned char)ID;
cprintf("%lu slvnets(%d): Tx Req-DownloadWallPaperCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliDownloadKernel(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_DOWNLOAD_KERNEL_CR;
	*p++ = (unsigned char)ID;
cprintf("%lu slvnets(%d): Tx Req-DownloadKernelCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliDownloadVideo(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_DOWNLOAD_VIDEO_CR;
	*p++ = (unsigned char)ID;
cprintf("%lu slvnets(%d): Tx Req-DownloadVideoCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void CliDownloadPoll(CREDENTIAL_READER *cr)
{
	unsigned char	*p, *p0;
	int		ID;

	ID = cr->ID;
	p = p0 = SlaveNetGetTxReqData(ID);
	*p++ = P_DOWNLOAD_POLL_CR;
	*p++ = (unsigned char)ID;
//cprintf("%lu slvnets(%d): Tx Req-DownloadPollCR: size=%d\n", DS_TIMER, ID, p - p0);
	cr->Timer = DS_TIMER;
	SlaveNetSetTxReqDataSize(ID, p - p0, 0);
}

void ClisTAStateChanged(void)
{
	int		i;

	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) unitcrSetState(i);
		else if(crGetType(i)) unitSetCruState(i);
	}
}

void ClisLocalDateTimeChanged(void)
{
	int		i;

	gLocalDateTimeVersion++; if(!gLocalDateTimeVersion) gLocalDateTimeVersion = 1;
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) CliLocalDateTimeChanged(i);
		else	unitSetCruVersion(i);
	}
}

void ClisSCKeyChanged(void)
{
	CREDENTIAL_READER	*cr;
	int		i;

	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++)
		if(unitGetType(i) == 2) cr->Update |= SCKEY_CHANGED;
		else	unitSetCruVersion(i);
}

void ClisTAConfigChanged(void)
{
	CREDENTIAL_READER	*cr;
	int		i;

	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++)
		if(unitGetType(i) == 2) cr->Update |= TA_CONFIG_CHANGED;
		else	unitSetCruVersion(i);
}

void ClisAdminPINChanged(void)
{
	int		i;

	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) CliAdminPINChanged(i);
		//else	unitSetCruVersion(i);
	}
}

void ClisXferFileChanged(int IDMask)
{
	CREDENTIAL_READER	*cr;
	int		i;

	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++) {
		if(unitGetType(i) == 2 && crIsEnable(i) && (IDMask & 0x1)) {
			cr->XferState = 1; 	cr->XferRetry = 0;
		} else {
			cr->XferState = 0; 	cr->XferRetry = 0;				
		}
		IDMask >>= 1;
	}
cprintf("%lu ClisXferFileChanged....%d\n", DS_TIMER, CliGetXferSize());
//	CliID = -1;
	XferTimer = SEC_TIMER;
	CliSetXferState(1);
}

void ClisXferFileCanceled(void)
{
	CliSetXferState(0);
}

void CliLocalDateTimeChanged(int ID)
{
	CREDENTIAL_READER	*cr;

	cr = crsGetAt(ID);
	cr->Update |= LOCAL_DATETIME_CHANGED;
}

void CliIOConfigChanged(int ID)
{
	CREDENTIAL_READER	*cr;

	cr = crsGetAt(ID);
	cr->Update |= IO_CONFIG_CHANGED;
}

void CliCRConfigChanged(int ID)
{
	CREDENTIAL_READER	*cr;

	cr = crsGetAt(ID);
	cr->Update |= CR_CONFIG_CHANGED;
}

void CliAdminPINChanged(int ID)
{
	CREDENTIAL_READER	*cr;

	cr = crsGetAt(ID);
	cr->Update |= ADMIN_PIN_CHANGED;
}

