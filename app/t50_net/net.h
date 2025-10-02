#ifndef _NET_H_
#define _NET_H_


#define PROTOCOL_ID			1

#define NET_MAX_PKT_SZ		2300

#define MAX_REQ_TSM_SZ		8
#define MAX_RSP_TSM_SZ		8

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

#define T_RESULT			0x80


// Network Data
typedef struct _NET {
	unsigned short	dataLength;
	unsigned short	address;	
	unsigned char	type;
	unsigned char	invokeId;
	unsigned char	head0;
	unsigned char	head1;
	unsigned char	data[4];
} NET;

// TSM State definitions
#define NS_NULL					0
#define NS_IDLE					1
#define NS_SEG_REQ				2
#define NS_SEG_RSP				3
#define NS_AWAIT_ACK			4
#define NS_AWAIT_SEG_REQ		5
#define NS_AWAIT_RSP			6
#define NS_AWAIT_SEG_RSP		7

// NET TSM
typedef struct _NET_TSM {
	unsigned char	type;
	unsigned char	state;
	unsigned char	nextState;
	unsigned char	serviceChoice;
	unsigned short	address;
	unsigned char	invokeId;
	unsigned char	svcPending;
	unsigned char	pduPending;
	unsigned char	objectType;
	unsigned char	propertyID;
	unsigned char	wrState;
	unsigned char	abort;
	unsigned char	reserve[3];
	unsigned long	timer;
	int				timeout;
	int				length;
	long			position;
	unsigned long	pduNet[520];	// 2080
	unsigned long	svcNet[520];	// 2080
} NET_TSM;


int  netGetNextInvokeId(void);
void netCodeLogin(NET *net, unsigned short devID);

int  netGetNextInvokeId(void);
void netCodeLogin(NET *net, unsigned short devID);
void netCodeUnconfRequest(NET *net, int serviceChoice);
void netCodeConfRequest(NET *net, int serviceChoice);
void netCodeResponse(NET *net);
void netCodeResponseMor(NET *net);
void netCodeAck(NET *net);
void netCodeAckSrv(NET *net);
void netCodeError(NET *net, int errorClass, int errorCode);
void netCodeReject(NET *net, int rejectReason);
void netCodeAbort(NET *net, int abortReason);
void netCodeAbortSrv(NET *net, int abortReason);

void netLogTx(NET *net, unsigned short sourceAddress);
void netLogRx(NET *net, unsigned short destinationAddress);

void tsmsInit(NET_TSM *tsms, int size, int type);
NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net);
NET_TSM *tsmsAdd(NET_TSM *tsms, int size, NET *net);
int  tsmsGetNextPending(NET_TSM *tsms, int size, int index);
NET_TSM *tsmsFindServiceChoice(NET_TSM *tsms, int size, int ServiceChoice);
NET_TSM *tsmsWritingObject(NET_TSM *tsms, int size, NET_TSM *self_tsm, int objectType);
NET_TSM *tsmsWritingFile(NET_TSM *tsms, int size, NET_TSM *self_tsm, int objectType);
int  tsmsGetFree(NET_TSM *tsms, int size);


#endif

