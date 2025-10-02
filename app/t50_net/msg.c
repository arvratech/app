#include <stdio.h>
#include <string.h>
#include "lang.h"
#include "msg.h"

#include "msgeng.c"
#ifdef _JPN
#include "msgjpn.c"
#else
#include "msgkor.c"
#endif

#define MB_ICONNONE				0
#define MB_ICONINFORMATION		1
#define MB_ICONWARNING			2
#define MB_ICONERROR			3
#define MB_ICONWAITING			4

unsigned char xrsp_msgs_icon[MAX_RSP_MSGS_COUNT] = {
	MB_ICONWAITING,			//  0 R_PROCESSING
	MB_ICONERROR,			//  1 R_SYSTEM_ERROR
	MB_ICONWARNING,			//  2 R_INPUT_TIME_OVER
	MB_ICONWARNING,			//  3 R_INVALID_USER_ID
	MB_ICONWARNING,			//  4 R_INVALID_DATA
	MB_ICONWARNING,			//  5 R_USER_NOT_FOUND
	MB_ICONWARNING,			//  6 R_ACCESS_DENIED
	MB_ICONINFORMATION,		//  7 R_ACCESS_GRANTED		
	MB_ICONWARNING,			//  8 R_USER_ID_OVERLAPPED	
	MB_ICONWARNING,			//  9 R_CARD_DATA_OVERLAPPED
	MB_ICONWARNING,			// 10 R_FP_DATA_OVERLAPPED
	MB_ICONWARNING,			// 11 R_PW_MISMATCHED	
	MB_ICONWARNING,			// 12 R_DATA_MISMATCHED
	MB_ICONINFORMATION,		// 13 R_DATA_SAVED
	MB_ICONINFORMATION,		// 14 R_USER_DELETED
	MB_ICONINFORMATION,		// 15 R_ALL_USER_DELETED
	MB_ICONWARNING,			// 16 R_INVALID_TIME_INTERVAL
	MB_ICONWARNING,			// 17 R_INVALID_TID
	MB_ICONWARNING,			// 18 R_INVALID_DATE
	MB_ICONINFORMATION,		// 19 R_DATE_CHANGED
	MB_ICONWARNING,			// 20 R_INVALID_TIME			
	MB_ICONINFORMATION,		// 21 R_TIME_CHANGED
	MB_ICONWARNING,			// 22 R_INVALID_IP_ADDRESS
	MB_ICONWARNING,			// 23 R_INVALID_SUBNET_MASK
	MB_ICONWARNING,			// 24 R_INVALID_GW_ADDRESS
	MB_ICONWARNING,			// 25 R_INVALID_SERVER_HOST
	MB_ICONWARNING,			// 26 R_INVALID_SERVER_PORT
	MB_ICONINFORMATION,		// 27 R_ALL_DELETED
	MB_ICONWARNING,			// 28 R_USER_ARE_FULL
	MB_ICONWARNING,			// 29 R_INVALID_EXT_NUMBER
	MB_ICONWARNING,			// 30 R_TEL_NO_DIALTONE
	MB_ICONWARNING,			// 31 R_TEL_BUSY
	MB_ICONWARNING,			// 32 R_TEL_TEL_NUMBER_UNOBTAINABLE
	MB_ICONWARNING,			// 33 R_TEL_NO_ANSWER
	MB_ICONWARNING,			// 34 R_TEL_DISCONNECTED
	MB_ICONINFORMATION,		// 35 R_TEL_CONNECTED
	MB_ICONINFORMATION,		// 36 R_DOOR_UNLOCKED
	MB_ICONINFORMATION,		// 37 R_PROCESSED
	MB_ICONWARNING,			// 38 R_FAILED
	MB_ICONINFORMATION,		// 39 R_COMPLETED
	MB_ICONWARNING,			// 40 R_NOT_ALLOWED_CARD
	MB_ICONWARNING,			// 41 R_TOUCH_SINGLE_CARD
	MB_ICONWARNING,			// 42 R_NOT_FOUND
	MB_ICONWARNING,			// 43 R_OVERLAPPED
	MB_ICONWARNING,			// 44 R_DATA_ARE_FULL
	MB_ICONINFORMATION,		// 45 R_DELETED	
	MB_ICONWARNING,			// 46 R_NO_SD_DISK
	MB_ICONWARNING,			// 47 R_NOT_READY_SD_DISK
	MB_ICONERROR,			// 48 R_DIR_OPEN_ERROR
	MB_ICONERROR,			// 49 R_DIR_READ_ERROR
	MB_ICONERROR,			// 50 R_DIR_WRITE_ERROR
	MB_ICONERROR,			// 51 R_FILE_OPEN_ERROR
	MB_ICONERROR,			// 52 R_FILE_READ_ERROR
	MB_ICONERROR,			// 53 R_FILE_WRITE_ERROR
	MB_ICONERROR,			// 54 R_FILE_SYNC_ERROR
	MB_ICONWARNING,			// 55 R_DIRECTORY_NOT_FOUND
	MB_ICONWARNING,			// 56 R_FILE_NOT_FOUND
	MB_ICONWARNING,			// 57 R_UNSUPPORTED_SECURITY
	MB_ICONINFORMATION,		// 58 R_SYSTEM_RESTART
	MB_ICONINFORMATION,		// 59 R_SAFELY_REMOVE_SD_DISK
	MB_ICONERROR,			// 60 R_SD_DISK_ERROR_CHECK
	MB_ICONERROR,			// 61 R_ACCESS_DENIED_FAIL_SERVER
	MB_ICONERROR,			// 62 R_ACU_TIMEOUT
	MB_ICONWARNING,			// 63 R_ACU_WRITE_CONFLICT
	MB_ICONERROR,			// 64 R_ACU_OTHER_ERROR
	MB_ICONERROR,			// 65 R_NETWORK_NOT_OBTAINABLE
	MB_ICONERROR,			// 66 R_NETWORK_FAILED
	MB_ICONERROR,			// 67 R_SERVER_NOT_RESPONDED
	MB_ICONERROR,			// 66 R_TAMPER_ALARM
	MB_ICONERROR,			// 67 R_POWER_FAIL
	MB_ICONERROR,			// 68 R_POWER_OFF
	MB_ICONINFORMATION,		// 69 R_SLAVE_INITIALIZING
};

