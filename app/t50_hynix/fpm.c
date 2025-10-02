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
#include "slvmlib.h"
#include "slvm.h"
#include "fpm.h"

static unsigned char	fpmBuf[832];
static unsigned char	fpmCmd;
static int		fpmSize;


void fpmTouched(void)
{
	unsigned char	msg[12];

	msg[0] = GM_FPM_TOUCHED; memset(msg+1, 0, 9);
	appPostMessage(msg);
}

int fpmCommand(void)
{
	return (int)fpmCmd;
}

void fpmWriteRequest(void)
{
printf("fpmWriteRequest: 0x%02x size=%d\n", (int)fpmBuf[1], fpmSize);
	slvmWriteCredBuffer(fpmBuf, fpmSize);
}

void fpmReadResponse(unsigned char *buf, int size)
{
	memcpy(fpmBuf, buf, size); fpmSize = size;
	buf[0] = GM_FPM_RESPONSE;
	appPostMessage(buf);
}

void MainSvcPost(int type);

void fpmRequestCapture(void)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x40; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmSize = 7;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

void fpmResponseCapture(void)
{
	CREDENTIAL_READER	*cr;

	cr = crsGet(0);
	if(fpmBuf[1] == 0x40 && fpmBuf[2] == 0) {
		cr->credType = CREDENTIAL_FP;
		cr->dataLength = 400;
	} else {
printf("fpmResponseCaptuture...cmd=0x%02x 0x%0x\n", (int)fpmBuf[1], (int)fpmBuf[2]);
		cr->credType = CREDENTIAL_NONE;
	}
	fpmCmd = 0;
}

void fpmCopyTemplate(unsigned char *buf)
{
	memcpy(buf, fpmBuf+7, 400);
}

void fpmRequestIdentify(void)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x56; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmSize = 7;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

void fpmRequestIdentifyMatch(void)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x79; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmSize = 407;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

unsigned long bcd2fpid(unsigned char *s)
{
	char	temp[16];
	int		i, c, c2;

	for(i = 0;i < 10;i += 2) {
		c = *s++;
		temp[i] = (c >> 4) + '0';
		temp[i+1] = c = (c & 0xf) + '0';
	}
	temp[10] = 0;
	return n_atou(temp);
}	

void fpid2bcd(unsigned long fpId, unsigned char *d)
{
	char	temp[16];
	int		i, c;

	sprintf(temp, "%010u", fpId);
	for(i = 0;i < 10;i += 2) {
		c = (temp[i] - '0') << 4;
		c += temp[i+1] - '0';
		*d++ = c;  
	}
}

void fpmResponseIdentify(void)
{
	CREDENTIAL_READER	*cr;
	int		rval;

	cr = crsGet(0);
	if(fpmBuf[1] == 0x56) {
		cr->credType = CREDENTIAL_FP;
		cr->result = 0;
		rtcGetDateTime(cr->accessTime);
		rval = fpmBuf[2];
		if(rval == 0) {
			cr->accessEvent	= E_ACCESS_GRANTED_FP;
			cr->userId = bcd2fpid(fpmBuf+7) >> 1;
			cr->userName[0] = cr->userExternalId[0] = 0;
		} else if(rval == 0x08) {
			cr->result = 3;
		} else if(rval == 0x1b) {
			cr->accessEvent	= E_ACCESS_DENIED_UNREGISTERED_FP;
			cr->userId = 0; cr->userName[0] = cr->userExternalId[0] = 0;
		} else {
			cr->result = 2;
		}
	} else {
		cr->result = 3;
	}
	fpmCmd = 0;
}

void fpmResponseIdentifyMatch(void)
{
	CREDENTIAL_READER	*cr;
	int		rval;

	cr = crsGet(0);
	if(fpmBuf[1] == 0x56) {
		cr->credType = CREDENTIAL_FP;
		cr->result = 0;
		rtcGetDateTime(cr->accessTime);
		rval = fpmBuf[2];
		if(rval == 0) {
			cr->accessEvent	= E_ACCESS_GRANTED_FP;
			cr->userId = bcd2fpid(fpmBuf+7) >> 1;
			cr->userName[0] = cr->userExternalId[0] = 0;
		} else if(rval == 0x08) {
			cr->result = 3;
		} else if(rval == 0x1b) {
			cr->accessEvent	= E_ACCESS_DENIED_UNREGISTERED_FP;
			cr->userId = 0; cr->userName[0] = cr->userExternalId[0] = 0;
		} else {
			cr->result = 2;
		}
	} else {
		cr->result = 3;
	}
	fpmCmd = 0;
}

void fpmRequestFind(unsigned long fpId)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x60; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = 0xff; fpmBuf[5] = fpmBuf[6] = 0;
	fpid2bcd(fpId, fpmBuf+7);
	fpmSize = 12;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseFind(void)
{
	int		rval;

	if(fpmBuf[1] == 0x60) rval = fpmBuf[2];
	else	rval = -1;
	fpmCmd = 0;
	return rval;
}

void fpmRequestCreateTemplate(unsigned char *tmpl)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0xfa; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	memcpy(fpmBuf+7, tmpl, 800);
	fpmSize = 807;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseCreateTemplate(void)
{
	int		rval;

	if(fpmBuf[1] == 0xfa) rval = fpmBuf[2];
	else	rval = -1;
	fpmCmd = 0;
	return rval;
}

void fpmRequestAddTemplate(unsigned long fpId, unsigned char *tmpl)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x71; fpmBuf[2] = 0;
	fpmBuf[3] = 0x01; fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmBuf[7] = fpmBuf[8] = 0xff;
	fpmBuf[9] = fpmBuf[10] = 0;
	memcpy(fpmBuf+11, tmpl, 800);
	fpid2bcd(fpId, fpmBuf+811);
	memset(fpmBuf+816, 0xff, 6);
	fpmBuf[822] = 5;
	fpmSize = 823;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseAddTemplate(void)
{
	int		rval;

	if(fpmBuf[1] == 0x71) rval = fpmBuf[2];
	else	rval = -1;
	fpmCmd = 0;
	return rval;
}

void fpmRequestDeleteTemplate(unsigned long fpId)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x72; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = 0xff; fpmBuf[5] = fpmBuf[6] = 0;
	fpid2bcd(fpId, fpmBuf+7);
	fpmSize = 12;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseDeleteTemplate(void)
{
	int		rval;

	if(fpmBuf[1] == 0x72) rval = fpmBuf[2];
	else	rval = -1;
	fpmCmd = 0;
	return rval;
}

void fpmRequestDeleteAllTemplate(void)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x76; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmSize = 7;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseDeleteAllTemplate(void)
{
	int		rval;

	if(fpmBuf[1] == 0x76) rval = fpmBuf[2];
	else	rval = -1;
	fpmCmd = 0;
	return rval;
}

void fpmRequestTemplateCount(void)
{
	fpmBuf[0] = 0x40; fpmBuf[1] = 0x70; fpmBuf[2] = 0;
	fpmBuf[3] = fpmBuf[4] = fpmBuf[5] = fpmBuf[6] = 0;
	fpmSize = 7;
	fpmCmd = fpmBuf[1];
	MainSvcPost(GM_FPM_REQUEST);
}

int fpmResponseTemplateCount(void)
{
	unsigned short	sVal;
	int		rval;

	if(fpmBuf[1] == 0x70) {
		BYTEtoSHORT(fpmBuf+2, &sVal);
		rval = sVal;
	} else	rval = -1;
	fpmCmd = 0;
	return rval;
}


