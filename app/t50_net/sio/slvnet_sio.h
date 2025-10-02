#ifndef _SLVNET_SIO_H_
#define _SLVNET_SIO_H_


#define MAX_IBUF_SZ				824
#define MAX_IDATA_SZ			812

// SLAVE data structure
typedef struct _SLAVE {
	unsigned char	address;
	unsigned char	state;
	unsigned char	cmd;
	unsigned char	invokeId;
	unsigned char	cmdLoop;
	unsigned char	txSN, rxSN;
	unsigned char	rxError;
	unsigned char	dataRetryCount;
	unsigned char	reserve[1];
	unsigned short	dataSize;
	unsigned long	inactivityTimer;
	unsigned char	ipAddress[4];
	unsigned char	data[MAX_IDATA_SZ];
	unsigned long	commErrorCount;
	unsigned long	crcErrorCount;
	unsigned long	timeoutCount;
	unsigned long	timer;
} SLAVE;


// Timeout definitions
#define INACTIVITY_TIMEOUT		15000	// 15seconds
#define POLL_TIMEOUT			100000	// 100ms
#define RESPONSE_TIMEOUT		30000	//  30ms
#define IDLE_TIMEOUT			5000	//   5ms
//#define IDLE_TIMEOUT			65500


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


void slvnetSioOpen(long baudRate);
void slvnetSioClose(void);
void slvnetSioCheckSanity(void);
void slvnetSiosSubdevDisconnect(SLAVE *slv);
void slvnetSiomSubdevDisconnect(void);


#endif
