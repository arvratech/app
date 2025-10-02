char eng_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"CompanyName",					// 0
	"Enter User ID",				// 1
	"Verify User ID",				// 2
	"Enter Password",				// 3
	"Verify Password",				// 4
	"Enter New Password",			// 5
	"Verify New Password",			// 6
	"Place Your Finger",			// 7
	"Place Your Finger Again",		// 8
	"Touch Your Card",				// 9
	"Touch Your Card Again",		// 10
	"One More Time, Please",		// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"seconds",						// 14
	"Always",						// 15
    "Are you delete users ?",		// 16
	"Dial extention Number",		// 17
	"Enter Speed-dial Number",		// 18
	"Are yoy really execute it ?",	// 19
	"Wait a moment"					// 20
};
//	"New Time",						// 12
//	"New TID",						// 13

char eng_rsp_msgs[MAX_RSP_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"Processing...",					// 0
	"Sorry!\nSystem Error",				// 1
	"Input Time Over",					// 2
	"Invalid\nUser ID\n",				// 3
	"Invalid Data",						// 4
	"Sorry!\nNot Found",				// 5
	"Access Denied",					// 6
	"Access Granted",					// 7
	"User ID\nOverlapped",				// 8
	"Card Data\nOverlapped",			// 9
	"Fingerprint\nOverlapped",			// 10
	"Sorry!\nP/W Mismatched",			// 11
	"Sorry!\nData Mismatched",			// 12
	"OK!\nData Saved",					// 13
	"OK!\nUser Deleted",				// 14
	"OK!\nAll User Deleted",			// 15
	"Invalid Time",						// 16
	"Invalid TID",						// 17
	"Invalid Date",						// 18
	"OK!\nDate Changed",				// 19
	"Invalid Time",						// 20
	"OK!\nTime Changed",				// 21
	"Invalid\nIP Address",				// 22
	"Invalid\nSubnet Mask",				// 23
	"Invalid\nGateway Address",			// 24
	"Invalid\nServer Host",				// 25
	"Invalid\nServer Port",				// 26
	"OK!\nAll Deleted",					// 27
	"Sorry!\nUser are full", 			// 28
	"Invalid\nExtention Number",		// 29
	"No Dialtone",						// 30
	"Busy",								// 31
	"Number is not in service",			// 32
	"No Answer",						// 33
	"Call\nDisconnected",				// 34
	"Call\nTerminated",					// 35
	"Door\nUnlocked",					// 36
	"OK!\nProcessed",					// 37
	"Sorry!\nFailed",					// 38
	"OK!\nCompleted",					// 39
	"Not Allowed Card",					// 40
	"Touch\nsingle card only",			// 41
	"Sorry!\nNot Found",				// 42
	"Sorry!\nOverlapped",				// 43
	"Sorry!\nData are full",			// 44
	"OK!\nDeleted",						// 45
	"Non exist\nSD memory card",		// 46
	"Not Ready\nSD memory card",		// 47
	"\nDirectory open error",			// 48
	"\nDirectory read error",			// 49
	"\nDirectory write error",			// 50
	"\nFile open error",				// 51
	"\nFile read error",				// 52
	"\nFile write error",				// 53
	"\nFile sync error",				// 54
	"\ndirectory not found",			// 55
	"\nfile not found",					// 56
	"Unsupported\nsecurity",			// 57
	"Wait a moment\nSystem restart",	// 58
	"You can now safely remove\nSD memory card",	// 59
	"SD memory error\nCheck it",		// 60
	"Failed to server\nTry again later",// 61
	"Failed to ACU\nTimeout",			// 62
	"Write conflict occurred\nfrom ACU",// 63
	"Other error occurred\nfrom ACU",	// 64
	"Network\nNot Obtainable",			// 65
	"Network\nFailure",					// 66
	"Server\nNot responded",			// 67
	"Tamper\nAlarm",					// 68
	"Power\nFail",						// 69
	"Power\nOff",						// 70
	"Slave\ninitializing",				// 71
};

char eng_auth_msgs[9][MAX_MULTIMSG_LEN] = {
	"Not\nActive Period",				// 0
	"Door\nIs Opened",					// 1
	"Door\nIs Closed",					// 2
	"No\nAccess Rights",				// 3
	"Lost\nCard",						// 4
	"Invalid\nAccess Mode",				// 5
	"Anti-Passback\nDenied",			// 6
	"Already\nProcessed Card",			// 7
	"Denied\nAll"						// 8
};

