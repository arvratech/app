#ifndef _SLAVE_NET_H_
#define _SLAVE_NET_H_


// Request/Response definitions
#define RESET_REQ			0x10
#define CONTROL_REQ			0x11
#define DATA_REQ			0x12
#define CONNECT_RSP			0x20
#define DISCONNECT_RSP		0x21
#define DATA_IND			0x22
#define REQ_IND				0x23
#define RSP_CNF				0x24
#define SEG_REQ_IND			0x25
#define SEG_RSP_CNF			0x26
#define REQ2_IND			0x27
#define RSP2_CNF			0x28


#define MAX_IBUF_SZ			824
#define MAX_IDATA_SZ		812
#define MAX_SDATA_SZ		520


// SLAVE data structure
typedef struct _SLAVE {
	unsigned char	Address;
	unsigned char	State;		// 0:Null 1:Inactive 2:Active
	unsigned char	ChState[2];
	unsigned char	Inner[2];
	unsigned char	Type;
	unsigned char	TxSN, RxSN;
	unsigned char	DataRetryCount;
	unsigned char	CommLoop;
	unsigned char	Version;
	unsigned char	AckSend;
	unsigned char	RxError;
	unsigned short	DataSize;
	unsigned long	InactiveTimer;
	unsigned char	IPAddress[4];
	unsigned char	MACAddress[8];
	unsigned char	Data[MAX_IDATA_SZ];
	unsigned long	CommErrorCount;
	unsigned long	CRCErrorCount;
	unsigned long	TimeoutCount;
	unsigned long	Timer;
} SLAVE;


unsigned char *SlaveNetGetUnitCfgData(int unitID);
unsigned char *SlaveNetGetIOUnitCfgData(int unitID);
unsigned char *SlaveNetGetTxReqData(int unitID);
void SlaveNetSetTxReqDataSize(int unitID, int size, int seg);
unsigned char *SlaveNetGetRxRspData(int unitID);
int  SlaveNetGetRxRspDataSize(int unitID);
unsigned char *SlaveNetGetRxReqData(int unitID);
int  SlaveNetGetRxReqDataSize(int unitID);
unsigned char *SlaveNetGetTxRspData(int unitID);
void SlaveNetSetTxRspDataSize(int unitID, int size, int seg);
unsigned char *SlaveNetGetRxReq2Data(int unitID);
int  SlaveNetGetRxReq2DataSize(int unitID);
unsigned char *SlaveNetGetTxRsp2Data(int unitID);
void SlaveNetSetTxRsp2DataSize(int unitID, int size);

void SlaveSioInit(long BaudRate);
void SlaveSioResetCommStatistics(void);
SLAVE *SioGetSlave(unsigned char IDAddr);
int  SlaveSioWriteBuffer(void *buf, int size);

void SlaveSinInit(void);
int  SlaveSinWriteBuffer(void *buf, int size);
void SlaveSinGetIPAddress(int ID, unsigned char *IPAddr);
void SlaveSinOpen(SIN_PORT *sinport);
void SlaveSinClose(SIN_PORT *sinport);
void SlaveSinRxTask(SIN_PORT *sinport);
void SlaveSinTxTask(SIN_PORT *sinport);
void SlaveSinComTask(void);
void SlaveSinLogRx(unsigned char *buf, int size);
void SlaveSinLogTxAck(SLAVE *slv, unsigned char *buf, int size);
void SlaveSinLogTxDat(SLAVE *slv, unsigned char *buf, int size);



#endif

