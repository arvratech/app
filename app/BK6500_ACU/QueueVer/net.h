#ifndef _NET_H_
#define _NET_H_


#define PROTOCOL_ID			1

#define NET_NAX_PKT_SZ			2300

#define CONFIRMED_REQUEST_PDU		0
#define UNCONFIRMED_REQUEST_PDU		1
#define RESPONSE_PDU				3
#define ACK_PDU						4
#define ERROR_PDU					5
#define REJECT_PDU					6
#define ABORT_PDU					7


#define T_CONF_REQUEST			0x00
#define T_CONF_REQUEST_MOR		0x04
#define T_UNCONF_REQUEST		0x10
#define T_RESPONSE				0x30
#define T_RESPONSE_MOR			0x34
#define T_ACK					0x40
#define T_ACK_SRV				0x41
#define T_ERROR					0x50
#define T_REJECT				0x60
#define T_ABORT					0x70
#define T_ABORT_SRV				0x71
#define T_ABORT_PDU				0x72
#define T_ABORT_SRV_PDU			0x73


// Network Data
typedef struct _NET {
	unsigned short	Address;	
	unsigned char	Type;
	unsigned char	InvokeID;
	unsigned char	Head0;
	unsigned char	Head1;
	unsigned short	BufferOffset;
	unsigned short	BufferLength;
	unsigned short	Count;	
	unsigned char	*Buffer;
} NET;

// Requesting User(Client) TSM State definitions
#define REQ_IDLE					0
#define REQ_AWAIT_MORE_REQUEST		1
#define REQ_AWAIT_CONF				2
#define REQ_SEGMENTED_REQUEST		3
#define REQ_AWAIT_CONFIRMATION		4
#define REQ_SEGMENTED_CONF			5

// Responding User(Server) TSM State definitions
#define RSP_IDLE					0
#define RSP_SEGMENTED_CONF			11
#define RSP_SEGMENTED_RESPONSE		12
#define RSP_AWAIT_CONF				13
#define RSP_AWAIT_RESPONSE			14
#define RSP_AWAIT_MORE_RESPONSE		15


// NET TSM
typedef struct _NET_TSM {
	unsigned char	State;
	unsigned char	InvokeID;
	unsigned short	Address;
	unsigned long	Timer;
	int				Timeout;
} NET_TSM;


int  netGetRequest(NET *net);
int  netGetResponse(NET *net);
int  netPutRequest(NET *net);
int  netPutResponse(NET *net);
int  netAllocRequest(NET *net);
int  netAllocResponse(NET *net);
void netReduceAllocRequest(NET *net, int Length);
void netReduceAllocResponse(NET *net, int Length);
void netFreeRequest(NET *net);
void netFreeResponse(NET *net);
int  netGetNextInvokeID(void);
void netCodeLogin(NET *net, unsigned char *Buffer, int Length);
void netCodeHeartBeat(NET *net);
void netCodeConfRequest(NET *net, int ServiceChoice);
void netCodeResponse(NET *net);
void netCodeError(NET *net, int ErrorClass, int ErrorCode);
void netCodeReject(NET *net, int RejectReason);

void netLogTx(NET *net, unsigned short SourceAddress);
void netLogRx(NET *net, unsigned short DestinationAddress);

void tsmsInit(NET_TSM *tsms, int size);
NET_TSM *tsmsFind(NET_TSM *tsms, int size, NET *net);
NET_TSM *tsmsAdd(NET_TSM *tsms, int size, NET *net);


#endif	/* _NET_H_ */