char eng_alarm_msgs[4][MAX_MULTIMSG_LEN] = {
	"Violation",
	"Door is opend forcely",
	"Door is opened",
	"Close the door"
};

char eng_ta_msgs[MAX_TA_MSGS_COUNT][MAX_TAMSG_LEN] = {
	"IN",				// 1
	"OUT",				// 2
	"ABSENCE",			// 3
	"BACK",				// 4
	"GO OUT",			// 5
	"NORMAL",			// 6
	"OT IN",			// 7
	"OT OUT"			// 8
};

char eng_menu_top_title[MAX_MSG_LEN] = "Settings";

char eng_menu_top[6][MAX_MSG_LEN] = {
	"User",
	"Hardware",
	"Access Control",
	"Network",
	"Device Setup",
	"System"
};

char eng_menu_lang[2][MAX_MSG_LEN] = {
	"English",
#ifdef _CHN
	"Chinese",
#else
#ifdef _JPN
	"Japaness",
#else
#ifdef _TWN
	"Taiwan",
#else
	"Korean",
#endif
#endif
#endif
};

char eng_menu_user[3][MAX_MSG_LEN] = {
	"Add user",
	"Edit/Delete user",
	"Delete all user"
};

char eng_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",	
	"Access mode",
	"FP 1",
	"FP 2",
	"Card",
	"PIN",
	"Security level"
};

char eng_menu_user_regex[4][MAX_MSG_LEN] = {
	"Access rights",
	"Activate date",
	"Expire date",
	"Card status"
};

char eng_menu_access_mode[7][MAX_MSG_LEN] = {
	"FP",
	"Card",
	"PIN",
	"Card+FP",
	"Card+PIN",
	"Carta+FP+PIN",
	"FP+PIN"
};

char eng_menu_hardware_class[12][MAX_MSG_LEN] = {
	"Network ports",
	"Units",
	"IoUnits",
	"Subdevices",
	"Readers",
	"Inputs",
	"Outputs",
	"Doors",
	"Access zones",
	"Fire zones",
	"Intrusion zones",
	"Elevators",
};

char eng_menu_device[3][MAX_MSG_LEN] = {
	"Inputs/Outputs",
	"Master network port",
	"Slave devices",
};

char eng_menu_hardware[11][MAX_MSG_LEN] = {
	"Address",
	"Device ID",
	"Network address",
	"Model name",
	"Assigned to",
	"Type",
	"Channel",
	"Network port",
	"Reader",
	"Reader device",
	"Unit"
};

char eng_menu_unit_model[4][MAX_MSG_LEN] = {
	"Virtual",
	"K200-4",
	"K200-2",
	"Local"
};

char eng_menu_iounit_model[2][MAX_MSG_LEN] = {
    "K500-I/24",
    "K700-R/12"
};

char eng_menu_unit_ch[4][MAX_MSG_LEN] = {
	"CH1",
	"CH2",
	"CH3",
	"CH4",
};

char eng_menu_reader_type[5][MAX_MSG_LEN] = {
	"General",
	"Primary reader",
	"Secondary reader",
	"Elevator",
	"Undefined"
};

char eng_menu_input_type[7][MAX_MSG_LEN] = {
	"General",
	"RequestToExit",
	"Door switch",
	"Elevator",
	"Intrusion zone",
	"Fire zone",
	"Undefined"
};

char eng_menu_output_type[5][MAX_MSG_LEN] = {
	"General",
	"Door lock",
	"Alarm output",
	"Elevator button",
	"Undefined"
};

char eng_menu_input_output[1][MAX_MSG_LEN] = {
	"Polarity",
};

char eng_menu_io_polarity[2][MAX_MSG_LEN] = {
	"Normal",
	"Reverse"
};

char eng_menu_armed[4][MAX_MSG_LEN] = {
	"Setup arm",
	"Armed mode",
	"Armed",
	"Intrusion"
};

char eng_menu_fire_zone[4][MAX_MSG_LEN] = {
	"Fire zone",
	"Fire inputs",
	"Unlock doors",
	"Alarm relays",
};

char eng_menu_dev_model[6][MAX_MSG_LEN] = {
	"K100-4",
	"K100-2",
	"Wiegand reader",
	"Wiegand reader+Keypad",
	"T35s-NonIO",
	"T40A",
};

