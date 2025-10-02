#include <string.h>
#include "gfont.h"
#include "lang.h"
#include "msg.h"

#include "msgeng.c"
#include "msgkor.c"

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
	MB_ICONWARNING,			// 32 R_TEL_NO_ANSWER
	MB_ICONWARNING,			// 33 R_TEL_DISCONNECTED
	MB_ICONINFORMATION,		// 34 R_TEL_CONNECTED
	MB_ICONINFORMATION,		// 35 R_DOOR_UNLOCKED
	MB_ICONINFORMATION,		// 36 R_PROCESSED
	MB_ICONWARNING,			// 37 R_FAILED
	MB_ICONINFORMATION,		// 38 R_COMPLETED
	MB_ICONWARNING,			// 39 R_NOT_ALLOWED_CARD
	MB_ICONWARNING,			// 40 R_TOUCH_SINGLE_CARD
	MB_ICONWARNING,			// 41 R_NOT_FOUND
	MB_ICONWARNING,			// 42 R_OVERLAPPED
	MB_ICONWARNING,			// 43 R_DATA_ARE_FULL
	MB_ICONINFORMATION,		// 44 R_DELETED	
	MB_ICONWARNING,			// 45 R_NO_SD_DISK
	MB_ICONWARNING,			// 46 R_NOT_READY_SD_DISK
	MB_ICONERROR,			// 47 R_DIR_OPEN_ERROR
	MB_ICONERROR,			// 48 R_DIR_READ_ERROR
	MB_ICONERROR,			// 49 R_DIR_WRITE_ERROR
	MB_ICONERROR,			// 50 R_FILE_OPEN_ERROR
	MB_ICONERROR,			// 51 R_FILE_READ_ERROR
	MB_ICONERROR,			// 52 R_FILE_WRITE_ERROR
	MB_ICONERROR,			// 53 R_FILE_SYNC_ERROR
	MB_ICONWARNING,			// 54 R_DIRECTORY_NOT_FOUND
	MB_ICONWARNING,			// 55 R_FILE_NOT_FOUND
	MB_ICONINFORMATION,		// 56 R_SYSTEM_RESTART
	MB_ICONINFORMATION,		// 57 R_SAFELY_REMOVE_SD_DISK
	MB_ICONERROR,			// 58 R_SD_DISK_ERROR_CHECK
	MB_ICONERROR,			// 59 R_ACCESS_DENIED_FAIL_SERVER
	MB_ICONWARNING			// 60 R_ALREADY_PROCESSING
};

char _xmenu_null[MAX_MSG_LEN] = "";
char _xmenu_unsupported[MAX_MSG_LEN] = "Unsupported";

char _xmenu_serial_speed[5][MAX_MSG_LEN] = {
	"9600",
	"19200",
	"38400",
	"57600",
	"115200"
};

