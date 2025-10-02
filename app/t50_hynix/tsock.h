#ifndef _TSOCK_H_
#define _TSOCK_H_


#define MAX_TCPWRITE_SZ		16

typedef struct _TSOCK {
	unsigned char	state;
	unsigned char	timeoutCount;
	unsigned char	writeIndex;
	unsigned char	rxHead;
	int				rxLength;
	int				rxCount;
	void			*parent;
	void			*context;
	uv_tcp_t		_tcp;
	uv_connect_t	_conn;
	uv_timer_t		_timer;
	uv_write_t		_writes[MAX_TCPWRITE_SZ];
	unsigned char	txBufs[MAX_TCPWRITE_SZ][MAX_PDU_DATA_SZ+12];
	unsigned char	rxBuf[MAX_PDU_DATA_SZ+12];
	void (*onTimer)(struct _TSOCK *sock);
} TSOCK;

#define MAX_SERVER_SOCK_SZ		32	 

typedef struct _TSOCK_SERVER {
	unsigned char	type;
	unsigned char	state;
	unsigned char	headLen;
	unsigned char	index;
	unsigned char	reserve[2];
	unsigned short	port;
	void			*context;
	uv_tcp_t		_tcp;
	uv_timer_t		_timer;
	void			*loop;
	void (*onOpen)(TSOCK *sock);
	void (*onRead)(TSOCK *sock, int length);
	TSOCK			_tsocks[MAX_SERVER_SOCK_SZ];
} TSOCK_SERVER;

#define MAX_CLIENT_SOCK_SZ		2	 

typedef struct _TSOCK_CLIENT {
	unsigned char	type;
	unsigned char	state;
	unsigned char	headLen;
	unsigned char	index;
	unsigned char	reserve[2];
	unsigned short	port;
	void			*context;
	uv_tcp_t		_tcp;
	uv_timer_t		_timer;
	void			*loop;
	void (*onConnect)(struct _TSOCK_CLIENT *sock);
	void (*onOpen)(TSOCK *sock);
	void (*onRead)(TSOCK *sock, int length);
	unsigned char	ipAddr[4];
	TSOCK			_tsocks[MAX_CLIENT_SOCK_SZ];
} TSOCK_CLIENT;

void tsockServerOpen(TSOCK_SERVER *server, int port, void (*onOpen)(TSOCK *), void (*onRead)(TSOCK *, int));
void tsockServerClose(TSOCK_SERVER *server);

void tsockClientInit(TSOCK_CLIENT *client);
void tsockClientOpen(TSOCK_CLIENT *client, void (*onConnect)(TSOCK_CLIENT *), void (*onOpen)(TSOCK *), void (*onRead)(TSOCK *, int));
void tsockClientClose(TSOCK_CLIENT *client);
void tsockClientCloseSock(TSOCK_CLIENT *client);
void tsockClientSetAddress(TSOCK_CLIENT *client, unsigned char *ipAddr, int port);
TSOCK *tsockClientActiveSock(TSOCK_CLIENT *client);

void tsockInit(TSOCK *sock);
int  tsockOpen(TSOCK *sock, void *loop, unsigned char *ipAddr, int port, void (*onOpen)(TSOCK *, int));
void tsockClose(TSOCK *sock);
int  tsockWrite(TSOCK *sock, int length);
void *tsockWriteBuffer(TSOCK *sock);
void tsockOpenTimer(TSOCK *sock, void (*onTimer)(TSOCK *));
void tsockCloseTimer(TSOCK *sock);
void tsockStartTimer(TSOCK *sock, int timeout);
void tsockStopTimer(TSOCK *sock);


#endif

