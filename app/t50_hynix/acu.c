#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "sysdep.h"
#include "tacfg.h"
#include "dev.h"
#include "cr.h"
#include "cam.h"
#include "ble.h"
#include "ad.h"
#include "user.h"
#include "slvmlib.h"
#include "slvm.h"
#include "nilib.h"
#include "uv.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "slvnet.h"
#include "acuobj.h"
#include "acu.h"

#define XFER_DATA_SIZE		800
#define XFER_MAX_SIZE		4096000

SYS_CFG		*tmp_syscfg, _tmp_syscfg;
TA_CFG		*tmp_tacfg, _tmp_tacfg;
static int	xferBusy, xferCount;
unsigned char	reqCmd, reqOt, reqOp;

#include "acu_com.c"
#include "acu_wr.c"
#include "acu_xfer.c"
#include "acu_req.c"


void SvrInit(void)
{
//	xferBusy = 0;
}

void acuProcessRequest(void)
{
	unsigned char	*p;
	int		cmd, size;

	p = SlvnetConfRspBuf(); size = SlvnetConfRspRxSize();
	cmd = *p++; size--;	
printf("Rx Req: cmd=0x%02x dataSize=%d\n", cmd, size);
	switch(cmd) {
	case P_WRITE_OBJECT:			acuWriteObject(p, size); break;
	case P_WRITE_PROPERTY:			acuWriteProperty(p, size); break;
	case P_READ_PROPERTY:			acuReadProperty(p, size); break;
	case P_REINITIALIZE_CR:			acuReinitializeCR(p, size); break;
	case P_DOWNLOAD_FIRMWARE_CR:	acuDownloadFirmwareCR(p, size); break;
	case P_DOWNLOAD_WALLPAPER_CR:	acuDownloadWallPaperCR(p, size); break;
	case P_DOWNLOAD_KERNEL_CR:		acuDownloadKernelCR(p, size); break;
	case P_DOWNLOAD_VIDEO_CR:		acuDownloadVideoCR(p, size); break;
	case P_DOWNLOAD_POLL_CR:		acuDownloadPollCR(p, size); break;
	case P_RUN_SCRIPT:				acuRunScriptFile(p, size); break;
	case P_CR_READ_EVENT:			acuReadEvent(p, size); break;	// commented at 2018.8.27
	case P_XFER_WRQ:				acuXferServerWRQ(p, size); break;
	}
}

void acuProcessSegRequest(void)
{
	unsigned char	*p;
	int		cmd, size;

	p = SlvnetConfRspBuf(); size = SlvnetConfRspRxSize();
	cmd = *p++; size--;	
//printf("Rx SegReq: cmd=0x%02x dataSize=%d\n", cmd, size);
	switch(cmd) {
	case P_XFER_DATA:		acuXferServerData(p, size); break;
	case P_XFER_ERROR:		acuXferServerError(p, size); break;
	}
}

void acuProcessResponse(void)
{
	unsigned char	*p, buf[12];
	int		len;

	p = SlvnetConfReqBuf(); len = SlvnetConfReqRxSize();
printf("Rx Rsp: size=%d reqCmd=0x%02x\n", len, reqCmd);
	if(reqCmd == 0x30 || reqCmd == 0x31) {
		buf[0] = GM_CONF_CNF;
		appPostMessage(buf);
	} else {
		AcuRequestCallOnResult(0, p, len);
	}
}

