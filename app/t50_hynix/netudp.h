#ifndef _NET_UDP_H_
#define _NET_UDP_H_


#define PROTOCOL_ID			1

#define NET_MAX_PKT_SZ		2300

// Type definitions
#define T_UNCONF_REQ		0x10
#define T_REQ				0x20
#define T_REQ_MOR			0x24
#define T_RSP				0x30
#define T_RSP_MOR			0x34
#define T_ACK				0x40
#define T_ACK_SRV			0x41
#define T_ERROR				0x50
#define T_REJECT			0x60
#define T_ABORT				0x70
#define T_ABORT_SRV			0x71
#define T_ABORT_PDU			0x72
#define T_ABORT_SRV_PDU		0x73


// Network UDP Data
typedef struct _NET_UDP {
	unsigned short	dataLength;
	unsigned short	sourceAddress;
	unsigned short  destinationAddress; 
	unsigned char	type;
	unsigned char	invokeId;
	unsigned char	head0;
	unsigned char	head1;
	unsigned char	data[2];	
} NET_UDP;


int  netudpGetNextInvokeId(void);
void netudpCodeLogin(NET_UDP *net, int devId, int readerId);
void netudpCodePing(NET_UDP *net, int devId, int readerId);
void netudpCodeConfRequest(NET_UDP *net, unsigned short sourceAddress, int serviceChoice);
void netudpCodeConfRequestMor(NET_UDP *net, unsigned short sourceAddress, int serviceChoice);
void netudpCodeHeartBeat(NET_UDP *net, unsigned short destAddress);
void netudpCodeResponse(NET_UDP *net, NET_UDP *rdnet);
void netudpCodeError(NET_UDP *net, NET_UDP *rdnet, int errorClass, int errorCode);
void netudpCodeReject(NET_UDP *net, NET_UDP *rdnet, int rejectReason);
void netudpCodeAbort(NET_UDP *net, NET_UDP *rdnet, int abortReason);

void netudpLogTx(NET_UDP *net, unsigned char *ipAddr, unsigned short ipPort);
void netudpLogRx(NET_UDP *net, unsigned char *ipAddr, unsigned short ipPort);


#endif