char _xmenu_null[MAX_MSG_LEN] = "";
char _xmenu_unsupported[MAX_MSG_LEN] = "Unsupported";

char _xmenu_serial_speed[6][MAX_MSG_LEN] = {
	"Automatic",
	"9600",
	"19200",
	"38400",
	"57600",
	"115200"
};

char _xmenu_parity[3][MAX_MSG_LEN] = {
	"None",
	"Odd parity",
	"Even parity"
};

char _xmenu_stop_bits[2][MAX_MSG_LEN] = {
	"1 stop bit",
	"2 stop bits",
};

char *xmenu_null;
char *xmenu_unsupported;
char *xmenu_serial_speed[6];
char *xmenu_parity[3];
char *xmenu_stop_bits[2];
char *xmsgs[MAX_MSGS_COUNT];
char *xrsp_msgs[MAX_RSP_MSGS_COUNT];
char *xauth_msgs[9];
char *xalarm_msgs[4];
char *xta_msgs[MAX_TA_MSGS_COUNT];
char *xmenu_top_title;
char *xmenu_top[6];
char *xmenu_lang[2];
char *xmenu_user[3];
char *xmenu_user_reg[7];
char *xmenu_user_regex[4];
char *xmenu_access_mode[7];
char *xmenu_hardware_class[12];
char *xmenu_device[3];
char *xmenu_hardware[11];
char *xmenu_unit_model[4];
char *xmenu_iounit_model[2];
char *xmenu_unit_ch[4];
char *xmenu_reader_type[5];
char *xmenu_input_type[7];
char *xmenu_output_type[5];
char *xmenu_input_output[1];
char *xmenu_io_polarity[2];
char *xmenu_armed[4];
char *xmenu_fire_zone[4];
char *xmenu_dev_model[6];
char *xmenu_door[13];
char *xmenu_lock_type[2];
char *xmenu_alarm[4];
char *xmenu_access_point[7];
char *xmenu_auth_mode[4];
char *xmenu_server_auth[3];
char *xmenu_ap_alarm[3];
char *xmenu_reader[7];
char *xmenu_readerop[6];
char *xmenu_authmode[5];
char *xmenu_authresult[4];
char *xmenu_ledbeep[4];
char *xmenu_ac[4];
char *xmenu_credential[5];
char *xmenu_field_type[2];
char *xmenu_cred_format[4];
char *xmenu_card_format[8];
char *xmenu_wiegand_format[5];
char *xmenu_pin_format[5];
char *xmenu_wiegand_pin[4];
char *xmenu_format_setting[8];
char *xmenu_card_type[5];
char *xmenu_cardtype_ext[3];
char *xmenu_cardapp_block[2];
char *xmenu_cardapp_iso7816[3];
char *xmenu_cardapp_serial[2];
char *xmenu_card_field[3];
char *xmenu_field_coding[4];
char *xmenu_wiegand_parity[3];
char *xmenu_reader_setting[8];
char *xmenu_ip_reader[3];
char *xmenu_action_idx[12];
char *xmenu_action_cmd[4];
char *xmenu_setup[11];
char *xmenu_admin[2];
char *xmenu_datetime[3];
char *xmenu_display[3];
char *xmenu_time_notation[2];
char *xmenu_date_notation[3];
char *xmenu_sound[6];
char *xmenu_sleepmode[3];
char *xmenu_sleepact[3];
char *xmenu_video[1];
char *xmenu_lockout[2];
char *xmenu_power_mgmt[2];
char *xmenu_cam[2];
char *xmenu_cam_control[3];
char *xmenu_ble[5];
char *xmenu_phone[6];
char *xmenu_phone_reg[2];
char *xmenu_call_state[3];
char *xmenu_network[11];
char *xmenu_network_ip[8];
char *xmenu_network_serial[2];
char *xmenu_network_type[2];
char *xmenu_network_wifi[8];
char *xmenu_wpa_state[12];
char *xmenu_wifi_security[8];
char *xmenu_wifi_state[6];
char *xmenu_system[9];
char *xmenu_download[4];
char *xmenu_mst_slv[2];
char *xmenu_net_remote[2];
char *xmenu_init_config[4];
char *xmenu_test[4];
char *xmenu_imexport[6];
char *xmenu_comm_stats[2];
char *xmenu_bat_status[6];
char *xmenu_yesno[2];
char *xmenu_noyes[2];
char *xmenu_okcancel[5];
char *xmenu_enable[2];
char *xmenu_unit[5];
char *xmenu_other[15];


