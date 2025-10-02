#include "prim.h"
#include "unit.h"
#include "syscfg.h"
#include "sinport.h"
#include "slavenet.h"

// Timeout definitions
#define INACTIVITY_TIMEOUT	  150	// 150 deci-seconds 	
//#define INACTIVITY_TIMEOUT	   50 	
#define POLL_TIMEOUT		20000	// 20000*5=100000us(100ms)
#ifdef _BLUETOOTH
#define RESPONSE_TIMEOUT	20000	// 	20000*5=100000us(100ms)	 >=12000
#define IDLE_TIMEOUT		 4000	//   4000*5= 20000us(20ms)
#else
#define RESPONSE_TIMEOUT	 6000	// 	6000*5=30000us(30ms)
#define IDLE_TIMEOUT		 1000	//  1000*5=5000us(5ms)
//#define IDLE_TIMEOUT		65500
#endif

#define MAX_IOU_SZ			   10	// IOU(Input Output Unit):10
#define MAX_UNIT_SZ			   22	// CRU(Credential Reader Unit):22
#define MAX_IOUNIT_SZ		   10	// IOUnit(Input/Output Unit):102
#define MAX_SLAVE_SZ		   42	// IOU:10 + CRU:22 + IOUnit:10


volatile unsigned long	slvLoop; 
static int			  Ttxdelay, Trxdelay, Tinterval;
static volatile int	  RxCount, TxCount, TxSize;
static unsigned char  RxBuf[MAX_IBUF_SZ], TxBuf[MAX_IBUF_SZ];
static unsigned char  IouReq[MAX_UNIT_SZ][4];
static unsigned char  CruReq[MAX_UNIT_SZ][6];
static unsigned char  IouRsp[MAX_UNIT_SZ][2];	
static unsigned char  CruRsp[MAX_UNIT_SZ];
static unsigned char  CfgData[MAX_UNIT_SZ][60];				// [0]Lock [1]Length [2]Version [3..]Data

static unsigned short TxReqDataSize[MAX_UNIT_SZ];
static unsigned char  TxReqCmd[MAX_UNIT_SZ];
static unsigned char  TxReqData[MAX_UNIT_SZ][MAX_IDATA_SZ];
static unsigned short RxRspDataSize[MAX_UNIT_SZ];
static unsigned char  RxRspData[MAX_UNIT_SZ][MAX_SDATA_SZ];

static unsigned short RxReqDataSize[MAX_UNIT_SZ];
static unsigned char  RxReqData[MAX_UNIT_SZ][MAX_IDATA_SZ];
static unsigned short TxRspDataSize[MAX_UNIT_SZ];
static unsigned char  TxRspCmd[MAX_UNIT_SZ];
static unsigned char  TxRspData[MAX_UNIT_SZ][MAX_IDATA_SZ];

static unsigned short RxReq2DataSize[MAX_UNIT_SZ];
static unsigned char  RxReq2Data[MAX_UNIT_SZ][MAX_SDATA_SZ];
static unsigned short TxRsp2DataSize[MAX_UNIT_SZ];
static unsigned char  TxRsp2Cmd[MAX_UNIT_SZ];
static unsigned char  TxRsp2Data[MAX_UNIT_SZ][8];

static unsigned char  IOUnitReq[MAX_IOUNIT_SZ][2];	
static unsigned char  IOUnitRsp[MAX_IOUNIT_SZ][8];	
static unsigned char  IOUnitCfgData[MAX_IOUNIT_SZ][60];		// [0]Lock [1]ClientVersion [2]Version [3]Length [4..]Data

static SLAVE		  *slvSio, slvSios[MAX_SLAVE_SZ];
static volatile unsigned char slvSioSize, slvSioIndex, RetryCount;


#define StartTimer(cnt) {	rTCNTB1=cnt; rTCON = (rTCON & 0xfffff0ff) | 0x00000200; rTCON = (rTCON & 0xfffff0ff) | 0x00000100; }
		// Timer 1 auto reload on/off[11]=0(One-Shot) manual update[9]=1(Update TCNTB1) start/stop[8]=0(Stop)
		// Timer 1 auto reload on/off[11]=0(One-Shot) manual update[9]=0(No operation)  start/stop[8]=1(Start)
#define StopTimer() {	rTCON &= 0xfffff0ff; }
#define _putc(c) { if((c>>4) <= 9) WrUTXH2((c>>4)+'0'); else WrUTXH2((c>>4)+'a'-10); if((c&0xf) <= 9) WrUTXH2((c&0xf)+'0'); else WrUTXH2((c&0xf)+'a'-10); }


void SlaveSioInit(long BaudRate)
{
	SLAVE	*slv;
	int		i;

//	InitCRC8();
//	InitCRC16_CCITT();
	//				1char(us)		2char(us)		3char(us)
	//   9600		1041.667		2083.000		3125.000
	//  19200		 520.833		1041.667		1562.500
	//  38400		 260.417		 520.833		 781.250
	//  57600		 173.611		 347.222		 520.833
	// 115200		  86.806		 173.611		 260.417
	switch(BaudRate) {	// 1 char      4 char                     3 char
	case   9600L:	Trxdelay = 208; Tinterval = 833; break;	// Tinterval = 625;
	case  19200L:	Trxdelay = 104; Tinterval = 417; break;	// Tinterval = 313;
	case  38400L:	Trxdelay =  52; Tinterval = 208; break;	// Tinterval = 156;
	case  57600L:	Trxdelay =  35; Tinterval = 139; break;	// Tinterval = 104;
	case 115200L:	Trxdelay =  17; Tinterval =  69; break;	// Tinterval =  52;
	}
#ifdef _BLUETOOTH
	Trxdelay  = 6000;	// 6000*5=30000us(30ms)
	Tinterval = 4400;	// 4400*5=22000us(22ms)
#endif
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		CfgData[i][0] = 1; CfgData[i][1] = 0; CfgData[i][2] = 0;
		TxRspDataSize[i] = RxReqDataSize[i] = TxReqDataSize[i] = RxRspDataSize[i] = 0;
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		IOUnitCfgData[i][0] = 1; IOUnitCfgData[i][4] = IOUnitCfgData[i][5] = 0;	// [0]Lock [1]ClientVersion [2]Version [3]Length [4..]Data
	}
	for(i = 0, slv = slvSios;i < MAX_SLAVE_SZ;i++, slv++) {
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
	}
	slvSioSize = slvSioIndex = RetryCount = 0;
	slvLoop = 0L;
	sio0_state = 0;
	StartTimer(POLL_TIMEOUT);
}

