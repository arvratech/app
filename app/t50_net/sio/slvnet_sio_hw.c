#include <plib.h>
#include <string.h>
#include "pic32lib.h"
#include "cbuf.h"
#include "prim.h"
#include "rtc.h"
#include "log.h"
#include "spis.h"
#include "sqnode.h"
#include "slvnet_sio.h"


extern SLAVE		*slave, slaves[];
extern volatile unsigned char slaveSize, slaveIndex;
extern unsigned char  _unitModels[], _unitSubdevModels[];
extern unsigned char  crc8tab[], crc16tab_h[], crc16tab_l[];
extern unsigned char	sioState, devMode, slvReset;
extern volatile unsigned long	slvLoop; 
extern int			Trxdelay, Ttxdelay, Tinterval;
static unsigned char	retryCount, globalTsmSubdevId;
static volatile int	rxCount, txCount, txSize;
static unsigned char	rxBuf[MAX_IBUF_SZ], txBuf[MAX_IBUF_SZ];
volatile unsigned long	sioLoop; 


// PrescalValue / pbclk = PrecaleValue * 1,000,000 / 40,000,000 
//	1=0.025us  2=0.05us  4=0.1us  8=0.2us  16=0.4us  32=0.8us  64=1.6us  256=6.4us
// PrescalValue / pbclk = PrecaleValue * 1,000,000 / 40,000,000 
//	1=0.020833us  2=0.04166us  4=0.08333us  8=0.1666us  16=0.3333us  32=0.6666us  64=1.3333us  256=5.3333us
#define StartTimer(timeout)	  { OpenTimer1(T1_ON | T1_IDLE_CON | T1_TMWDIS_OFF | T1_GATE_OFF | T1_PS_1_256 | T1_SYNC_EXT_OFF | T1_SOURCE_INT, ((timeout * 48) >> 8) - 1); EnableIntT1; }
#define StopTimer()		CloseTimer1()
#define _putc(c) { if((c>>4) <= 9) rs_putch((c>>4)+'0'); else rs_putch((c>>4)+'a'-10); if((c&0xf) <= 9) rs_putch((c&0xf)+'0'); else rs_putch((c&0xf)+'a'-10); }

#include "slvnet_siom_unit.c"
#include "slvnet_siom_subdev.c"
#include "slvnet_sios_subdev_hynix.c"

#define UART_MODULE		UART1
#define RS485_CTRL_H	mPORTBSetBits(BIT_7)		// RPB7=1
#define RS485_CTRL_L	mPORTBClearBits(BIT_7);		// RPB7=0


void slvnetSioHwOpen(long baudRate)
{
	mPORTFSetPinsDigitalIn(BIT_2);		// RPF2:Input(U1RX)
	mPORTFSetPinsDigitalOut(BIT_3);		// RPF3:Output(U1TX)
	CFGCONbits.IOLOCK = 0;
	PPSInput(1, U1RX, RPF2);			// Assign U1RX to pin RPF2
	PPSOutput(2, RPF3, U1TX);			// Assign U1TX to pin RPF3
	CFGCONbits.IOLOCK = 1;
	UARTConfigure(UART_MODULE, UART_ENABLE_PINS_TX_RX_ONLY);
	// FIFO size=4
	// UART_INTERRUPT_ON_RX_3_QUARTER_FULL의 경우 3개 이상이어야 INT 발생, 미만이면 시간 경과해도 INT 미발생
	UARTSetFifoMode(UART_MODULE, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY); 
	UARTSetLineControl(UART_MODULE, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
	UARTSetDataRate(UART_MODULE, pbclk, baudRate);
	UARTEnable(UART_MODULE, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
   	// Configure UART interrupt 
	// Bug fix 2019.3.21 : If set below, ADC interrpit stopped. therefore seperate INT_SOURCE_UART_RX,INT_SOURCE_UART_TX
	//INTEnable(INT_SOURCE_UART_RX(UART_MODULE) | INT_SOURCE_UART_TX(UART_MODULE), INT_DISABLED);
cprintf("%x %x %x\n", INT_SOURCE_UART_RX(UART_MODULE), INT_SOURCE_UART_TX(UART_MODULE), INT_AD1);
	INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_DISABLED);
	INTEnable(INT_SOURCE_UART_TX(UART_MODULE), INT_DISABLED);
	INTSetVectorPriority(INT_VECTOR_UART(UART_MODULE), INT_PRIORITY_LEVEL_2);
	INTSetVectorSubPriority(INT_VECTOR_UART(UART_MODULE), INT_SUB_PRIORITY_LEVEL_2);
 	ConfigIntTimer1(T1_INT_ON | T1_INT_PRIOR_3 | T1_INT_SUB_PRIOR_3);
	mPORTBSetPinsDigitalOut(BIT_7);		// RPB7:Output(RS-485)
	RS485_CTRL_L;
	retryCount = 0;
}

void slvnetSioHwClose(void)
{
	RS485_CTRL_L;
	ConfigIntTimer1(T1_INT_OFF);
	StopTimer();
	INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_DISABLED);
	INTEnable(INT_SOURCE_UART_TX(UART_MODULE), INT_DISABLED);
	UARTEnable(UART_MODULE, UART_DISABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));
}

