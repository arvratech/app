#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "rtc.h"
#include "syscfg.h"
#include "uv.h"
#include "dev.h"
#include "nwp.h"
#include "txn.h"
#include "tsock.h"


void tsockServerInit(TSOCK_SERVER *server)
{
}

TSOCK *_TsockServerAdd(TSOCK_SERVER *server)
{
	TSOCK	*sock;
	int		i;

	i = server->index;
	while(1) {
		sock = &server->_tsocks[i];
		if(!sock->state) break;
		i++; if(i >= MAX_SERVER_SOCK_SZ) i = 0;
		if(i == server->index) {
			sock = NULL;
			break;
		}
	}
	if(sock) {
		tsockInit(sock);
		i++; if(i >= MAX_SERVER_SOCK_SZ) i = 0;
		server->index = i;
	}
	return sock;
}

void _TsockServerRemove(TSOCK_SERVER *server, TSOCK *sock)
{
	sock->state = 0;
}

void _TsockServerRemoveAll(TSOCK_SERVER *server)
{
	TSOCK	*sock;
	int		i;

	for(i = 0, sock = server->_tsocks;i < MAX_SERVER_SOCK_SZ;i++, sock++)
		_TsockServerRemove(server, sock);
}

static void _OnTsockAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
static void _OnTsockRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf);

static void _OnTsockServerConnect(uv_stream_t *tcpServer, int status)
{
	TSOCK_SERVER	*server;
	TSOCK	*sock;
	uv_tcp_t	*tcp, _tcp;
	int		rval;

	server = tcpServer->data;
	if(status < 0) {
printf("### %lu _OnTsockServerConnect error: %s\n", SEC_TIMER, uv_err_name(status));
		return;
    }
	sock = _TsockServerAdd(server);
	if(sock) tcp = &sock->_tcp;
	else	 tcp = &_tcp;
	uv_tcp_init(server->loop, tcp);
	if(sock) sock->state = 2;
	rval = uv_accept(tcpServer, (uv_stream_t *)tcp);
	if(rval == 0 && sock) {
		tcp->data = sock;
		rval = uv_read_start((uv_stream_t *)tcp, _OnTsockAlloc, _OnTsockRead);
		sock->state = 3;
		sock->rxCount = 0; sock->rxHead = 0; sock->rxLength = server->headLen;
		(*server->onOpen)(sock);
	} else {
printf("### %lu _OnTsockServerConnect: accept overflow\n", SEC_TIMER);
		if(sock) tsockClose(sock); 
		else	uv_close((uv_handle_t *)tcp, NULL);
    }
}

static void _OnTsockServerTimer(uv_timer_t *timer)
{
	TSOCK_SERVER		*server;
	struct sockaddr_in	*addr, _addr;
	void	*nwp;
	unsigned char	ipAddr[4];
	int		rval, i;

	server = timer->data;
	nwp = nwpsGet(0);
	if(nwpIsOpened(nwp)) {
		rval = uv_tcp_init(server->loop, &server->_tcp);
		memset(ipAddr, 0, 4);
		addr = &_addr;
		memset(addr, 0, sizeof(struct sockaddr_in));
		addr->sin_family = AF_INET;
		memcpy(&addr->sin_addr.s_addr, ipAddr, 4);
		addr->sin_port = htons(server->port);
		uv_tcp_bind(&server->_tcp, (const struct sockaddr *)addr, 0);
		server->_tcp.data = server;
		rval = uv_listen((uv_stream_t *)&server->_tcp, 16, _OnTsockServerConnect);
		if(rval) {
			printf("_OnTsockServerTimer: uv_listen() error: %s\n", uv_strerror(rval));
			uv_close((uv_handle_t *)&server->_tcp, NULL);
		} else {
			server->state = 2;
printf("%lu _OnTsockServerTimer: server opened: %d\n", SEC_TIMER, (int)server->port);
			uv_timer_stop(timer);
		}
	} else {
		uv_timer_start(timer, _OnTsockServerTimer, 700, 0);
	}
}