char *xmenu_null;
char *xmenu_unsupported;
char *xmenu_serial_speed[5];
char *xmsgs[MAX_MSGS_COUNT];
char *xrsp_msgs[MAX_RSP_MSGS_COUNT];
char *xauth_msgs[9];
char *xta_msgs[MAX_TA_MSGS_COUNT];
char *xmenu_top_title;
char *xmenu_top[5];
char *xmenu_lang[2];
char *xmenu_user[6];
char *xmenu_user_enroll[7];
char *xmenu_access_mode[7];
char *xmenu_security_level[10];
char *xmenu_ta_sub[1];
char *xmenu_time_attend[4];
char *xmenu_ta_key[2];
char *xmenu_ta_display[3];
char *xmenu_ta_item[3];
char *xmenu_meal[9];
char *xmenu_meal_sub[2];
char *xmenu_meal_funckey[4];
char *xmenu_event[3];
char *xmenu_event_output[3];
char *xmenu_event_mask[3];
char *xmenu_event_wiegand[2];
char *xmenu_access_control[9];
char *xmenu_hw_setup[6];
char *xmenu_op_mode[3];
char *xmenu_all_readers[6];
char *xmenu_hw_config[19];
char *xmenu_unit_type[3];
char *xmenu_iounit_type[2];
char *xmenu_access_door[7];
char *xmenu_door_config[3];
char *xmenu_door_inout[5];
char *xmenu_lock_type[2];
char *xmenu_door_alarm[2];
char *xmenu_door_alarm_info[2];
char *xmenu_access_point[1];
char *xmenu_authorization_mode[4];
char *xmenu_access_zone[6];
char *xmenu_passback[6];
char *xmenu_passback_mode[3];
char *xmenu_passback_setting[4];
char *xmenu_binary_input[4];
char *xmenu_binary_output[1];
char *xmenu_io_polarity[2];
char *xmenu_armed[4];
char *xmenu_alarm[4];
char *xmenu_fire_zone[3];
char *xmenu_action_index[12];
char *xmenu_output_command[3];
char *xmenu_readers[3];
char *xmenu_card_readers[5];
char *xmenu_wiegand_reader[2];
char *xmenu_fp_reader[4];
char *xmenu_mifare_reader[4];
char *xmenu_mifare_format[3];
char *xmenu_mifare_type[3];
char *xmenu_special_card[6];
char *xmenu_data_type[3];
char *xmenu_byte_order[2];
char *xmenu_em_format[3];
char *xmenu_external_format[3];
char *xmenu_ip_reader[3];
char *xmenu_wiegand_26bit[3];
char *xmenu_duress[2];
char *xmenu_setup[8];
char *xmenu_admin[2];
char *xmenu_wiegand_type[2];
char *xmenu_datetime[2];
char *xmenu_monitor[6];
char *xmenu_time_notation[2];
char *xmenu_date_notation[3];
char *xmenu_sound[3];
char *xmenu_lockout[2];
char *xmenu_cam[7];
char *xmenu_cam_mode[3];
char *xmenu_cam_run[2];
char *xmenu_cam_channel[4];
char *xmenu_cam_direction[3];
char *xmenu_doorphone[8];
char *xmenu_doorphone_op_mode[4];
char *xmenu_phone_number[3];
char *xmenu_dialing_mode[3];
char *xmenu_ip_phone_number[5];
char *xmenu_ip_phone_item[1];
char *xmenu_doorphone_volume[3];
char *xmenu_doorphone_tone[3];
char *xmenu_tone_info[2];
char *xmenu_doorphone_hw_setup[2];
char *xmenu_doorphone_bgn[4];
char *xmenu_network[6];
char *xmenu_network_serial[2];
char *xmenu_serial_port[3];
char *xmenu_network_tcpip[7];
char *xmenu_network_mode[3];
char *xmenu_system[7];
char *xmenu_init_device[3];
char *xmenu_imexport[6];
char *xmenu_comm_stats[2];
char *xmenu_downfirmware[5];
char *xmenu_inuse[2];
char *xmenu_yesno[2];
char *xmenu_noyes[2];
char *xmenu_okcancel[2];
char *xmenu_enable;
char *xmenu_other[24];