static inline void _EnableRx(void)
{
	int		c, n;
	
	while(n = UARTGetLineStatus(UART_MODULE) & 0x0f) {
		c = UARTGetDataByte(UART_MODULE);
		if(n & UART_OVERRUN_ERROR) U1STACLR = UART_OVERRUN_ERROR;	// reset Rx FIFO, and all data in FIFO is lost.
	}
	INTClearFlag(INT_SOURCE_UART_RX(UART_MODULE));	// UxRxIF=0
	rxCount = 0;
	sioState = 3;
	INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_ENABLED);	// U1RxIE=1
}

static inline void _EnableTx(void)
{
	txCount = 0;
	RS485_CTRL_H;
	sioState = 1;
	// 아래를 수행해야 U1TxIF=1 로 설정되어 인터럽트 발생
	//	INTSetFlag(INT_SOURCE_UART_TX(UART_MODULE));
	UARTEnable(UART_MODULE, UART_DISABLE_FLAGS(UART_TX));
	UARTEnable(UART_MODULE, UART_ENABLE_FLAGS(UART_TX));
	INTEnable(INT_SOURCE_UART_TX(UART_MODULE), INT_ENABLED);
}

void __ISR(_UART1_VECTOR, IPL2SOFT) IntUart1Handler(void)
{
	unsigned char	c;
	int		n;

	if(sioState == 3 || sioState == 4) {
		if(n = UARTGetLineStatus(UART_MODULE) & 0x0f) {
			//StopTimer();
			do {
				c = UARTGetDataByte(UART_MODULE);
				if(n & UART_OVERRUN_ERROR) {	
					U1STACLR = UART_OVERRUN_ERROR;	// reset Rx FIFO, and all data in FIFO is lost.
				} else if(n & (UART_FRAMING_ERROR | UART_PARITY_ERROR)) {
				} else if(rxCount < MAX_IBUF_SZ) {
					rxBuf[rxCount] = c; rxCount++;
				}
			} while(n = UARTGetLineStatus(UART_MODULE) & 0x0f) ;
			if(rxCount) {
				sioState = 4; StartTimer(Tinterval);
			}
		} else {
rs_putch('n');
		}
		INTClearFlag(INT_SOURCE_UART_RX(UART_MODULE));	// UxRxIF=0
	} else if(sioState == 1) {
		if(txCount < txSize) {	// 맨처음에는 5(?)개가 그다음부터는 1개가 들어감
			while(UARTTransmitterIsReady(UART_MODULE)) {	// UTXBF
				UARTSendDataByte(UART_MODULE, txBuf[txCount]); txCount++;
				if(txCount >= txSize) {
					UARTSetFifoMode(UART_MODULE, UART_INTERRUPT_ON_TX_DONE | UART_INTERRUPT_ON_RX_NOT_EMPTY);
					break;
				}
			}
		} else {
			RS485_CTRL_L;
			UARTSetFifoMode(UART_MODULE, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
			INTEnable(INT_SOURCE_UART_TX(UART_MODULE), INT_DISABLED);	// U1TxIE=0
			sioState = 2;
			StartTimer(Trxdelay);
		}
		INTClearFlag(INT_SOURCE_UART_TX(UART_MODULE));	// UxTxIF=0
	}
}

// configure the Timer 1 interrupt handler
void __ISR(_TIMER_1_VECTOR, IPL3SOFT) Timer1Handler(void)
{
	SLAVE		*slv;
	int		c, n, i;

	StopTimer();
	if(devMode) {	// Slave
		if(sioState == 0) {
			txSize = _SiosSubdevTxStart(slave, txBuf);
			_EnableTx();
		} else if(sioState == 2) {
			_EnableRx();
			StartTimer(150000);	// 150ms for inactivity
		} else if(sioState == 3) {
			if(!(UARTGetLineStatus(UART_MODULE) & 0x0f)) {
				_SiosSubdevCheckInactivity(slave);
				StartTimer(150000);	// 150ms for inactivity
			}	
		} else if(sioState == 4) {
			if(!(UARTGetLineStatus(UART_MODULE) & 0x0f)) {
				_SiosSubdevCheckAddress(slave);
				n = _SiosSubdevRxCompleted(slave, rxBuf, rxCount);
				if(n) {
					slave->inactivityTimer = MS_TIMER;
					sioState = 0;
					INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_DISABLED);
					StartTimer(Ttxdelay);
				} else {
					rxCount = 0;
					sioState = 3;
					_SiosSubdevCheckInactivity(slave);
					StartTimer(150000);	// 150ms for inactivity
				}
			}
		}
	} else {		// Master
		if(sioState == 2) {
			_EnableRx();
			StartTimer(RESPONSE_TIMEOUT);
		} else if(sioState == 4) {
			if(!(UARTGetLineStatus(UART_MODULE) & 0x0f)) {
				if(slave->address < 0x20) n = _SiomUnitRxCompleted(slave, rxBuf, rxCount);
				else	n = _SiomSubdevRxCompleted(slave, rxBuf, rxCount);
				if(n) {
					INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_DISABLED);
					sioState = 0;
					StartTimer(IDLE_TIMEOUT);
				} else {
					rxCount = 0;
					sioState = 3;
					n = ReadPeriod1() - ReadTimer1(); if(n < 1) n = 1;
					StartTimer(n);
				}
			}
		} else {
			if(sioState == 3) {
rs_putch('D');
				if(!slave->rxError) slave->timeoutCount++;
				INTEnable(INT_SOURCE_UART_RX(UART_MODULE), INT_DISABLED);
				if(!slave->state || retryCount >= 3) {
					if(slave->address < 0x20) _SiomUnitDisconnected(slave, 1);
					else	_SiomSubdevDisconnected(slave);
				}
			} else {
				retryCount = 0;
			}
			_SiomSubdevsCheckQueue();
			if(!retryCount) {
				if(slvsSync) {
					slvsSync = 0;
					_SiomControlSlaves();
				}
				if(slaveSize) {
					if(slaveIndex >= slaveSize) slaveIndex = slaveSize - 1;
					i = slaveIndex;
					do {
						slaveIndex++;
						if(slaveIndex >= slaveSize) slaveIndex = 0;
						slv = &slaves[slaveIndex];
						if(slv->state || (MS_TIMER-slv->inactivityTimer) >= INACTIVITY_TIMEOUT) break;
						slv = (SLAVE *)0;
					} while(slaveIndex != i) ;
					slave = slv;
				} else	slave = (SLAVE *)0;
			}	
			if(slave) {
				slave->rxError = 0;
				if(!retryCount) {
					if(slave->address < 0x20) txSize = _SiomUnitTxStart(slave, txBuf);
					else	txSize = _SiomSubdevTxStart(slave, txBuf);
				}
				retryCount++;
				_EnableTx();
			} else {
				sioState = 0;
				StartTimer(POLL_TIMEOUT);
			}
		}
	}
	sioLoop++;
    mT1ClearIntFlag();		// clear the interrupt flag
}