void SlaveSioResetCommStatistics(void)
{
	SLAVE	*slv;
	int		i;

	sioClearError(0);
	for(i = 0, slv = slvSios;i < MAX_SLAVE_SZ;i++, slv++) {
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
	}
}	

unsigned char *SlaveNetGetUnitCfgData(int UnitID)
{	
	return CfgData[UnitID];	
}

unsigned char *SlaveNetGetIOUnitCfgData(int UnitID)
{	
	return IOUnitCfgData[UnitID];	
}

unsigned char *SlaveNetGetTxReqData(int unitID)
{
	return TxReqData[unitID];
}

void SlaveNetSetTxReqDataSize(int unitID, int size, int seg)
{
	TxReqDataSize[unitID] = size;
	if(seg) TxReqCmd[unitID] = 3; else TxReqCmd[unitID] = 1;
}

unsigned char *SlaveNetGetRxRspData(int unitID)
{
	return RxRspData[unitID];
}

int SlaveNetGetRxRspDataSize(int unitID)
{
	return (int)RxRspDataSize[unitID];
}

unsigned char *SlaveNetGetRxReqData(int unitID)
{
	return RxReqData[unitID];
}

int SlaveNetGetRxReqDataSize(int unitID)
{
	return (int)RxReqDataSize[unitID];
}

unsigned char *SlaveNetGetTxRspData(int unitID)
{
	return TxRspData[unitID];
}

void SlaveNetSetTxRspDataSize(int unitID, int size, int seg)
{
	TxRspDataSize[unitID] = size;
	if(seg) TxRspCmd[unitID] = 4; else TxRspCmd[unitID] = 2;
}

unsigned char *SlaveNetGetRxReq2Data(int unitID)
{
	return RxReq2Data[unitID];
}

int SlaveNetGetRxReq2DataSize(int unitID)
{
	return (int)RxReq2DataSize[unitID];
}

unsigned char *SlaveNetGetTxRsp2Data(int unitID)
{
	return TxRsp2Data[unitID];
}

void SlaveNetSetTxRsp2DataSize(int unitID, int size)
{
	TxRsp2DataSize[unitID] = size;
	TxRsp2Cmd[unitID] = 6;
}

SLAVE *SioGetSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	if(IDAddr < 0x20) Address = IDAddr >> 1;
	else	Address = IDAddr;
	for(i = 0, slv = slvSios;i < slvSioSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i >= slvSioSize) slv = (SLAVE *)0;
	return slv;
}

static void _SioAddSlave(unsigned char IDAddr, unsigned char Inner)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	if(IDAddr < 0x20) Address = IDAddr >> 1;
	else	Address = IDAddr;
	for(i = 0, slv = slvSios;i < slvSioSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i >= slvSioSize) {
		for(i = 0, slv = slvSios;i < slvSioSize;i++, slv++)
			if(!slv->State) break;
		if(i >= slvSioSize) {
			slv->State = 0; slvSioSize++; 
		}
	}
	if(!slv->State) {
		slv->Address		= Address;
		slv->State			= 1;
		slv->ChState[0] = slv->ChState[1] = 0;
		slv->Type			= 0;
		slv->TxSN			= 0;
		slv->DataRetryCount	= 0;
		slv->CommLoop		= 0;
		slv->InactiveTimer	= DS_TIMER - 150;
		slv->CommErrorCount = slv->CRCErrorCount = slv->TimeoutCount = 0L;
		slv->Version		  = 0;
	}
	if(IDAddr < 0x20) {
		slv->ChState[IDAddr & 1] = 1; slv->Inner[IDAddr & 1] = Inner;
	} else if(IDAddr >= 0x40) {
		slv->Type = Inner;
	}
}

static void _SioRemoveSlave(unsigned char IDAddr)
{
	SLAVE	*slv;
	unsigned char	Address;
	int		i;

	if(IDAddr < 0x20) Address = IDAddr >> 1;
	else	Address = IDAddr;
	for(i = 0, slv = slvSios;i < slvSioSize;i++, slv++)
		if(slv->State && slv->Address == Address) break;
	if(i < slvSioSize) {
		if(IDAddr < 0x20) {
			slv->ChState[IDAddr & 1] = 0;
			if(!slv->ChState[0] && !slv->ChState[1]) slv->State = 0;			
		} else	slv->State = 0;
		if(i == slvSioSize - 1 && !slv->State) slvSioSize--;
	}
}