void msgInitialize(void)
{
	int		i, lang;
	
	lang = GetLanguage();
	xmenu_null			= _xmenu_null;	
	xmenu_unsupported	= _xmenu_unsupported;
	for(i = 0;i < 5;i++) xmenu_serial_speed[i] = _xmenu_serial_speed[i];
	switch(lang) {
	case LANG_KOR:
		for(i = 0;i < MAX_MSGS_COUNT;i++) xmsgs[i]			= kor_msgs[i];
		for(i = 0;i < MAX_RSP_MSGS_COUNT;i++) xrsp_msgs[i]	= kor_rsp_msgs[i];
		for(i = 0;i < sizeof(xauth_msgs)>>2;i++)			xauth_msgs[i]				= cjk_auth_msgs[i];	
		for(i = 0;i < MAX_TA_MSGS_COUNT;i++) xta_msgs[i]	= kor_ta_msgs[i];
		xmenu_top_title = kor_menu_top_title;		
		for(i = 0;i < 5;i++) xmenu_top[i]				= kor_menu_top[i];
		for(i = 0;i < 2;i++) xmenu_lang[i]				= kor_menu_lang[i];
		for(i = 0;i < 6;i++) xmenu_user[i]				= kor_menu_user[i];
		for(i = 0;i < 7;i++) xmenu_user_enroll[i]		= kor_menu_user_enroll[i];
		for(i = 0;i < 7;i++) xmenu_access_mode[i]		= kor_menu_access_mode[i];
		for(i = 0;i < 10;i++) xmenu_security_level[i]	= kor_menu_security_level[i];
		for(i = 0;i < 1;i++) xmenu_ta_sub[i]			= kor_menu_ta_sub[i];
		for(i = 0;i < 4;i++) xmenu_time_attend[i]		= kor_menu_time_attend[i];
		for(i = 0;i < 2;i++) xmenu_ta_key[i]			= kor_menu_ta_key[i];
		for(i = 0;i < 3;i++) xmenu_ta_display[i]		= kor_menu_ta_display[i];
		for(i = 0;i < 3;i++) xmenu_ta_item[i]			= kor_menu_ta_item[i];
		for(i = 0;i < 9;i++) xmenu_meal[i]				= kor_menu_meal[i];
		for(i = 0;i < 2;i++) xmenu_meal_sub[i]			= kor_menu_meal_sub[i];
		for(i = 0;i < 4;i++) xmenu_meal_funckey[i]		= kor_menu_meal_funckey[i];
		for(i = 0;i < 3;i++) xmenu_event[i]				= kor_menu_event[i];
		for(i = 0;i < 3;i++) xmenu_event_output[i]		= kor_menu_event_output[i];
		for(i = 0;i < 3;i++) xmenu_event_mask[i]		= kor_menu_event_mask[i];
		for(i = 0;i < 2;i++) xmenu_event_wiegand[i]		= kor_menu_event_wiegand[i];
		for(i = 0;i < 9;i++) xmenu_access_control[i]	= kor_menu_access_control[i];
		for(i = 0;i < 6;i++) xmenu_hw_setup[i]			= kor_menu_hw_setup[i];
		for(i = 0;i < 3;i++) xmenu_op_mode[i]			= kor_menu_op_mode[i];
		for(i = 0;i < 6;i++) xmenu_all_readers[i]		= kor_menu_all_readers[i];
		for(i = 0;i < 19;i++) xmenu_hw_config[i]		= kor_menu_hw_config[i];
		for(i = 0;i < 3;i++) xmenu_unit_type[i]			= kor_menu_unit_type[i];
		for(i = 0;i < 2;i++) xmenu_iounit_type[i]		= kor_menu_iounit_type[i];
		for(i = 0;i < 7;i++) xmenu_access_door[i]		= kor_menu_access_door[i];
		for(i = 0;i < 3;i++) xmenu_door_config[i]		= kor_menu_door_config[i];
		for(i = 0;i < 5;i++) xmenu_door_inout[i]		= kor_menu_door_inout[i];
		for(i = 0;i < 2;i++) xmenu_lock_type[i]			= kor_menu_lock_type[i];
		for(i = 0;i < 2;i++) xmenu_door_alarm[i]		= kor_menu_door_alarm[i];
		for(i = 0;i < 2;i++) xmenu_door_alarm_info[i]	= kor_menu_door_alarm_info[i];
		for(i = 0;i < 1;i++) xmenu_access_point[i]		= kor_menu_access_point[i];
		for(i = 0;i < 4;i++) xmenu_authorization_mode[i]= kor_menu_authorization_mode[i];
		for(i = 0;i < 6;i++) xmenu_access_zone[i]		= kor_menu_access_zone[i];
		for(i = 0;i < 6;i++) xmenu_passback[i]			= kor_menu_passback[i];
		for(i = 0;i < 3;i++) xmenu_passback_mode[i]		= kor_menu_passback_mode[i];
		for(i = 0;i < 4;i++) xmenu_passback_setting[i]	= kor_menu_passback_setting[i];
		for(i = 0;i < 4;i++) xmenu_binary_input[i]		= kor_menu_binary_input[i];
		for(i = 0;i < 1;i++) xmenu_binary_output[i]		= kor_menu_binary_output[i];
		for(i = 0;i < 2;i++) xmenu_io_polarity[i]		= kor_menu_io_polarity[i];
		for(i = 0;i < 4;i++) xmenu_armed[i]				= kor_menu_armed[i];
		for(i = 0;i < 4;i++) xmenu_alarm[i]				= kor_menu_alarm[i];
		for(i = 0;i < 3;i++) xmenu_fire_zone[i]			= kor_menu_fire_zone[i];
		for(i = 0;i < 12;i++) xmenu_action_index[i]		= kor_menu_action_index[i];
		for(i = 0;i < 3;i++) xmenu_output_command[i]	= kor_menu_output_command[i];
		for(i = 0;i < 3;i++) xmenu_readers[i]			= kor_menu_readers[i];
		for(i = 0;i < 5;i++) xmenu_card_readers[i]		= kor_menu_card_readers[i];
		for(i = 0;i < 2;i++) xmenu_wiegand_reader[i]	= kor_menu_wiegand_reader[i];
		for(i = 0;i < 4;i++) xmenu_fp_reader[i]			= kor_menu_fp_reader[i];
		for(i = 0;i < 4;i++) xmenu_mifare_reader[i]		= kor_menu_mifare_reader[i];
		for(i = 0;i < 3;i++) xmenu_mifare_format[i]		= kor_menu_mifare_format[i];
		for(i = 0;i < 3;i++) xmenu_mifare_type[i]		= kor_menu_mifare_type[i];
		for(i = 0;i < 6;i++) xmenu_special_card[i]		= kor_menu_special_card[i];
		for(i = 0;i < 3;i++) xmenu_data_type[i]			= kor_menu_data_type[i];
		for(i = 0;i < 2;i++) xmenu_byte_order[i]		= kor_menu_byte_order[i];
		for(i = 0;i < 3;i++) xmenu_em_format[i]			= kor_menu_em_format[i];
		for(i = 0;i < 3;i++) xmenu_external_format[i]	= kor_menu_external_format[i];
		for(i = 0;i < 3;i++) xmenu_ip_reader[i]			= kor_menu_ip_reader[i];
		for(i = 0;i < 3;i++) xmenu_wiegand_26bit[i]		= kor_menu_wiegand_26bit[i];
		for(i = 0;i < 2;i++) xmenu_duress[i]			= kor_menu_duress[i];
		for(i = 0;i < 8;i++) xmenu_setup[i]				= kor_menu_setup[i];
		for(i = 0;i < 2;i++) xmenu_admin[i]				= kor_menu_admin[i];
		for(i = 0;i < 2;i++) xmenu_wiegand_type[i]		= kor_menu_wiegand_type[i];		
		for(i = 0;i < 2;i++) xmenu_datetime[i]			= kor_menu_datetime[i];
		for(i = 0;i < 6;i++) xmenu_monitor[i]			= kor_menu_monitor[i];
		for(i = 0;i < 2;i++) xmenu_time_notation[i]		= kor_menu_time_notation[i];
		for(i = 0;i < 3;i++) xmenu_date_notation[i]		= kor_menu_date_notation[i];
		for(i = 0;i < 3;i++) xmenu_sound[i]				= kor_menu_sound[i];
		for(i = 0;i < 2;i++) xmenu_lockout[i]			= kor_menu_lockout[i];
		for(i = 0;i < 7;i++) xmenu_cam[i]				= kor_menu_cam[i];		
		for(i = 0;i < 3;i++) xmenu_cam_mode[i]			= kor_menu_cam_mode[i];		
		for(i = 0;i < 2;i++) xmenu_cam_run[i]			= kor_menu_cam_run[i];		
		for(i = 0;i < 4;i++) xmenu_cam_channel[i]		= kor_menu_cam_channel[i];
		for(i = 0;i < 2;i++) xmenu_cam_direction[i]		= kor_menu_cam_direction[i];		
		for(i = 0;i < 8;i++) xmenu_doorphone[i]			= kor_menu_doorphone[i];		
		for(i = 0;i < 4;i++) xmenu_doorphone_op_mode[i]	= kor_menu_doorphone_op_mode[i];
		for(i = 0;i < 3;i++) xmenu_phone_number[i]		= kor_menu_phone_number[i];
		for(i = 0;i < 3;i++) xmenu_dialing_mode[i]		= kor_menu_dialing_mode[i];
		for(i = 0;i < 5;i++) xmenu_ip_phone_number[i]	= kor_menu_ip_phone_number[i];
		for(i = 0;i < 1;i++) xmenu_ip_phone_item[i]		= kor_menu_ip_phone_item[i];
		for(i = 0;i < 3;i++) xmenu_doorphone_volume[i]	= kor_menu_doorphone_volume[i];		
		for(i = 0;i < 3;i++) xmenu_doorphone_tone[i]	= kor_menu_doorphone_tone[i];		
		for(i = 0;i < 2;i++) xmenu_tone_info[i]			= kor_menu_tone_info[i];		
		for(i = 0;i < 2;i++) xmenu_doorphone_hw_setup[i]= kor_menu_doorphone_hw_setup[i];		
		for(i = 0;i < 4;i++) xmenu_doorphone_bgn[i]		= kor_menu_doorphone_bgn[i];		
		for(i = 0;i < 6;i++) xmenu_network[i]			= kor_menu_network[i];
		for(i = 0;i < 2;i++) xmenu_network_serial[i]	= kor_menu_network_serial[i];
		for(i = 0;i < 3;i++) xmenu_serial_port[i]		= kor_menu_serial_port[i];
		for(i = 0;i < 7;i++) xmenu_network_tcpip[i]		= kor_menu_network_tcpip[i];
		for(i = 0;i < 3;i++) xmenu_network_mode[i]		= kor_menu_network_mode[i];
		for(i = 0;i < 7;i++) xmenu_system[i]			= kor_menu_system[i];
		for(i = 0;i < 3;i++) xmenu_init_device[i]		= kor_menu_init_device[i];
		for(i = 0;i < 6;i++) xmenu_imexport[i]			= kor_menu_imexport[i];
		for(i = 0;i < 2;i++) xmenu_comm_stats[i]		= kor_menu_comm_stats[i];
		for(i = 0;i < 5;i++) xmenu_downfirmware[i]		= kor_menu_downfirmware[i];
		for(i = 0;i < 2;i++) xmenu_inuse[i]				= kor_menu_inuse[i];
		for(i = 0;i < 2;i++) xmenu_yesno[i]				= kor_menu_yesno[i];
		for(i = 0;i < 2;i++) xmenu_noyes[i]				= kor_menu_noyes[i];
		for(i = 0;i < 2;i++) xmenu_okcancel[i]			= kor_menu_okcancel[i];
		xmenu_enable = kor_menu_enable;
		for(i = 0;i < 24;i++) xmenu_other[i]			= kor_menu_other[i];
		break;
	default:	
		for(i = 0;i < MAX_MSGS_COUNT;i++) xmsgs[i]			= eng_msgs[i];
		for(i = 0;i < MAX_RSP_MSGS_COUNT;i++) xrsp_msgs[i]	= eng_rsp_msgs[i];
		for(i = 0;i < sizeof(xauth_msgs)>>2;i++)			xauth_msgs[i]				= eng_auth_msgs[i];
		for(i = 0;i < MAX_TA_MSGS_COUNT;i++) xta_msgs[i]	= eng_ta_msgs[i];
		xmenu_top_title = eng_menu_top_title;
		for(i = 0;i < 5;i++) xmenu_top[i]				= eng_menu_top[i];
		for(i = 0;i < 2;i++) xmenu_lang[i]				= eng_menu_lang[i];
		for(i = 0;i < 6;i++) xmenu_user[i]				= eng_menu_user[i];
		for(i = 0;i < 7;i++) xmenu_user_enroll[i]		= eng_menu_user_enroll[i];
		for(i = 0;i < 7;i++) xmenu_access_mode[i]		= eng_menu_access_mode[i];
		for(i = 0;i < 10;i++) xmenu_security_level[i]	= eng_menu_security_level[i];
		for(i = 0;i < 1;i++) xmenu_ta_sub[i]			= eng_menu_ta_sub[i];
		for(i = 0;i < 4;i++) xmenu_time_attend[i]		= eng_menu_time_attend[i];
		for(i = 0;i < 2;i++) xmenu_ta_key[i]			= eng_menu_ta_key[i];
		for(i = 0;i < 3;i++) xmenu_ta_display[i]		= eng_menu_ta_display[i];
		for(i = 0;i < 3;i++) xmenu_ta_item[i]			= eng_menu_ta_item[i];
		for(i = 0;i < 9;i++) xmenu_meal[i]				= eng_menu_meal[i];
		for(i = 0;i < 2;i++) xmenu_meal_sub[i]			= eng_menu_meal_sub[i];
		for(i = 0;i < 4;i++) xmenu_meal_funckey[i]		= eng_menu_meal_funckey[i];
		for(i = 0;i < 3;i++) xmenu_event[i]				= eng_menu_event[i];
		for(i = 0;i < 3;i++) xmenu_event_output[i]		= eng_menu_event_output[i];
		for(i = 0;i < 3;i++) xmenu_event_mask[i]		= eng_menu_event_mask[i];
		for(i = 0;i < 2;i++) xmenu_event_wiegand[i]		= eng_menu_event_wiegand[i];
		for(i = 0;i < 9;i++) xmenu_access_control[i]	= eng_menu_access_control[i];
		for(i = 0;i < 6;i++) xmenu_hw_setup[i]			= eng_menu_hw_setup[i];
		for(i = 0;i < 3;i++) xmenu_op_mode[i]			= eng_menu_op_mode[i];
		for(i = 0;i < 6;i++) xmenu_all_readers[i]		= eng_menu_all_readers[i];
		for(i = 0;i < 19;i++) xmenu_hw_config[i]		= eng_menu_hw_config[i];
		for(i = 0;i < 3;i++) xmenu_unit_type[i]			= eng_menu_unit_type[i];
		for(i = 0;i < 2;i++) xmenu_iounit_type[i]		= eng_menu_iounit_type[i];
		for(i = 0;i < 7;i++) xmenu_access_door[i]		= eng_menu_access_door[i];
		for(i = 0;i < 3;i++) xmenu_door_config[i]		= eng_menu_door_config[i];
		for(i = 0;i < 5;i++) xmenu_door_inout[i]		= eng_menu_door_inout[i];
		for(i = 0;i < 2;i++) xmenu_lock_type[i]			= eng_menu_lock_type[i];
		for(i = 0;i < 2;i++) xmenu_door_alarm[i]		= eng_menu_door_alarm[i];
		for(i = 0;i < 2;i++) xmenu_door_alarm_info[i]	= eng_menu_door_alarm_info[i];
		for(i = 0;i < 1;i++) xmenu_access_point[i]		= eng_menu_access_point[i];
		for(i = 0;i < 4;i++) xmenu_authorization_mode[i]= eng_menu_authorization_mode[i];
		for(i = 0;i < 6;i++) xmenu_access_zone[i]		= eng_menu_access_zone[i];
		for(i = 0;i < 6;i++) xmenu_passback[i]			= eng_menu_passback[i];
		for(i = 0;i < 3;i++) xmenu_passback_mode[i]		= eng_menu_passback_mode[i];
		for(i = 0;i < 4;i++) xmenu_passback_setting[i]	= eng_menu_passback_setting[i];
		for(i = 0;i < 4;i++) xmenu_binary_input[i]		= eng_menu_binary_input[i];
		for(i = 0;i < 1;i++) xmenu_binary_output[i]		= eng_menu_binary_output[i];
		for(i = 0;i < 2;i++) xmenu_io_polarity[i]		= eng_menu_io_polarity[i];
		for(i = 0;i < 4;i++) xmenu_armed[i]				= eng_menu_armed[i];
		for(i = 0;i < 4;i++) xmenu_alarm[i]				= eng_menu_alarm[i];
		for(i = 0;i < 3;i++) xmenu_fire_zone[i]			= eng_menu_fire_zone[i];
		for(i = 0;i < 12;i++) xmenu_action_index[i]		= eng_menu_action_index[i];
		for(i = 0;i < 3;i++) xmenu_output_command[i]	= eng_menu_output_command[i];
		for(i = 0;i < 3;i++) xmenu_readers[i]			= eng_menu_readers[i];
		for(i = 0;i < 5;i++) xmenu_card_readers[i]		= eng_menu_card_readers[i];
		for(i = 0;i < 2;i++) xmenu_wiegand_reader[i]	= eng_menu_wiegand_reader[i];
		for(i = 0;i < 4;i++) xmenu_fp_reader[i]			= eng_menu_fp_reader[i];
		for(i = 0;i < 4;i++) xmenu_mifare_reader[i]		= eng_menu_mifare_reader[i];
		for(i = 0;i < 3;i++) xmenu_mifare_format[i]		= eng_menu_mifare_format[i];
		for(i = 0;i < 3;i++) xmenu_mifare_type[i]		= eng_menu_mifare_type[i];
		for(i = 0;i < 6;i++) xmenu_special_card[i]		= eng_menu_special_card[i];
		for(i = 0;i < 3;i++) xmenu_data_type[i]			= eng_menu_data_type[i];
		for(i = 0;i < 2;i++) xmenu_byte_order[i]		= eng_menu_byte_order[i];	
		for(i = 0;i < 3;i++) xmenu_em_format[i]			= eng_menu_em_format[i];
		for(i = 0;i < 3;i++) xmenu_external_format[i]	= eng_menu_external_format[i];
		for(i = 0;i < 3;i++) xmenu_ip_reader[i]			= eng_menu_ip_reader[i];	
		for(i = 0;i < 3;i++) xmenu_wiegand_26bit[i]		= eng_menu_wiegand_26bit[i];
		for(i = 0;i < 2;i++) xmenu_duress[i]			= eng_menu_duress[i];
		for(i = 0;i < 8;i++) xmenu_setup[i]				= eng_menu_setup[i];
		for(i = 0;i < 2;i++) xmenu_admin[i]				= eng_menu_admin[i];
		for(i = 0;i < 2;i++) xmenu_wiegand_type[i]		= eng_menu_wiegand_type[i];		
		for(i = 0;i < 2;i++) xmenu_datetime[i]			= eng_menu_datetime[i];
		for(i = 0;i < 6;i++) xmenu_monitor[i]			= eng_menu_monitor[i];
		for(i = 0;i < 2;i++) xmenu_time_notation[i]		= eng_menu_time_notation[i];
		for(i = 0;i < 3;i++) xmenu_date_notation[i]		= eng_menu_date_notation[i];
		for(i = 0;i < 3;i++) xmenu_sound[i]				= eng_menu_sound[i];
		for(i = 0;i < 2;i++) xmenu_lockout[i]			= eng_menu_lockout[i];
		for(i = 0;i < 7;i++) xmenu_cam[i]				= eng_menu_cam[i];		
		for(i = 0;i < 3;i++) xmenu_cam_mode[i]			= eng_menu_cam_mode[i];		
		for(i = 0;i < 2;i++) xmenu_cam_run[i]			= eng_menu_cam_run[i];		
		for(i = 0;i < 4;i++) xmenu_cam_channel[i]		= eng_menu_cam_channel[i];
		for(i = 0;i < 2;i++) xmenu_cam_direction[i]		= eng_menu_cam_direction[i];		
		for(i = 0;i < 8;i++) xmenu_doorphone[i]			= eng_menu_doorphone[i];		
		for(i = 0;i < 4;i++) xmenu_doorphone_op_mode[i]	= eng_menu_doorphone_op_mode[i];
		for(i = 0;i < 3;i++) xmenu_phone_number[i]		= eng_menu_phone_number[i];
		for(i = 0;i < 3;i++) xmenu_dialing_mode[i]		= eng_menu_dialing_mode[i];
		for(i = 0;i < 5;i++) xmenu_ip_phone_number[i]	= eng_menu_ip_phone_number[i];
		for(i = 0;i < 1;i++) xmenu_ip_phone_item[i]		= eng_menu_ip_phone_item[i];
		for(i = 0;i < 3;i++) xmenu_doorphone_volume[i]	= eng_menu_doorphone_volume[i];		
		for(i = 0;i < 3;i++) xmenu_doorphone_tone[i]	= eng_menu_doorphone_tone[i];		
		for(i = 0;i < 2;i++) xmenu_tone_info[i]			= eng_menu_tone_info[i];		
		for(i = 0;i < 2;i++) xmenu_doorphone_hw_setup[i]= eng_menu_doorphone_hw_setup[i];		
		for(i = 0;i < 4;i++) xmenu_doorphone_bgn[i]		= eng_menu_doorphone_bgn[i];		
		for(i = 0;i < 6;i++) xmenu_network[i]			= eng_menu_network[i];
		for(i = 0;i < 2;i++) xmenu_network_serial[i]	= eng_menu_network_serial[i];
		for(i = 0;i < 3;i++) xmenu_serial_port[i]		= eng_menu_serial_port[i];
		for(i = 0;i < 7;i++) xmenu_network_tcpip[i]		= eng_menu_network_tcpip[i];
		for(i = 0;i < 3;i++) xmenu_network_mode[i]		= eng_menu_network_mode[i];
		for(i = 0;i < 7;i++) xmenu_system[i]			= eng_menu_system[i];
		for(i = 0;i < 3;i++) xmenu_init_device[i]		= eng_menu_init_device[i];
		for(i = 0;i < 6;i++) xmenu_imexport[i]			= eng_menu_imexport[i];
		for(i = 0;i < 2;i++) xmenu_comm_stats[i]		= eng_menu_comm_stats[i];
		for(i = 0;i < 5;i++) xmenu_downfirmware[i]		= eng_menu_downfirmware[i];
		for(i = 0;i < 2;i++) xmenu_inuse[i]				= eng_menu_inuse[i];
		for(i = 0;i < 2;i++) xmenu_yesno[i]				= eng_menu_yesno[i];
		for(i = 0;i < 2;i++) xmenu_noyes[i]				= eng_menu_noyes[i];
		for(i = 0;i < 2;i++) xmenu_okcancel[i]			= eng_menu_okcancel[i];
		xmenu_enable = eng_menu_enable;
		for(i = 0;i < 24;i++) xmenu_other[i]			= eng_menu_other[i];
	}
}

