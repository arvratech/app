#ifndef _SLV_NET_H_
#define _SLV_NET_H_


// slvnetState definitions
#define S_SLVNET_NULL		0
#define S_SLVNET_IDLE		1
#define S_SLVNET_READY		2

int  SlvnetSioInit(void);
int  SlvnetSioExit(void);
void SlvnetSioReset(void);
void SlvnetSetAddress(void);
int  SlvnetSioWriteData(unsigned char *buf, int length);
int  SlvnetIsConnected(void);

void SlvnetSioSetAddress(void);

void SlvnetUdpInit(void);
void SlvnetUdpSetAddress(void);
void SlvnetUdpDisconnect(void);
void SlvnetUdpSendRequest(void);
void SlvnetUdpOpen(void);
void SlvnetUdpClose(void);
int  SlvnetMasterDevID(void);


int  SlvnetMasterDevId(void);
void SlvnetSetConfReqTxSize(int size, int seg);
void SlvnetSetConfRspTxSize(int size, int seg);
int  SlvnetConfReqRxSize(void);
int  SlvnetConfRspRxSize(void);
unsigned char *SlvnetConfReqBuf(void);
unsigned char *SlvnetConfRspBuf(void);

void SlvnetSetConfReq2TxSize(int size);
int  SlvnetConfReq2RxSize(void);
unsigned char *SlvnetConfReq2Buf(void);
void SlvnetSendRequest(void);


void _PostSlvnetChanged(void);

void sioClearRxData(void);
int  SlvnetSinWriteBuffer(int port, void *buf, int size);
unsigned char *sioGetUnitTxData(int UnitID);


#endif

