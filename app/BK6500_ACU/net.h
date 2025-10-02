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


// Network Data
typedef struct _NET {
	unsigned short	DataLength;
	unsigned short	Address;	
	unsigned char	Type;
	unsigned char	InvokeID;
	unsigned char	Head0;
	unsigned char	Head1;
	unsigned char	Data[4];
} NET;

// TSM State definitions
#define N_IDLE				0
#define N_SEG_REQ			1
#define N_SEG_RSP			2
#define N_AWAIT_ACK			3
#define N_AWAIT_SEG_REQ		4
#define N_AWAIT_RSP			5
#define N_AWAIT_SEG_RSP		6

// NET TSM
typedef struct _NET_TSM {
	unsigned char	Type;
	unsigned char	State;
	unsigned short	Address;
	unsigned char	InvokeID;
	unsigned char	ServiceChoice;
	unsigned char	SvcPending;
	unsigned char	PduPending;
	unsigned char	ObjectType;
	unsigned char	PropertyID;
	unsigned char	WrState;
	unsigned char	reserve[2];
	unsigned long	Timer;
	int				Timeout;
	long			Position;
	unsigned long	PduNet[520];	// 2080
	unsigned long	SvcNet[520];	// 2080
} NET_TSM;


int  netGetNextInvokeID(void);
void netCodeLogin(NET *net, unsigned short DevID);
void netCodeUnconfRequest(NET *net, int ServiceChoice);
void netCodeConfRequest(NET *net, int ServiceChoice);
void netCodeResponse(NET *net);
void netCodeResponseMor(NET *net);
void netCodeAck(NET *net);
void netCodeAckSrv(NET *net);
void netCodeError(NET *net, int ErrorClass, int ErrorCode);
void netCodeReject(NET *net, int RejectReason);
void netCodeAbort(NET *net, int AbortReason);
void netCodeAbortSrv(NET *net, int AbortReason);

void netLogTx(NET *net, unsigned short SourceAddress);
void netLogRx(NET *net, unsigned short DestinationAddress);

void tsmsInit(NET_TSM *tsms, int size, int Type);
NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net);
NET_TSM *tsmsAdd(NET_TSM *tsms, int size, NET *net);
int  tsmsGetNextPending(NET_TSM *tsms, int size, int index);
NET_TSM *tsmsFindServiceChoice(NET_TSM *tsms, int size, int ServiceChoice);
NET_TSM *tsmsWritingObject(NET_TSM *tsms, int size, NET_TSM *self_tsm, int ObjectType);
NET_TSM *tsmsWritingFile(NET_TSM *tsms, int size, NET_TSM *self_tsm, int ObjectType);
int  tsmsGetFree(NET_TSM *tsms, int size);


#endif