static void _SioControlSlave(unsigned char *buf)
{
	unsigned char	IDAddr, Control, Inner;
	
	if(buf[1] < 0x40) {
		IDAddr = buf[1];
		if(buf[2]) Control = 0x02; else Control = 0x00;
		if(buf[3]) Control |= 0x01;
		if(IDAddr > 1) {
			if(Control & 0x02) {
				if(Control == 2) Inner = 1; else Inner = 0; 
				_SioAddSlave(IDAddr, Inner);
			} else	_SioRemoveSlave(IDAddr);
		}
		IDAddr |= 0x20;
		if(Control & 1) _SioAddSlave(IDAddr, 0);
		else	_SioRemoveSlave(IDAddr);
	} else if(buf[1] < 0x60) {
		if(buf[3])  _SioAddSlave(buf[1], buf[2]);
		else	_SioRemoveSlave(buf[1]);
	} else {
		if(buf[3])  _SioAddSlave(buf[1], 0);
		else	_SioRemoveSlave(buf[1]);
	}		
}

void SioIouTxStart(void);
void SioCruTxStart(void);
void SioIOUnitTxStart(void);
void SioACUnitTxStart(void);
void SioIouRxCompleted(void);
void SioCruRxCompleted(void);
void SioIOUnitRxCompleted(void);
void SioACUnitRxCompleted(void);

void __irq UART0_Slaves_ISR(void)
{
	unsigned char	c;
	int		i, n, subsrcpnd;
	
	subsrcpnd = rSUBSRCPND & (BIT_SUB_RXD0 | BIT_SUB_TXD0);	// None BIT_SUB_ERR0
	if(sio0_state == 3 || sio0_state == 4) {
		if(rUFSTAT0 & 0x007f) {	
			StopTimer();
			i = 0;
			do {
				n = rUERSTAT0;	// if c=RdURXH0() before n=UERSTAT0, then n=0 
				c = RdURXH0(); i++;
				if(n) {
					if(n & 0x01) overrun_error[0]++;
					if(n & 0x02) parity_error[0]++;
					if(n & 0x04) frame_error[0]++;
					if(n & 0x08) break_detect[0]++;
WrUTXH2('e'); _putc(n);
				} else if(RxCount < MAX_IBUF_SZ) {
					RxBuf[RxCount] = c; RxCount++;
				}
			} while(rUFSTAT0 & 0x007f) ;	// During the RX FIFO is not empty
			sio0_state = 4;
#ifdef _BLUETOOTH
			StartTimer(Tinterval);
#else
			if(i < 32) {
//WrUTXH2('r');
				if(slvSio->Address < 0x20) SioIouRxCompleted();
				else if(slvSio->Address < 0x40) SioCruRxCompleted();
				else if(slvSio->Address < 0x60) SioIOUnitRxCompleted();
				else	SioACUnitRxCompleted();
			} else {
//WrUTXH2('R');
				StartTimer(Tinterval);
			}
#endif
		} else {
			n = rUERSTAT0;
//WrUTXH2('n');
		}
	} else if(sio0_state == 1) {
		if(TxCount < TxSize) {
			while(!(rUFSTAT0 & 0x4000)) {	// until TX FIFO is full
				WrUTXH0(TxBuf[TxCount]); TxCount++;
				if(TxCount >= TxSize) {
					rUFCON0 = 0x31;	// Tx FIFO Trigger Level[7:6]=00(empty) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
					break;			
				}
			}
		} else { // if(!(rUFSTAT0 & 0x7f00)) {
//n = rUFSTAT0 & 0x7f00;
//if(n) { WrUTXH2('t');  _putc(n>>8); }
			RS485_CTRL0_L;
			rUFCON0 = 0x71;	// Tx FIFO Trigger Level[7:6]=01(16-byte) Rx FIFO Trigger Level[5:4]=11(32-byte) FIFO Enable[0]=1(Enable)
			rINTSUBMSK |= BIT_SUB_TXD0;
			sio0_state = 2;
			StartTimer(Trxdelay);
		}
	} else {	// sio0_state=0 에서 rSUBSRCPND=BIT_SUB_RXD0|BIT_SUB_TXD0 로 INT 발생
WrUTXH2('u'); 
		rUFCON0 |= 0x6;		// reset Tx FIFO/Rx FIFO and UERSTAT0
	}
	rSUBSRCPND = subsrcpnd; rSRCPND = BIT_UART0; rINTPND = BIT_UART0;
}

