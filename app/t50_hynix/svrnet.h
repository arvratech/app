#ifndef _SVR_NET_H_
#define _SVR_NET_H_

#include "netudp.h"


// slvnetState definitions
#define S_SVRNET_NULL		0
#define S_SVRNET_IDLE		1
#define S_SVRNET_CONN		2
#define S_SVRNET_READY		3


int  SvrnetIsConnected(void);

void SvrnetUdpInit(void);
void SvrnetUdpSetAddress(void);
void SvrnetUdpDisconnect(void);
void SvrnetUdpSendRequest(void);
void SvrnetUdpOpen(void);
void SvrnetUdpClose(void);
void SvrnetSendCamEvent(void);
void SvrnetSendCamServer(unsigned char *jpegBuf, int jpegBufLenth);

void SvrProcessConfRequest(NET_UDP *reqnet, NET_UDP *rspnet);
void ReadProperty(NET_UDP *reqnet, NET_UDP *rspnet);
void WriteProperty(NET_UDP *reqnet, NET_UDP *rspnet);


#endif

