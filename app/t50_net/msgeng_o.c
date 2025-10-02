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
	"Tamper\nAlarm",					// 66
	"Power\nFail"						// 67
};

//#ifdef _HYNIX

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

//#endif

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
	"Setup",
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

char eng_menu_user[4][MAX_MSG_LEN] = {
	"Add user",
	"Edit user"
	"Delete user",
	"Delete all user"
};

char eng_menu_user_reg[7][MAX_MSG_LEN] = {
	"ID",	
	"Access mode",
	"FP1",
	"FP2",
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
	"PIN+FP"
};

char eng_menu_security_level[10][MAX_MSG_LEN] = {
	"Terminal Set",
	"Lowest",
	"Very Low",
	"Low",
	"Somewhat Low",
	"Moderate",
	"Somewhat High",
	"High",
	"Very High",
	"Highest"		// Extremely High
};

char eng_menu_time_attend[3][MAX_MSG_LEN] = {
	"Timer",
	"Cancel",
	"T&A Items"
};

char eng_menu_ta_item[3][MAX_MSG_LEN] = {
	"Enable",
	"Text"
	"Time range"
};

char eng_menu_meal[9][MAX_MSG_LEN] = {
	"Lockout",
	"Breakfast",
	"Lunch",
	"Dinner",
	"Late Supper",
	"Snack",
	"No multiple",
	"Ticket Print",
	"Input Foods Quantity"
};

char eng_menu_meal_sub[2][MAX_MSG_LEN] = {
	"Time",
	"Menu count"
};

char eng_menu_meal_funckey[4][MAX_MSG_LEN] = {
	"Menu 1",
	"Menu 2",
	"Menu 3",
	"Menu 4"
};

char eng_menu_event[3][MAX_MSG_LEN] = {
	"Delete Event",
	"Output Type",
	"Mask Event"
};

char eng_menu_event_output[3][MAX_MSG_LEN] = {
	"Network",
	"26 Wiegand",
	"34 Wiegand"
};

char eng_menu_event_mask[3][MAX_MSG_LEN] = {
	"Lock Event",
	"Access Event",
	"Other Event"
};

char eng_menu_event_wiegand[2][MAX_MSG_LEN] = {
	"User ID",
	"Card Data"
};

char eng_menu_hardware[4][MAX_MSG_LEN] = {
	"Door",
	"Readers",
	"Inputs",
	"Outputs"
};

char eng_menu_access_door[7][MAX_MSG_LEN] = {
	"Reader",
	"Lock",
	"Request to exit",
	"Door status",
	"Pulse time",
	"Lock type",
	"Alarm"
};

char eng_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM lock",
	"Deadbolt"
};

char eng_menu_door_alarm[3][MAX_MSG_LEN] = {
	"Open too long alarm",
	"Forced open alarm",
	"Open too long time"
};

char eng_menu_alarm[4][MAX_MSG_LEN] = {
	"Alarm time delay",
	"Alarm post time delay",
	"Alarm output",
	"Alarm Inhibit"
};

