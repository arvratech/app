#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include "defs_obj.h"
#include "defs.h"
#include "NSEnum.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "user.h"
#include "hw.h"
#include "cf.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "actprim.h"
#include "wpanet.h"
#include "appact.h"
#include "svcwork.h"
#include "fpm.h"
#include "uv.h"
#include "psu.h"
#include "cr.h"
#include "crcred.h"
#include "slvnet.h"
#include "event.h"
#include "intrusion.h"
#include "slvmlib.h"
#include "slvm.h"

#define COMM_REG_RDONLY_SZ		7

extern int		slvm_fd;

unsigned char	commRegs[MAX_COMM_REGS_SZ];
unsigned char	unitRegs[MAX_UNIT_HALF_SZ][MAX_UNIT_REGS_SZ];
unsigned char	commStats[MAX_COMM_STATS_SZ];
unsigned char	unitStats[MAX_UNIT_HALF_SZ][MAX_UNIT_STATS_SZ];
unsigned char	doorCommandId;


void slvmCommonStatus(unsigned char *buf)
{
	int		val;

	commStats[0] = buf[0];
	commStats[1] = buf[1];
	val = commStats[0] >> 6 & 0x03;
	devSetRestartReason(val);
	val = commStats[0] >> 5 & 0x01;
	TamperOnTrackValueChanged(NULL, val);
}

void slvmExtendedData(unsigned char *buf)
{
	int		volt;

	volt = buf[0] * 1000;
	PsuOnAcVoltageNowChanged(NULL, volt);
}

unsigned char *_BiStat(int ioId)
{
	unsigned char	*p;

	if(ioId < 32) {
		p = commStats + 2;
	} else {
		p = unitStats[CH2UNIT(ioId)] + 1 + (ioId & 3) * 3;
	}
	return p;
}

unsigned char *_BoAdStat(int ioId)
{
	unsigned char	*p;

	if(ioId < 32) {
		p = commStats + 3;
	} else {
		p = unitStats[CH2UNIT(ioId)] + 2 + (ioId & 3) * 3;
	}
	return p;
}

void _BiPvChanged(void *bi, int pv)
{
	unsigned char	buf[12];

if(pv) printf("BiPvChanged: id=%d pv=1\n", biId(bi));
else   printf("BiPvChanged: id=%d pv=0\n", biId(bi));
	if(biId(bi) == 2 && intrusionIsEnable(NULL)) ;
	else {
		if(biId(bi) == 1) {
			buf[0] = GM_AD_DOOR_CHANGED; buf[1] = 0; memset(buf+2, 8, 0);
			appPostMessage(buf);
		}
		SlvnetSendRequest();
	}
}

void _BiAlarmChanged(void *bi, int alarm)
{
	ACCESS_DOOR     *ad;
	unsigned char   ctm[8];

if(alarm) printf("BiAlarmChanged: id=%d alarm=1\n", biId(bi));
else	  printf("BiAlarmChanged: id=%d alarm=0\n", biId(bi));
	if(alarm && !(devId(NULL) & 1) && biId(bi) == 3 && !SlvnetIsConnected()) {     // insert 2020.4.21
		ad = adsGet(0);
		ad->hymo = 1;
        slvmDoorCommand(PV_UNLOCK, 0);
		rtcGetDateTime(ctm);
		EventAdd(OT_FIRE_ZONE, 0, E_FIRE_ALARM, ctm, NULL);
	}
	SlvnetSendRequest();
}

void _BoPvChanged(void *bo, int pv)
{
}

void _AdAlarmChanged(void *ad, int alarm)
{
	SlvnetSendRequest();
}

void _AdStatusChanged(void *ad, int ioId, int status)
{
	ACCESS_DOOR		*ads;
	unsigned char	msg[12];
	int		i, commandId, priArray[8];

	commandId = status >> 2 & 0x03;	
printf("_AdStatusChanged: 0x%02x cmdId=%d %d\n", status, doorCommandId, commandId);
	if(doorCommandId == commandId) {
		ads = (ACCESS_DOOR *)ad;
		for(i = 0;i < 8;i++) priArray[i] = -1;
		if(status & 0x2) {	// LockPv=1:Unlock
			if(ads->hymo) priArray[ads->hymo-1] = PV_UNLOCK;
			else {
				priArray[7] = PV_PULSE_UNLOCK;
				i = adPresentValue(ad);
				if(i != PV_PULSE_UNLOCK && !audioActive()) MentAddUserNormal(170);	// append 2024.1.12
			}
		} else if(ads->hymo) { 
			priArray[ads->hymo-1] = PV_LOCK;
		}
		adSetPriorityArray(ad, priArray);
		SlvnetSendRequest();
		msg[0] = GM_AD_LOCK_CHANGED; msg[1] = adId(ad); memset(msg+2, 8, 0);
		appPostMessage(msg);
		intrusionDisarm(NULL);
	}
}

