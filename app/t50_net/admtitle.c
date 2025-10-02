#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "dirent.h"
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "dev.h"
#include "hw.h"
#include "crcred.h"
#include "cf.h"
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"
#include "wpaprim.h"
#include "viewprim.h"
#include "actprim.h"
#include "pref.h"
#include "topprim.h"
#include "admprim.h"


char *admTopTitle(void)
{
	return xmenu_top_title;
}

char *admYesTitle(void)
{
	return xmenu_yesno[0];
}

char *admNoTitle(void)
{
	return xmenu_yesno[1];
}

char *admEnableTitle(void)
{
	return  xmenu_enable[0];
}

char *admDisableTitle(void)
{
	return  xmenu_enable[1];
}

char *admConfirmTitle(void)
{
	return  xmenu_okcancel[0];
}

char *admCancelTitle(void)
{
	return  xmenu_okcancel[1];
}

char *admCompleteTitle(void)
{
	return  xmenu_okcancel[2];
}

char *admSaveTitle(void)
{
	return  xmenu_okcancel[3];
}

char *admDeleteTitle(void)
{
	return  xmenu_okcancel[4];
}

char *admBitUnitTitle(void)
{
	return  xmenu_unit[0];
}

char *admDigitUnitTitle(void)
{
	return  xmenu_unit[1];
}

char *admByteUnitTitle(void)
{
	return  xmenu_unit[2];
}

char *admSecondUnitTitle(void)
{
	return  xmenu_unit[3];
}

char *admMinuteUnitTitle(void)
{
	return  xmenu_unit[4];
}

char *admNetworkPortsTitle(void)
{
	return xmenu_hardware_class[0];
}

char *admUnitsTitle(void)
{
	return xmenu_hardware_class[1];
}

char *admIounitsTitle(void)
{
	return xmenu_hardware_class[2];
}

char *admSubdevicesTitle(void)
{
	return xmenu_hardware_class[3];
}

char *admReadersTitle(void)
{
	return xmenu_hardware_class[4];
}

char *admInputsTitle(void)
{
	return xmenu_hardware_class[5];
}

char *admOutputsTitle(void)
{
	return xmenu_hardware_class[6];
}

char *admDoorsTitle(void)
{
	return xmenu_hardware_class[7];
}

char *admZonesTitle(void)
{
	return xmenu_hardware_class[8];
}

char *admFireZonesTitle(void)
{
	return xmenu_hardware_class[9];
}

char *admIntrusionZonesTitle(void)
{
	return xmenu_hardware_class[10];
}

char *admElevatorsTitle(void)
{
	return xmenu_hardware_class[11];
}

char *admInputsOutputsTitle(void)
{
	return xmenu_device[0];
}

char *admMasterNetworkPortTitle(void)
{
	return xmenu_device[1];
}

char *admSlaveDevicesTitle(void)
{
	return xmenu_device[2];
}

char *admSetupNetworkTitle(void)
{
	return xmenu_network[0];
}

char *admSetupIpAddressTitle(void)
{
	return xmenu_network[1];
}

char *admSetupServerTitle(void)
{
	return xmenu_network[2];
}

char *admServerIpAddressTitle(void)
{
	return xmenu_network[3];
}

char *admServerIpPortTitle(void)
{
	return xmenu_network[4];
}

char *admSetupMasterTitle(void)
{
	return xmenu_network[5];
}

char *admMasterIpAddressTitle(void)
{
	return xmenu_network[6];
}

char *admMasterIpPortTitle(void)
{
	return xmenu_network[7];
}

char *admPingTestTitle(void)
{
	return xmenu_network[8];
}

char *admPingRunTitle(void)
{
	return xmenu_network[9];
}

char *admMacAddressTitle(void)
{
	return xmenu_network_ip[0];
}

char *admIpDhcpEnableTitle(void)
{
	return xmenu_network_ip[1];
}

char *admIpAddressTitle(void)
{
	return xmenu_network_ip[2];
}

char *admSubnetMaskTitle(void)
{
	return xmenu_network_ip[3];
}

char *admDefaultGatewayTitle(void)
{
	return xmenu_network_ip[4];
}

char *admIpDhcpDnsEnableTitle(void)
{
	return xmenu_network_ip[5];
}

char *admPrimaryDnsServerTitle(void)
{
	return xmenu_network_ip[6];
}

char *admSecondaryDnsServerTitle(void)
{
	return xmenu_network_ip[7];
}

char *admSerialSpeedTitle(void)
{
	return xmenu_network_serial[0];
}

char *admSerialProtocolTitle(void)
{
	return xmenu_network_serial[1];
}

char *admWifiSsidTitle(void)
{
	return xmenu_network_wifi[0];
}

char *admWifiStateTitle(void)
{
	return xmenu_network_wifi[1];
}

char *admWifiSignalStrengthTitle(void)
{
	return xmenu_network_wifi[2];
}

char *admWifiChannelTitle(void)
{
	return xmenu_network_wifi[3];
}

char *admWifiSecurityTitle(void)
{
	return xmenu_network_wifi[4];
}