char eng_menu_access_point[7][MAX_MSG_LEN] = {
	"Authentication mode",
	"Supported card formats",
	"Support PIN",
	"Alarm",
	"Server Authentication",
	"Reader setting",
	"Time & Attendance"
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

char eng_menu_server_auth[3][MAX_MSG_LEN] = {
	"Standalone",
	"Standalone+Server",
	"Server Only"
};

char eng_menu_reader_setting[6][MAX_MSG_LEN] = {
	"Mode",
	"Type A / Type B",
	"Speed",
	"Parity",
	"Stop bits", 
	"IP Address and Port"
};

char eng_menu_fp_reader[4][MAX_MSG_LEN] = {
	"Security Level",
	"Free Scan",
	"Exposure",
	"Fast Capture"
};

char eng_menu_reader[3][MAX_MSG_LEN] = {
	"FP Reader",
	"PIN Reader",
	"Card Reader"
};

char eng_menu_card_reader[5][MAX_MSG_LEN] = {
	"Smart card reader",
	"EM reader",
	"Wiegand reader",
	"Serial reader",
	"IP reader"
};

char eng_menu_sc_type[3][MAX_MSG_LEN] = {
	"Type A",
	"Type B",
	"Type A + Type B"
};

char eng_menu_binary_inputs[4][MAX_MSG_LEN] = {
	"Exit",
	"Input1",
	"Input2",
	"Input3"
};

char eng_menu_binary_outputs[2][MAX_MSG_LEN] = {
	"Lock",
	"Relay",
};

char eng_menu_binary_io[2][MAX_MSG_LEN] = {
	"Polarity",
	"Type"
};

char eng_menu_io_polarity[2][MAX_MSG_LEN] = {
	"Normal(Normal Open)",
	"Reverse(Normal Close)"
};

char eng_menu_bi_type[5][MAX_MSG_LEN] = {
	"General",
	"Request to exit",
	"Door status",
	"Elevator",
	"Intrusion zone"
};

char eng_menu_bo_type[3][MAX_MSG_LEN] = {
	"General",
	"Lock",
	"Elevator button"
};

char eng_menu_access_control[3][MAX_MSG_LEN] = {
	"Card format",
	"Smart card configuration",
	"Output command"
};

char eng_menu_credential[7][MAX_MSG_LEN] = {
	"32-bit Integer",
	"64-bit Integer",
	"Facility(8-bit)+CardNo(16-bit)",
	"Facility(16-bit)+CardNo(32-bit)",
	"Facility(32-bit)+CardNo(32-bit)",
	"BCD digits",
	"Character string"
};

char eng_menu_field_type[2][MAX_MSG_LEN] = {
	"Card number",
	"Facility code"
};

char eng_menu_cred_format[3][MAX_MSG_LEN] = {
	"Finger Print",
	"PIN",
	"Smart card format"
};

char eng_menu_card_format[7][MAX_MSG_LEN] = {
	"Serial Number(UID)",
	"Block",
	"ISO7816-4 file",
	"EM format",
	"Serial reader format",
	"IP reader format",
	"Wiegand format",
};

char eng_menu_wiegand_format[5][MAX_MSG_LEN] = {
	"Standard 26-bit format",
	"HID 37-bit format",
	"HID 37-bit format(facility)",
	"35-bit HID Coporate 1000",
	"User defined"
};

char eng_menu_format_setting[6][MAX_MSG_LEN] = {
	"Credential",
	"Card type",
	"Card app",
	"Output wiegand format",
	"Wiegand parity",
	"Field type"
};

char eng_menu_card_type[5][MAX_MSG_LEN] = {
	"ISo14443A",
	"ISo14443B",
	"ISO14443A+ISO14443B",
	"ISO15693",
	"EM card"
};

char eng_menu_card_field[4][MAX_MSG_LEN] = {
	"Format type",
	"Offset",
	"Length",
	"Coding",
};

char eng_menu_field_coding[3][MAX_MSG_LEN] = {
	"Binary",
	"BCD",
	"Character(ASCII)"
};

char eng_menu_mifare_blk[2][MAX_MSG_LEN] = {
	"Sector",
	"Block"
};

char eng_menu_wiegand_parity[3][MAX_MSG_LEN] = {
	"None",
	"Even/odd parity",
	"Other parity"
};

char eng_menu_output_command[3][MAX_MSG_LEN] = {
	"Repeat Count",
	"On Time",
	"Off Time"
};

char eng_menu_armed[4][MAX_MSG_LEN] = {
	"Setup Arm",
	"Armed Mode",
	"Armed",
	"Intrusion"
};

char eng_menu_byte_order[2][MAX_MSG_LEN] = {
	"Normal order",
	"Reverse order"
};

char eng_menu_em_format[3][MAX_MSG_LEN] = {
	"32-bit",
	"24-bit",
	"20-bit"
};

char eng_menu_external_format[3][MAX_MSG_LEN] = {
	"900MHz Reader",
	"Barcode Reader",
	"EM Reader"
};

char eng_menu_ip_reader[3][MAX_MSG_LEN] = {
	"Reader Network Setup",
	"IP Address",
	"Port"
};

char eng_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"Normal",
	"Card Number Only",
	"Normal + parity"
};

char eng_menu_duress[2][MAX_MSG_LEN] = {
	"Duress Digits",
	"Duress Alarm"
};

char eng_menu_setup[8][MAX_MSG_LEN] = {
	"Administrator",
	"Date and Time",
	"Monitor",
	"Sound",
	"Lockout",
	"Language",
	"Camera",
	"Door Phone"
};

char eng_menu_admin[2][MAX_MSG_LEN] = {
	"Admin PIN",
	"Admin FP"
};

char eng_menu_wiegand_type[2][MAX_MSG_LEN] = {
	"26 Wiegand",
	"34 Wiegand"
};

char eng_menu_datetime[3][MAX_MSG_LEN] = {
	"Set Date",
	"Set Time"
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

char eng_menu_sound[3][MAX_MSG_LEN] = {
	"User Volume",
	"Admin Volume",
	"Button Volume"
};

char eng_menu_lockout[2][MAX_MSG_LEN] = {
	"Tamper Enable",	
	"Key Lockout"
};

char eng_menu_cam[4][MAX_MSG_LEN] = {
	"Camera Control",
	"Camera Channel",
	"Camera View",
	"Event with Camera Image"
};

char eng_menu_cam_control[3][MAX_MSG_LEN] = {
	"Diable",
	"Off",
	"On"
};

char eng_menu_doorphone[8][MAX_MSG_LEN] = {
	"Operation Mode",
	"Phone Number",
	"Control Volume",
	"Silence Time",
	"Open Number",
	"Tone",
	"Tone Check",
	"Hardware Setting"
};

char eng_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"Disable",
	"IP Phone",
	"Interphone",
	"Telephone"
};