void __irq Timer1Done(void)
{
	SLAVE	*slv;
	unsigned char	c, type, buf[8];
	int		i;

	if(sio0_state == 2) {
		if(rUFSTAT0 & 0x007f) {	// 삼인 양산 보드에서 이 시점에 rUFSTAT0=2,3 rUERSTAT0=0x04,0x0c 되어 있슴
			rUFCON0 |= 0x6;  			// reset Tx FIFO/Rx FIFO and UERSTAT0
			rSUBSRCPND = BIT_SUB_RXD0;	// 이것 없으면 rUFSTAT0=0 상태에서 INT 발생
		}
		RxCount = 0;
		sio0_state = 3;
		rINTSUBMSK &= ~BIT_SUB_RXD0;
		StartTimer(RESPONSE_TIMEOUT);
	} else if(sio0_state == 4) {
		if(!(rUFSTAT0 & 0x007f)) {
			if(slvSio->Address < 0x20) SioIouRxCompleted();
			else if(slvSio->Address < 0x40) SioCruRxCompleted();
			else if(slvSio->Address < 0x60) SioIOUnitRxCompleted();
			else	SioACUnitRxCompleted();
		}
	} else {
		if(sio0_state == 3) {
WrUTXH2('D');
			if(!slvSio->RxError) slvSio->TimeoutCount++;
			rINTSUBMSK |= BIT_SUB_RXD0; rSUBSRCPND = BIT_SUB_RXD0; 	// 2013.10.31 이것 없으면 0 상태에서 INT 발생 가능
			if(slvSio->State == 1 || RetryCount >= 3) {
				if(slvSio->State >= 2) {
					buf[0] = DISCONNECT_RSP;
					if(slvSio->Address < 0x20) buf[1] = slvSio->Address << 1;
					else	buf[1] = slvSio->Address;
					buf[2] = buf[3] = buf[4] = buf[5] = buf[6] = buf[7] = 0;
					//memset(buf+2, 0, 6);
					cbuf_put(sp_icbuf[0], buf, 8);
					slvSio->State = 1;
					if(slvSio->Address < 0x20) {
						if(slvSio->ChState[0]) slvSio->ChState[0] = 1;
						if(slvSio->ChState[1]) slvSio->ChState[1] = 1;
					}
				}
				slvSio->InactiveTimer = DS_TIMER;
				RetryCount = 0;
				slvSio->CommLoop = slvSio->DataRetryCount = slvSio->TxSN = 0;
			}
		}
		if(!RetryCount) {
			while(1) {
				c = cbuf_get(sp_ocbuf[0], buf, 8);
				if(c < 8) break;
				c = buf[0]; i = buf[1] & 0x1f; type = buf[1] >> 5;
				if(c == DATA_REQ) {
					if(type == 0) IouReq[i][0] = buf[2];
					else if(type == 1) memcpy(CruReq[i], buf+2, 6);
					else if(type == 2) memcpy(IOUnitReq[i], buf+2, 2);
					else {
						buf[4] = IouReq[i][2]; buf[5] = IouReq[i][3];
						IouReq[i][2] = buf[2]; IouReq[i][3] = buf[3];
						if(buf[4] != IouReq[i][2] || buf[5] != IouReq[i][3]) IouReq[i][1] = 0x01;
					}	
				} else if(c == CONTROL_REQ) {
					_SioControlSlave(buf);
				} else if(c == RESET_REQ) {
					slvSioSize = 0;
				}
			}
			if(slvSioSize) {
				if(slvSioIndex >= slvSioSize) slvSioIndex = slvSioSize - 1;
				i = slvSioIndex;
				do {
					slvSioIndex++;
					if(slvSioIndex >= slvSioSize) slvSioIndex = 0;
					slv = &slvSios[slvSioIndex];
					if(slv->State >= 2 || slv->State == 1 && (DS_TIMER-slv->InactiveTimer) >= INACTIVITY_TIMEOUT) break;
					slv = (SLAVE *)0;
				} while(slvSioIndex != i) ;
				slvSio = slv;
			} else	slvSio = (SLAVE *)0;
		}
		if(slvSio) {
			// 485 케이블을 탈착후 부착하면 rUERSTAT0=0x0c,0x04,0x00 rUFSTAT0=0x00-0x07
			// i=rUERSTAT0 => 다음에 타임아웃(D) 발생
			// FIFO reset  => 다음에 정상 
			rUFCON0 |= 0x6;		// reset Tx FIFO/Rx FIFO and UERSTAT0
			rSUBSRCPND = BIT_SUB_RXD0;
			slvSio->RxError = 0;
			if(slvSio->Address < 0x20) SioIouTxStart();
			else if(slvSio->Address < 0x40) SioCruTxStart();
			else if(slvSio->Address < 0x60) SioIOUnitTxStart();
			else	SioACUnitTxStart();
			RetryCount++;
			TxCount = 0;
			RS485_CTRL0_H;
			sio0_state = 1;
			rINTSUBMSK &= ~BIT_SUB_TXD0;
		} else {
			sio0_state = 0;
			StartTimer(POLL_TIMEOUT);
		}
	}
	slvLoop++;
	rSRCPND = BIT_TIMER1; rINTPND = BIT_TIMER1;
}

void SioIouTxStart(void)
{
	unsigned char	*p, *d;
	unsigned char	c;
	int		i, n, len;

	if(!RetryCount) {
		p = TxBuf;
		*p++ = 0x55;
		*p++ = slvSio->Address;
		n = slvSio->Address & 0x1f; n <<= 1;
		d = CfgData[n];
		if(slvSio->State > 1 && !d[0] && d[1] != d[2]) {
			slvSio->Version = d[2];
			len = d[3] + 1;
		} else	len = 0;
		if(slvSio->TxSN & 0x01) c = 0x01; else c = 0x00;
		if(len) c += 2;
		*p++ = c;
		//if(IouReq[n][1]) c = 0x00;
		//else	c = IouReq[n][0] << 3;
		c = IouReq[n][0] << 3;
		if(slvSio->Inner[0]) c |= CruReq[n][0] >> 1;
		*p++ = c;
		//if(IouReq[n+1][1]) c = 0x00;
		//else	c = IouReq[n+1][0] << 3;
		c = IouReq[n+1][0] << 3;
		if(slvSio->Inner[1]) c |= CruReq[n+1][0] >> 1;
		*p++ = c;
		if(slvSio->Inner[0]) c = CruReq[n][1] & 0x0f;
		else	c = 0;
		if(slvSio->Inner[1]) c |= CruReq[n+1][1] << 4;
		*p++ = c;
		*p++ = len;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ TxBuf[i]];		
		*p++ = c;
		if(len) {
			p[0] = d[2];
			memcpy(p+1, d+4, len-1);
			for(i = 0, c = 0xff;i < len;i++) c = crc8tab[c ^ p[i]];
			p += len; *p++ = c;
//WrUTXH2('d');
		}
		*p++ = 0xff;		// null padding for RS-485
		TxSize = p - TxBuf;
	}
}

