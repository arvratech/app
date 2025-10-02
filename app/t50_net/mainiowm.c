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
#include "spimlib.h"
#include "slvmlib.h"
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
#include "slvm.h"
#include "blem.h"
#include "fpm.h"
#include "evt.h"
#include "uv.h"
#include "av.h"
#include "psu.h"
#include "beacon.h"
#include "pjsualib.h"
#include "pjsua_app.h"
#include "appact.h"
#include "slvnet.h"
#include "admprim.h"
#include "maininput.h"

extern int			slvm_fd;

uv_poll_t		_poll_slvm;
static int		slvLoop;


int MainOpenSlvm(void)
{
	int		fd;

	fd = slvmEventOpen();
	if(fd < 0) return -1;
	uv_poll_init(MainLoop(), &_poll_slvm, fd);
	ioInitIo();
	PsuOnPoeStatusChanged(NULL, ioGetPoe());
	return 0;
}

void MainCloseSlvm(void)
{
	int		fd;

	fd = _poll_slvm.io_watcher.fd;
	uv_poll_stop(&_poll_slvm);
printf(" uv_poll_stop .... \n");
	slvmEventClose(fd);
}

static void _MainOnPollSlvm(uv_poll_t *handle, int status, int events)
{
	int		fd;

	fd = handle->io_watcher.fd;
	slvmEventReadEvent(fd);
//printf("### onPollSlvm %d ###\n", slvmEventRead(fd));
	MainOnReadSlvm();
}

#define SLVM_DELAY		50		// 7

void MainStart(void);
void MainProcSlvmBufInterrupt(void);
void MainProcSlvmInterrupt(unsigned char *irqs);

void MainOnReadSlvm(void)
{
	unsigned char	buf[24];
	unsigned long	timer;
	int		 rval, fd, systemStatus, slvSystemStatus, scount, sloop;

	sloop = scount = 0;
	fd = _poll_slvm.io_watcher.fd;
	while(1) {
		systemStatus = devSystemStatus(NULL);
		us_sleep(SLVM_DELAY);
		timer = MS_TIMER;
		rval = slvmReadInterrupt(slvm_fd, buf);
		if(rval) {
if(rval != 99) printf("%ld ### slvmReadInterrupt ERROR...%d tm=%ld\n", MS_TIMER, rval, MS_TIMER-timer);
			usleep(99000);  // insert at 2022.1.10
			if(systemStatus) break;
			else {
				scount++;
				if(scount < 4) continue;
				else    break;
			}
		}
		scount = 0;
//printf("%lu slvm-irq=[%02x-%02x]\n", MS_TIMER, (int)buf[0], (int)buf[1]);
		slvSystemStatus = buf[0] >> 6;
		switch(systemStatus) {
		case SS_OPERATIONAL:
			if(slvSystemStatus == SS_OPERATIONAL) {
				MainProcSlvmInterrupt(buf);
				if(buf[0] & 0x20) MainProcSlvmBufInterrupt();
                if(!buf[0] && !buf[1]) sloop++;
                else    sloop = 0;
                if(sloop >= 3) break;
			} else {
printf("### op: systemStatus isn't Operational %d\n", slvSystemStatus);
				if(slvSystemStatus == SS_NON_OPERATIONAL) slvSystemStatus = SS_IDLE;
				devSetSystemStatus(NULL, slvSystemStatus);
			}
			break;
		case SS_IDLE:
			if(slvSystemStatus == SS_IDLE) {
printf("### id: slvmDownload...\n");
				rval = slvmDownload();
				usleep(99000);
				if(rval) {
printf("### id: slvmDownload ERROR...%d\n", rval);
				} else {
#ifdef _ESP32
printf("### id: blemDownload...\n");
					rval = blemDownload();
					usleep(99000);
					if(rval) {
printf("### id: blemDownload ERROR...%d\n", rval);
					} 
#endif
				}
				if(!rval) {
					slvmReset(2);
					devSetSystemStatus(NULL, SS_DOWNLOADING);
					slvLoop = 0;
				}
			} else {
				slvmReset(1);
				usleep(99000);
			}
			break;
		case SS_DOWNLOADING:
			if(slvSystemStatus == SS_IDLE || slvSystemStatus == SS_DOWNLOADING) {
				slvLoop++;
				if(slvLoop > 30) {
printf("### dl: systemStatus isn't to Operational ###\n");
					devSetSystemStatus(NULL, SS_IDLE);
				}
			} else if(slvSystemStatus == SS_OPERATIONAL) {
printf("### Operational ###\n");
				uv_poll_start(&_poll_slvm, UV_READABLE, _MainOnPollSlvm);
				devSetSystemStatus(NULL, slvSystemStatus);
				MainStart();
buf[1] |= 0xe0; 
				MainProcSlvmInterrupt(buf);
			} else {
printf("### dl: systemStatus isn't unexpected %d\n", slvSystemStatus);
				devSetSystemStatus(NULL, slvSystemStatus);
			}
			break;
		//default:
			//if(slvSystemStatus != SS_IDLE) slvmReset(1);
			//devSetSystemStatus(NULL, SS_IDLE);
		}
		rval = slvmEventRead(fd);
		systemStatus = devSystemStatus(NULL);
		if(systemStatus != SS_OPERATIONAL || rval) break;
	}
}

extern unsigned char	bleUpdated;

void bleCredential(unsigned char *buf, int size);

