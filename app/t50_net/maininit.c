#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>
#include "defs.h"
#include "defs_obj.h"
#include "NSEnum.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "slvmlib.h"
#include "slvm.h"
#include "user.h"
#include "psu.h"
#include "hw.h"
#include "cam.h"
#include "crcred.h"
#include "cf.h"
#include "syscfg.h"
#include "tacfg.h"
#include "audio.h"
#include "gactivity.h"
#include "gapp.h"
#include "gesture.h"
#include "nilib2.h"
#include "dhcp.h"
#include "wpanet.h"
#include "wpactrl.h"
#include "viewprim.h"
#include "psmem.h"
#include "actprim.h"
#include "epolllib.h"
#include "inputlib.h"
#include "slvmlib.h"
#include "slvm.h"
#include "blem.h"
#include "fpm.h"
#include "evt.h"
#include "uv.h"
#include "av.h"
#include "fsuser.h"
#include "beacon.h"
#include "appdev.h"
//#include "apphw.h"
//#include "appnet.h"
#include "intrusion.h"
#include "appact.h"

void OnTamperPresentValueChanged(void *self);
void OnIntrusionValueChanged(void *self, int modec, int pvc);

void InitializeHardware(void)
{
	void	*cf;
	int		i;

	devInit();
	unitsInit();
	subdevsInit();
	crsInit();
	bisInit();
	bosInit();
	adsInit();
	aasInit();
	cfsInit();
	cf = cfPinGet();
	cfPinValidate(cf);
	for(i = 0;i < MAX_CF_CARD_SZ;i++) {
		cf = cfCardsGet(i);
		cfCardValidate(cf);
 	}
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) {
		cf = cfWiegandsGet(i, 0);
		if(cf) cfWiegandValidate(cf);
 	}
	for(i = 4;i < MAX_CF_WIEGAND_SZ;i++) {
		cf = cfWiegandsGet(i, 1);
		if(cf) cfWiegandValidate(cf);
 	}
    psuInit(OnPsuAcAlarmStateChanged, OnPsuBatAlarmStateChanged, NULL, OnPsuPoeStatusChanged);
    tamperSetOnPresentValueChanged(NULL, OnTamperPresentValueChanged);
    intrusionSetOnValueChanged(NULL, OnIntrusionValueChanged);
}

void OnTamperPresentValueChanged(void *self)
{
	unsigned char	msg[12];
	int		pv, evtId, id;

	pv = tamperPresentValue(NULL);
	if(pv) {
printf("tamper: alarm\n");
		//evtId = E_READER_TAMPER_ALARM;
		MoveHomeActivity();
	} else {
printf("tamper: normal\n");
		//evtId = E_READER_TAMPER_NORMAL;
		msg[0] = GM_TAMPER_NORMAL; memset(msg+1, 0, 9);
		appPostMessage(msg);
	}
	id = devId(NULL) + 1;
	//evtAdd(OT_SUBDEVICE, id, evtId, tamperChangeOfStateTime(NULL), NULL);
	//SubdevNotifyStatus(NULL);
	SlvnetSendRequest();
}

void OnIntrusionValueChanged(void *self, int modec, int pvc)
{
	unsigned char	msg[12];
	int		mode, pv, evtId, id;

	if(modec) {
		mode = intrusionMode(NULL);
		if(mode) {
printf("intrusion: armed\n");
			//evtId = E_INTRUSION_ARMED;
		} else {
printf("intrusion: disarmed\n");
			//evtId = E_INTRUSION_DISARMED;
		}
		id = devId(NULL) + 1;
		//evtAdd(OT_SUBDEVICE, id, evtId, intrusionChangeOfModeTime(NULL), NULL);
	}
	if(pvc) {
		pv = intrusionPresentValue(NULL);
		if(pv) {
printf("intrusion: alarm\n");
			evtId = E_INTRUSION_ALARM;
		} else {
printf("intrusion: normal\n");
			evtId = E_INTRUSION_NORMAL;
		}
		id = devId(NULL) + 1;
		//evtAdd(OT_SUBDEVICE, id, evtId, intrusionChangeOfStateTime(NULL), NULL);
	}
	//SubdevNotifyStatus(NULL);
	SlvnetSendRequest();
	msg[0] = GM_ALERT_CHANGED; memset(msg+1, 0, 9);
	appPostMessage(msg);
}

