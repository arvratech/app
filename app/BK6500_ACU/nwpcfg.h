#ifndef _NWP_CFG_H_
#define _NWP_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


// Network Type definitions
#define NETWORK_SERIAL			1
#define NETWORK_DIALUP			2
#define NETWORK_TCPIP			3

// TCPIPOption definitions
#define USE_DHCP			0x01


typedef struct _NETWORK_PORT_CFG {
	unsigned char	NetworkType;
	unsigned char	SerialPort;
	unsigned char	SerialSpeed;
	unsigned char	TCPIPOption;
	unsigned char	IPAddress[4];
	unsigned char	SubnetMask[4];
	unsigned char	GatewayIPAddress[4];
} NETWORK_PORT_CFG;


_EXTERN_C void nwpSetDefault(NETWORK_PORT_CFG *nwp_cfg);
_EXTERN_C int  nwpEncode(NETWORK_PORT_CFG *nwp_cfg, void *buf);
_EXTERN_C int  nwpDecode(NETWORK_PORT_CFG *nwp_cfg, void *buf);
_EXTERN_C int  nwpValidate(NETWORK_PORT_CFG *nwp_cfg);
_EXTERN_C void SetDefaultIPAddress(unsigned char *IPAddress);


#endif	/* _NWP_CFG_H_ */