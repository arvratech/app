#include "sam.h"

extern unsigned short	fileId;
extern unsigned char	fileOffset, fileLen;

unsigned char aid_sksam[12] = { 0xa0, 0x00, 0x00, 0x05, 0x61, 0x00, 0x00, 0x01, 0x03, 0xfe, 0x01  };

int _SkSamTask(void)
{
	unsigned char	*p, buf[300], head[8], temp[32], csn[12];
    int     rval, len, idLen;
	unsigned long timer;
timer = MS_TIMER;

	rval = iso14443_4SelectDF(aid_sksam, 11, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4GetData(8, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	memcpy(csn, buf, 8);
	rval = iso14443_4SelectFile(fileId, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4GetChallenge(8, buf, &len);
	if(rval || len < 10 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	head[0] = 0x90; head[1] = 0xf0; head[2] = 0x00; head[3] = 0x00; head[4] = 0x12;
	memcpy(temp, buf, 8); memcpy(temp+8, csn, 8); temp[16] = fileId >> 8; temp[17] = fileId;
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x61 || buf[len-1] != 0x20) {
		return rval;
	}
	head[0] = 0x00; head[1] = 0xc0; head[2] = 0x00; head[3] = 0x00; head[4] = 0x20;
	rval = samReceiveT0(head, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 34 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	memcpy(temp, buf, 32);
	rval = iso14443_4Authenticate(temp, 32, buf, &len);
	if(rval || len < 34 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	head[0] = 0x90; head[1] = 0xf2; head[2] = 0x00; head[3] = 0x00; head[4] = 0x20;
	memcpy(temp, buf, 32);
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4SecureReadBinary((int)fileOffset, (int)fileLen, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	idLen = len - 2;
printf("EF_ID len=%d\n", idLen);
	head[0] = 0x90; head[1] = 0xf4; head[2] = 0x00; head[3] = 0x00; head[4] = idLen;
	memcpy(temp, buf, idLen);
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x61) {
		return rval;
	}
	head[0] = 0x00; head[1] = 0xc0; head[2] = 0x00; head[3] = 0x00; head[4] = fileLen;
	rval = samReceiveT0(head, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	scAddData(2, buf, (int)fileLen);
printf("duration=%d ms\n", MS_TIMER-timer);
	return rval;
}

