#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include "defs.h"
#include "defs_pkt.h"
#include "prim.h"
#include "NSEnum.h"
#include "rtc.h"
#include "sin.h"
#include "cbuf.h"
#include "syscfg.h"
//#include "cr.h"
#include "dev.h"
#include "user.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "nilib.h"
#include "acu.h"
#include "evt.h"
#include "uv.h"
#include "netudp.h"
#include "svrnet.h"

unsigned char	svrnetState;


int SvrnetIsConnected(void)
{
	if(svrnetState >= S_SVRNET_READY) return 1;
	else	return 0;
}

//#define _DEBUG		1

static void _SvrnetConnected(void)
{
	unsigned char	ipAddr[4];
	unsigned short	port;

//#ifdef _DEBUG
printf("%u camserver connected\n", MS_TIMER);
//#endif
	svrnetState = S_SVRNET_READY;
//	_PostSlvnetChanged();
}

static void _SvrnetDisconnected(void)
{
	if(svrnetState > S_SVRNET_IDLE) {
//#ifdef _DEBUG
printf("%u camserver disconnected...\n", MS_TIMER);
//#endif
		svrnetState = S_SVRNET_IDLE;
//		_PostSlvnetChanged();
	}
}

#include "svrnet_udp.c"

