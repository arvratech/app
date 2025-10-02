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
#include "defs.h"
#include "NSEnum.h"
#include "lang.h"
#include "lcdc.h"
#include "rtc.h"
#include "prim.h"
#include "slvmlib.h"
#include "slvm.h"
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
#include "blem.h"

extern int		slvm_fd;
#ifndef _ESP32
extern int		blem_fd;
#endif
unsigned char	blemRegs[40];


static int _CodeBleConfig(unsigned char *buf)
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

int blemDownload(void)
{
	unsigned char	*p, buf[64];
	int		rval, len;

	p = blemRegs;
	len = _CodeBleConfig(buf);
	memcpy(p, buf, len);
printf("blemDownload...%d\n", len);
#ifdef _ESP32
	rval = slvmWriteRegister(slvm_fd, 21, buf, len);
#else
	rval = spimWriteRegister(blem_fd, 0, 1, buf, len);
#endif
	return rval;
}

void blemWrite(void)
{
	unsigned char	*p, buf[64];
	int		rval, len;

	p = blemRegs;
	len = _CodeBleConfig(buf);
	if(memcmp(p, buf, len)) {
		memcpy(p, buf, len);
printf("blemWrite...%d\n", len);
#ifdef _ESP32
		rval = slvmWriteRegister(slvm_fd, 21, buf, len);
#else
		rval = spimWriteRegister(blem_fd, 0, 1, buf, len);
#endif
	}
}

void blemReset(int param)
{
	unsigned char	buf[4];

printf("blemReset: %d\n", param);
	buf[0] = param;
#ifdef _ESP32
#else
	spimWriteCommand(blem_fd, 0, 0, buf, 3);
#endif
}