void MainProcSlvmBufInterrupt(void)
{
	unsigned char	*p, buf[832], evtData[12], ctm[8];
	unsigned long	timer;
	int		type, index, rval, size;

	while(1) {
#ifdef _ESP32
		size = 3;
#else
		size = 2;
#endif
		while(1) {
			us_sleep(SLVM_DELAY);
			timer = MS_TIMER;
			rval = slvmReadBuffer(slvm_fd, 0, buf, size);
			if(!rval) break;
if(rval != 99) printf("%lu ### slvmReadBuffer(0) ERROR...%d tm=%ld\n", MS_TIMER, rval, MS_TIMER-timer);
		}
		type = buf[0];
		if(type == 0xff) break;
#ifdef _ESP32
		size = (buf[1] << 8) | buf[2];
		if(type > 2) {
printf("%lu ### slvmReadBuffer(0) unexpected: type=%d size=%d %04x\n", MS_TIMER, type, size, size);
			continue;
		}
#else
		size = buf[1];
		if(type > 1 && type != 9) {
printf("%lu ### slvmReadBuffer(0) unexpected: type=%d size=%d %04x\n", MS_TIMER, type, size, size);
			continue;
		}
#endif
		if(size <= 0 || size > 248) {
printf("%lu ### slvmReadBuffer(0) unexpected: type=%d size=%d %04x\n", MS_TIMER, type, size, size);
			continue;
		}
		while(1) {
			us_sleep(SLVM_DELAY);
			timer = MS_TIMER;
			rval = slvmReadBuffer(slvm_fd, 1, buf, size);
			if(rval) printf("%lu ### slvmReadBuffer(1) ERROR...%d type=%d size=%d tm=%ld\n", MS_TIMER, rval, type, size, MS_TIMER-timer);
			if(rval == 0 || rval == 6) break;
		}
		if(rval) break;
		if(type == 9) {
			buf[size] = 0;
			printf("@%s", buf);
		} else if(type == 0) {
			//sleepTimer = 0;
			//if(sleepMode) WakeupSleepMode();
			slvmCredential(buf, size);
#ifdef _ESP32
        } else if(type == 2) {
			p = buf;
			beaconsScan(); timer = MS_TIMER; rtcGetDateTime(ctm);
			while(size >= 8) {
				rval = beaconsAdd(&index, p+1, timer, ctm); // rssi + MACaddress
				if(rval > 0) {
#ifdef _BLE_CRED
					if(offset < 24) {
						memcpy(tbuf+offset, p+2, 6); offset += 6;
					}
#else
					bin2card(p+2, 6, evtData);
					if(SlvnetIsConnected()) type = E_BEACON_DETECTED;
					else    type = E_BEACON_DETECTED+1;
                    evtAdd(OT_ACCESS_POINT, devId(NULL), type, ctm, evtData);
#endif
                }
                p += 8; size -= 8;
            }
#ifdef _BLE_CRED
            if(offset > 0) bleCredential(tbuf, offset);
#endif
            bleUpdated = 1;
            beaconsTimer(5000);
#endif
        } else {
            printf("### Unknown data received: type=%d size=%d\n", type, size);
        }
    }
}

void MainProcSlvmInterrupt(unsigned char *irqs)
{
	unsigned char	buf[128];
	int		rval, i;

	if(irqs[1] & 0x80) {	// CommonStatusInterrupt
		while(1) {
			us_sleep(SLVM_DELAY);
			rval = slvmReadStatus(slvm_fd, 0, buf, 2);
			if(!rval) break;
if(rval != 99) printf("%lu ### slvmReadStatus:cs ERROR.....\n", MS_TIMER);
		}
//printf("cs=[%02x", (int)buf[0]); for(i = 1;i < 2;i++) printf("-%02x", (int)buf[i]); printf("]\n");
		slvmCommonStatus(buf);
	}
	if(irqs[1] & 0x40) {	// LocalIOStatusInterrupt
		while(1) {
			us_sleep(SLVM_DELAY);
			rval = slvmReadStatus(slvm_fd, 2, buf, 3);
			if(!rval) break;
if(rval != 99) printf("%lu ### slvmReadStatus:ios ERROR.....\n", MS_TIMER);
		}
printf("ios=[%02x", (int)buf[0]); for(i = 1;i < 3;i++) printf("-%02x", (int)buf[i]); printf("]\n");
		slvmIoStatus(0, buf);
	}
	if(irqs[1] & 0x20) {	// ExtendedDataInterrupt
		while(1) {
			us_sleep(SLVM_DELAY);
			rval = slvmReadStatus(slvm_fd, 5, buf, 1);
			if(!rval) break;
if(rval != 99) printf("%lu ### slvmReadStatus:exd ERROR.....\n", MS_TIMER);
		}
printf("exd=[%02x]\n", (int)buf[0]);
		slvmExtendedData(buf);
	}
}

void bleCredential(unsigned char *buf, int size)
{
	CREDENTIAL_READER   *cr;
	unsigned char   *s, *d, msg[12];

	cr = crsGet(0);
	cr->cardType = 2;
	cr->credType = CREDENTIAL_CARD;
	cr->readerFormat = 0x02;
	s = buf; d = cr->data;
	while(size > 0) {
		bin2card(s, 6, d);
		s += 6; d += 9; size -= 6;
	}
	cr->dataLength = d - cr->data;
	msg[0] = GM_CRED_CAPTURED; msg[1] = 0; memset(msg+2, 0, 8);
	appPostMessage(msg);
}