char eng_menu_door[13][MAX_MSG_LEN] = {
	"Primary reader",
	"Secondary reader",
	"Inputs/Outputs usage",
	"Door lock relay",
	"Request to exit input",
	"Door switch monitor",
	"Alarm",
	"Alarm relay",
	"Alarm action",
	"Extended pulse time",
	"Pulse time",
	"Lock type",
	"Secondary lock"
};

char eng_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM lock",
	"Deadbolt"
};

char eng_menu_alarm[4][MAX_MSG_LEN] = {
	"Alarm time delay",
	"Alarm post time delay",
	"Open too long time"
	"Alarm Inhibit"
};

char eng_menu_access_point[7][MAX_MSG_LEN] = {
	"Supported credential formats",
	"Authentication mode",
	"Support PIN",
	"Alarm",
	"Server Authentication",
	"Reader setting",
	"Time & Attendance"
};

char eng_menu_server_auth[3][MAX_MSG_LEN] = {
	"Standalone",
	"Standalone+Server",
	"Server Only"
};

char eng_menu_auth_mode[4][MAX_MSG_LEN] = {
	"Authorize",
	"Grant active",
	"Deny all",
	"Card grant all"
};

char eng_menu_ap_alarm[3][MAX_MSG_LEN] = {
	"Unregistered alarm",
	"Expired alarm",
	"Access denied alarm"
};

char eng_menu_reader[7][MAX_MSG_LEN] = {
	"Smart card reader",
	"EM reader",
	"Wiegand reader",
	"Serial reader",
	"IP reader",
	"PIN reader",
	"Fingerprint reader"
};

char eng_menu_readerop[6][MAX_MSG_LEN] = {
	"Authentication mode",
	"Authentication result",
	"Sleep mode",
	"Display date/time",
	"Antena output level",
	"T&A display time" 
};

char eng_menu_authmode[5][MAX_MSG_LEN] = {
	"General mode",
	"Card only",
	"Card + PIN",
	"Card or PIN",
	"PIN only"
};

char eng_menu_authresult[4][MAX_MSG_LEN] = {
	"Control LED and Beep",
	"Wait time",
	"Display Time",
	"Inhibit Authentication"
};

char eng_menu_ledbeep[4][MAX_MSG_LEN] = {
	"Disable",
	"Red LED + Beep",
	"Blue LED + Beep",
	"Dual line LED "
};

char eng_menu_ac[4][MAX_MSG_LEN] = {
	"Format setting",
	"Alarm actions",
	"Input formats",
	"Wiegand output formats"
};

char eng_menu_credential[5][MAX_MSG_LEN] = {
	"32-bit Integer",
	"64-bit Integer",
	"Facility+CardNo",
	"BCD digits",
	"Character string"
};

char eng_menu_field_type[2][MAX_MSG_LEN] = {
	"Card number",
	"Facility code"
};

char eng_menu_cred_format[4][MAX_MSG_LEN] = {
	"Smart card format",
	"Wiegand card format",
	"PIN format",
	"Wiegand PIN format"
};

char eng_menu_card_format[8][MAX_MSG_LEN] = {
	"UID format",
	"Block format",
	"EMV(ISO7816) format",
	"EM format",
	"Serial reader format",
	"IP reader format",
	"Coding format",
	"Mobile(EMV) card",
};

char eng_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"Standard 26-bit format",
	"35-bit HID Coporate 1000",
//	"HID 37-bit format",
//	"HID 37-bit format(facility)",
	"34-bit format with parity",
	"66-bit format with parity",
	"User defined"
};

char eng_menu_pin_format[5][MAX_MSG_LEN] = {
	"Entry interval",
	"Entry stop character",
	"Maximum length",
	"Facility code",
	"PIN number",
};

char eng_menu_wiegand_pin[4][MAX_MSG_LEN] = {
	"4-bit burst mode",
	"8-bit burst mode",
	"Standard 26-bit format",
	"Fixed length mode"
};

char eng_menu_format_setting[8][MAX_MSG_LEN] = {
	"Credential",
	"Card type",
	"Card app",
	"Output wiegand format",
	"Wiegand parity",
	"Coding",
	"Card fields",
	"Reader setting"
};

char eng_menu_card_type[5][MAX_MSG_LEN] = {
	"ISO14443A",
	"ISO14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM card"
};

char eng_menu_cardtype_ext[3][MAX_MSG_LEN] = {
	"900MHz",
	"Bar code",
	"Other card"
};

char eng_menu_cardapp_block[2][MAX_MSG_LEN] = {
	"Normal",
	"Diversified"
};

