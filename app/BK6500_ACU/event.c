#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "sysdep.h"
#include "syscfg.h"
#include "ff.h"
#include "evt.h"
#include "msg.h"
#include "sdi.h"
#include "fsuser.h"
#include "acad.h"
#include "acap.h"
#include "gwnd.h"
#include "topprim.h"
#include "event.h"

#define EVT_RECORD_SIZE		64

static FIL	file;
static int	w_count, w_yy, w_mm;


void EventInit(void)
{
	evtInit();
	w_yy = w_mm = w_count = -1;			
}

static int _EvtGetSerialNo()
{
	return w_count+1;
}

void DesktopSetStat(char *Text);
int  GetCamNetState(void);
void AddEventCamImage(unsigned char *evt);

void EventAdd(int objType, int objId, int evtId, unsigned char *ctm, unsigned char *evtData)
{
	unsigned char	buf[80];
	char	*p;
	int		rval, no;

	rval = evtCheckMask(evtId);
	if(rval) return;
	evtAdd(objType, objId, evtId, ctm, evtData);
	no = sdiGetState();
	if(no > 2) {
		p = (char *)buf;
		sprintf(p, "%03d", evtId); p += 3;
		sprintf(p, "%02d%02d%02d%02d%02d%02d", (int)ctm[0], (int)ctm[1], (int)ctm[2], (int)ctm[3], (int)ctm[4], (int)ctm[5]); p += 12;
		if(evtData) string_co2db(evtData, 8, p);			
		else	memset(p, ' ', 17);
		p += 17;
		sprintf(p, "%03d", sys_cfg->DeviceID); p += 3;
		no = _EvtGetSerialNo();
		sprintf(p, "%08d", no); p += 8;
		memset(p, 'x', 19);
		SDCardAddEvtFile(ctm, buf);
	}
}

/*
int _CodeEvent(unsigned char *buf, int Address, int DataSize)
{
	unsigned char	*p, c;
	short	length;
	int		i, size;

	p = buf;
	*p++ = ASCII_ACK; *p++ = ASCII_STX;
	length = DataSize + 9;
	SHORTtoBYTE(length, p); p += 2;
	term2addr(Address, p); p += 2; 
	*p++ = (unsigned char)P_EVENT_OLD;
	p += DataSize;
	size = p - buf;
	length = size + 2;
	SHORTtoBYTE(length, buf+2);
	for(c = i = 0;i < size;i++) c += buf[i];
	*p++ = c;
	*p++ = ASCII_ETX;
	return p - buf;
}

#include "sio.h"

void EventAdd(int EventID, unsigned char *ctm, char *DigitString)
{
	unsigned char buf[32];
	int		rval;

	rval = evtMakeDigit(buf+7, EventID, ctm, DigitString);
	if(!rval) return;
	rval = _CodeEvent(buf, (int)sys_cfg->DeviceID, 16);
	sioWrite(1, buf, rval);
}

void EventAddRaw(int EventID, unsigned char *ctm, char *data)
{
	unsigned char	buf[32];
	int		rval;
	
	rval = evtMakeRaw(buf+7, EventID, ctm, data);
	rval = _CodeEvent(buf, (int)sys_cfg->DeviceID, 16);
	sioWrite(1, buf, rval);
}
*/
static int _EvtFileIsOpened(void)
{
	if(w_count < 0) return 0;
	else	return 1;
}

char *GetResultMsg(int MsgIndex);

static _SDMemoryError(void)
{
	DesktopSetStat(GetResultMsg(R_SD_DISK_ERROR_CHECK));
}

static _SDMemoryNoError(void)
{
	DesktopSetStat(NULL);
}

static char	dir_evt[12] = "/DataEvt";

//static FIL	file2;