void SioCruTxStart(void)
{
	unsigned char	*p;
	unsigned char	c, c_h, c_l;
	int		i, n, len;
	
	if(!RetryCount) {	
		n = slvSio->Address & 0x1f;
		if(slvSio->State < 2) TxRspDataSize[n] = 0;	
		len = TxRspDataSize[n];
		if(len) {
			if(!slvSio->DataRetryCount) {
				if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;
			}
			slvSio->DataRetryCount++;
		}
//_putc(len);
		p = TxBuf;
		*p++ = 0x55;
		*p++ = slvSio->Address;
		if(slvSio->TxSN) c = 6; else c = 4;
		if(!slvSio->RxSN) c++;
		*p++ = c;
		if(slvSio->CommLoop & 0x01) {
			c = ((slvSio->CommLoop >> 1) & 0x03) + 1;
			*p++ = CruReq[n][0] | (c << 5); *p++ = CruReq[n][c+1];
		} else {
			*p++ = CruReq[n][0]; *p++ = CruReq[n][1];
		}
		slvSio->CommLoop++;
		*p++ = len >> 8; *p++ = len;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ TxBuf[i]];		
		*p++ = c;
		if(len) {
			memcpy(p, TxRspData[n], len);
			c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
			for(i = 0;i < len;i++) {
				c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
			}
			p += len; *p++ = c_h; *p++ = c_l;
		}
		*p++ = 0xff;		// null padding for RS-485
		TxSize = p - TxBuf;
	}
//WrUTXH2('0'+ (slvSio->Address & 0x1f));
//WrUTXH2('a'+RetryCount);
}

void SioIOUnitTxStart(void)
{
	unsigned char	*p, *d;
	unsigned char	c;
	int		i, n, len;

	if(!RetryCount) {
		p = TxBuf;
		*p++ = 0x55;
		*p++ = slvSio->Address;
		n = slvSio->Address & 0x1f;
		d = IOUnitCfgData[n];
		if(slvSio->DataRetryCount) {
			slvSio->DataRetryCount++;
		} else if(slvSio->State > 1 && !d[0] && d[1] & 0x03) {
			if(d[1] & 0x01) d[1] = 0x02;
			slvSio->DataSize = d[3];
			memcpy(slvSio->Data, d+3, (int)slvSio->DataSize);
			if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;
			slvSio->DataRetryCount = 1;
		} else	slvSio->DataSize = 0;
		len = slvSio->DataSize;
		if(slvSio->TxSN) c = 0x06; else c = 0x04;
		if(!slvSio->RxSN) c++;	
		*p++ = c;
		*p++ = IOUnitReq[n][0];
		*p++ = IOUnitReq[n][1];
		*p++ = 0;
		*p++ = len;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ TxBuf[i]];		
		*p++ = c;
		if(len) {
			memcpy(p, slvSio->Data, len);
			for(i = 0, c = 0xff;i < len;i++) c = crc8tab[c ^ p[i]];
			p += len; *p++ = c;
		}
		*p++ = 0xff;		// null padding for RS-485
		TxSize = p - TxBuf;
	}
}

void SioACUnitTxStart(void)
{
	unsigned char	*p;
	unsigned char	c, c_h, c_l;
	int		i, n, len;

	if(slvSio->State < 2) {
		p = TxBuf;
		*p++ = 0x55;
		*p++ = slvSio->Address;
		*p++ = 0x00;	// Connect
		*p++ = 0;
		SHORTtoBYTE(sys_cfg->DeviceID, p); p += 2;
		*p++ = 0;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ TxBuf[i]];		
		*p++ = c;
		*p++ = 0xff;		// null padding for RS-485
		TxSize = p - TxBuf;
	} else if(!RetryCount) {
		n = slvSio->Address & 0x1f;
		if(slvSio->DataRetryCount) {
			slvSio->DataRetryCount++;
			len = slvSio->DataSize;
		} else {
			if(TxRspDataSize[n]) {
				len = TxRspDataSize[n]; p = TxRspData[n]; TxRspDataSize[n] = 0;
				c = TxRspCmd[n];
			} else if(TxReqDataSize[n]) {
				len = TxReqDataSize[n]; p = TxReqData[n]; TxReqDataSize[n] = 0;
//_putc(len);
				c = TxReqCmd[n];
			} else if(TxRsp2DataSize[n]) {
				len = TxRsp2DataSize[n]; p = TxRsp2Data[n]; TxRsp2DataSize[n] = 0;
				c = TxRsp2Cmd[n];
			} else if(IouReq[n][0] || IouReq[n][1]) {
				len = 2; p = NULL;
				c = 0;
			} else	len = 0;
			if(len) {
				if(p) {
					slvSio->Data[2] = c; memcpy(slvSio->Data+3, p, len); len += 3;
				}
				p = IouReq[n];
				memcpy(slvSio->Data, p+2, 2);
				p[0] |= p[1]; p[1] = 0;
				slvSio->DataSize = len;
				if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;
				slvSio->DataRetryCount = 1;
			}
		}
		p = TxBuf;
		*p++ = 0x55;
		*p++ = slvSio->Address;
		if(slvSio->TxSN) c = 6; else c = 4; if(!slvSio->RxSN) c++;
		*p++ = c;
		*p++ = 0; *p++ = 0;
		*p++ = len >> 8; *p++ = len;
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ TxBuf[i]];		
		*p++ = c;
		if(len) {
			memcpy(p, slvSio->Data, len);
			c_h = 0xff; c_l = 0xff;		// CRC High - CRC Low
			for(i = 0;i < len;i++) {
				c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
			}
			p += len; *p++ = c_h; *p++ = c_l;
		}
		*p++ = 0xff;		// null padding for RS-485
		TxSize = p - TxBuf;		
	}
}