void slvmIoStatus(int ioId, unsigned char *buf)
{
	BINARY_INPUT	*bi;
	BINARY_OUTPUT	*bo;
	void			*ad;
	unsigned char	*p, val, msk1, msk2;
	int		i, id;

	p = _BiStat(ioId);
	val = p[0] ^ buf[0];
	if(val) {
		p[0] = buf[0];
		id = IO2BI(ioId);
		msk1 = 0x80; msk2 = 0x08;
		for(i = 0;i < 4;i++) {
			bi = bisGet(id+i);
			if(val & msk1) _BiPvChanged(bi, p[0] & msk1);
			if(val & msk2) _BiAlarmChanged(bi, p[0] & msk2);
			msk1 >>= 1; msk2 >>= 1;
		}
	}
	val = p[1] ^ buf[1];
	if(val) {
		p[1] = buf[1];
		id = IO2BO(ioId);
		msk1 = 0x80;
		for(i = 0;i < 2;i++) {
			bo = bosGet(id+i);
			if(val & msk1) _BoPvChanged(bo, p[1] & msk1);
			msk1 >>= 1;
		}
		ad = adsGet(IO2CH(ioId));
		if(ad && ioId == adIoSet(ad)) {
			if(val & 0x30) _AdAlarmChanged(ad, p[1] >> 4 & 0x03);
			if(val & 0x0f) _AdStatusChanged(ad, ioId, p[1] & 0x0f);
		}
	}
	val = p[2] ^ buf[2];
	if(val) {
		p[2] = buf[2];
	}
}

void slvmCredential(unsigned char *buf, int size)
{
	CREDENTIAL_READER	*cr;
	unsigned char	msg[12];
	int		rval, i;

	cr = crsGet(0);
	rval = crCapturedCard(cr, buf, size);
	if(rval > 0) {
printf("cred=[%02x", (int)cr->data[0]); for(i = 1;i < 9;i++) printf("-%02x", (int)cr->data[i]); printf("]\n");
		if(!devLocalUnitModel(NULL)) {
			msg[0] = GM_CRED_CAPTURED; msg[1] = 0; memset(msg+2, 0, 8);
			appPostMessage(msg);
		} else {
			msg[0] = GM_IDENTIFY_REQUEST; msg[1] = 0;
			//svcWorkPostMessage(msg);
		}
	} else {
		cr->result = 6;
printf("authResult=%d\n", (int)cr->result);
		msg[0] = GM_IDENTIFY_RESULT; msg[1] = 1; memset(msg+2, 0, 8);
		appPostMessage(msg);
	}
}

int _ModCodeIoConfig(int ioId, unsigned char *buf)
{
	void	*ad, *bi, *bo;
	unsigned char	*p, val, msk1, msk2;
	int		i, id;

	p = buf;
	val = 0; msk1 = 0x80; msk2 = 0x08;
	id = IO2BI(ioId);
	for(i = 0;i < 4;i++, msk1 >>= 1, msk2 >>= 1) {
		bi = bisGet(id+i);
		if(!devTestMode(NULL)) {
			if(biPolarity(bi)) val |= msk1; 
			if(biIsDoorType(bi)) val |= msk2; 
		}
	}
	*p++ = val;
	for(i = 0;i < 4;i++) {
		bi = bisGet(id+i);
		*p++ = biTimeDelay(bi); *p++ = biTimeDelayNormal(bi);
	}
	val = 0; msk1 = 0x80; msk2 = 0x20;
	id = IO2BO(ioId);
	for(i = 0;i < 2;i++, msk1 >>= 1, msk2 >>= 1) {
		bo = bosGet(id+i);
		if(!devTestMode(NULL)) {
			if(boPolarity(bo)) val |= msk1; 
			if(boIsDoorType(bo)) val |= msk2; 
		}
	}
	id = IO2CH(ioId);
	ad = adsGet(id);
	if(adLockType(ad)) val |= msk2; 
	*p++ = val;
printf("BI=%02x BO=%02x\n", (int)*(p-10), (int)*(p-1));
	*p++ = adDoorPulseTime(ad);
	*p++ = adDoorExtendedPulseTime(ad);
	*p++ = adTimeDelay(ad);
	*p++ = adTimeDelayNormal(ad);
	*p++ = adDoorOpenTooLongTime(ad);
printf("ad(0): %d,%d,%d,%d,%d\n", adDoorPulseTime(ad), adDoorExtendedPulseTime(ad), adTimeDelay(ad), adTimeDelayNormal(ad), adDoorOpenTooLongTime(ad));
printf("%d IoConfig=%d ad(%d): %d,%d,%d,%d,%d\n", ioId, p-buf, id, (int)*(p-5), (int)*(p-4), (int)*(p-3), (int)*(p-2), (int)*(p-1));
	return p - buf;
}