void tsockServerOpen(TSOCK_SERVER *server, int port, void (*onOpen)(TSOCK *), void (*onRead)(TSOCK *, int))
{
	TSOCK	*sock;
	int		i;

	server->type	= 1;
	server->state	= 0;
	server->index	= 0;
	server->port	= port;
	server->onOpen	= onOpen;
	server->onRead	= onRead;
	uv_timer_init(server->loop, &server->_timer);
	for(i = 0, sock = server->_tsocks;i < MAX_SERVER_SOCK_SZ;i++, sock++) {
		tsockInit(sock);
		sock->parent = server;
	}
	server->_timer.data = server;
	uv_timer_start(&server->_timer, _OnTsockServerTimer, 700, 0);
printf("tsockServerOpen...\n");
}

void tsockServerClose(TSOCK_SERVER *server)
{
	if(server->state == 0) uv_timer_stop(&server->_timer);
	if(server->state) {
		uv_close((uv_handle_t *)&server->_tcp, NULL);
		server->state = 0;
	}
printf("tsockServerClose...\n");
}

TSOCK *_TsockClientAdd(TSOCK_CLIENT *client)
{
	TSOCK	*sock;
	int		i;

	i = client->index;
	while(1) {
		sock = &client->_tsocks[i];
		if(!sock->state) break;
		i++; if(i >= MAX_CLIENT_SOCK_SZ) i = 0;
		if(i == client->index) {
			sock = NULL;
			break;
		}
	}
	if(sock) {
		tsockInit(sock);
		i++; if(i >= MAX_CLIENT_SOCK_SZ) i = 0;
		client->index = i;
	}
	return sock;
}

void _TsockClientRemove(TSOCK_CLIENT *client, TSOCK *sock)
{
	sock->state = 0;
}

void _TsockClientRemoveAll(TSOCK_CLIENT *client)
{
	TSOCK	*sock;
	int		i;

	for(i = 0, sock = client->_tsocks;i < MAX_CLIENT_SOCK_SZ;i++, sock++)
		_TsockClientRemove(client, sock);
}

static void _OnTsockClientTimer(uv_timer_t *timer);

static void _OnTsockClientConnect(uv_connect_t *conn, int status)
{
	TSOCK_CLIENT	*client;
	TSOCK	*sock;
	int		rval;

	sock = conn->data;
	client = sock->parent;
	if(status) {
		// 3 seconds elapse EHOSTUNREACH after uv_tcp_connect()
printf("### %lu _OnTsockClientConnect(%s.%d) error: %s\n", SEC_TIMER, 
					inet2addr(client->ipAddr), (int)client->port, uv_err_name(status));
		//if(status != -ECANCELED) uv_close((uv_handle_t *)&sock->_tcp, NULL);
		tsockClose(sock);
        return;
    }
	sock->state	= 3;
	rval = uv_read_start(conn->handle, _OnTsockAlloc, _OnTsockRead);
	sock->rxCount = 0; sock->rxHead = 0; sock->rxLength = client->headLen;
	if(client->onOpen) (*client->onOpen)(sock);
}

static void _OnTsockClientTimer(uv_timer_t *timer)
{
	TSOCK_CLIENT	*client;
	TSOCK	*sock;
	struct sockaddr_in	*addr, _addr;
	void	*nwp;
	int		rval, i, on;

	client = timer->data;
	nwp = nwpsGet(0);
	if(nwpIsOpened(nwp)) {
		if(client->onConnect) (*client->onConnect)(client);
		sock = _TsockClientAdd(client);
		rval = uv_tcp_init(client->loop, &sock->_tcp);
		addr = &_addr;
		memset(addr, 0, sizeof(struct sockaddr_in));
		addr->sin_family = AF_INET;
		memcpy(&addr->sin_addr.s_addr, client->ipAddr, 4);
		addr->sin_port = htons(client->port);
		sock->_tcp.data = sock;
		sock->_conn.data = sock;
		sock->state = 2;
//setsockopt(sock->_tcp.io_watcher.fd, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on)))
		rval = uv_tcp_connect(&sock->_conn, &sock->_tcp, (const struct sockaddr *)addr, _OnTsockClientConnect);
		if(rval) {
			printf("tsockOpenClient: uv_connect() error: %s\n", uv_strerror(rval));
			tsockClose(sock);
		} else {
			uv_timer_stop(timer);
		}
	} else {
		uv_timer_start(&client->_timer, _OnTsockClientTimer, 700, 0);
	}
}

