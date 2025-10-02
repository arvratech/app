#include <string.h>
#include "cbuf.h"
#include "prim.h"
#include "rtc.h"
#include "log.h"
#include "spis.h"
#include "sqnode.h"
#include "slvnet_sio_hw.h"
#include "slvnet_sio.h"

SLAVE		*slave, slaves[MAX_UNIT_SZ+MAX_SUBDEV_SZ];
volatile unsigned char slaveSize, slaveIndex;
unsigned char	_unitModels[MAX_UNIT_SZ], _unitSubdevModels[MAX_UNIT_SZ];
unsigned char	crc8tab[256], crc16tab_h[256], crc16tab_l[256];
unsigned char	sioState, devMode, slvReset;
int			Trxdelay, Ttxdelay, Tinterval;
unsigned long	sioLoop, oldSioLoop, sioLoopTimer;

#define StartTimer(timeout)	  { OpenTimer1(T1_ON | T1_IDLE_CON | T1_TMWDIS_OFF | T1_GATE_OFF | T1_PS_1_256 | T1_SYNC_EXT_OFF | T1_SOURCE_INT, ((timeout * 48) >> 8) - 1); EnableIntT1; }
#define StopTimer()		CloseTimer1()

void InitCRC8(unsigned char *crc8tab);
void InitCRC16_CCITT(unsigned char *crc16tab_h, unsigned char *crc16tab_l);

void slvnetSioOpen(long baudRate)
{
	SLAVE	*slv;
	int		i;

	InitCRC8(crc8tab);
	InitCRC16_CCITT(crc16tab_h, crc16tab_l);
	//			  	1char(us)		2char(us)		3char(us)
	//   9600		1041.667		2083.000		3125.000
	//  19200		 520.833		1041.667		1562.500
	//  38400		 260.417		 520.833		 781.250
	//  57600		 173.611		 347.222		 520.833
	// 115200		  86.806		 173.611		 260.417
	switch(baudRate) {	// 1 char        3 char
	case   9600L:  Ttxdelay = 1042; Trxdelay = 2083; Tinterval = 3125; break;
	case  19200L:  Ttxdelay =  521; Trxdelay = 1042; Tinterval = 1563; break;
	case  38400L:  Ttxdelay =  261; Trxdelay =  521; Tinterval =  782; break;
	case  57600L:  Ttxdelay =  174; Trxdelay =  348; Tinterval =  521; break;
	case 115200L:  Ttxdelay =   87; Trxdelay =  174; Tinterval =  261; break;
	}
//	for(i = 0;i < MAX_SLVDEV_SZ;i++) {
//		confTxSizes[i][0] = confTxSizes[i][1] = 0;
//	}
	devMode = devMode();
	if(devMode == 0) {	// Master
		for(i = 0, slv = slaves;i < MAX_UNIT_SZ+MAX_SUBDEV_SZ;i++, slv++) {
			slv->commErrorCount = slv->crcErrorCount = slv->timeoutCount = 0L;
		}
		slaveSize = slaveIndex = 0;
		memset(_unitModels, 0xff, MAX_UNIT_SZ);
		memset(_unitSubdevModels, 0x00, MAX_UNIT_SZ);		
		sioState = 0;
		sioLoop = oldSioLoop = 0L;
		sioLoopTimer = MS_TIMER;
		StartTimer(POLL_TIMEOUT);
cprintf("slvnetSioOpen: %lu master\n", baudRate);
	} else if(unitSubdevModel(0,0)) {	// Slave
		slave = slaves;
		slave->state		= 0;
		slave->address		= 0xff;	
		slave->txSN			= 0;
		slave->rxSN			= 1;
		slave->dataRetryCount = 0;
		slave->commErrorCount = slave->crcErrorCount = slave->timeoutCount = 0L;
		unitSubdevDisconnected(0, 0);
		sioState = 2;
		sioLoop = oldSioLoop = 0L;
		sioLoopTimer = MS_TIMER;
		StartTimer(Trxdelay);
cprintf("slvnetSioOpen: %lu slave=%d\n", baudRate, (int)devSubdeviceId());
	} else {
		sioState = 0xff;
cprintf("slvnetSioOpen: none\n");
	}
	slvnetSioHwOpen(baudRate);
}

void slvnetSioClose(void)
{
	if(sioState != 0xff) {
		if(devMode) slvnetSiosSubdevDisconnect(slave);
		else	slvnetSiomSubdevDisconnect();
cprintf("slvnetSioClose...\n");
		slvnetSioHwClose();
		sioState = 0xff;
	}
}

void slvnetSioCheckSanity(void)
{
	if(sioState == 0xff) return;
	if(oldSioLoop == sioLoop) {
		 if((MS_TIMER-sioLoopTimer) > 2000) {
			//failCount++;
			//sioReset();
cprintf("%lu sio fatal error, reset...\n", MS_TIMER);
			appClose();
			devSetSystemStatus(SS_NON_OPERATIONAL);
			while(1)  ;		// reset system
		}
	} else {
		oldSioLoop = sioLoop;
		sioLoopTimer = MS_TIMER;	
	}
}