static int _ModCodeReaderConfig(unsigned char *buf)
{
	void	*cr;
	unsigned char	*p;
	int		val;

	cr = crsGet(0);
	p = buf;
	val = 0;
	if(crIsEnableWiegandReader(cr)) val |= 0x40;
//val = 0x80;
	*p++ = val;
printf("ReaderConfig=0x%02x\n", val);
	return p - buf;
}

static int _ModCodeBleConfig(unsigned char *buf)
{
	unsigned char	*p, *s;
	int		i;

	p = buf;
	*p++ = bleConfig(NULL);
	*p++ = 0; *p++ = 0; *p++ = 0;
	*p++ = bleMinBeaconSignal(NULL);		// -90
	for(i = 0;i < 10;i++) { 
		s = bleGetMacFilter(NULL, i);
		memcpy(p, s, 3); p += 3;
	}
printf("blecfg=%d: cfg=%02x rssi=%d\n", p-buf, bleConfig(NULL), bleMinBeaconSignal(NULL));
	return p - buf;
}

int slvmDownload(void)
{
	unsigned char	*p, *p0;

	p0 = p = commRegs + 1;
	p += _ModCodeIoConfig(0, p);
	p += _ModCodeReaderConfig(p);
	return slvmWriteRegister(slvm_fd, 1, p0, p - p0);
}

void slvmWriteLocalIoConfig(void)
{
	unsigned char	*p, buf[64];
	int		len;

	p = commRegs + 1;
	len = _ModCodeIoConfig(0, buf);
	if(memcmp(p, buf, len)) {
		memcpy(p, buf, len);
printf("slvmWriteLocalIoConfig...%d\n", len);
		slvmWriteRegister(slvm_fd, 1, buf, len);
	}
}

void slvmWriteReaderConfig(void)
{
	unsigned char	*p, buf[64];
	int		len;

	p = commRegs + 16;
	len = _ModCodeReaderConfig(buf);
	if(memcmp(p, buf, len)) {
		memcpy(p, buf, len);
printf("slvmWriteReaderConfig...%d\n", len);
		slvmWriteRegister(slvm_fd, 16, buf, len);
	}
}

void slvmReset(int param)
{
	unsigned char	buf[4];

printf("slvmReset: %d\n", param);
	buf[0] = param;
	slvmWriteCommand(slvm_fd, 0, buf, 3);
}

void slvmResetMaster(int param)
{	
	unsigned char	buf[4];

printf("slvmResetMaster: %d\n", param);
	buf[0] = param;
	slvmWriteCommand(slvm_fd, 1, buf, 3);
}

void slvmWriteCredBuffer(unsigned char *data, int length)
{
	unsigned char	buf[4];
	int     size;

	buf[0] = 0x00;
#ifdef _ESP32
	buf[1] = length >> 8; buf[2] = length; size = 3;
#else
	buf[1] = length; size = 2;
#endif
	slvmWriteBuffer(slvm_fd, 0, buf, size);
	slvmWriteBuffer(slvm_fd, 1, data, length);
}

void slvmDoorCommand(int pv, int rte)
{
	unsigned char	buf[4];

	doorCommandId++; if(doorCommandId > 3) doorCommandId = 0;
printf("DoorCommand: cmdId=%d pv=%d rte=%d\n", doorCommandId, pv, rte);
	buf[0] = (doorCommandId << 3) | (pv << 1) | rte;
	slvmWriteCommand(slvm_fd, 2, buf, 3);	
}

void slvmOutputCommand(int boId, unsigned char *buf)
{
	slvmWriteCommand(slvm_fd, 3+boId, buf, 3);	
}

#define EVIOCGKDK1		_IOR('E', 0xe1, int)
#define EVIOCGKDK2		_IOR('E', 0xe2, int)
#define EVIOC_INITIO	_IOR('E', 0xe6, int)

extern uv_poll_t	_poll_slvm;
extern int			blem_fd;

void ioInitIo(void)
{
	unsigned int	val;
	int     fd, rval;

#ifdef _ESP32
	fd = blem_fd;
#else
	fd = _poll_slvm.io_watcher.fd;
#endif
	rval = ioctl(fd, EVIOC_INITIO, &val);
	if(rval < 0) printf("ioInitIo error ############: %s\n", strerror(errno));
}

int ioGetPoe(void)
{
	unsigned int	val;
	int     fd, rval;

#ifdef _ESP32
	fd = blem_fd;
#else
	fd = _poll_slvm.io_watcher.fd;
#endif
	rval = ioctl(fd, EVIOCGKDK2, &val);
	if(rval < 0) printf("ioGerPoe error ############: %s\n", strerror(errno));
	if(val) val = 1;
	return val;
}