char eng_menu_cardapp_iso7816[3][MAX_MSG_LEN] = {
	"General",
	"Transport card",
	"SK SAM"	
};

char eng_menu_cardapp_serial[2][MAX_MSG_LEN] = {
	"General",
	"Special"
};

char eng_menu_card_field[3][MAX_MSG_LEN] = {
	"Format type",
	"Offset",
	"Length"
};

char eng_menu_field_coding[4][MAX_MSG_LEN] = {
	"Binary",
	"BCD",
	"Character(ASCII)",
	"Reverse binary"
};

char eng_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"None",
	"Even/odd parity",
	"Other parity"
};

char eng_menu_reader_setting[8][MAX_MSG_LEN] = {
	"Block number",
	"Sector",
	"Block",
	"Speed",
	"Parity",
	"Stop bits", 
	"Mifare key",
	"AID"
};

char eng_menu_ip_reader[3][MAX_MSG_LEN] = {
	"Reader Network Setup",
	"IP Address",
	"Port"
};

char eng_menu_action_idx[12][MAX_MSG_LEN] = {
	"Input(Exit)",
	"Input(Door)",
	"Input(Input)",
	"Input(Fire,Tamper)",
	"Fire Alarm",
	"Door Open Too Long",
	"Door Forced Open",
	"Unregistered",
	"Expired",
	"Passback Detected",
	"Access Denied",
	"Intrusion"
};

char eng_menu_action_cmd[4][MAX_MSG_LEN] = {
	"Alarm",
	"On time",
	"Off time",
	"Repeat Count"
};

char eng_menu_setup[11][MAX_MSG_LEN] = {
	"Admin password",
	"Date and Time",
	"Display",
	"Sounds",
	"Language",
	"Authentication control",
	"Power management",
	"Camera",
	"BLE(Bluetooth Low Energy)",
	"IP phone",
	"Antena output level"
};

char eng_menu_admin[2][MAX_MSG_LEN] = {
	"Change password",
	"Digits only"
};

char eng_menu_datetime[3][MAX_MSG_LEN] = {
	"Set Date",
	"Set Time",
	"Use 24-hour Format"
};

char eng_menu_display[3][MAX_MSG_LEN] = {
	"BackLight Time",
	"LCD Contrast",
	"LCD Brightness",
};

char eng_menu_time_notation[2][MAX_MSG_LEN] = {
	"PM 1:00",
	"13:00"
};

char eng_menu_date_notation[3][MAX_MSG_LEN] = {
	"No",
	"MM/DD",
	"DD MON"
};

char eng_menu_sound[6][MAX_MSG_LEN] = {
	"User volume",
	"Media volume",
	"Call volume",
	"System volume",
	"Keypad tone",
	"Touch sounds"
};

char eng_menu_sleepmode[4][MAX_MSG_LEN] = {
	"Set sleep mode",
	"Wait time",
	"Video"
};

char eng_menu_sleepact[3][MAX_MSG_LEN] = {
	"Disable",
	"Off display",
	"Play video"
};

char eng_menu_video[1][MAX_MSG_LEN] = {
	"Copy from USB"
};

char eng_menu_lockout[2][MAX_MSG_LEN] = {
	"Tamper Enable",	
	"Key Lockout"
};

char eng_menu_power_mgmt[2][MAX_MSG_LEN] = {
	"Use battery",
	"Use lock at battery"
};

char eng_menu_cam[2][MAX_MSG_LEN] = {
	"Frames per second",
	"Event with camera image"
};

char eng_menu_cam_control[3][MAX_MSG_LEN] = {
	"Diable",
	"Off",
	"On"
};

char eng_menu_ble[5][MAX_MSG_LEN] = {
	"Collect beacons",
	"Beacon signal level",
	"Beacon interval",
	"MAC address filter",
	"View beacons"
};

char eng_menu_phone[6][MAX_MSG_LEN] = {
	"Phone accounts",
	"Add account",
	"Edit account"
	"Delete account",
	"Delete all account",
	"Dial phone"
};

char eng_menu_phone_reg[2][MAX_MSG_LEN] = {
	"Name",
	"SIP account"
};

char eng_menu_call_state[3][MAX_MSG_LEN] = {
	"Dialing",
	"Connecting",
	"Call ended"
};

char eng_menu_network[10][MAX_MSG_LEN] = {
	"Setup network",
	"Setup IP address",
	"Server setup",
	"Server IP address",
	"Server port",
	"Master setup",
	"Master IP address",
	"Master port",
	"Ping test",
	"Ping execute"
};