void tsockClientOpen(TSOCK_CLIENT *client, void (*onConnect)(TSOCK_CLIENT *), void (*onOpen)(TSOCK *), void (*onRead)(TSOCK *, int))
{
	TSOCK	*sock;
	int		i;

signal(SIGPIPE, SIG_IGN);
	client->type		= 0;
	client->state		= 0;
	client->index		= 0;
	memset(client->ipAddr, 0, 4);
	client->port		= 0;
	client->onConnect	= onConnect;
	client->onOpen		= onOpen;
	client->onRead		= onRead;
	for(i = 0, sock = client->_tsocks;i < MAX_CLIENT_SOCK_SZ;i++, sock++) {
		tsockInit(sock);
		sock->parent = client;
	}
	uv_timer_init(client->loop, &client->_timer);
	client->_timer.data = client;
	uv_timer_start(&client->_timer, _OnTsockClientTimer, 700, 0);
printf("tsockClientOpen...\n");
}

void tsockClientClose(TSOCK_CLIENT *client)
{
	uv_timer_stop(&client->_timer);
	client->state = 0;
printf("tsockClientClose...\n");
}

void tsockClientCloseSock(TSOCK_CLIENT *client)
{
	TSOCK	*sock;
	int		i;

	for(i = 0, sock = client->_tsocks;i < MAX_CLIENT_SOCK_SZ;i++, sock++) {
		if(sock->state > 0) tsockClose(sock);
	}
}

void tsockClientSetAddress(TSOCK_CLIENT *client, unsigned char *ipAddr, int port)
{
	memcpy(client->ipAddr, ipAddr, 4);
	client->port	= port;
}

TSOCK *tsockClientActiveSock(TSOCK_CLIENT *client)
{
	TSOCK	*sock;
	int		i;

	for(i = 0, sock = client->_tsocks;i < MAX_CLIENT_SOCK_SZ;i++, sock++)
		if(sock->state >= 3) break;
	if(i >= MAX_CLIENT_SOCK_SZ) sock = NULL;
	return sock;
}

void tsockInit(TSOCK *sock)
{
	sock->state			= 0;
	sock->writeIndex	= 0;
	sock->context		= NULL;
	sock->onTimer		= NULL;
}

static int _TsockParentType(TSOCK *sock)
{
	TSOCK_SERVER	*server;

	server = sock->parent;
	return (int)server->type;
}

void tsockClose(TSOCK *sock)
{
	TSOCK_SERVER	*server;
	TSOCK_CLIENT	*client;
	unsigned long	data;
	int		val;

	server = sock->parent;
	if(server->type) client = NULL;
	else {
		client = (TSOCK_CLIENT *)server; server = NULL;
	}
//printf("sock->state=%d client=%x server=%x\n", (int)sock->state, client, server);
	if(sock->state > 2) {
		uv_read_stop((uv_stream_t *)&sock->_tcp);
	}
	if(sock->state > 1) {
		uv_close((uv_handle_t *)&sock->_tcp, NULL);
	}
	if(server) {
		_TsockServerRemove(server, sock); 
	} else {
		_TsockClientRemove(client, sock);
		data = rand();
		val = data & 0xfff; val -= 2048;	// -2048 .. 2047
		val = 6000 + val;		// 3952 msec .. 8047 msec 
		uv_timer_start(&client->_timer, _OnTsockClientTimer, val, 0);
	}
//printf("tscokClose...\n");
}

static void _OnTsockAlloc(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	TSOCK	*sock;

	sock = handle->data;
	buf->base = sock->rxBuf + sock->rxCount;
	buf->len  = sock->rxLength - sock->rxCount;
} 

