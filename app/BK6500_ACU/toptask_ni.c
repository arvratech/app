#include "net.h"
#include "sinnet.h"

int		gCommand;
unsigned long	_SvrBuffer[MAX_SVRBUF_QTR_SZ+64];	// Byte alignment for Wiznet5300 16-bit access


int CheckMACAddress(void)
{
	unsigned char	*p, addr[8];
	int		rval;

	p = syscfgGetMACAddress();
	memset(addr, 0, 6); 
	sockGetMACAddress(addr);
	if(n_memcmp(addr, p, 6)) rval = 1;
	else	rval = 0;
#ifdef BK_DEBUG
cprintf("%ld CheckMacAddress=%d\n", SEC_TIMER, rval);
#endif
	return rval;
}

unsigned long	niTimer;
int		niState;
void (*niCallBack)(int State);

// Network Interface Initialize
void niInit(void (*CallBack)())
{
	int		seed;

	memcpy(&seed, syscfgGetMACAddress()+2, 4);
	seed += (sys_cfg->DeviceID << 16) + sys_cfg->DeviceID;
	dhcpOpen(syscfgGetMACAddress(), seed);
	niCallBack = CallBack;
	niTimer = DS_TIMER - 51;
	niState = 0;
}

// Network Interface Task
void niTask(void)
{
	NETWORK_PORT_CFG	*nwp_cfg;
	unsigned char	IPAddr[4];
	int		rval, link, reset;

	nwp_cfg = &sys_cfg->NetworkPort;
	if(sys_cfg->DeviceID == 9999) link = 0;
	else	link = sockGetLinkStatus();
	reset = 0;
	switch(niState) {
	case 0:		// Init
		if((DS_TIMER-niTimer) > 50L) {
			rval = sockInit();
			if(!rval) {
				sockSetMACAddress(syscfgGetMACAddress());
				IPSetNull(IPAddr);
				sockSetIfConfig(IPAddr, IPAddr, IPAddr);
				niState = 1;		
			} else {
				niTimer = DS_TIMER;
			}
		}
		break;
	case 1:		// Inititalized and Link Down
		if(link) {
			if(nwp_cfg->TCPIPOption & USE_DHCP) {
				rval = dhcpEnable(nwp_cfg->IPAddress);
				if(!rval) {
					niState = 2;
					if(niCallBack) (*niCallBack)(niState);
				} else {
					niTimer = DS_TIMER;
					niState = 0;
				}
			} else {
				sockSetIfConfig(nwp_cfg->IPAddress, nwp_cfg->SubnetMask, nwp_cfg->GatewayIPAddress);
				niState = 3;
				if(niCallBack) (*niCallBack)(niState);
			}
		}
		break;
	case 2:		// Link Up and DHCP Unbounded
		if(link) {
			rval = dhcpTask();
			if(rval) {
				niState = 3;
				if(niCallBack) (*niCallBack)(niState);
				dhcpGetIPAddress(IPAddr);	
				if(n_memcmp(nwp_cfg->IPAddress, IPAddr, 4)) {	
					memcpy(nwp_cfg->IPAddress, IPAddr, 4);
					syscfgWrite(sys_cfg);
				}
			}
		} else {
			reset = 1;	
		}
		break;
	case 3:		// Data Transfer
		if(link) {
			if(dhcpIsEnabled()) {
				rval = dhcpTask();
				if(rval) {
					dhcpGetIPAddress(IPAddr);	
					if(n_memcmp(nwp_cfg->IPAddress, IPAddr, 4)) {	
						memcpy(nwp_cfg->IPAddress, IPAddr, 4);
						syscfgWrite(sys_cfg);
					}
				} else if(!dhcpIsBound()) {
					niState = 2;
					if(niCallBack) (*niCallBack)(niState);
				}
			}
		} else {
			reset = 1;	
		}
		break;
	}
	if(gRequest & G_TCP_IP_CHANGED) {
		gRequest &= ~G_TCP_IP_CHANGED;
		if(niState > 1) reset = 1;
	}
	if(reset) {
		if(dhcpIsEnabled()) dhcpDisable();
		else	sockResetNetwork();
		niState = 1;
		if(niCallBack) (*niCallBack)(niState);
	}
}

int niGetState(void)
{
	return (int)niState;
}
