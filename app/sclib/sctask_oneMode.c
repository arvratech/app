#include "plib.h"
#include "pic32lib.h"
#include "iodev.h"
#include "rtc.h"
#include "prim.h"
#include "as3911.h"
#include "sc_iso15693_2.h"
#include "sc_iso15693_3.h"
#include "sctask_com.h"
#include "spi_slv.h"
#include "sctask.h"

static unsigned char  scTaskState, scTaskMode, scSleep;

#define	SC_TS_NULL			0
#define	SC_TS_IDLE			1
#define	SC_TS_ACTIVE		2
#define SC_TS_EXIT			3

#define SC_ISO14443A       1
#define SC_ISO14443B       2
#define SC_ISO15693        3


void scInit(void)
{
	scTaskState = SC_TS_NULL; scTimeout = 0;
	scSleep = 0;
}

iso15693PhyConfig_t _iso15693cfg;

static void _scOpenMode(void)
{
	switch(scTaskMode) {
	case SC_ISO14443A:	iso14443AOpen(); break;
	case SC_ISO14443B:	iso14443BOpen(); break;
	case SC_ISO15693:
		_iso15693cfg.coding = ISO15693_VCD_CODING_1_4;
		_iso15693cfg.mi = ISO15693_MODULATION_INDEX_10;
//		_iso15693cfg.mi = ISO15693_MODULATION_INDEX_OOK;
		iso15693Open(&_iso15693cfg); break;
	}
}

static void _scCloseMode(void)
{
	switch(scTaskMode) {
	case SC_ISO14443A:	iso14443AClose(); break;
	case SC_ISO14443B:	iso14443BClose(); break;
	case SC_ISO15693:  iso15693Close(); break;  
	}
	scTaskMode = 0;
}

static void _ToIdleTask(void)
{
	unsigned char   mode;
 
	if(crGetSCReaderEnable()) {
		mode = crGetSCReaderMode();
		if(mode == 2) mode = SC_ISO15693;
		else if(mode == 1) mode = SC_ISO14443B;
		else    mode = SC_ISO14443A;
		if(mode != scTaskMode) {
			_scCloseMode();
			scTaskMode = mode;
			_scOpenMode();
		}
		scOption = crGetSCReaderOption();
		scTaskState = SC_TS_IDLE; scTimer = SC_TIMER; scTimeout = 100;      
	} else {
		scClose();
	}
}

int scOpen(void)
{
	unsigned char	val;
	int		rval;
	
	as3911OpenCom();
	as3911GetReg(AS3911_REG_IC_IDENTITY, &val);		// 0x0b: AS3991B r3.3
printf("AS3911 IC ID=%02x\n", (int)val);
	rval = as3911Open();
	if(rval) {
		as3911Close();
		as3911CloseCom();
		scTimer = SC_TIMER; scTimeout = 5000;
	} else {
        scTaskMode = 0;
		_ToIdleTask();
 	}
printf("scOpen...%d\n", rval);	
	scInitData();
	return rval;
}

void scClose(void)
{
    _scCloseMode();
	scInitData();
	as3911Close();
	as3911CloseCom();	
	scTaskState = SC_TS_NULL; scTimeout = 0;
printf("scClose...\n");
}

void scSetSleepMode(int sleepMode)
{
	if(sleepMode) {
		scSleep = 1;
	} else {
		scSleep = 0;
        _ToIdleTask();
	}
}

void scTask(void)
{
	int		n;

	switch(scTaskState) {
	case SC_TS_ACTIVE:		
		switch(scTaskMode) {
		case SC_ISO14443A:	iso14443AActiveTask(); break;
		case SC_ISO14443B:	iso14443BActiveTask(); break;
		case SC_ISO15693:	iso15693ActiveTask(); break;
		}
		if(!scState) {
    		switch(scTaskMode) {
        	case SC_ISO14443A:	n = iso14443ACardPresence(); break;
            case SC_ISO14443B:	n = iso14443BCardPresence(); break; 
            case SC_ISO15693:	n = iso15693CardPresence(); break; 
            }
            if(n) {
printf("Exit start..........\n");
                scTaskState = SC_TS_EXIT; scState = SC_OFF;
            } else  _ToIdleTask();
  		} else if(scState == SC_FATAL) scClose();
		break;
	case SC_TS_EXIT:
		switch(scTaskMode) {
		case SC_ISO14443A: iso14443AExitTask(); break;
		case SC_ISO14443B:	iso14443BExitTask(); break;
		case SC_ISO15693:	iso15693ExitTask(); break;
		}
		if(!scState) {
rintf("Exit end  ..........\n");
			_ToIdleTask();
		} else if(scState == SC_FATAL) scClose();
		break;
	case SC_TS_IDLE:
        if(scSleep) {
        	if(scTaskMode) _scCloseMode();
		} else if((int)(SC_TIMER-scTimer) > scTimeout) {
            scTaskState = SC_TS_ACTIVE; scState = SC_OFF;
		}
		break;
	default:
		n = crGetSCReaderEnable();
		if(n && (!scTimeout || (int)(SC_TIMER-scTimer) > scTimeout)) scOpen();
		else if(!n && scTimeout) scTimeout = 0;
	}
}
