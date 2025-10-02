#ifndef _MSG_H_
#define _MSG_H_


#define MAX_MSGS_COUNT			21

#define M_MAKER_NAME				0
#define M_ENTER_USER_ID				1
#define M_VERIFY_USER_ID			2
#define M_ENTER_PASSWORD			3
#define M_VERIFY_PASSWORD			4
#define M_ENTER_NEW_PW				5
#define M_VERIFY_NEW_PW				6
#define M_PLACE_YOUR_FINGER			7
#define M_PLACE_YOUR_FINGER_AGAIN	8
#define M_TOUCH_YOUR_CARD			9
#define M_TOUCH_YOUR_CARD_AGAIN		10
#define M_ONE_MORE_TIME_PLEASE		11
#define M_YYMMDD					12
#define M_HHMMSS					13
#define M_SECOND					14
#define M_ALWAYS					15
#define M_DELETE_CONFIRM			16
#define M_DIAL_EXT_NUMBER			17
#define M_ENTER_SPEED_DIAL_NNUMBER	18
#define M_ARE_YOU_REALLY			19
#define M_WAIT_A_MOMENT				20

#define MAX_RSP_MSGS_COUNT		72

#define R_PROCESSING				0
#define R_MSG_SYSTEM_ERROR			1
#define R_INPUT_TIME_OVER			2
#define R_INVALID_USER_ID			3
#define R_INVALID_DATA				4
#define R_USER_NOT_FOUND			5
#define R_ACCESS_DENIED				6
#define R_ACCESS_GRANTED			7
#define R_USER_ID_OVERLAPPED		8
#define R_CARD_DATA_OVERLAPPED		9
#define R_FP_DATA_OVERLAPPED		10
#define R_PW_MISMATCHED				11
#define R_DATA_MISMATCHED			12
#define R_DATA_SAVED				13
#define R_USER_DELETED				14
#define R_ALL_USER_DELETED			15
#define R_INVALID_TIME_INTERVAL		16
#define R_INVALID_TID				17
#define R_INVALID_DATE				18
#define R_DATE_CHANGED				19
#define R_INVALID_TIME				20
#define R_TIME_CHANGED				21
#define R_INVALID_IP_ADDRESS		22
#define R_INVALID_SUBNET_MASK		23
#define R_INVALID_GW_ADDRESS		24
#define R_INVALID_SERVER_HOST		25
#define R_INVALID_SERVER_PORT		26
#define R_ALL_DELETED				27
#define R_USER_ARE_FULL				28
#define R_INVALID_EXT_NUMBER		29
#define R_TEL_NO_DIALTONE			30
#define R_TEL_BUSY					31
#define R_TEL_NUMBER_UNOBTAINABLE	32 
#define R_TEL_NO_ANSWER				33
#define R_TEL_DISCONNECTED			34
#define R_TEL_TERMINATED			35
#define R_DOOR_UNLOCKED				36
#define R_PROCESSED					37
#define R_FAILED					38
#define R_COMPLETED					39
#define R_NOT_ALLOWED_CARD			40
#define R_TOUCH_SINGLE_CARD			41
#define R_NOT_FOUND					42
#define R_OVERLAPPED				43
#define R_DATA_ARE_FULL				44
#define R_DELETED					45
#define R_NO_SD_DISK				46
#define R_NOT_READY_SD_DISK			47
#define R_DIR_OPEN_ERROR			48
#define R_DIR_READ_ERROR			49
#define R_DIR_WRITE_ERROR			50
#define R_FILE_OPEN_ERROR			51
#define R_FILE_READ_ERROR			52
#define R_FILE_WRITE_ERROR			53
#define R_FILE_SYNC_ERROR			54
#define R_DIRECTORY_NOT_FOUND		55
#define R_FILE_NOT_FOUND			56
#define R_UNSUPPORTED_SECURITY		57
#define R_SYSTEM_RESTART			58
#define R_SAFELY_REMOVE_SD_DISK		59
#define R_SD_DISK_ERROR_CHECK		60
#define R_ACCESS_DENIED_FAIL_SERVER 61
#define R_ACU_TIMEOUT				62
#define R_ACU_WRITE_CONFLICT		63
#define R_ACU_OTHER_ERROR			64
#define R_NETWORK_NOT_OBTAINABLE	65
#define R_NETWORK_FAILED			66
#define R_SERVER_NOT_RESPONDED		67
#define R_TAMPER_ALARM				68
#define R_POWER_FAIL				69
#define R_POWER_OFF					70
#define R_SLAVE_INITIALIZING		71

#define MAX_TA_MSGS_COUNT	8

#define MAX_MULTIMSG_LEN			60
#define MAX_MSG_LEN					32
#define MAX_TAMSG_LEN				12

extern unsigned char xrsp_msgs_icon[];

