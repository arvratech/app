extern int		gRequest;

static unsigned char  RspData[2], Changed;


void unitSetDefault(void)
{
	Changed = 0;
}

void unitSetChanged(void)
{
	Changed = 1;
}

void unitGetReq(unsigned char *Req)
{
	BIO_CFG		*bio_cfg;	
	ACCESS_DOOR	*ad;
	unsigned char	val, c, PV;

	bio_cfg = &sys_cfg->BinaryInOut;
	ad = ads;
	if(ad->cfg->DoorEnable) {
		val = ad->PV << 6; 	// DoorPV
		c = ad->State;
		if(c > 3) c -= 3;	// 1:OpenTooLong 2:ForcedOpen
		else	c = 0;
		val |= c << 4;
	} else	val = 0;
	PV = biPV[0] ^ bio_cfg->biPolarity[0];
	if(PV) val |= 0x08;
	if((!ad->cfg->DoorEnable || !ad->cfg->RequestToExitEnable) && (PV && !biTimeout[0] || !PV && biTimeout[0])) val |= 0x04;
	PV = biPV[1] ^ bio_cfg->biPolarity[1];
	if(PV) val |= 0x02;
	if((!ad->cfg->DoorEnable || !ad->cfg->DoorStatusEnable) && (PV && !biTimeout[1] || !PV && biTimeout[1])) val |= 0x01;
	Req[0] = val;
	val = 0;
	PV = biPV[2] ^ bio_cfg->biPolarity[2];	
	if(PV) val |= 0x80;
	if(!(sys_cfg->CredentialReaders[0].IntrusionMode) && (PV && !biTimeout[2] || !PV && biTimeout[2])) val |= 0x40;
	PV = biPV[3] ^ bio_cfg->biPolarity[3];
	if(PV) val |= 0x20;
	if(PV && !biTimeout[3] || !PV && biTimeout[3]) val |= 0x10;
	if(crGetTamperPresentValue(0)) val |= 0x08;
	if(crGetTamperAlarmState(0)) val |= 0x04;
	if(crGetIntrusionPresentValue(0)) val |= 0x02;
	if(crGetIntrusionAlarmState(0)) val |= 0x01;
	Req[1] = val;
	Req[2] = (unsigned char)Changed;
	Changed = 0;
}

void unitProcessData(unsigned char *Req, int State)
{
	BIO_CFG		*bio_cfg;	
	ACCESS_DOOR		*ad;
	unsigned char	PV, Pri, val0, val1;

	bio_cfg = &sys_cfg->BinaryInOut;
	ad = ads;
	val0 = RspData[0]; val1 = RspData[1];
	RspData[0] = Req[0]; RspData[1] = Req[1];
	if(State < 3) val0 = val1 = 0xff;
	else {
		val0 ^= RspData[0]; val1 ^= RspData[1];
	}
	if(val0) {
		if(val0 & 0x03) DesktopDrawDoor();
	}
	if(val1) {
		if((val1 & 0xf8) && ad->cfg->DoorEnable) {
			PV = (RspData[1] >> 6) & 0x03; Pri = (RspData[1] >> 3) & 0x07;
			if(PV < 2 || State > 2) {
				if(PV >= 2) Pri = 0;
				adSetPV(PV, Pri);
				Changed = 1;
	  		}
	  	}
		if((val1 & 0x04) && !ad->cfg->DoorEnable) {
	  		if(RspData[1] & 0x04) {
				if(bio_cfg->boPolarity[0]) ioOffLock(0);
				else	ioOnLock(0);
			} else {
				if(bio_cfg->boPolarity[0]) ioOnLock(0);
				else	ioOffLock(0);
			}
	  	}
		if((val1 & 0x02) && (!ad->cfg->DoorEnable || !ad->cfg->AuxLockEnable)) {
			if(RspData[1] & 0x02) {
				if(bio_cfg->boPolarity[1]) ioOffRelay(0);
				else	ioOnRelay(0);
			} else {
				if(bio_cfg->boPolarity[1]) ioOnRelay(0);
				else	ioOffRelay(0);
			}	  			
	  	}
	}
}

int unitGetCaptureMode(void)
{
	int		rval;
	
	rval = RspData[0] & 0x80;
	if(rval) rval = (RspData[0] >> 4) & 0x07;
	return rval;
}

int adGetDoorAlarmState(int ID)
{
	return (int)(RspData[0] & 0x03);
} 