int SDCardOpenEvtFile(unsigned char *ctm)
{
    DIR		dir;
	FRESULT	res;
	char	*p, path[64];
	unsigned long	offset;

	strcpy(path, dir_evt);
	res = f_opendir(&dir, path);
	if(res == FR_NO_PATH) res = f_mkdir(path);
	if(res) {
#ifdef SD_DEBUG
cprintf("Evt f_opendir(%s) error: %d\n", path, res);
#endif
		_SDMemoryError();
		w_yy = w_mm = w_count = -1;
		return -1;
	}
	w_yy = ctm[0]; w_mm = ctm[1];
	p = path; p += strlen(path); *p++ = '/';
	sprintf(p, "evt-%02d%02d.txt", w_yy, w_mm);
	res = f_open(&file, path, FA_OPEN_ALWAYS | FA_WRITE);
	if(res) {
#ifdef SD_DEBUG
cprintf("Evt f_open(%s) error: %d\n", path, res);
#endif
		_SDMemoryError();
		w_yy = w_mm = w_count = -1;
		return -1;
	}
	offset = file.fsize;
	f_lseek(&file, 	offset);
	if(offset > 0) w_count = offset / EVT_RECORD_SIZE;
	else	w_count = 0;
/*
res = f_open(&file2, "/fpresult", FA_OPEN_ALWAYS | FA_WRITE);
if(res) {
#ifdef SD_DEBUG
cprintf("Evt f_open(fpresult) error: %d\n", res);
#endif
} else {
	offset = file2.fsize;
	f_lseek(&file2, offset);
}
*/
	_SDMemoryNoError();
cprintf("%s opened: Count=%d Size=%d\n", path, w_count, offset);
	return 0;
}

void SDCardCloseEvtFile(void)
{
	if(_EvtFileIsOpened()) {
		f_sync(&file);
		f_close(&file);
//f_sync(&file2);
//f_close(&file2);
		_SDMemoryNoError();
cprintf("Evt closed...\n");
	}
	w_yy = w_mm = w_count = -1;
}
/*
void FPResultSave(unsigned char *data, int size)
{
	FRESULT	res;
	unsigned int	br;
unsigned long	*pFPID;

  	if(_EvtFileIsOpened()) {
		res = f_write(&file2, data, size, &br);
		f_sync(&file2);
pFPID = (unsigned long *)data;
if(*pFPID == 0xffffffff) cprintf("Save FPResult: Fail\n");
else	cprintf("Save FPResult: ID=[%ld-%ld]\n", (*pFPID)>>1, (*pFPID)&1);
	}		
}
*/
void SDCardAddEvtFile(unsigned char *ctm, unsigned char *data)
{
	FRESULT		res;
	unsigned int	br;

	if(w_yy != ctm[0] || w_mm != ctm[1]) {
		SDCardCloseEvtFile();
		SDCardOpenEvtFile(ctm);
	}
	if(!_EvtFileIsOpened()) return;
	data[EVT_RECORD_SIZE-2] = '\r'; data[EVT_RECORD_SIZE-1] = '\n';
	res = f_write(&file, data, EVT_RECORD_SIZE, &br);
	if(res) {
//cprintf("Evt write() error: %d\n", res);
		SDCardCloseEvtFile();
	} else {
		w_count++;
//cprintf("Evt write: Count=%d\n", w_count);
		if(!(w_count & 7)) {
//cprintf("%d %d-%d\n", file.fptr, file.fptr/512, file.fptr&511); 		
			f_sync(&file);
		}
	}
}
/*
#include <stdio.h>
#include <stdarg.h>

void SDCardAddLog(char *fmt, ...)
{
	va_list	arg;
	unsigned char	ctm[8];
	char	*p, buf[256];
	FRESULT		res;
	unsigned int	br;
	int		No;

	No = sdiGetState();
	if(No > 2 && _EvtFileIsOpened()) {
		rtcGetDateTime(ctm);
		p = buf;
		sprintf(p, "%02d:%02d:%02d ", (int)ctm[3], (int)ctm[4], (int)ctm[5]); p += strlen(p);
		va_start(arg, fmt);
		vsprintf(p, fmt, arg);
		p += strlen(p);
		*p++ = '\r'; *p++ = '\n'; *p = 0;
		res = f_write(&file, buf, strlen(buf), &br);
		if(res) {
//cprintf("Evt write() error: %d\n", res);
			SDCardCloseEvtFile();
		} else {
//cprintf("%d %d-%d\n", file.fptr, file.fptr/512, file.fptr&511); 		
			f_sync(&file);
		}	
		va_end(arg);
	}
}
*/