extern char *xmenu_null;
extern char *xmenu_unsupported;
extern char *xmenu_serial_speed[];
extern char *xmenu_parity[];
extern char *xmenu_stop_bits[];

extern char *xmsgs[];
extern char *xrsp_msgs[];
extern char *xauth_msgs[];
extern char *xalarm_msgs[];
extern char *xta_msgs[];
extern char *xmenu_top_title;
extern char *xmenu_top[];
extern char *xmenu_lang[];
extern char *xmenu_user[];
extern char *xmenu_user_reg[];
extern char *xmenu_user_regex[];
extern char *xmenu_access_mode[];
extern char *xmenu_hardware_class[];
extern char *xmenu_device[];
extern char *xmenu_hardware[];
extern char *xmenu_unit_model[];
extern char *xmenu_iounit_model[];
extern char *xmenu_unit_ch[];
extern char *xmenu_reader_type[];
extern char *xmenu_input_type[];
extern char *xmenu_output_type[];
extern char *xmenu_input_output[];
extern char *xmenu_io_polarity[];
extern char *xmenu_armed[];
extern char *xmenu_fire_zone[];
extern char *xmenu_dev_model[];
extern char *xmenu_door[];
extern char *xmenu_lock_type[];
extern char *xmenu_door_alarm[];
extern char *xmenu_alarm[];
extern char *xmenu_access_point[];
extern char *xmenu_auth_mode[];
extern char *xmenu_server_auth[];
extern char *xmenu_ap_alarm[];
extern char *xmenu_reader[];
extern char *xmenu_readerop[];
extern char *xmenu_authmode[];
extern char *xmenu_authresult[];
extern char *xmenu_ledbeep[];
extern char *xmenu_ac[];
extern char *xmenu_credential[];
extern char *xmenu_field_type[];
extern char *xmenu_cred_format[];
extern char *xmenu_card_format[];
extern char *xmenu_wiegand_format[];
extern char *xmenu_pin_format[];
extern char *xmenu_wiegand_pin[];
extern char *xmenu_format_setting[];
extern char *xmenu_card_type[];
extern char *xmenu_cardtype_ext[];
extern char *xmenu_cardapp_block[];
extern char *xmenu_cardapp_iso7816[];
extern char *xmenu_cardapp_serial[];
extern char *xmenu_card_field[];
extern char *xmenu_field_coding[];
extern char *xmenu_wiegand_parity[];
extern char *xmenu_reader_setting[];
extern char *xmenu_ip_reader[];
extern char *xmenu_action_idx[];
extern char *xmenu_action_cmd[];
extern char *xmenu_setup[];
extern char *xmenu_admin[];
extern char *xmenu_datetime[];
extern char *xmenu_display[];
extern char *xmenu_time_notation[];
extern char *xmenu_date_notation[];
extern char *xmenu_sound[];
extern char *xmenu_sleepmode[];
extern char *xmenu_sleepact[];
extern char *xmenu_video[];
extern char *xmenu_lockout[];
extern char *xmenu_power_mgmt[];
extern char *xmenu_cam[];
extern char *xmenu_cam_control[];
extern char *xmenu_ble[];
extern char *xmenu_phone[];
extern char *xmenu_phone_reg[];
extern char *xmenu_call_state[];
extern char *xmenu_network[];
extern char *xmenu_network_ip[];
extern char *xmenu_network_serial[];
extern char *xmenu_network_type[];
extern char *xmenu_network_wifi[];
extern char *xmenu_wpa_state[];
extern char *xmenu_wifi_security[];
extern char *xmenu_wifi_state[];
extern char *xmenu_serial_speed[];
extern char *xmenu_system[];
extern char *xmenu_download[];
extern char *xmenu_mst_slv[];
extern char *xmenu_net_remote[];
extern char *xmenu_init_config[];
extern char *xmenu_test[];
extern char *xmenu_imexport[];
extern char *xmenu_comm_stats[];
extern char *xmenu_bat_status[];
extern char *xmenu_yesno[];
extern char *xmenu_noyes[];
extern char *xmenu_okcancel[];
extern char *xmenu_enable[];
extern char *xmenu_unit[];
extern char *xmenu_other[];

void msgInitialize(void);
void dtGetShortDayOfWeek(int lang, int wday, char *buf);
void dtGetLongDayOfWeek(int lang, int wday, char *buf);
void dtGetHour(int hour24, int hour, char *buf);
void dtGetMinute(int minute, char *buf);
void dtGetNoon(int lang, int hour, char *buf);
void dtGetShortMonth(int lang, int month, char *buf);
void dtGetLongMonth(int lang, int month, char *buf);
int  dtShortMonthString2Month(char *buf);
char *GetCJKYear(void);
char *GetCJKMonth(void);
char *GetCJKDay(void);


#endif
