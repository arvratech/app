#ifndef _DHCP_PRIVATE_H_
#define _DHCP_PRIVATE_H_


// DHCP state machine
#define S_DHCP_NULL				0
#define S_DHCP_INIT				1
#define S_DHCP_INIT_REBOOT		2
#define S_DHCP_SELECTING		3
#define S_DHCP_REQUESTING		4
#define S_DHCP_REBOOTING		5
#define S_DHCP_BOUNDING			6
#define S_DHCP_BOUND			7
#define S_DHCP_RENEWING			8
#define S_DHCP_REBINDING		9


// DHCP structure
typedef struct _DHCP {
	int				fd;
	unsigned char	option;
	unsigned char	state;
	unsigned char	tryCount;
	unsigned char	msgType;
	unsigned long	timer;
	int				ifIndex;
	unsigned long	xid;
	long			leasedTime;
	unsigned long	leasedTimer;
	unsigned char	macAddr[8];
	unsigned char	ipAddr[4];
	unsigned char	gwAddr[4];
	unsigned char	subnet[4];
	unsigned char	dnsAddr[4];
	unsigned char	svrAddr[4];
	unsigned char	realSvrAddr[4];
	unsigned char	toAddr[4];
} DHCP;


#define MAC_BCAST_ADDR		(unsigned char *) "\xff\xff\xff\xff\xff\xff"

//
#define DHCP_FLAGSBROADCAST		0x8000

// UDP port numbers for DHCP
#define	DHCP_SERVER_PORT		67		// from server to client
#define DHCP_CLIENT_PORT		68		// from client to server

// DHCP message OP code
#define DHCP_BOOTREQUEST		1
#define DHCP_BOOTREPLY			2

// DHCP message type
#define	DHCP_DISCOVER			1
#define DHCP_OFFER				2
#define	DHCP_REQUEST			3
#define	DHCP_DECLINE			4
#define	DHCP_ACK				5
#define DHCP_NAK				6
#define	DHCP_RELEASE			7
#define DHCP_INFORM				8

// DHCP RETRANSMISSION TIMEOUT (microseconds)
#define DHCP_INITIAL_RTO		( 4*1000000)
#define DHCP_MAX_RTO			(64*1000000)

#define DHCP_HTYPE10MB			1
#define DHCP_HTYPE100MB			2

#define DHCP_HLENETHERNET		6
#define DHCP_HOPS				0
#define DHCP_SECS				0

#define MAGIC_COOKIE			0x63825363

#define DEFAULT_LEASETIME		0xffffffff	/**< infinite lease time */

// DHCP option and value (cf. RFC1533)
enum {
	padOption				=	0,
	subnetMask				=	1,
	timerOffset				=	2,
	routersOnSubnet			=	3,
	timeServer				=	4,
	nameServer				=	5,
	dns						=	6,
	logServer				=	7,
	cookieServer			=	8,
	lprServer				=	9,
	impressServer			=	10,
	resourceLocationServer	=	11,
	hostName				=	12,
	bootFileSize			=	13,
	meritDumpFile			=	14,
	domainName				=	15,
	swapServer				=	16,
	rootPath				=	17,
	extentionsPath			=	18,
	IPforwarding			=	19,
	nonLocalSourceRouting	=	20,
	policyFilter			=	21,
	maxDgramReasmSize		=	22,
	defaultIPTTL			=	23,
	pathMTUagingTimeout		=	24,
	pathMTUplateauTable		=	25,
	ifMTU					=	26,
	allSubnetsLocal			=	27,
	broadcastAddr			=	28,
	performMaskDiscovery	=	29,
	maskSupplier			=	30,
	performRouterDiscovery	=	31,
	routerSolicitationAddr	=	32,
	staticRoute				=	33,
	trailerEncapsulation	=	34,
	arpCacheTimeout			=	35,
	ethernetEncapsulation	=	36,
	tcpDefaultTTL			=	37,
	tcpKeepaliveInterval	=	38,
	tcpKeepaliveGarbage		=	39,
	nisDomainName			=	40,
	nisServers				=	41,
	ntpServers				=	42,
	vendorSpecificInfo		=	43,
	netBIOSnameServer		=	44,
	netBIOSdgramDistServer	=	45,
	netBIOSnodeType			=	46,
	netBIOSscope			=	47,
	xFontServer				=	48,
	xDisplayManager			=	49,
	dhcpRequestedIPaddr		=	50,
	dhcpIPaddrLeaseTime		=	51,
	dhcpOptionOverload		=	52,
	dhcpMessageType			=	53,
	dhcpServerIdentifier	=	54,
	dhcpParamRequest		=	55,
	dhcpMsg					=	56,
	dhcpMaxMsgSize			=	57,
	dhcpT1value				=	58,
	dhcpT2value				=	59,
	dhcpClassIdentifier		=	60,
	dhcpClientIdentifier	=	61,
	endOption				=	255
};

// for the DHCP message
typedef struct _RIP_MSG {
	unsigned char	op; 
	unsigned char	htype; 
	unsigned char	hlen;
	unsigned char	hops;
	unsigned long	xid;
	unsigned short	secs;
	unsigned short	flags;
	unsigned char	ciaddr[4];
	unsigned char	yiaddr[4];
	unsigned char	siaddr[4];
	unsigned char	giaddr[4];
	unsigned char	chaddr[16];
	unsigned char	sname[64];
	unsigned char	file[128];
	unsigned char	OPT[312];
} RIP_MSG;

#define MAX_DHCP_OPT	16


#endif