char *admWifiForgetTitle(void)
{
	return xmenu_network_wifi[5];
}

char *admWifiConnectTitle(void)
{
	return xmenu_network_wifi[6];
}

char *admWifiUnchangedTitle(void)
{
	return xmenu_network_wifi[7];
}

char *admAddressTitle(void)
{
	return xmenu_hardware[0];
}

char *admDeviceIdTitle(void)
{
	return xmenu_hardware[1];
}

char *admNetworkAddressTitle(void)
{
	return xmenu_hardware[2];
}

char *admModelNameTitle(void)
{
	return xmenu_hardware[3];
}

char *admAssignedToTitle(void)
{
	return xmenu_hardware[4];
}

char *admTypeTitle(void)
{
	return xmenu_hardware[5];
}

char *admChannelTitle(void)
{
	return xmenu_hardware[6];
}

char *admNetworkPortTitle(void)
{
	return xmenu_hardware[7];
}

char *admReaderTitle(void)
{
	return xmenu_hardware[8];
}

char *admReaderDeviceTitle(void)
{
	return xmenu_hardware[9];
}

char *admUnitTitle(void)
{
	return xmenu_hardware[10];
}

char *admUnitChTitle(int chId)
{
	return xmenu_unit_ch[CH2UNITCH(chId)];
}

char *admPrimaryReaderTitle(void)
{
	return xmenu_door[0];
}

char *admSecondaryReaderTitle(void)
{
	return xmenu_door[1];
}

char *admIosetTitle(void)
{
	return xmenu_door[2];
}

char *admDoorLockRelayTitle(void)
{
	return xmenu_door[3];
}

char *admExitButtonInputTitle(void)
{
	return xmenu_door[4];
}

char *admDoorStatusDetectInputTitle(void)
{
	return xmenu_door[5];
}

char *admAlarmTitle(void)
{
	return xmenu_door[6];
}

char *admAlarmRelayTitle(void)
{
	return xmenu_door[7];
}

char *admAlarmCommandTitle(void)
{
	return xmenu_door[8];
}

char *admDoorPulseTimeTitle(void)
{
	return xmenu_door[9];
}

char *admDoorExPulseTimeTitle(void)
{
	return xmenu_door[10];
}

char *admLockTypeTitle(void)
{
	return xmenu_door[11];
}

char *admTimeDelayTitle(void)
{
	return xmenu_alarm[0];
}

char *admTimeDelayNormalTitle(void)
{
	return xmenu_alarm[1];
}

char *admDoorOpenTooLongTimeTitle(void)
{
	return xmenu_alarm[2];
}

char *admAlarmInhibitTitle(void)
{
	return xmenu_alarm[3];
}

char *admDoorOpenTooLongAlarmTitle(void)
{
	return xmenu_action_idx[5];
}

char *admDoorForcedOpenAlarmTitle(void)
{
	return xmenu_action_idx[6];
}

char *admIoPolarityTitle(void)
{
	return xmenu_input_output[0];
}

char *admFireZoneTitle(void)
{
	return xmenu_fire_zone[0];
}

char *admFireInputsTitle(void)
{
	return xmenu_fire_zone[1];
}

char *admUnlockDoorsTitle(void)
{
	return xmenu_fire_zone[2];
}

char *admAlarmRelaysTitle(void)
{
	return xmenu_fire_zone[3];
}

char *admFormatSettingTitle(void)
{
	return xmenu_ac[0];
}

char *admAlarmCommandsTitle(void)
{
	return xmenu_ac[1];
}

char *admInputFormatsTitle(void)
{
	return xmenu_ac[2];
}

char *admWiegandOutputFormatsTitle(void)
{
	return xmenu_ac[3];
}

char *admSvrnetRemoteTitle(void)
{
	return xmenu_net_remote[0];
}

char *admSlvnetRemoteTitle(void)
{
	return xmenu_net_remote[1];
}

char *admKernelVersionTitle(void)
{
	return  xmenu_other[0];
}

char *admFirmwareVersionTitle(void)
{
	return  xmenu_other[1];
}

char *admModelTitle(void)
{
	return  xmenu_other[2];
}

char *admUnlimitedTitle(void)
{
	return  xmenu_other[3];
}

char *admAllTitle(void)
{
	return  xmenu_other[4];
}

char *admInputVoltageTitle(void)
{
	return  xmenu_other[5];
}

char *admBatteryTitle(void)
{
	return  xmenu_other[6];
}

char *admFileSizeTitle(void)
{
	return  xmenu_other[7];
}

char *admUnspecifiedTitle(void)
{
	return  xmenu_other[8];
}

char *admNeverTitle(void)
{
	return  xmenu_other[9];
}

char *admAlwaysTitle(void)
{
	return  xmenu_other[10];
}

char *admScheduleTitle(void)
{
	return  xmenu_other[11];
}

char *admMasterPasswordTitle(void)
{
	return  xmenu_other[12];
}

char *admEventCountTitle(void)
{
	return  xmenu_other[13];
}

char *admUserCountTitle(void)
{
	return  xmenu_other[14];
}

