#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "nwpcfg.h"


unsigned char Default_IPAddress[4] =	{ 192, 168, 2, 2 };
unsigned char Default_SubnetMask[4] = 	{ 255, 255, 255, 0 };

void nwpSetDefault(NETWORK_PORT_CFG *nwp_cfg)
{
	nwp_cfg->NetworkType		= NETWORK_TCPIP;
	nwp_cfg->SerialPort			= 1;
	nwp_cfg->SerialSpeed		= 2;
	nwp_cfg->TCPIPOption		= USE_DHCP;
	memset(nwp_cfg->IPAddress, 0, 4);
	memcpy(nwp_cfg->SubnetMask, Default_SubnetMask, 4);
	memcpy(nwp_cfg->GatewayIPAddress, Default_IPAddress, 4); nwp_cfg->GatewayIPAddress[3] = 1; 
}

int nwpEncode(NETWORK_PORT_CFG *nwp_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	*p++ = nwp_cfg->NetworkType;
	*p++ = nwp_cfg->SerialPort;
	*p++ = nwp_cfg->SerialSpeed;
	*p++ = nwp_cfg->TCPIPOption;
	memcpy(p, nwp_cfg->IPAddress, 4); p += 4;
	memcpy(p, nwp_cfg->SubnetMask, 4); p += 4;
	memcpy(p, nwp_cfg->GatewayIPAddress, 4); p += 4;
	return p - (unsigned char *)buf;
}

int nwpDecode(NETWORK_PORT_CFG *nwp_cfg, void *buf)
{
	unsigned char	*p;

	p = (unsigned char *)buf;
	nwp_cfg->NetworkType	= *p++;
	nwp_cfg->SerialPort 	= *p++;
	nwp_cfg->SerialSpeed	= *p++;
	nwp_cfg->TCPIPOption	= *p++;
	memcpy(nwp_cfg->IPAddress, p, 4); p += 4;
	memcpy(nwp_cfg->SubnetMask, p, 4); p += 4;
	memcpy(nwp_cfg->GatewayIPAddress, p, 4); p += 4;
 	return p - (unsigned char *)buf;
}

int nwpValidate(NETWORK_PORT_CFG *nwp_cfg)
{
	return 1;
}

void SetDefaultIPAddress(unsigned char *IPAddress)
{
	memcpy(IPAddress, Default_IPAddress, 4);
}
