#include <plib.h>
#include "pic32lib.h"
#include "iodev.h"
#include "rtc.h"
#include "prim.h"
#include "mfrc.h"
#include "mfrc_io.h"
#include "cardcrypto.h"
#include "spi_slave.h"
#include "crmifare.h"

#define SC_NULL				0
#define SC_OFF				1
#define SC_IDLE				2
#define SC_READY			3
#define SC_ACTIVE			4
#define SC_DONE_IDLE		5
#define SC_DONE_READY		6

#define FIELD_OFF_TIMER		100		// 100ms

static unsigned char  scState, scCount, scScore, scDoneMode, scMode, scPosition, scRetry, scN, scTimeslot, sak, uid[12];
static unsigned long  scTimer;
static int			  scTimeout;
static unsigned char  cds_pend[MAX_CARD_SIZE][18];
static unsigned char  cds_done[MAX_CARD_SIZE][18];


static void cdsClear(void)
{
	int		i;

	for(i = 0;i < MAX_CARD_SIZE;i++) cds_pend[i][0] = 0;
//printf("clear....\n");
}

void crmifareInitTask(void)
{
	scState = SC_NULL;
	scTimer = MFRC_TIMER - 5001;
}

int crmifareOpen(void)
{
	int		i, rval;

	mfrcOpen();
	rval = mfrcInit();
	if(!rval) {
		uDelay(1500);
		rval = mfrcInit();
	}
	cdsClear();
	for(i = 0;i < MAX_CARD_SIZE;i++) cds_done[i][0] = 0;
	if(rval) {
		mfrcClose();
		scState = SC_NULL; scTimer = MFRC_TIMER;
	} else {
		PcdFieldOff();
		scState = SC_OFF; scTimer = MFRC_TIMER;
	}
printf("OpenMifare...%d\n", rval);
	return rval;
}

void crmifareClose(void)
{
	int		i;

	cdsClear();
	for(i = 0;i < MAX_CARD_SIZE;i++) cds_done[i][0] = 0;
	PcdFieldOff();
	mfrcClose();
	scState = SC_NULL;
printf("CloseMifare...S=%d\n", (int)scState);
}

static void _scToOff(void)
{
	if(FIELD_OFF_TIMER) PcdFieldOff();
	scState = SC_OFF; scTimer = MFRC_TIMER;
}

static void _scToIdle(void)
{
	scState = SC_IDLE; scTimer = MFRC_TIMER; scTimeout = 5;		// 5ms
}

static void _scToDoneIdle(void)
{
	scState = SC_DONE_IDLE; scTimer = MFRC_TIMER; scTimeout = 5;	// 5ms
}

static void _scCaptured(unsigned char *data, int len)
{
	int		i, found;

	PiccHalt();
	if(len) {
		found = 0;
		for(i = 0;i < MAX_CARD_SIZE;i++) {
			if(!cds_pend[i][0]) break;
			if(cds_pend[i][0] == len && !memcmp(&cds_pend[i][1], data, len)) {
				found = 1;
printf("Found [%d] Len=%d %02x...\n", i, (int)cds_pend[i][0], (int)cds_pend[i][1]);
				break;
			}
		}
		if(!found) {
			for(i = 0;i < MAX_CARD_SIZE;i++)
				if(!cds_pend[i][0]) break;
			if(i >= MAX_CARD_SIZE) i = 0;
			cds_pend[i][0] = len; memcpy(&cds_pend[i][1], data, len);
printf("Added [%d] Len=%d %02x-%02x-%02x-%02x...\n", i, (int)cds_pend[i][0], (int)cds_pend[i][1], (int)cds_pend[i][2], (int)cds_pend[i][3], (int)cds_pend[i][4]);
		}
	}
	_scToIdle();
	uid[0] = 0;
}

static void _scDone(void)
{
	int		i;

	for(i = 0;i < MAX_CARD_SIZE;i++) {
		if(!cds_pend[i][0]) break;
		memcpy(cds_done[i], cds_pend[i], 18);
	}
	if(i < MAX_CARD_SIZE) cds_done[i][0] = 0;
	cdsClear();
	if(i) scDoneMode = scMode;
//	scPrevCount = scCount;
//printf("cdDone...%d\n", scCount);
	scScore = 0;
	_scToDoneIdle();
}

static void _scError(void)
{
	int		val;

	val = cds_pend[0][0];
printf("_PiccError: Pend=%d Count=%d\n", val, (int)scCount);
	if(val && scCount >= 2 || !val && scCount >= 10) _scDone();
	else	_scToIdle();
}

unsigned char scNs[4] = { 1, 2, 4, 8 };
/*
 *     X:Request Timeout  E:Card readed, but error occurred  O:Card is sucessfully readed
 *  X X
 *  E E X X
 *  E X E E E E E E E E E
 *  O X X
 *  O E E
 *  E E E O E E
 */