void GetMonthName(int mm, char *buf)
{
	int		lang;
	
	lang = GetLanguage();
	if(mm < 1 || mm > 12) buf[0] = 0;
	else {
		strcpy(buf, _eng_months[mm-1]);
	}
}

void GetDayOfWeekName(int wday, char *buf)
{
	int		lang;
	
	lang = GetLanguage();
	if(wday >= 0 && wday < 7) {
		if(lang == LANG_KOR) strcpy(buf, _kor_day_of_weeks[wday]);
		else	strcpy(buf, _eng_day_of_weeks[wday]);
	} else {
		buf[0] = buf[1] = ' '; buf[2] = 0;
	}
}

#include "defs.h"
#include "syscfg.h"

void syscfgSetDefaultByLanguage(SYS_CFG *sys_cfg)
{
	int		i, size;

	strcpy(sys_cfg->Device.Maker, eng_msgs[M_MAKER_NAME]);	
	sys_cfg->TATexts[0][0] = 0;
	size = MAX_TA_MSGS_COUNT;
	for(i = 0;i < size;i++) strcpy(sys_cfg->TATexts[i+1], eng_ta_msgs[i]);
	size = taGetTextSize(); 	
	for( ;i < size;i++) sys_cfg->TATexts[i][0] = 0;
}

void syscfgSetByLanguage(SYS_CFG *sys_cfg)
{
	int		i, size;
	
	strcpy(sys_cfg->Device.Maker, xmsgs[M_MAKER_NAME]);
	sys_cfg->TATexts[0][0] = 0;
	size = MAX_TA_MSGS_COUNT;
	for(i = 0;i < size;i++) strcpy(sys_cfg->TATexts[i+1], xta_msgs[i]);
	size = taGetTextSize(); 	
	for( ;i < size;i++) sys_cfg->TATexts[i][0] = 0;
}