void msgInitialize(void)
{
	int		i, lang;
	
	lang = GetLanguage();
	xmenu_null			= _xmenu_null;	
	xmenu_unsupported	= _xmenu_unsupported;
	for(i = 0;i < 6;i++) xmenu_serial_speed[i]	= _xmenu_serial_speed[i];
	for(i = 0;i < 3;i++) xmenu_parity[i]		= _xmenu_parity[i];
	for(i = 0;i < 2;i++) xmenu_stop_bits[i]		= _xmenu_stop_bits[i];
	switch(lang) {
	case LANG_KOR:
	case LANG_JPN:
		for(i = 0;i < sizeof(xmsgs)>>2;i++)					xmsgs[i] 					= cjk_msgs[i];
		for(i = 0;i < sizeof(xrsp_msgs)>>2;i++)				xrsp_msgs[i]				= cjk_rsp_msgs[i];
		for(i = 0;i < sizeof(xauth_msgs)>>2;i++)			xauth_msgs[i]				= cjk_auth_msgs[i];
		for(i = 0;i < sizeof(xalarm_msgs)>>2;i++)			xalarm_msgs[i]				= cjk_alarm_msgs[i];
		for(i = 0;i < sizeof(xta_msgs)>>2;i++)				xta_msgs[i]					= cjk_ta_msgs[i];
		xmenu_top_title = cjk_menu_top_title;
		for(i = 0;i < sizeof(xmenu_top)>>2;i++)				xmenu_top[i]				= cjk_menu_top[i];
		for(i = 0;i < sizeof(xmenu_lang)>>2;i++)			xmenu_lang[i]				= cjk_menu_lang[i];
		for(i = 0;i < sizeof(xmenu_user)>>2;i++)			xmenu_user[i]				= cjk_menu_user[i];
		for(i = 0;i < sizeof(xmenu_user_reg)>>2;i++)		xmenu_user_reg[i]			= cjk_menu_user_reg[i];
		for(i = 0;i < sizeof(xmenu_user_regex)>>2;i++)		xmenu_user_regex[i]			= cjk_menu_user_regex[i];
		for(i = 0;i < sizeof(xmenu_access_mode)>>2;i++)		xmenu_access_mode[i]		= cjk_menu_access_mode[i];
		for(i = 0;i < sizeof(xmenu_hardware_class)>>2;i++)	xmenu_hardware_class[i]		= cjk_menu_hardware_class[i];
		for(i = 0;i < sizeof(xmenu_device)>>2;i++)			xmenu_device[i]				= cjk_menu_device[i];
		for(i = 0;i < sizeof(xmenu_hardware)>>2;i++)		xmenu_hardware[i]			= cjk_menu_hardware[i];
		for(i = 0;i < sizeof(xmenu_unit_ch)>>2;i++)			xmenu_unit_ch[i]			= cjk_menu_unit_ch[i];
		for(i = 0;i < sizeof(xmenu_unit_model)>>2;i++)		xmenu_unit_model[i]			= cjk_menu_unit_model[i];
		for(i = 0;i < sizeof(xmenu_iounit_model)>>2;i++)	xmenu_iounit_model[i]		= cjk_menu_iounit_model[i];
		for(i = 0;i < sizeof(xmenu_reader_type)>>2;i++)		xmenu_reader_type[i]		= cjk_menu_reader_type[i];
		for(i = 0;i < sizeof(xmenu_input_type)>>2;i++)		xmenu_input_type[i]			= cjk_menu_input_type[i];
		for(i = 0;i < sizeof(xmenu_output_type)>>2;i++)		xmenu_output_type[i]		= cjk_menu_output_type[i];
		for(i = 0;i < sizeof(xmenu_input_output)>>2;i++)	xmenu_input_output[i]		= cjk_menu_input_output[i];
		for(i = 0;i < sizeof(xmenu_io_polarity)>>2;i++)		xmenu_io_polarity[i]		= cjk_menu_io_polarity[i];
		for(i = 0;i < sizeof(xmenu_armed)>>2;i++)			xmenu_armed[i]				= cjk_menu_armed[i];
		for(i = 0;i < sizeof(xmenu_fire_zone)>>2;i++)		xmenu_fire_zone[i]			= cjk_menu_fire_zone[i];
		for(i = 0;i < sizeof(xmenu_dev_model)>>2;i++)		xmenu_dev_model[i]			= cjk_menu_dev_model[i];
		for(i = 0;i < sizeof(xmenu_door)>>2;i++)			xmenu_door[i]				= cjk_menu_door[i];
		for(i = 0;i < sizeof(xmenu_lock_type)>>2;i++)		xmenu_lock_type[i]			= cjk_menu_lock_type[i];
		for(i = 0;i < sizeof(xmenu_alarm)>>2;i++)			xmenu_alarm[i]				= cjk_menu_alarm[i];
		for(i = 0;i < sizeof(xmenu_access_point)>>2;i++)	xmenu_access_point[i]		= cjk_menu_access_point[i];
		for(i = 0;i < sizeof(xmenu_auth_mode)>>2;i++)		xmenu_auth_mode[i]			= cjk_menu_auth_mode[i];
		for(i = 0;i < sizeof(xmenu_ap_alarm)>>2;i++)		xmenu_ap_alarm[i]			= cjk_menu_ap_alarm[i];
		for(i = 0;i < sizeof(xmenu_reader)>>2;i++)			xmenu_reader[i]				= cjk_menu_reader[i];
		for(i = 0;i < sizeof(xmenu_readerop)>>2;i++)		xmenu_readerop[i]			= cjk_menu_readerop[i];
		for(i = 0;i < sizeof(xmenu_authmode)>>2;i++)		xmenu_authmode[i]			= cjk_menu_authmode[i];
		for(i = 0;i < sizeof(xmenu_server_auth)>>2;i++)		xmenu_server_auth[i]		= cjk_menu_server_auth[i];
		for(i = 0;i < sizeof(xmenu_authresult)>>2;i++)		xmenu_authresult[i]			= cjk_menu_authresult[i];
		for(i = 0;i < sizeof(xmenu_ledbeep)>>2;i++)			xmenu_ledbeep[i]			= cjk_menu_ledbeep[i];
		for(i = 0;i < sizeof(xmenu_ac)>>2;i++)				xmenu_ac[i]					= cjk_menu_ac[i];
		for(i = 0;i < sizeof(xmenu_credential)>>2;i++)		xmenu_credential[i]			= cjk_menu_credential[i];
		for(i = 0;i < sizeof(xmenu_field_type)>>2;i++)		xmenu_field_type[i]			= cjk_menu_field_type[i];
		for(i = 0;i < sizeof(xmenu_cred_format)>>2;i++)		xmenu_cred_format[i]		= cjk_menu_cred_format[i];
		for(i = 0;i < sizeof(xmenu_card_format)>>2;i++)		xmenu_card_format[i]		= cjk_menu_card_format[i];
		for(i = 0;i < sizeof(xmenu_wiegand_format)>>2;i++)	xmenu_wiegand_format[i]		= cjk_menu_wiegand_format[i];
		for(i = 0;i < sizeof(xmenu_pin_format)>>2;i++)		xmenu_pin_format[i]			= cjk_menu_pin_format[i];
		for(i = 0;i < sizeof(xmenu_wiegand_pin)>>2;i++)		xmenu_wiegand_pin[i]		= cjk_menu_wiegand_pin[i];
		for(i = 0;i < sizeof(xmenu_format_setting)>>2;i++)	xmenu_format_setting[i]		= cjk_menu_format_setting[i];
		for(i = 0;i < sizeof(xmenu_card_type)>>2;i++)		xmenu_card_type[i]			= cjk_menu_card_type[i];
		for(i = 0;i < sizeof(xmenu_cardtype_ext)>>2;i++)	xmenu_cardtype_ext[i]		= cjk_menu_cardtype_ext[i];
		for(i = 0;i < sizeof(xmenu_cardapp_block)>>2;i++)	xmenu_cardapp_block[i]		= cjk_menu_cardapp_block[i];
		for(i = 0;i < sizeof(xmenu_cardapp_iso7816)>>2;i++)	xmenu_cardapp_iso7816[i]	= cjk_menu_cardapp_iso7816[i];
		for(i = 0;i < sizeof(xmenu_cardapp_serial)>>2;i++)	xmenu_cardapp_serial[i]		= cjk_menu_cardapp_serial[i];
		for(i = 0;i < sizeof(xmenu_card_field)>>2;i++)		xmenu_card_field[i]			= cjk_menu_card_field[i];
		for(i = 0;i < sizeof(xmenu_field_coding)>>2;i++)	xmenu_field_coding[i]		= cjk_menu_field_coding[i];
		for(i = 0;i < sizeof(xmenu_wiegand_parity)>>2;i++)	xmenu_wiegand_parity[i]		= cjk_menu_wiegand_parity[i];
		for(i = 0;i < sizeof(xmenu_reader_setting)>>2;i++)	xmenu_reader_setting[i]		= cjk_menu_reader_setting[i];
		for(i = 0;i < sizeof(xmenu_ip_reader)>>2;i++)		xmenu_ip_reader[i]			= cjk_menu_ip_reader[i];
		for(i = 0;i < sizeof(xmenu_action_idx)>>2;i++)		xmenu_action_idx[i]			= cjk_menu_action_idx[i];
		for(i = 0;i < sizeof(xmenu_action_cmd)>>2;i++)		xmenu_action_cmd[i]			= cjk_menu_action_cmd[i];
		for(i = 0;i < sizeof(xmenu_setup)>>2;i++)			xmenu_setup[i]				= cjk_menu_setup[i];
		for(i = 0;i < sizeof(xmenu_admin)>>2;i++)			xmenu_admin[i]				= cjk_menu_admin[i];
		for(i = 0;i < sizeof(xmenu_datetime)>>2;i++)		xmenu_datetime[i]			= cjk_menu_datetime[i];
		for(i = 0;i < sizeof(xmenu_display)>>2;i++)			xmenu_display[i]			= cjk_menu_display[i];
		for(i = 0;i < sizeof(xmenu_time_notation)>>2;i++)	xmenu_time_notation[i]		= cjk_menu_time_notation[i];
		for(i = 0;i < sizeof(xmenu_date_notation)>>2;i++)	xmenu_date_notation[i]		= cjk_menu_date_notation[i];
		for(i = 0;i < sizeof(xmenu_sound)>>2;i++)			xmenu_sound[i]				= cjk_menu_sound[i];
		for(i = 0;i < sizeof(xmenu_sleepmode)>>2;i++)		xmenu_sleepmode[i]			= cjk_menu_sleepmode[i];
		for(i = 0;i < sizeof(xmenu_sleepact)>>2;i++)		xmenu_sleepact[i]			= cjk_menu_sleepact[i];
		for(i = 0;i < sizeof(xmenu_lockout)>>2;i++)			xmenu_lockout[i]			= cjk_menu_lockout[i];
		for(i = 0;i < sizeof(xmenu_power_mgmt)>>2;i++)		xmenu_power_mgmt[i]			= cjk_menu_power_mgmt[i];
		for(i = 0;i < sizeof(xmenu_video)>>2;i++)			xmenu_video[i]				= cjk_menu_video[i];
		for(i = 0;i < sizeof(xmenu_cam)>>2;i++)				xmenu_cam[i]				= cjk_menu_cam[i];
		for(i = 0;i < sizeof(xmenu_cam_control)>>2;i++)     xmenu_cam_control[i]		= cjk_menu_cam_control[i];
		for(i = 0;i < sizeof(xmenu_ble)>>2;i++)				xmenu_ble[i]				= cjk_menu_ble[i];
		for(i = 0;i < sizeof(xmenu_phone)>>2;i++)			xmenu_phone[i]				= cjk_menu_phone[i];
		for(i = 0;i < sizeof(xmenu_phone_reg)>>2;i++)		xmenu_phone_reg[i]			= cjk_menu_phone_reg[i];
		for(i = 0;i < sizeof(xmenu_call_state)>>2;i++)		xmenu_call_state[i]			= cjk_menu_call_state[i];
		for(i = 0;i < sizeof(xmenu_network)>>2;i++)			xmenu_network[i]			= cjk_menu_network[i];
		for(i = 0;i < sizeof(xmenu_network_ip)>>2;i++)		xmenu_network_ip[i]			= cjk_menu_network_ip[i];
		for(i = 0;i < sizeof(xmenu_network_serial)>>2;i++)	xmenu_network_serial[i]		= cjk_menu_network_serial[i];
		for(i = 0;i < sizeof(xmenu_network_type)>>2;i++)	xmenu_network_type[i]		= cjk_menu_network_type[i];
		for(i = 0;i < sizeof(xmenu_network_wifi)>>2;i++)	xmenu_network_wifi[i]		= cjk_menu_network_wifi[i];
		for(i = 0;i < sizeof(xmenu_wpa_state)>>2;i++)		xmenu_wpa_state[i]			= cjk_menu_wpa_state[i];
		for(i = 0;i < sizeof(xmenu_wifi_security)>>2;i++)	xmenu_wifi_security[i]		= cjk_menu_wifi_security[i];
		for(i = 0;i < sizeof(xmenu_wifi_state)>>2;i++)		xmenu_wifi_state[i]			= cjk_menu_wifi_state[i];
		for(i = 0;i < sizeof(xmenu_system)>>2;i++)			xmenu_system[i]				= cjk_menu_system[i];
		for(i = 0;i < sizeof(xmenu_download)>>2;i++)		xmenu_download[i]			= cjk_menu_download[i];
		for(i = 0;i < sizeof(xmenu_mst_slv)>>2;i++)			xmenu_mst_slv[i]			= cjk_menu_mst_slv[i];
		for(i = 0;i < sizeof(xmenu_net_remote)>>2;i++)		xmenu_net_remote[i]			= cjk_menu_net_remote[i];
		for(i = 0;i < sizeof(xmenu_init_config)>>2;i++)		xmenu_init_config[i]		= cjk_menu_init_config[i];
		for(i = 0;i < sizeof(xmenu_test)>>2;i++)			xmenu_test[i]				= cjk_menu_test[i];
		for(i = 0;i < sizeof(xmenu_imexport)>>2;i++)		xmenu_imexport[i]			= cjk_menu_imexport[i];
		for(i = 0;i < sizeof(xmenu_comm_stats)>>2;i++)		xmenu_comm_stats[i]			= cjk_menu_comm_stats[i];
		for(i = 0;i < sizeof(xmenu_bat_status)>>2;i++)		xmenu_bat_status[i]			= cjk_menu_bat_status[i];
		for(i = 0;i < sizeof(xmenu_yesno)>>2;i++)			xmenu_yesno[i]				= cjk_menu_yesno[i];
		for(i = 0;i < sizeof(xmenu_noyes)>>2;i++)			xmenu_noyes[i]				= cjk_menu_noyes[i];
		for(i = 0;i < sizeof(xmenu_okcancel)>>2;i++)		xmenu_okcancel[i]			= cjk_menu_okcancel[i];
		for(i = 0;i < sizeof(xmenu_enable)>>2;i++)			xmenu_enable[i]				= cjk_menu_enable[i];
		for(i = 0;i < sizeof(xmenu_unit)>>2;i++)			xmenu_unit[i]				= cjk_menu_unit[i];
		for(i = 0;i < sizeof(xmenu_other)>>2;i++)			xmenu_other[i]				= cjk_menu_other[i];
		break;
	default:
		for(i = 0;i < sizeof(xmsgs)>>2;i++)					xmsgs[i] 					= eng_msgs[i];
		for(i = 0;i < sizeof(xrsp_msgs)>>2;i++)				xrsp_msgs[i]				= eng_rsp_msgs[i];
		for(i = 0;i < sizeof(xauth_msgs)>>2;i++)			xauth_msgs[i]				= eng_auth_msgs[i];
		for(i = 0;i < sizeof(xalarm_msgs)>>2;i++)			xalarm_msgs[i]				= eng_alarm_msgs[i];
		for(i = 0;i < sizeof(xta_msgs)>>2;i++)				xta_msgs[i]					= eng_ta_msgs[i];
		xmenu_top_title = eng_menu_top_title;
		for(i = 0;i < sizeof(xmenu_top)>>2;i++)				xmenu_top[i]				= eng_menu_top[i];
		for(i = 0;i < sizeof(xmenu_lang)>>2;i++)			xmenu_lang[i]				= eng_menu_lang[i];
		for(i = 0;i < sizeof(xmenu_user)>>2;i++)			xmenu_user[i]				= eng_menu_user[i];
		for(i = 0;i < sizeof(xmenu_user_reg)>>2;i++)		xmenu_user_reg[i]			= eng_menu_user_reg[i];
		for(i = 0;i < sizeof(xmenu_user_regex)>>2;i++)		xmenu_user_regex[i]			= eng_menu_user_regex[i];
		for(i = 0;i < sizeof(xmenu_access_mode)>>2;i++)		xmenu_access_mode[i]		= eng_menu_access_mode[i];
		for(i = 0;i < sizeof(xmenu_hardware_class)>>2;i++)	xmenu_hardware_class[i]		= eng_menu_hardware_class[i];
		for(i = 0;i < sizeof(xmenu_device)>>2;i++)			xmenu_device[i]				= eng_menu_device[i];
		for(i = 0;i < sizeof(xmenu_hardware)>>2;i++)		xmenu_hardware[i]			= eng_menu_hardware[i];
		for(i = 0;i < sizeof(xmenu_unit_ch)>>2;i++)			xmenu_unit_ch[i]			= eng_menu_unit_ch[i];
		for(i = 0;i < sizeof(xmenu_unit_model)>>2;i++)		xmenu_unit_model[i]			= eng_menu_unit_model[i];
		for(i = 0;i < sizeof(xmenu_iounit_model)>>2;i++)	xmenu_iounit_model[i]		= eng_menu_iounit_model[i];
		for(i = 0;i < sizeof(xmenu_reader_type)>>2;i++)		xmenu_reader_type[i]		= eng_menu_reader_type[i];
		for(i = 0;i < sizeof(xmenu_input_type)>>2;i++)		xmenu_input_type[i]			= eng_menu_input_type[i];
		for(i = 0;i < sizeof(xmenu_output_type)>>2;i++)		xmenu_output_type[i]		= eng_menu_output_type[i];
		for(i = 0;i < sizeof(xmenu_input_output)>>2;i++)	xmenu_input_output[i]		= eng_menu_input_output[i];
		for(i = 0;i < sizeof(xmenu_io_polarity)>>2;i++)		xmenu_io_polarity[i]		= eng_menu_io_polarity[i];
		for(i = 0;i < sizeof(xmenu_armed)>>2;i++)			xmenu_armed[i]				= eng_menu_armed[i];
		for(i = 0;i < sizeof(xmenu_fire_zone)>>2;i++)		xmenu_fire_zone[i]			= eng_menu_fire_zone[i];
		for(i = 0;i < sizeof(xmenu_dev_model)>>2;i++)		xmenu_dev_model[i]			= eng_menu_dev_model[i];
		for(i = 0;i < sizeof(xmenu_door)>>2;i++)			xmenu_door[i]				= eng_menu_door[i];
		for(i = 0;i < sizeof(xmenu_lock_type)>>2;i++)		xmenu_lock_type[i]			= eng_menu_lock_type[i];
		for(i = 0;i < sizeof(xmenu_alarm)>>2;i++)			xmenu_alarm[i]				= eng_menu_alarm[i];
		for(i = 0;i < sizeof(xmenu_access_point)>>2;i++)	xmenu_access_point[i]		= eng_menu_access_point[i];
		for(i = 0;i < sizeof(xmenu_auth_mode)>>2;i++)		xmenu_auth_mode[i]			= eng_menu_auth_mode[i];
		for(i = 0;i < sizeof(xmenu_ap_alarm)>>2;i++)		xmenu_ap_alarm[i]			= eng_menu_ap_alarm[i];
		for(i = 0;i < sizeof(xmenu_reader)>>2;i++)			xmenu_reader[i]				= eng_menu_reader[i];
		for(i = 0;i < sizeof(xmenu_readerop)>>2;i++)		xmenu_readerop[i]			= eng_menu_readerop[i];
		for(i = 0;i < sizeof(xmenu_authmode)>>2;i++)		xmenu_authmode[i]			= eng_menu_authmode[i];
		for(i = 0;i < sizeof(xmenu_server_auth)>>2;i++)		xmenu_server_auth[i]		= eng_menu_server_auth[i];
		for(i = 0;i < sizeof(xmenu_authresult)>>2;i++)		xmenu_authresult[i]			= eng_menu_authresult[i];
		for(i = 0;i < sizeof(xmenu_ledbeep)>>2;i++)			xmenu_ledbeep[i]			= eng_menu_ledbeep[i];
		for(i = 0;i < sizeof(xmenu_ac)>>2;i++)				xmenu_ac[i]					= eng_menu_ac[i];
		for(i = 0;i < sizeof(xmenu_credential)>>2;i++)		xmenu_credential[i]			= eng_menu_credential[i];
		for(i = 0;i < sizeof(xmenu_field_type)>>2;i++)		xmenu_field_type[i]			= eng_menu_field_type[i];
		for(i = 0;i < sizeof(xmenu_cred_format)>>2;i++)		xmenu_cred_format[i]		= eng_menu_cred_format[i];
		for(i = 0;i < sizeof(xmenu_card_format)>>2;i++)		xmenu_card_format[i]		= eng_menu_card_format[i];
		for(i = 0;i < sizeof(xmenu_wiegand_format)>>2;i++)	xmenu_wiegand_format[i]		= eng_menu_wiegand_format[i];
		for(i = 0;i < sizeof(xmenu_pin_format)>>2;i++)		xmenu_pin_format[i]			= eng_menu_pin_format[i];
		for(i = 0;i < sizeof(xmenu_wiegand_pin)>>2;i++)		xmenu_wiegand_pin[i]		= eng_menu_wiegand_pin[i];
		for(i = 0;i < sizeof(xmenu_format_setting)>>2;i++)	xmenu_format_setting[i]		= eng_menu_format_setting[i];
		for(i = 0;i < sizeof(xmenu_card_type)>>2;i++)		xmenu_card_type[i]			= eng_menu_card_type[i];
		for(i = 0;i < sizeof(xmenu_cardtype_ext)>>2;i++)	xmenu_cardtype_ext[i]		= eng_menu_cardtype_ext[i];
		for(i = 0;i < sizeof(xmenu_cardapp_block)>>2;i++)	xmenu_cardapp_block[i]		= eng_menu_cardapp_block[i];
		for(i = 0;i < sizeof(xmenu_cardapp_iso7816)>>2;i++)	xmenu_cardapp_iso7816[i]	= eng_menu_cardapp_iso7816[i];
		for(i = 0;i < sizeof(xmenu_cardapp_serial)>>2;i++)	xmenu_cardapp_serial[i]		= eng_menu_cardapp_serial[i];
		for(i = 0;i < sizeof(xmenu_card_field)>>2;i++)		xmenu_card_field[i]			= eng_menu_card_field[i];
		for(i = 0;i < sizeof(xmenu_field_coding)>>2;i++)	xmenu_field_coding[i]		= eng_menu_field_coding[i];
		for(i = 0;i < sizeof(xmenu_wiegand_parity)>>2;i++)	xmenu_wiegand_parity[i]		= eng_menu_wiegand_parity[i];
		for(i = 0;i < sizeof(xmenu_reader_setting)>>2;i++)	xmenu_reader_setting[i]		= eng_menu_reader_setting[i];
		for(i = 0;i < sizeof(xmenu_ip_reader)>>2;i++)		xmenu_ip_reader[i]			= eng_menu_ip_reader[i];
		for(i = 0;i < sizeof(xmenu_action_idx)>>2;i++)		xmenu_action_idx[i]			= eng_menu_action_idx[i];
		for(i = 0;i < sizeof(xmenu_action_cmd)>>2;i++)		xmenu_action_cmd[i]			= eng_menu_action_cmd[i];
		for(i = 0;i < sizeof(xmenu_setup)>>2;i++)			xmenu_setup[i]				= eng_menu_setup[i];
		for(i = 0;i < sizeof(xmenu_admin)>>2;i++)			xmenu_admin[i]				= eng_menu_admin[i];
		for(i = 0;i < sizeof(xmenu_datetime)>>2;i++)		xmenu_datetime[i]			= eng_menu_datetime[i];
		for(i = 0;i < sizeof(xmenu_display)>>2;i++)			xmenu_display[i]			= eng_menu_display[i];
		for(i = 0;i < sizeof(xmenu_time_notation)>>2;i++)	xmenu_time_notation[i]		= eng_menu_time_notation[i];
		for(i = 0;i < sizeof(xmenu_date_notation)>>2;i++)	xmenu_date_notation[i]		= eng_menu_date_notation[i];
		for(i = 0;i < sizeof(xmenu_sound)>>2;i++)			xmenu_sound[i]				= eng_menu_sound[i];
		for(i = 0;i < sizeof(xmenu_sleepmode)>>2;i++)		xmenu_sleepmode[i]			= eng_menu_sleepmode[i];
		for(i = 0;i < sizeof(xmenu_sleepact)>>2;i++)		xmenu_sleepact[i]			= eng_menu_sleepact[i];
		for(i = 0;i < sizeof(xmenu_video)>>2;i++)			xmenu_video[i]				= eng_menu_video[i];
		for(i = 0;i < sizeof(xmenu_lockout)>>2;i++)			xmenu_lockout[i]			= eng_menu_lockout[i];
		for(i = 0;i < sizeof(xmenu_power_mgmt)>>2;i++)		xmenu_power_mgmt[i]			= eng_menu_power_mgmt[i];
		for(i = 0;i < sizeof(xmenu_cam)>>2;i++)				xmenu_cam[i]				= eng_menu_cam[i];
		for(i = 0;i < sizeof(xmenu_cam_control)>>2;i++)		xmenu_cam_control[i]		= eng_menu_cam_control[i];
		for(i = 0;i < sizeof(xmenu_ble)>>2;i++)				xmenu_ble[i]				= eng_menu_ble[i];
		for(i = 0;i < sizeof(xmenu_phone)>>2;i++)			xmenu_phone[i]				= eng_menu_phone[i];
		for(i = 0;i < sizeof(xmenu_phone_reg)>>2;i++)		xmenu_phone_reg[i]			= eng_menu_phone_reg[i];
		for(i = 0;i < sizeof(xmenu_call_state)>>2;i++)		xmenu_call_state[i]			= eng_menu_call_state[i];
		for(i = 0;i < sizeof(xmenu_network)>>2;i++)			xmenu_network[i]			= eng_menu_network[i];
		for(i = 0;i < sizeof(xmenu_network_ip)>>2;i++)		xmenu_network_ip[i]			= eng_menu_network_ip[i];
		for(i = 0;i < sizeof(xmenu_network_serial)>>2;i++)	xmenu_network_serial[i]		= eng_menu_network_serial[i];
		for(i = 0;i < sizeof(xmenu_network_type)>>2;i++)	xmenu_network_type[i]		= eng_menu_network_type[i];
		for(i = 0;i < sizeof(xmenu_network_wifi)>>2;i++)	xmenu_network_wifi[i]		= eng_menu_network_wifi[i];
		for(i = 0;i < sizeof(xmenu_wpa_state)>>2;i++)		xmenu_wpa_state[i]			= eng_menu_wpa_state[i];
		for(i = 0;i < sizeof(xmenu_wifi_security)>>2;i++)	xmenu_wifi_security[i]		= eng_menu_wifi_security[i];
		for(i = 0;i < sizeof(xmenu_wifi_state)>>2;i++)		xmenu_wifi_state[i]			= eng_menu_wifi_state[i];
		for(i = 0;i < sizeof(xmenu_system)>>2;i++)			xmenu_system[i]				= eng_menu_system[i];
		for(i = 0;i < sizeof(xmenu_download)>>2;i++)		xmenu_download[i]			= eng_menu_download[i];
		for(i = 0;i < sizeof(xmenu_mst_slv)>>2;i++)			xmenu_mst_slv[i]			= eng_menu_mst_slv[i];
		for(i = 0;i < sizeof(xmenu_net_remote)>>2;i++)		xmenu_net_remote[i]			= eng_menu_net_remote[i];
		for(i = 0;i < sizeof(xmenu_init_config)>>2;i++)		xmenu_init_config[i]		= eng_menu_init_config[i];
		for(i = 0;i < sizeof(xmenu_test)>>2;i++)			xmenu_test[i]				= eng_menu_test[i];
		for(i = 0;i < sizeof(xmenu_imexport)>>2;i++)		xmenu_imexport[i]			= eng_menu_imexport[i];
		for(i = 0;i < sizeof(xmenu_comm_stats)>>2;i++)		xmenu_comm_stats[i]			= eng_menu_comm_stats[i];
		for(i = 0;i < sizeof(xmenu_bat_status)>>2;i++)		xmenu_bat_status[i]			= eng_menu_bat_status[i];
		for(i = 0;i < sizeof(xmenu_yesno)>>2;i++)			xmenu_yesno[i]				= eng_menu_yesno[i];
		for(i = 0;i < sizeof(xmenu_noyes)>>2;i++)			xmenu_noyes[i]				= eng_menu_noyes[i];
		for(i = 0;i < sizeof(xmenu_okcancel)>>2;i++)		xmenu_okcancel[i]			= eng_menu_okcancel[i];
		for(i = 0;i < sizeof(xmenu_enable)>>2;i++)			xmenu_enable[i]				= eng_menu_enable[i];
		for(i = 0;i < sizeof(xmenu_unit)>>2;i++)			xmenu_unit[i]				= eng_menu_unit[i];
		for(i = 0;i < sizeof(xmenu_other)>>2;i++)			xmenu_other[i]				= eng_menu_other[i];
	}
}

