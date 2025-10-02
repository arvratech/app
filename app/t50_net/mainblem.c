#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <time.h>
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
#include "spimlib.h"
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
#include "beacon.h"
#include "pjsualib.h"
#include "pjsua_app.h"
#include "appact.h"
#include "maininput.h"

#ifndef _ESP32

extern int		blem_fd;

uv_poll_t		_poll_blem;
static int		bleSlvLoop;
static int		bleSystemStatus = SS_IDLE;

void *MainLoop(void);


int MainOpenBlem(void)
{
	int		fd;

	fd = blemEventOpen();
	if(fd < 0) return -1;
	uv_poll_init(MainLoop(), &_poll_blem, fd);
	return 0;
}

void MainCloseBlem(void)
{
	uv_poll_stop(&_poll_blem);
	blemEventClose(_poll_blem.io_watcher.fd);
}

static void _MainOnPollBlem(uv_poll_t *handle, int status, int events)
{
	int		fd;

	fd = handle->io_watcher.fd;
	blemEventReadEvent(fd);
//printf("### onPollBlem %d ###\n", blemEventRead(fd));
	MainOnReadBlem();
}

#define BLEM_DELAY		95

void MainProcBlemBufInterrupt(void);
void MainProcBlemInterrupt(unsigned char *irqs);

void MainOnReadBlem(void)
{
	unsigned char	buf[20];
	int		 rval, fd, slvSystemStatus;

	fd = _poll_blem.io_watcher.fd;
	while(1) {
		rval = spimReadInterrupt2(blem_fd, buf);
		if(rval) {
printf("### %d spimReadInterrupt ERROR...\n", (int)bleSystemStatus);
			break;
		}
//printf("%lu blem-irq=[%02x-%02x]\n", MS_TIMER, (int)buf[0], (int)buf[1]);
		if(bleSystemStatus == SS_OPERATIONAL && (buf[0] & 0x20)) {
			MainProcBlemBufInterrupt();
		}
		slvSystemStatus = buf[0] >> 6;
		switch(bleSystemStatus) {
		case SS_OPERATIONAL:
			if(slvSystemStatus == SS_OPERATIONAL) {
				MainProcBlemInterrupt(buf);
			} else {
printf("### ble op: blemStatus isn't Operational %d\n", slvSystemStatus);
				bleSystemStatus = slvSystemStatus;
			}
			break;
		case SS_IDLE:
			if(slvSystemStatus == SS_IDLE) {
printf("### ble id: blemDownload...\n");
				rval = blemDownload();
				if(rval) {
printf("### ble id: blemDownload ERROR...%d\n", rval);
					usleep(99000);
				} else {
					blemReset(2);
					bleSystemStatus = SS_DOWNLOADING;
					bleSlvLoop = 0;
				}
			} else {
				blemReset(1);
				usleep(99000);
			}
			break;
		case SS_DOWNLOADING:
			if(slvSystemStatus == SS_IDLE || slvSystemStatus == SS_DOWNLOADING) {
				bleSlvLoop++;
				if(bleSlvLoop > 30) {
printf("### ble dl: blemStatus isn't to operational ###\n");
					bleSystemStatus = SS_IDLE;
				}
			} else if(slvSystemStatus == SS_OPERATIONAL) {
printf("### ble Operational ###\n");
                uv_poll_start(&_poll_blem, UV_READABLE, _MainOnPollBlem);
				bleSystemStatus = slvSystemStatus;
				//buf[0] = GM_SS_CHANGED; memset(buf+1, 9, 0);
				//appPostMessage(buf);
				MainProcBlemInterrupt(buf);
			} else {
printf("### ble dl: blemStatus isn't unexpected %d\n", slvSystemStatus);
				bleSystemStatus = slvSystemStatus;
			}
			break;
		//default:
			//if(slvSystemStatus != SS_IDLE) slvmReset(1);
			bleSystemStatus = SS_IDLE;
		}
		rval = blemEventRead(fd);
		if(bleSystemStatus != SS_OPERATIONAL || rval) break;
		us_sleep(BLEM_DELAY);
	}
}

extern unsigned char	bleUpdated;

void MainProcBlemBufInterrupt(void)
{
	unsigned char	*p, tbuf[8], buf[512], evtData[12], ctm[8];
	unsigned long	curtm;
	void	*subdev;
	int		i, type, index, rval, size;

	size = 0;
	while(1) {
		while(1) {
			us_sleep(BLEM_DELAY);
			rval = spimReadBuffer(blem_fd, 0, 0, tbuf, 3);
			if(!rval) break;
			printf("spimReadBuffer(0) ERROR.......\n");
		}
		if(tbuf[0] == 0xff) break;
        size = (tbuf[1] << 8) | tbuf[2];
		while(1) {
			us_sleep(BLEM_DELAY);
			rval = spimReadBuffer(blem_fd, 0, 1, buf, size);
			if(rval) printf("spimReadBuffer(1) ERROR...\n");
			if(rval == 0 || rval == 6) break;
		}
		if(rval) break;
		type = tbuf[0];
		if(type == 9) {
			buf[size] = 0;
			printf("@%s", buf);
        } else if(bleSystemStatus != SS_OPERATIONAL) {
            printf("### Unexpected data received: size=%d\n", size);
		} else if(type == 2) {
			p = buf;
			beaconsScan(); curtm = MS_TIMER; rtcGetDateTime(ctm);
			while(size >= 8) {
				rval = beaconsAdd(&index, p+1, curtm, ctm); // rssi + MACaddress
				if(rval > 0) {
					bin2card(p+2, 6, evtData);
					if(SlvnetIsConnected()) type = E_BEACON_DETECTED;
					else    type = E_BEACON_DETECTED+1;
					evtAdd(OT_ACCESS_POINT, devId(NULL), type, ctm, evtData);
				}
				p += 8; size -= 8;
			}
			bleUpdated = 1;
			beaconsTimer(5000);
		} else {
			printf("### Unknown data received: type=%d size=%d\n", type, size);
		}
	}
}

void MainProcBlemInterrupt(unsigned char *irqs)
{
}

#endif