void crmifareTask(void)
{
	unsigned char	*p, buf[60], temp[8];
	int		rval, len, scok;

//printf("%d %ld d=%d\n", (int)scState, MFRC_TIMER, MFRC_TIMER-scTimer);
	rval = crGetSCReaderEnable();
	if(scState && !rval) {
		crmifareClose();
	} else if(!scState && rval && (int)(MFRC_TIMER-scTimer) > 5000) {
		crmifareOpen();
	}
	if(!scState) return;
	switch(scState) {
	case SC_OFF:
		if((int)(MFRC_TIMER-scTimer) > FIELD_OFF_TIMER) {
			PcdFieldOn();
			_scToIdle(); scTimeout = 16;	// if below 300, read continually.
			scMode = crGetSCReaderMode(); scPosition = crGetSCPosition();
			scCount = scScore = scN = 0; scTimeslot = 1; uid[0] = 0; 
		}
		break;
	case SC_IDLE:
		if((int)(MFRC_TIMER-scTimer) > scTimeout) {
			if(scTimeslot > 1) PiccSendTimeslotMarker((int)scTimeslot);
			else	PiccSendRequestB(ISO14443_3_REQIDL, (int)scN);
			scState = SC_READY;
		}
		break;
	case SC_READY:
		rval = PiccReceiveRequestB(buf);
//if(rval >= 0) printf("PiccReceiveRequest=0x%x\n", rval);
		if(!rval) {
			rval = PiccAttrib(uid, 0);
			if(!rval) {
				scCount++;
printf("%d SE UID=[%02x", (int)scRetry, (int)uid[1]); for(rval = 1;rval < uid[0];rval++) printf("-%02x", uid[rval+1]); printf("]\n");
				scState = SC_ACTIVE;
			} else {
				PiccHalt(); _scError(); uid[0] = 0;
			}
			scScore = 0;
		} else if(rval == STATUS_LOWPOWER) {
			crmifareClose();
		} else if(rval == STATUS_CRC_ERROR) {
			if(scTimeslot == 1) {
				sc++; if(scN > 3) scN = 3;
			} else	scTimeslot = 1;
			_scToIdle();
		} else if(rval > 0) {
			if(rval == 1) scScore += 2;
			else	scScore++;
			if(scScore < 2 || scN == 0 && scScore < 4) _scToIdle();
			else if(scTimeslot < scNs[scN]) { scTimeslot++; scScore = 0; _scIdle(); }
			else if(scCount) _scDone();
			else	_scToOff();
		}
		break;
	case SC_ACTIVE:
		scok = 0;
		rval = PiccAnswerToSelect();
		if(!rval) {
			p = crGetISO7816Command();
			if(crGetGetAIDLength() > 0) {
				rval = PiccSelectAID(crGetAID(), crGetGetAIDLength(), buf, &len);
				if(!rval) {
					if(p[0] == 0 && len >= crGetSCPosition()+18) {
						memcpy(buf, buf+crGetSCPosition(), 17); scok = 2;
					} else if(p[1] && !rval && len >= 2 && buf[len-2] == 0x90 && buf[len-1] == 0x00) scok = 1;
				}
			} else	scok = 1;
			if(scok == 1) {
				rval = PiccCommand7816(p, buf, (int)crGetSCPosition());
				if(!rval) scok = 2;
			}
		}
		if(scok > 1) {
			PiccDeselect();
			_scCaptured(buf, 16);
		} else {
			_scError();
			if(cds_pend[0][0]) {
				PiccDeselect(); PiccHalt();
			} else {
				PcdFieldOff(); PcdFieldOn();
			}
		}
		break;
	case SC_DONE_IDLE:
		if((int)(MFRC_TIMER-scTimer) > scTimeout) {
			PiccSendRequestB(ISO14443_3_REQALL, 0);
			scState = SC_DONE_READY;
		}
		break;
	case SC_DONE_READY:
		rval = PiccReceiveRequestB(buf);
//if(rval >= 0) printf("PiccReceiveRequest=0x%x\n", rval);
		if(!rval) {
			PiccHalt(); _scToDoneIdle();
			scScore = 0;
		} else if(rval == STATUS_LOWPOWER) {
			crmifareClose();
		} else if(rval > 0) {
			if(rval == 1) scScore += 2;
			else	scScore++;
			if(scScore < 4) _scToDoneIdle();
			else	_scToOff();
		}
		break;
	}
}

int crmifarePeek(void)
{
	if(cds_done[0][0]) return 1;
	else	return 0;
}

int crmifareRead(unsigned char *Data)
{
	unsigned char	*p;
	int		i, val;

	p = Data;
	*p++ = scDoneMode;
	for(i = 0;i < MAX_CARD_SIZE;i++) {
		val = cds_done[i][0];
		if(!val) break;
		memcpy(p, &cds_done[i], val+1); p += val+1;
	}
	cds_done[0][0] = 0;
	val = p - Data;
	if(val < 2) val = 0;
	return val;
}

void crmifareClear(void)
{
	cds_done[0][0] = 0;
}