char eng_menu_phone_number[3][MAX_MSG_LEN] = {
	"Dialiing Mode",
	"IP Phone Number", 
	"Seed Dial Number"
};

char eng_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"Standard",
	"Single Number", 
	"Speed Dial"
};

char eng_menu_ip_phone_item[2][MAX_MSG_LEN] = {
	"Phone Number",
	"IP Address"
};

char eng_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"Speaker Volume",
	"Mic Volume",
	"Line Volume"
};

char eng_menu_doorphone_tone[3][MAX_MSG_LEN] = {
	"Ringback",
	"Busy",
	"Fast Busy"
};

char eng_menu_tone_info[2][MAX_MSG_LEN] = {
	"High Time",
	"Low Time"
};

char eng_menu_doorphone_hw_setup[2][MAX_MSG_LEN] = {
	"Switching Speed",
	"Back. Noise"
};

char eng_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char eng_menu_network[7][MAX_MSG_LEN] = {
	"Unit Address",
	"Network Type",
	"IP Address",
	"Master Setup",
	"Server Setup",
	"Wi-Fi",
	"Ping test"	
};

char eng_menu_network_ip[2][MAX_MSG_LEN] = {
	"LAN",
	"Wi-Fi"
};

char eng_menu_network_serial[2][MAX_MSG_LEN] = {
	"Serial Speed",
	"Serial Port"
};

char eng_menu_serial_port[3][MAX_MSG_LEN] = {
	"Port 0(RS-485)",
	"Port 1(RS-232)",
	"Port 2"
};

char eng_menu_network_tcpip[7][MAX_MSG_LEN] = {
	"MAC Address",
	"Dynamic Address",
	"Local IP Address",
	"Subnet Mask",
	"Gateway",
	"Server Host",
	"Server Port"
};

char eng_menu_network_type[2][MAX_MSG_LEN] = {
	"Serial(RS-485)",
	"IP Network"
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

char eng_menu_system[5][MAX_MSG_LEN] = {
	"View All Settings",
	"Initialize",
	"Import from USB",
	"Export to USB",
	"Re-initialize Device"
};

char eng_menu_init_config[4][MAX_MSG_LEN] = {
	"Init Event",
	"Init Setup",
	"Init Factory",
	"Format Flash"
};

char eng_menu_imexport[4][MAX_MSG_LEN] = {
	"Firmware",
	"Resources(Icons,Voices)",
	"Setup Files",
	"User Files"
};

char eng_menu_comm_stats[2][MAX_MSG_LEN] = {
	"View Statistics",
	"Init Statistics"
};

char eng_menu_yesno[2][MAX_MSG_LEN] = {
	"Yes",
	"No"
};

char eng_menu_noyes[2][MAX_MSG_LEN] = {
	"No",
	"Yes"
};

char eng_menu_okcancel[4][MAX_MSG_LEN] = {
	"OK",
	"Cancel",
	"Done",
	"Save"
};

char eng_menu_enable[2][MAX_MSG_LEN] = {
	"Enable",
	"Disable"
};

char eng_menu_hw_config[9][MAX_MSG_LEN] = {
	"Local",
	"Unit",
	"Door",
	"Reader",
	"Input",
	"Output",
	"Wiegand",
	"RS-485",
	"CH"
};

char eng_menu_unit[5][MAX_MSG_LEN] = {
	"bytes",
	"bits",
	"digits",
	"seconds",
	"minutes"
};

char eng_menu_other[25][MAX_MSG_LEN] = {
	"Firmware Version",
	"Model Name",
	"User Count",
	"Fingerprint Count",
	"Minutes",
	"Unlimited",		// Never timeout
	"Coupon",
	"0:String(null)",
	"IP Phone No. Count",
	"All",
	"0.5 Second",
	"Unspecified",
	"Never",
	"Always",
	"Schedule",
	"Always OFF",
	"Always ON",
	"Wait",
	"OK",
	"Failed",
	"Disconnected",
	"Downloading",
	"Download Completed",
	"Master Password",
	"Total Event Count"
};

char _eng_day_of_weeks[7][4] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Ffi", "Sat"
};

char _eng_am_pm[2][4] = {
	"AM", "PM"
};

char _eng_months[12][4] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
