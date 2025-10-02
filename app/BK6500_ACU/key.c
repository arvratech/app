#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "system_hdr.h"
////////////////////////////////
#include "cotask.h"
#include "rtc.h"
#include "key.h"

static void (*keyCallBack)(void);

static int	KeyReturn,  ScanCode, ScanMaskCode;

const unsigned char keymap_val[4][5] = {
	KEY_F1,   '1',      '4',      '7',      '*',
	KEY_F2,   '2',      '5',      '8',      '0',
	KEY_F3,   '3',      '6',      '9',      '#',
	KEY_F4, KEY_CALL, KEY_MENU, KEY_CANCEL, KEY_OK
};

void keyInit(void (*KeyCallBack)())
{
	rGPBCON = (rGPBCON & 0xffc3c3ff) | 0x00141400;	// GPB10-GPB9[21:18], GPB6-GPB5[13:10}=01(Output):X3-X0
	rGPDCON &= 0x003fffff;							// GPD15-GPD11[31:22]=00(Input):Y4-Y0
	KeyReturn = ScanCode = ScanMaskCode = 0;
	keyCallBack = KeyCallBack;
}

void keySetCallBack(void (*KeyCallBack)())
{
	keyCallBack = KeyCallBack;
}

static int _keyMap(int code)
{
	int		c, i, j;

	for(i = 0;i < 4;i++) {
		c = code & 0x1f;
		code >>= 5;
		for(j = 0;j < 5;j++) {
			if(c & 1) break;
			c >>= 1;
		}
		if(j < 5) break;
	}
	c = keymap_val[i][j];
	return c;
}

static void _keyWrite(int row)
{
	unsigned long	rVal;

	rVal = rGPBDAT & 0xfffff99f;
	switch(row) {
	case 0:
		rGPBDAT = rVal | 0x00000640;	//	GPB10-GPB9,GPB6-GPB5(X3-X0) = 1 1 1 0
		break;
	case 1:
		rGPBDAT = rVal | 0x00000620;	//	GPB10-GPB9,GPB6-GPB5(X3-X0) = 1 1 0 1
		break;
	case 2:
		rGPBDAT = rVal | 0x00000460;	//	GPB10-GPB9,GPB6-GPB5(X3-X0) = 1 0 1 1
		break;
	case 3:
		rGPBDAT = rVal | 0x00000260;	//	GPB10-GPB9,GPB6-GPB5(X3-X0) = 0 1 1 1
		break;
	}
}

static int _keyRead(int row)
{
	int		c;

	c = rGPDDAT & 0x0000f800;		// GPD15-GPD11(Y4-Y0)
	c >>= 11;
	c = (~c) & 0x0000001f;
	return c;
}

static int	ScanKey;
static unsigned long	KeyTimer;

/*
 *        Start            2000ms             End
 *   -------|----------------|----------------|-------
 *      0  KEY      0     KEY_LONG    0       |   0
 */ 
void ctKey(void *arg)
{
	int		i, c, code, max_row, shift;
	
	max_row = 4;
	c = 0;
	for(i = shift = 0;i < max_row;i++, shift += 5) {
		_keyWrite(i);
		//for(i = 0;i < KEY_REG_DELAY;i++) ;	// 60 이면 키가 2번
		taskDelayTick(1);
		c += _keyRead(i) << shift;
	}
	code = ScanCode;
	ScanCode = c;
	if(c) c &= (~code);
//cprintf("%d [%05x - %05x] %05x\n", c, code, ScanCode, ScanMaskCode);
	if(c) {
		ScanKey = KeyReturn = _keyMap(c); ScanMaskCode = c;
		KeyTimer = TICK_TIMER;
	} else if(ScanMaskCode && ScanMaskCode == code && ScanMaskCode == ScanCode && (TICK_TIMER-KeyTimer) > 128) { 
		KeyReturn = ScanKey + 0x80; ScanMaskCode = 0;
	}
}

// Return	0=No key  >0=Key	
int keyGet(void)
{
	int		c;

	c = KeyReturn;
	KeyReturn = 0;
	if(c && keyCallBack) (*keyCallBack)();
//if(c) cprintf("KEY=%02x [%c]\n", c, c);
	return c;
}

int keyGetScanCode(void)
{
	if(ScanCode) return _keyMap(ScanCode);
	else	return 0;
}

// timeout	-1=Forever  0=No wait >0=wait in deci-seconds
// Return	0=No key/Timeout  >0=Key		
int keyGetEx(int timeout)
{
	unsigned long	timer;
	int		c;

	if(timeout > 0) timer = DS_TIMER + timeout;
	while(1) {
		c = KeyReturn;
		KeyReturn = 0;
		if(c) {
			if(keyCallBack) (*keyCallBack)();
			break;
		} else { 
			if(timeout == 0 || timeout > 0 && DS_TIMER >= timer) break;
			else	taskYield();
		}
	}
	return c;
}

// timeout	-1=Forever  0=No wait >0=wait in deci-seconds
// Return	0=No key/Timeout  >0=Key		
int keyGetWithExternalCancel(int timeout, int (*ExternalCallBack)())
{
	unsigned long	timer;
	int		c;

	if(timeout > 0) timer = DS_TIMER + timeout;
	while(1) {
		if(ExternalCallBack && (*ExternalCallBack)()) {
			c = KEY_CANCEL;
			break;
		}
		c = KeyReturn;
		KeyReturn = 0;
		if(c) {
			if(keyCallBack) (*keyCallBack)();
			break;
		} else { 
			if(timeout == 0 || timeout > 0 && DS_TIMER >= timer) break;
			else	taskYield();
		}
	}
	return c;
}

void keyFlush(void)
{
	KeyReturn  = 0;
}