void dtGetShortDayOfWeek(int lang, int wday, char *buf)
{
	if(wday >= 0 && wday < 7) {
		if(lang == LANG_KOR || lang == LANG_JPN) strcpy(buf, _cjk_day_of_weeks[wday]);
		else {
			memcpy(buf, _eng_day_of_weeks[wday], 3); buf[3] = 0;
		}
	} else {
		buf[0] = buf[1] = ' '; buf[2] = 0;
	}
}

void dtGetLongDayOfWeek(int lang, int wday, char *buf)
{
	if(wday >= 0 && wday < 7) {
		if(lang == LANG_KOR || lang == LANG_JPN) sprintf(buf, "%s%s", _cjk_day_of_weeks[wday], _cjk_day_of_week_postfix);
		else	strcpy(buf, _eng_day_of_weeks[wday]);
	} else {
		buf[0] = buf[1] = ' '; buf[2] = 0;
	}
}

int convert12hour(int hour)
{
	if(hour == 0 || hour == 12) hour = 12;
	else if(hour > 12) hour -= 12;
	return hour;
}

void dtGetHour(int hour24, int hour, char *buf)
{
	if(hour24) sprintf(buf, "%02d", (int)hour);
	else	sprintf(buf, "%d", convert12hour(hour));
}

void dtGetMinute(int minute, char *buf)
{
	sprintf(buf, "%02d", minute);
}