void SioIouRxCompleted(void)
{
	unsigned char	*p, buf[8];
	unsigned char	c, val, iRsp[2][2], cRsp[2];
	int		i, n, len;

	n = 0; len = RxCount;
	if(len >= 8 && RxBuf[0] == 0x66 && RxBuf[1] == slvSio->Address && RxBuf[2] < 4) {
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ RxBuf[i]];
		if(c == RxBuf[7]) n = 1;
		else { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	} else if(!slvSio->RxError) { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	if(n) {
		c = RxBuf[2];			// Frame Type
		n = RxBuf[1] & 0x1f; n <<= 1;
//_putc(RxBuf[3]); _putc(RxBuf[4]); _putc(RxBuf[5]); if(len > 8) WrUTXH2('d'); else WrUTXH2('\n'); 
		if(c > 1) val = RxBuf[5];					// Head[2]: Version
		else	  val = RxBuf[6];					// Head[3]: Version
		if(val != CfgData[n][1]) CfgData[n][1] = val;	// ClientVersion
		//if(!val && IouReq[n][0] && IouReq[n][0] < 0x40) IouReq[n][1] = IouReq[n][0];
		//else  	IouReq[n][1] = 0;
		iRsp[0][0] = RxBuf[3] & 0xfe;				// Head[0]: CH1 Rsp[0]
		if(slvSio->Inner[0]) cRsp[0] = (CruRsp[n] & 0xfe) | (RxBuf[3] & 0x01);
		else	cRsp[0] = CruRsp[n];
		iRsp[1][0] = RxBuf[4] & 0xfe;			// Head[1]: CH2 Rsp[0]
		if(slvSio->Inner[1]) cRsp[1] = (CruRsp[n+1] & 0xfe) | (RxBuf[4] & 0x01);
		else	cRsp[1] = CruRsp[n+1];
		if(c > 1) {
			iRsp[0][1] = IouRsp[n][1]; iRsp[1][1] = IouRsp[n+1][1];
		} else {
			if(RxBuf[5] & 0x80) {
				iRsp[1][1] = RxBuf[5] & 0x1e;	// Head[2]: CH2 Rsp[1]
				if(slvSio->Inner[1]) cRsp[1] = (cRsp[1] & 0xfd) | ((RxBuf[5] << 1) & 0x02);				
				iRsp[0][1] = IouRsp[n][1];			
			} else {
				iRsp[0][1] = RxBuf[5] & 0x1e;	// Head[2]: CH2 Rsp[1]
				if(slvSio->Inner[0]) cRsp[0] = (cRsp[0] & 0xfd) | ((RxBuf[5] << 1) & 0x02);
				iRsp[1][1] = IouRsp[n+1][1];
			}
		}
		p = RxBuf + 8;
		if(c > 1) {
			len -= 9;
			if(c > 1 && len == RxBuf[6]) {	// IOU,CRU with Normal Data
				for(i = 0, c = 0xff;i < len;i++) c = crc8tab[c ^ p[i]];
				if(c != p[i]) len = 0;
			} else	len = 0;
		} else	len = 0;
		val = 0;
		if(len) {
			slvSio->TxSN++;
			if(p[0] & 0x80) c = 1; else c = 0;	
			p[0] &= 0x7f;
			i = 2 + ((p[1] + 7) >> 3);
			if(c) val = 0x02; else val = 0x01; 
			memcpy(RxReqData[n+c], p, i);
			RxReqDataSize[n+c] = i;
			CruReq[n+c][0] = 0x08;		// State=1 Card-PIN-FP=000 => Wait
			if(len > i) {
				p += i;
				if(p[0] & 0x80) c = 1; else c = 0;						
				p[0] &= 0x7f;
				i = 2 + ((p[1] + 7) >> 3);
				memcpy(RxReqData[n+c], p, i);
				RxReqDataSize[n+c] = i;
				if(c) val |= 0x02; else val |= 0x01; 
				CruReq[n+c][0] = 0x08;		// State=1 Card-PIN-FP=000 => Wait
			}
		}
		c = slvSio->ChState[0];
		if(c && (c == 1 || iRsp[0][0] != IouRsp[n][0] || iRsp[0][1] != IouRsp[n][1])) {
			buf[0] = DATA_IND; buf[1] = n; 
			buf[2] = IouRsp[n][0] = iRsp[0][0]; buf[3] = IouRsp[n][1] = iRsp[0][1];
			buf[4] = buf[5] = buf[6] = buf[7] = 0;					
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		if(c && ((val & 0x01) || cRsp[0] != CruRsp[n])) {
			if(val & 0x01) buf[0] = REQ_IND; else buf[0] = DATA_IND;
			buf[1] = 0x20 | n;
			buf[2] = CruRsp[n] = cRsp[0];
			buf[3] = buf[4] = buf[5] = buf[6] = buf[7] = 0;
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		if(c) slvSio->ChState[0] = 2;
		n++;
		c = slvSio->ChState[1];
		if(c && (c == 1 || iRsp[1][0] != IouRsp[n][0] || iRsp[1][1] != IouRsp[n][1])) {
			buf[0] = DATA_IND; buf[1] = n; 
			buf[2] = IouRsp[n][0] = iRsp[1][0]; buf[3] = IouRsp[n][1] = iRsp[1][1];
			buf[4] = buf[5] = buf[6] = buf[7] = 0;					
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		if(c && ((val & 0x02) || cRsp[1] != CruRsp[n])) {
			if(val & 0x02) buf[0] = REQ_IND; else buf[0] = DATA_IND;
			buf[1] = 0x20 | n;
			buf[2] = CruRsp[n] = cRsp[1];
			buf[3] = buf[4] = buf[5] = buf[6] = buf[7] = 0;					
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		if(c) slvSio->ChState[1] = 2;
		slvSio->State = 2;
		RetryCount = 0;
		rINTSUBMSK |= BIT_SUB_RXD0;		// 2014.1.15 이것 없으면 0 상태에서 INT 발생 가능
		sio0_state = 0;
		StartTimer(IDLE_TIMEOUT);
	} else {
		RxCount = 0;
		sio0_state = 3;
		n = rTCNTO1; if(n < 1) n = 1;	// BugFix 2014.5.13 dTCNT00 => rTCNTO1
		StartTimer(n);
	}
}

void SioCruRxCompleted(void)
{
	unsigned char	*p, buf[8];
	unsigned char	c, c_h, c_l;
	int		i, n, len;

	n = 0; c = RxBuf[2];	// Frame Type
	if(RxCount >= 8 && RxBuf[0] == 0x66 && RxBuf[1] == slvSio->Address && c > 3 && c < 8) {
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ RxBuf[i]];
		if(c == RxBuf[7]) n = 1;
		else { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	} else if(!slvSio->RxError) { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	if(n) {
//WrUTXH2('r');
		n = 0;
		len = (RxBuf[5] << 8) + RxBuf[6];
//if(len) {
//	_putc(len); _putc(RxCount); _putc(RxBuf[RxCount-1]);
//}
		if(RxCount >= len + 10) {
			p = RxBuf + 8; c_h = 0xff; c_l = 0xff;	// CRC High - CRC Low
			for(i = 0;i < len;i++) {
				c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
			}
			if(c_h == p[i] && c_l == p[i+1]) n = 1;
		}
		c = RxBuf[2];			// Frame Type
		// Data
		if(c < 6) c_h = 0; else c_h = 1;
		if(slvSio->RxSN == c_h && (!len || n)) {
			if(slvSio->RxSN) slvSio->RxSN = 0; else slvSio->RxSN = 1;
		} else	len = 0;
		n = RxBuf[1] & 0x1f;
		// Ack
		if(c & 1) c_h = 1; else c_h = 0;
		if(slvSio->TxSN == c_h && slvSio->DataRetryCount || slvSio->TxSN != c_h && slvSio->DataRetryCount > 2) {
			if(slvSio->DataRetryCount > 2) {
				if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;			
			}
			slvSio->DataRetryCount = 0;
			TxRspDataSize[n] = 0;
		}
		c = RxBuf[3] & 0x03;			// Head[0]
		if(slvSio->State == 1 || c != CruRsp[n] || len) {
			if(len) {
				buf[0] = REQ_IND;
				memcpy(RxReqData[n], RxBuf+8, len); RxReqDataSize[n] = len;
			} else 	buf[0] = DATA_IND;
			buf[1] = 0x20 | n;
			buf[2] = CruRsp[n] = c;
			buf[3] = buf[4] = buf[5] = buf[6] = buf[7] = 0;					
			//memset(buf+3, 0, 5);
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		slvSio->State = 2;
		RetryCount = 0;
		rINTSUBMSK |= BIT_SUB_RXD0;		// 2014.1.15 이것 없으면 0 상태에서 INT 발생 가능
		sio0_state = 0;
		StartTimer(IDLE_TIMEOUT);
	} else {
//_putc(RxCount);
		RxCount = 0;
		sio0_state = 3;
		n = rTCNTO1; if(n < 1) n = 1;	// BugFix 2014.5.13 dTCNT00 => rTCNTO1
		StartTimer(n);
	}
}

void SioIOUnitRxCompleted(void)
{
	unsigned char	buf[8];
	unsigned char	c, Rsp[6];
	int		i, n, len;

	n = 0; len = RxCount;
	if(len >= 8 && RxBuf[0] == 0x66 && RxBuf[1] == slvSio->Address && RxBuf[2] < 2) {
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ RxBuf[i]];
		if(c == RxBuf[7]) n = 1;
		else { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	} else if(!slvSio->RxError) { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	if(n) {
		n = RxBuf[1] & 0x1f;		
//_putc(RxBuf[3]); _putc(RxBuf[4]); _putc(RxBuf[5]); if(len > 8) WrUTXH2('d'); else WrUTXH2('\n'); 
		// Ack
		if(RxBuf[2] & 1) c = 1; else c = 0;
		if(slvSio->TxSN == c && slvSio->DataRetryCount) {	// Data Ack
//WrUTXH2('a');
			slvSio->DataRetryCount = 0;
			IOUnitCfgData[n][1] &= 0xfd;
		} else if(slvSio->TxSN != c && slvSio->DataRetryCount > 2) {
			if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;			
			slvSio->DataRetryCount = 0;
		}
		if(RxBuf[3] & 0x80) IOUnitCfgData[n][1] = 0;	// ClientVersion=0
		if(slvSio->Type) memset(Rsp, 0, 6);
		else {
			if(RxBuf[3] & 0x01) {		
				Rsp[3] = RxBuf[4]; Rsp[4] = RxBuf[5]; Rsp[5] = RxBuf[6];	// Head[1]: Rsp[3]  Head[2]: Rsp[4]  Head[3]: Rsp[5]
				Rsp[0] = IOUnitRsp[n][0]; Rsp[1] = IOUnitRsp[n][1]; Rsp[2] = IOUnitRsp[n][2]; 
			} else {
				Rsp[0] = RxBuf[4]; Rsp[1] = RxBuf[5]; Rsp[2] = RxBuf[6];	// Head[1]: Rsp[0]  Head[2]: Rsp[1]  Head[3]: Rsp[2]
				Rsp[3] = IOUnitRsp[n][3]; Rsp[4] = IOUnitRsp[n][4]; Rsp[5] = IOUnitRsp[n][5]; 
			}
		}
		if(slvSio->State == 1 || n_memcmp(Rsp, IOUnitRsp[n], 6)) {
			memcpy(IOUnitRsp[n], Rsp, 6);
			buf[0] = DATA_IND; buf[1] = 0x40 | n; 
			memcpy(buf+2, Rsp, 6);
			cbuf_put(sp_icbuf[0], buf, 8);
		}
		slvSio->State = 2;
		RetryCount = 0;
		rINTSUBMSK |= BIT_SUB_RXD0;		// 2014.1.15 이것 없으면 0 상태에서 INT 발생 가능
		sio0_state = 0;
		StartTimer(IDLE_TIMEOUT);
	} else {
		RxCount = 0;
		sio0_state = 3;
		n = rTCNTO1; if(n < 1) n = 1;	// BugFix 2014.5.13 dTCNT00 => rTCNTO1
		StartTimer(n);
	}
}

void SioACUnitRxCompleted(void)
{
	unsigned char	*p, buf[8];
	unsigned char	c, c_h, c_l;
	int		i, n, len;

//cprintf("Rx: %d [%02x", RxCount, (int)RxBuf[0]); for(i = 1;i < RxCount;i++) cprintf("-%02x", (int)RxBuf[i]); cprintf("]\n");
	n = 0; c = RxBuf[2];	// Frame Type
	if(RxCount >= 8 && RxBuf[0] == 0x66 && RxBuf[1] == slvSio->Address && (c < 2 || c > 3 && c < 8)) {
		for(i = 1, c = 0xff;i < 7;i++) c = crc8tab[c ^ RxBuf[i]];
		if(c == RxBuf[7]) n = 1;
		else { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	} else if(!slvSio->RxError) { slvSio->CRCErrorCount++; slvSio->RxError = 1; }
	if(n) {
//_putc(RxBuf[3]); _putc(RxBuf[4]); _putc(RxBuf[5]); if(len > 8) WrUTXH2('d'); else WrUTXH2('\n'); 
		if(RxBuf[2] < 2) {
			n = RxBuf[1] & 0x1f;
			buf[0] = CONNECT_RSP; buf[1] = slvSio->Address; buf[2] = RxBuf[2]; 
			cbuf_put(sp_icbuf[0], buf, 8);
			slvSio->TxSN = 0; slvSio->RxSN = 1; slvSio->DataRetryCount = 0;
			TxReqDataSize[n] = TxRspDataSize[n] = RxReqDataSize[n] = RxRspDataSize[n] = 0;
			IouReq[n][1] = 0x01;
			//if(RxBuf[2]) // CfgData[n][2] = 0xc0;
			slvSio->State = 2;
		} else {
			n = 0; len = (RxBuf[5] << 8) + RxBuf[6];
			if(RxCount >= len + 10) {
				p = RxBuf + 8; c_h = 0xff; c_l = 0xff;	// CRC High - CRC Low
				for(i = 0;i < len;i++) {
					c = c_h ^ p[i]; c_h = c_l ^ crc16tab_h[c]; c_l = crc16tab_l[c];
				}
				if(c_h == p[i] && c_l == p[i+1]) n = 1;
			}
			c = RxBuf[2];	// Frame Type
			// Data
			if(c < 6) c_h = 0; else c_h = 1;
			if(slvSio->RxSN == c_h && (!len || n)) {
				if(slvSio->RxSN) slvSio->RxSN = 0; else slvSio->RxSN = 1;
			} else	len = 0;
			n = RxBuf[1] & 0x1f;
			// Ack
			if(c & 1) c_h = 1; else c_h = 0;
			if(slvSio->TxSN == c_h) {
				if(slvSio->DataRetryCount) {
					slvSio->DataRetryCount = 0;
					IouReq[n][0] = 0;
				}				 
			} else if(slvSio->DataRetryCount > 2) {
				if(slvSio->TxSN) slvSio->TxSN = 0; else slvSio->TxSN = 1;			
				slvSio->DataRetryCount = 0;
			}
			if(len) {
				buf[2] = RxBuf[8]; buf[3] = RxBuf[9]; len -= 2;
				if(len < 1) buf[0] = DATA_IND;
				else {
					len--;
					if(RxBuf[10] == 5) {
						buf[0] = REQ2_IND;
						memcpy(RxReq2Data[n], RxBuf+11, len); RxReq2DataSize[n] = len;
					} else if(RxBuf[10] & 1) {
						if(RxBuf[10] == 1) buf[0] = REQ_IND; else buf[0] = SEG_REQ_IND;
						memcpy(RxReqData[n], RxBuf+11, len); RxReqDataSize[n] = len;
					} else {
						if(RxBuf[10] == 2) buf[0] = RSP_CNF; else buf[0] = SEG_RSP_CNF;
						memcpy(RxRspData[n], RxBuf+11, len); RxRspDataSize[n] = len;
					}
				}
				buf[1] = slvSio->Address;
				cbuf_put(sp_icbuf[0], buf, 8);
			}
		}
		RetryCount = 0;
		rINTSUBMSK |= BIT_SUB_RXD0;		// 2014.1.15 이것 없으면 0 상태에서 INT 발생 가능
		sio0_state = 0;
		StartTimer(IDLE_TIMEOUT);
	} else {
		RxCount = 0;
		sio0_state = 3;
		n = rTCNTO1; if(n < 1) n = 1;	// BugFix 2014.5.13 dTCNT00 => rTCNTO1
		StartTimer(n);
	}
}