char eng_menu_network_ip[8][MAX_MSG_LEN] = {
	"Hardware(MAC) address",
	"Get an IP Adddress automatically",		// Obtain an IP Address automatically
	"Local IP address",
	"Subnet mask",
	"Default gateway",
	"Get DNS server automatically",	// Obtain DNS server address automatically
	"Preferred DNS server",
	"Alternate DNS server",
};

char eng_menu_network_serial[2][MAX_MSG_LEN] = {
	"Serial speed",
	"Protocol"
};

char eng_menu_network_type[2][MAX_MSG_LEN] = {
	"IP",
	"RS-485"
};

char eng_menu_network_wifi[8][MAX_MSG_LEN] = {
	"SSID",
	"State",
	"Signal strength",
	"Channel",
	"Security",
	"Forget",
	"Connect",
	"unchanged"
};

char eng_menu_wpa_state[12][MAX_MSG_LEN] = {
	"Hardware disabled",
	"Inactive",
	"Disconnected",
	"Dormant",
	"Scanning...",
	"Authenticating...",
	"Connecting...",
	"Connecting...",
	"Authenticating...",
	"Authenticating...",
	"Connected",
	"Receiving IP Address"
};

char eng_menu_wifi_security[8][MAX_MSG_LEN] = {
	"None",
	"WPS",
	"WEP",
	"WPA PSK",
	"WPA PSK",
	"WPA2 PSK",
	"802.1x EAP"
	"802.1x EAP"
};

char eng_menu_wifi_state[6][MAX_MSG_LEN] = {
	"Disabled",
	"Saved",
	"Not in range",
	"Secured with",
	"Authentication error",
	"Other error"
};

char eng_menu_system[9][MAX_MSG_LEN] = {
	"Master/Slave device",
	"View all settings",
	"Initialize system",
	"Test system",
	"Import from USB",
	"Export to USB",
	"Download",
	"Re-initialize device",
	"Power off"
};

char eng_menu_download[4][MAX_MSG_LEN] = {
	"Background screen",
	"Firmware",
	"OS Kernel",
	"Video"
};

char eng_menu_mst_slv[2][MAX_MSG_LEN] = {
	"Master device",
	"Slave device"
};

char eng_menu_net_remote[2][MAX_MSG_LEN] = {
	"Server connect",
	"Master Connect"
};

char eng_menu_init_config[4][MAX_MSG_LEN] = {
	"Init event",
	"Init setup",
	"Init factory",
	"Format flash"
};

char eng_menu_test[4][MAX_MSG_LEN] = {
	"Voice",
	"Inputs",
	"Door inputs/outputs",
	"Card reader"
};

char eng_menu_imexport[6][MAX_MSG_LEN] = {
	"Firmware",
	"OS Kernel",
	"Resources(Icons,Voices)",
	"Background screen",
	"Setup files",
	"User files"
};

char eng_menu_comm_stats[2][MAX_MSG_LEN] = {
	"View statistics",
	"Init statistics"
};

char eng_menu_bat_status[6][MAX_MSG_LEN] = {
	"Not present",
	"Charging",
	"Discharging",
	"Not charging",
	"Full",
	"Present"
};

char eng_menu_yesno[2][MAX_MSG_LEN] = {
	"Yes",
	"No"
};

char eng_menu_noyes[2][MAX_MSG_LEN] = {
	"No",
	"Yes"
};

char eng_menu_okcancel[5][MAX_MSG_LEN] = {
	"OK",
	"Cancel",
	"Done",
	"Save",
	"Delete"
};

char eng_menu_enable[2][MAX_MSG_LEN] = {
	"Enable",
	"Disable"
};

char eng_menu_unit[5][MAX_MSG_LEN] = {
	"bits",
	"digits",
	"bytes",
	"seconds",
	"minutes"
};

char eng_menu_other[15][MAX_MSG_LEN] = {
	"OS kernel version",
	"Firmware version",
	"Model name",
	"Unlimited",		// Never timeout
	"All",
	"Input power level",
	"Battery",
	"File size",
	"Unspecified",
	"Never",
	"Always",
	"Schedule",
	"Master password",
	"Event count",
	"user count",
};

char _eng_am_pm[2][4] = {
	"AM", "PM"
};

char _eng_day_of_weeks[7][12] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};

char _eng_months[12][12] = {
	"January", "February", "March", "April", "May", "June", "Juy", "Auguest", 
	"September", "October", "November", "December"
};