void dtGetNoon(int lang, int hour, char *buf)
{
	int		idx;
	
	if(hour < 12) idx = 0;
	else	idx = 1;
	if(lang == LANG_KOR || lang == LANG_JPN) strcpy(buf, _cjk_am_pm[idx]);
	else	strcpy(buf, _eng_am_pm[idx]);
}

void dtGetShortMonth(int lang, int month, char *buf)
{
	if(month < 1 || month > 12) buf[0] = 0;
	else if(lang == LANG_KOR || lang == LANG_JPN) sprintf(buf, "%02d", month);
	else {
		memcpy(buf, _eng_months[month-1], 3); buf[3] = 0;
	}
}

void dtGetLongMonth(int lang, int month, char *buf)
{
	if(month < 1 || month > 12) buf[0] = 0;
	else if(lang == LANG_KOR || lang == LANG_JPN) sprintf(buf, "%d%s", month, _cjk_year_mon_day[1]);
	else	strcpy(buf, _eng_months[month-1]);
}

int dtShortMonthString2Month(char *buf)
{
	int		i;

	for(i = 0;i < 12;i++)
		if(!memcmp(buf, _eng_months[i], 3)) break;
	if(i < 12) i++;
	else	i = 0;
	return i;
}

char *GetCJKYear(void)
{
	return _cjk_year_mon_day[0];
}

char *GetCJKMonth(void)
{
	return _cjk_year_mon_day[1];
}

char *GetCJKDay(void)
{
	return _cjk_year_mon_day[2];
}

#include "defs.h"
#include "syscfg.h"
#include "tacfg.h"

void syscfgSetDefaultByLanguage(SYS_CFG *sys_cfg)
{
	int		i, size;

	ta_cfg->TATexts[0][0] = 0;
	size = MAX_TA_MSGS_COUNT;
	for(i = 0;i < size;i++) strcpy(ta_cfg->TATexts[i+1], eng_ta_msgs[i]);
	size = taGetTextSize(); 	
	for( ;i < size;i++) ta_cfg->TATexts[i][0] = 0;
}

void syscfgSetByLanguage(SYS_CFG *sys_cfg)
{
	int		i, size;
	
	ta_cfg->TATexts[0][0] = 0;
	size = MAX_TA_MSGS_COUNT;
	for(i = 0;i < size;i++) strcpy(ta_cfg->TATexts[i+1], xta_msgs[i]);
	size = taGetTextSize(); 	
	for( ;i < size;i++) ta_cfg->TATexts[i][0] = 0;
}

