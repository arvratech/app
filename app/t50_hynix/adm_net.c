#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "dev.h"
#include "oc.h"
#include "cf.h"
#include "cfprim.h"
#include "pfprim.h"
#include "dlg.h"
#include "pref.h"
#include "nilib.h"
#include "viewprim.h"
#include "actprim.h"
#include "topprim.h"
#include "admtitle.h"
#include "admprim.h"
#include "actalert.h"
#include "appact.h"

void _AdmSysReinitializeDevice(void);


void _AdmNetChanged(void *pss, int index, void *act)
{
	if(index == 1) {
		//PSremoveChildren(pss);
		//AdmNet(pss);
		//PrefReloadData(act);
		syscfgWrite(sys_cfg);
		_AdmSysReinitializeDevice();
	}
}

void _AdmHwDevId(void *pss);

void AdmNet(void *pss)
{
	void	*ps;
	char	temp[64];

	PSsetOnPrefChanged(pss, _AdmNetChanged);
	_AdmHwDevId(pss);
	ps = PSaddSpinner(pss, admSetupNetworkTitle(), syscfgNetworkType, syscfgSetNetworkType);
	PSspinnerAddTitleValue(ps, xmenu_network_type[0], 0);
	PSspinnerAddTitleValue(ps, xmenu_network_type[1], 1);
	if(syscfgNetworkType(sys_cfg)) {
		//ps = PSaddScreen(pss, xmenu_network[5], NULL);
		//PSsetUserAction(ps, AdmNetWifi);
	} else {
		PSaddTitle(pss, admMacAddressTitle() , mac2addr(niMacAddress()), PS_TITLE_STYLE_SUBTITLE);
		ps = PSaddScreen(pss, admSetupIpAddressTitle(), AdmNetLan);
		ps = PSaddTextField(pss, admSetupMasterTitle(), AdmDoneValidateIpAddress);
		PStextFieldAddItem(ps, PStitle(ps), syscfgMasterIpAddressName, syscfgSetMasterIpAddressName, TI_IP_ADDRESS, 15);
		sprintf(temp, "%s.%s", syscfgServerIpAddressName(sys_cfg), syscfgServerIpPortName(sys_cfg));
		PSaddTitle(pss, admServerIpAddressTitle(), temp, PS_TITLE_STYLE_SUBTITLE);
		ps = PSaddScreen(pss, admPingTestTitle(), AdmNetPing);
	}
}

void _AdmNetLanChanged(void *pss, int index, void *act)
{
	if(index == 0) {
		PSremoveChildren(pss);
		AdmNetLan(pss);
		PrefReloadData(act);
	} else if(index == 4) {
		niAddDnsServers();
	}
}

void AdmNetLan(void *pss)
{
	void	*ps;
	unsigned char	ipAddr[4], subnet[4], gwIpAddr[4], dnsIpAddr[4];
	int		rval;

	PSsetOnPrefChanged(pss, _AdmNetLanChanged);
	ps = PSaddCheck(pss, admIpDhcpEnableTitle(), syscfgUseDhcp, syscfgSetUseDhcp);
	if(syscfgUseDhcp(sys_cfg)) {
		rval = niGetIpAddress(ipAddr, subnet, NULL);
		if(rval < 0) {
			ipSetNull(ipAddr); ipSetNull(subnet);
		}
		niGetDefaultGateway(gwIpAddr);
		PSaddTitle(pss, admIpAddressTitle(), inet2addr(ipAddr), PS_TITLE_STYLE_SUBTITLE);
		PSaddTitle(pss, admSubnetMaskTitle(), inet2addr(subnet), PS_TITLE_STYLE_SUBTITLE);
		PSaddTitle(pss, admDefaultGatewayTitle(), inet2addr(gwIpAddr), PS_TITLE_STYLE_SUBTITLE);
	} else {
		ps = PSaddTextField(pss, admIpAddressTitle(), AdmDoneValidateIpAddress);
		PStextFieldAddItem(ps, PStitle(ps), syscfgIpAddressName, syscfgSetIpAddressName, TI_IP_ADDRESS, 15);
		ps = PSaddTextField(pss, admSubnetMaskTitle(), AdmDoneValidateIpAddress);
		PStextFieldAddItem(ps, PStitle(ps), syscfgSubnetMaskName, syscfgSetSubnetMaskName, TI_IP_ADDRESS, 15);
			ps = PSaddTextField(pss, admDefaultGatewayTitle(), AdmDoneValidateIpAddress);
		PStextFieldAddItem(ps, PStitle(ps), syscfgGatewayIpAddressName, syscfgSetGatewayIpAddressName, TI_IP_ADDRESS, 15);
	}
	ps = PSaddTextField(pss, admPrimaryDnsServerTitle(), AdmDoneValidateIpAddress);
	PStextFieldAddItem(ps, PStitle(ps), syscfgDnsIpAddressName, syscfgSetDnsIpAddressName, TI_IP_ADDRESS, 15);
}

unsigned char pingIpAddress[4];

char *PingIpAddressName(void *self)
{
	return inet2addr(pingIpAddress);
}

void PingSetIpAddressName(void *self, char *name)
{
	addr2inet(name, pingIpAddress);
}

void AdmNetPing(void *pss)
{
	void	*ps;
	
	memcpy(pingIpAddress, syscfgServerIpAddress(NULL), 4);
	ps = PSaddTextField(pss, admIpAddressTitle(), AdmDoneValidateIpAddress);
	PStextFieldAddItem(ps, PStitle(ps), PingIpAddressName, PingSetIpAddressName, TI_IP_ADDRESS, 15);
	ps = PSaddScreen(pss, admPingTestTitle(), NULL);
	PSsetUserAction(ps, AdmNetPingRun);
}