static void _OnTsockRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf)
{
	TSOCK_CLIENT	*client;
	TSOCK_SERVER	*server;
	TSOCK	*sock;
	unsigned char	*p;
	unsigned short	sval;
	int		val, rval, len, address, head, headLen;

	sock = stream->data;
	server = sock->parent;
	if(server->type) {
		client = NULL; headLen = server->headLen;
	} else {
		client = (TSOCK_CLIENT *)server; server = NULL; headLen = client->headLen;
	}
	if(nread < 0) {
printf("### %lu _OnTsockRead error: %d %s\n", SEC_TIMER, nread, uv_strerror(nread));
		if(server && server->onRead) (*server->onRead)(sock, -1);
		else if(client && client->onRead) (*client->onRead)(sock, -1);
		sock->rxCount = 0; sock->rxHead = 0; sock->rxLength = headLen;
	} else if(nread > 0) {
		sock->rxCount += nread;
		if(sock->rxCount >= headLen) {
			rval = 0;
			if(!sock->rxHead) {
				p = sock->rxBuf;
				val = *p++;						// Link(OnTCP): ProtocolID
				if(val != PROTOCOL_ID) rval = -2;
				BYTEtoSHORT(p, &sval); p += 2;	// Link(OnTCP): Length
				len = sval;
				if(len < headLen-3 || len > MAX_PDU_DATA_SZ+headLen-3) rval = -3;
				address = *p++;					// Address
				sock->rxHead = 1;
				sock->rxLength = len + 3;
			}
			if(rval) {
				p = sock->rxBuf;
printf("head [%02x", (int)p[0]); for(val=1;val < sock->rxCount;val++) printf("-%02x", (int)p[val]); printf("]\n");
				if(server && server->onRead) (*server->onRead)(sock, 0);
				else if(client && client->onRead) (*client->onRead)(sock, 0);
				sock->rxCount = 0; sock->rxHead = 0; sock->rxLength = headLen;
			} else if(sock->rxHead && sock->rxCount >= sock->rxLength) {
				if(server && server->onRead) (*server->onRead)(sock, sock->rxCount);
				else if(client && client->onRead) (*client->onRead)(sock, sock->rxCount);
				sock->rxCount = 0; sock->rxHead = 0; sock->rxLength = headLen;
			}
		}
	} else {
//printf("### _OnTsockRead: %d\n", nread);
	}
}

static void _OnTsockWrite(uv_write_t *req, int status)
{
//printf("### _OnTsockWrite: %d\n", status);
}

int tsockWrite(TSOCK *sock, int length)
{
	uv_buf_t	uvbuf;
	int		rval, len;

//for(len = 0;len < length;len++) printf("-%02x", (int)sock->txBufs[sock->writeIndex][len]); printf("\n"); 
//printf("Tx: type=%02x data=%d\n", (int)pdu1->type, pdu1->dataLength);
	uvbuf.base = sock->txBufs[sock->writeIndex];
	uvbuf.len  = length;
	rval = uv_write(&sock->_writes[sock->writeIndex], (uv_stream_t *)&sock->_tcp, &uvbuf, 1, _OnTsockWrite);
	sock->writeIndex++; if(sock->writeIndex >= MAX_TCPWRITE_SZ) sock->writeIndex = 0;
	if(rval) {
		printf("tsockWrite: uv_write() error: %s\n", uv_strerror(rval));
		rval = -1;
	}
	return rval;
}

void *tsockWriteBuffer(TSOCK *sock)
{
	return sock->txBufs[sock->writeIndex];
}

static void _OnTsockTimer(uv_timer_t *timer)
{
	TSOCK	*sock;

	sock = timer->data;
	if(sock->onTimer) (*sock->onTimer)(sock);
}

void tsockOpenTimer(TSOCK *sock, void (*onTimer)(TSOCK *))
{
	TSOCK_CLIENT	*client;
	TSOCK_SERVER	*server;
	void	*loop;

	server = sock->parent;
	if(server->type) {
		loop = server->loop;
	} else {
		client = (TSOCK_CLIENT *)server; loop = client->loop;
	}
	uv_timer_init(loop, &sock->_timer);
	sock->onTimer = onTimer;
	sock->_timer.data = sock;
}

void tsockCloseTimer(TSOCK *sock)
{
	uv_timer_stop(&sock->_timer);
}

void tsockStartTimer(TSOCK *sock, int timeout)
{
	uv_timer_start(&sock->_timer, _OnTsockTimer, timeout, 0);
}

void tsockStopTimer(TSOCK *sock)
{
	uv_timer_stop(&sock->_timer);
}

