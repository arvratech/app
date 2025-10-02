void _PostDoorChanged(void)
{
	void	*ad;
	unsigned char	buf[12];
	int		alarm;

	ad = adsGet(0);
	alarm = adAlarmState(ad);
	if(alarm == 2) {
		MentOpenTooLongAlarm();
		intrusionDisarm(NULL);
	} else if(alarm == 3) {
		MentForcedOpenAlarm();
		intrusionDisarm(NULL);
	} else if(alarm == 0) {
		audioFlushAlarm();
		intrusionDisarm(NULL);
	}
	buf[0] = GM_AD_DOOR_CHANGED; buf[1] = 0; memset(buf+2, 8, 0);
	appPostMessage(buf);
}

static unsigned char  rspData[2], acuChanged;

void acuProcessRxCommon(unsigned char *cbuf, int newRx)
{
	ACCESS_DOOR		*ad;
	unsigned char	pv, pri, val[2], buf[12];
	int		i, n;

	ad = adsGet(0);
	for(i = 0;i < 2;i++) {
		val[i] = rspData[i];
		rspData[i] = cbuf[i];
		val[i] ^= rspData[i];
	}
	if(newRx) val[0] = val[1] = 0xff;
	if(val[0]) {
		if(val[0] & 0x03) {
			n = rspData[0] & 0x03; if(n > 0) n++;
			if(adIsNonNull(ad)) {
				adSetAlarmState(ad, n);
				_PostDoorChanged();
			}
		}
	}
	if(val[1]) {
		if((val[1] & 0xf8) && adIsNonNull(ad)) {
			pv = (rspData[1] >> 6) & 0x03; pri = (rspData[1] >> 3) & 0x07;
			if(pv < 2 || !newRx) {
				if(pv >= 2) pri = 0;
				else if(pv == 1 || pri > 1) pri = 1;
printf("RxCommon: DoorCommand: pv=%d pri=%d\n", pv, pri);
				ad->hymo = pri;
				if(pri) n = 0; else n = 1;	// rte
				if(!psuBatPower(NULL) || devEnableBatteryLock(NULL) || pri) {
					slvmDoorCommand((int)pv, n);
					acuChanged = 1;
				}
	  		}
	  	}
		if((val[1] & 0x04) && !adIsNonNull(ad)) {
	  		if(rspData[1] & 0x04) buf[1] = 1;
			else	buf[1] = 0;
			buf[0] = buf[2] = 0;
printf("RxCommon: OutputCommand(0): %d\n", (int)buf[1]);
			slvmOutputCommand(0, buf);
		}
		//if((val[1] & 0x02) && (!adIsNonNull(ad) || !ad->cfg->AuxLockEnable)) {
		if(val[1] & 0x02) {
	  		if(rspData[1] & 0x02) buf[1] = 1;
			else	buf[1] = 0;
			buf[0] = buf[2] = 0;
printf("RxCommon: OutputCommand(1): %d\n", (int)buf[1]);
			slvmOutputCommand(1, buf);
	  	}
	}
}

unsigned char *_BiStat(int ioId);

void acuSetChanged(void)
{
	acuChanged = 1;
}

void acuGetReq(unsigned char *req)
{
	ACCESS_DOOR	*ad;
	unsigned char	*p, val, c, PV;

	p = _BiStat(0);
	ad = adsGet(0);
	if(ad && adIsNonNull(ad)) {
		val = adPresentValue(ad) << 6;
		c = p[1] >> 4 & 0x03;
		if(c > 0) c--;
		val |= c << 4;
	} else	val = 0;
	if(p[0] & 0x80) val |= 0x08; if(p[0] & 0x08) val |= 0x04; 
	if(p[0] & 0x40) val |= 0x02; if(p[0] & 0x04) val |= 0x01;
	req[0] = val;
	val = 0;
	if(p[0] & 0x20) val |= 0x80; if(p[0] & 0x02) val |= 0x40; 
	if(p[0] & 0x10) val |= 0x20; if(p[0] & 0x01) val |= 0x10;
	if(tamperTrackValue(NULL)) val |= 0x08;
	if(tamperPresentValue(NULL)) val |= 0x04;
	if(intrusionMode(NULL)) val |= 0x02;
	if(intrusionPresentValue(NULL)) val |= 0x01;
	req[1] = val;
//printf("### [%02x-%02x] ###\n", (int)req[0], (int)req[1]);
	req[2] = (unsigned char)acuChanged;
	acuChanged = 0;
}

