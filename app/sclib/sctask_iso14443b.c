#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "sc_iso14443b.h"
#include "sccfg.h"
#include "sc_iso14443_4.h"
#include "sctask_com.h"

static unsigned char  scScore, scSlotNo, scSlotCount, pupi[4];


static void _scbToFatal(void)
{
printf("Fatal error...........\n");    
	scState = SC_FATAL;
}

static void _scbToOff(void)
{
	scState = SC_OFF;
}

static void _scbToIdleNoWait(void)
{
    if(scSlotNo >= scSlotCount) {
        scState = SC_IDLE;
    } else {
        scSlotNo++; scState = SC_ANTICOLLSION;
    }
}

static void _scbToIdle(void)
{
    if(scSlotNo >= scSlotCount) {
        scState = SC_IDLE; ms_sleep(10);	// 5ms
    } else {
        scSlotNo++; scState = SC_ANTICOLLSION;
    }
}

static void _scbHalt(void)
{
    if(scState >= SC_ACTIVE) iso14443_4Deselect(0);
     else    iso14443BHalt(pupi);    // set PICC to halt state
	_scbToIdle();
}

int  _Iso14443_4Task(void);

void iso14443BActiveTask(void)
{
   int     rval;

	switch(scState) {
	case SC_OFF:
		if(scOpenMode) iso14443AClose();
		iso14443BOpen(); scOpenMode = SC_ISO14443B;
		ms_sleep(10);
		_scbToIdleNoWait();
		scSlotCount = 1; scSlotNo = 1; _scbToIdle();
		scCount = scScore = 0;
		break;
	case SC_IDLE:
		//iso14443BSendRequest(ISO14443B_CMD_REQB, CrAFI());
		scSlotCount = 1; scSlotNo = 1;
		scState = SC_REQUEST;
/*/
rval = iso14443BAntiCollsion(ISO14443B_CMD_REQB, ISO14443B_SLOT_COUNT_1, CrAFI(), pupi);
printf("iso14443BAntiCollsion=%d\n", rval);
if(!rval) {
    scState = SC_READY;
    scCount++; scScore = 0;
} else {
     _scbToIdle();
 }
 */
		break;
    case SC_REQUEST:
		rval = iso14443BReceiveRequest(pupi);
        if(!rval) {
            scState = SC_READY;
            scCount++; scScore = 0;
		} else if(rval == ERR_CRC || rval == ERR_PARITY || rval == ERR_HARD_FRAMING|| rval == ERR_SOFT_FRAMING) {
            scSlotCount = 4; scSlotNo = 0;
			_scbToIdle();
			scScore = 0;
		} else if(rval == ERR_CHIP) {
			_scbToFatal();
		} else if(rval > 0) {
			if(rval == 1) scScore += 2;
			else	scScore++;
			if(scScore < 4) _scbToIdle();
			else  {
            	if(scCount) scDoneData();
                _scbToOff();
            }
        }
		break;
    case SC_ANTICOLLSION:
        if(scSlotNo == 1) rval = iso14443BAntiCollsion(ISO14443B_CMD_REQB, ISO14443B_SLOT_COUNT_4, CrAFI(), pupi);
        else    rval = iso14443BSlotMarker(scSlotNo, pupi);
        if(!rval) {
            scState = SC_READY;
            scCount++; scScore = 0;
        } else {
            _scbToIdle();
        }
        break;
    case SC_READY: 
		rval = iso14443BEnterProtocolMode(pupi);
		if(rval) _scbToIdle();
        else {
            scState = SC_ACTIVE; scSubState = 0;
    	}
        break;
    case SC_ACTIVE:
        rval = _Iso14443_4Task();
        if(!rval) _scbHalt();
        else if(rval > 0) _scbToIdle();
        else    scSubState++;
		break;
	}
}

void iso14443BExitTask(void)
{
	unsigned char	buf[12];
	int		rval;

	switch(scState) {
	case SC_OFF:
		if(scOpenMode != SC_ISO14443B) {
			if(scOpenMode) iso14443AClose();
			iso14443BOpen(); scOpenMode = SC_ISO14443B;
		}
		_scbToIdleNoWait();
		scScore = 0;
		break;
	case SC_IDLE:
		iso14443BSendRequest(ISO14443B_CMD_WUPB, CrAFI());
		scState = SC_REQUEST;
		break;
	case SC_REQUEST:
		rval = iso14443BReceiveRequest(buf);
if(rval == ERR_TIMEOUT) printf("ReceiveRequest: Timeout\n");
		if(!rval || rval == ERR_COLLISION || rval == ERR_PARITY) {
           _scbToIdle();
			scScore = 0;
		} else if(rval == ERR_CHIP) {
			_scbToFatal();
		} else if(rval > 0) {
			if(rval == 1) scScore += 2;
			else	scScore++;
			if(scScore < 4) _scbToIdle();
			else	_scbToOff();
		}
		break;
	}
}

