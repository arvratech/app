char eng_msgs[MAX_MSGS_COUNT][MAX_MULTIMSG_LEN] = {
	"CompanyName",					// 0
	"Enter User ID:",				// 1
	"Verify User ID:",				// 2
	"Enter Password:",				// 3
	"Verify Password:",				// 4
	"Enter New Password:",			// 5
	"Verify New Password:",			// 6
	"Place\nYour Finger",			// 7
	"Place Your\nFinger Again",		// 8
	"Touch\nYour Card",				// 9
	"Touch\nYour Card Again",		// 10
	"One More Time,\nPlease",		// 11
	"YY/MM/DD",						// 12
	"HH:MM:SS",						// 13
	"Seconds",						// 14
	"Always",						// 15
	"Delete All Users?",			// 16
	"Dial Extention Number",		// 17
	"Enter Speed-dial Number",		// 18
	"Confirm Execution ?",			// 19
	"Wait a Moment"					// 20
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
	"Access\nDenied",					// 6
	"Access Granted",					// 7
	"User ID\nOverlapped",				// 8
	"Card Data\nOverlapped",			// 9
	"Fingerprint\nOverlapped",			// 10
	"Sorry!\nP/W Mismatched",			// 11
	"Sorry!\nData Mismatched",			// 12
	"Yes!\nData Saved",					// 13
	"Yes!\nUser Deleted",				// 14
	"Yes!\nAll User Deleted",			// 15
	"Invalid Time",						// 16
	"Invalid TID",						// 17
	"Invalid Date",						// 18
	"Yes!\nDate Changed",				// 19
	"Invalid Time",						// 20
	"Yes!\nTime Changed",				// 21
	"Invalid\nIP Address",				// 22
	"Invalid\nSubnet Mask",				// 23
	"Invalid\nGateway Address",			// 24
	"Invalid\nServer Host",				// 25
	"Invalid\nServer Port",				// 26
	"Yes!\nAll Deleted",				// 27
	"Sorry!\nUsers Are full", 			// 28
	"Invalid\nExt. Number",				// 29
	"No Dialtone",						// 30
	"Busy",								// 31
	"No Answer",						// 32
	"Disconnected",						// 33
	"Connected",						// 34
	"Door\nUnlocked",					// 35
	"Yes!\nProcessed",					// 36
	"Sorry!\nFailed",					// 37
	"Yes!\nCompleted",					// 38
	"Invalid Card",						// 39
	"Touch\nSingle Card Only",			// 40
	"Sorry!\nNot Found",				// 41
	"Sorry!\nOverLapped",				// 42
	"Sorry!\nData Are Full",			// 43
	"Yes!\nDeleted",					// 44
	"SD Memory Card\nNot Available",	// 45
	"SD Memory Card\nNot Ready to Use",	// 46
	"\nDirectory Open Error",			// 47
	"\nDirectory Read Error",			// 48
	"\nDirectory Write Error",			// 49
	"\nFile Open Error",				// 50
	"\nFile Read Error",				// 51
	"\nFile Write Error",				// 52
	"\nFile Sync Error",				// 53
	"\nDirectory Not Found",			// 54
	"\nFile Not Found",					// 55
	"Wait a Moment\nSystem Restart",	// 56
	"You Can Now Safely Remove\nSD Memory Card",	// 57
	"SD Memory Eror\nCheck It",			// 58
	"Server Connection Failure\nTry Again Later", 	// 59
	"Already Processed\nPlease Cancel"	// 60	// "Already Processing\nFirst Cancel it"
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

char eng_menu_top_title[MAX_MSG_LEN] = "Administration";

char eng_menu_top[5][MAX_MSG_LEN] = {
	"User",
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

char eng_menu_user[6][MAX_MSG_LEN] = {
	"Enroll User",
	"Batch Enroll User",
	"Edit User",
	"Delete User",
	"List All Users",
	"Enroll User Photo"
};

char eng_menu_user_enroll[7][MAX_MSG_LEN] = {
	"Save & Exit",	
	"Access Mode",
	"FP1",
	"FP2",
	"Card",
	"PIN",
	"Security Level"
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
	"Device Setting",
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

char eng_menu_ta_sub[1][MAX_MSG_LEN] = {
	"Time"
};

char eng_menu_time_attend[4][MAX_MSG_LEN] = {
	"Key",
	"Display",
	"Items",
	"Timer"
};

char eng_menu_ta_key[2][MAX_MSG_LEN] = {
	"Up/Down Key",
	"F1-F2-F3-F4"
};

char eng_menu_ta_display[3][MAX_MSG_LEN] = {
	"Disable",
	"Text",
	"Icon"
};

char eng_menu_ta_item[3][MAX_MSG_LEN] = {
	"Enable",
	"Text",
	"Icon"
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
	"Menu Count"
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

char eng_menu_access_control[9][MAX_MSG_LEN] = {
	"Hardware Configuration",
	"All Readers",
	"Access Doors",
	"Access Points(Readers)",
	"Inputs",
	"Outputs(Relays)",
	"Access Zones",
	"Alarms",
	"Expansion Input/Outputs"
};

char eng_menu_hw_setup[6][MAX_MSG_LEN] = {
	"Units Configuration",
	"View Units",
	"View Units Input/Output",
	"View Reader IP Address",
	"In/Output Units Config",
	"View In/Output Units"
};

char eng_menu_op_mode[3][MAX_MSG_LEN] = {
	"Mode Select",
	"Access Control Mode",
	"Meals Mode"
};

char eng_menu_all_readers[6][MAX_MSG_LEN] = {
	"Setup Readers",
	"Assign Readers",
	"Tamper Alarm",
	"Setup Time & Attend",
	"Setup Meals",
	"Operation Mode"
};

char eng_menu_hw_config[19][MAX_MSG_LEN] = {
	"Unit",
	"Reader",
	"Wiegand",
	"RS-485",
	"Local",
	"Setup",
	"Door",
	"Type",
	"Point2",
	"Channel 1",
	"Channel 2 ",
	"Point",
	"Zone",	
	"FireZone",
	"AlarmAction",
	"IOUnit",
	"Usage",
	"FirePoint",
	"IP"
};

char eng_menu_unit_type[3][MAX_MSG_LEN] = {
	"K-200",
	"K-220",
	"K-300(StandAlone)"
};

char eng_menu_iounit_type[2][MAX_MSG_LEN] = {
	"K-500(Input)",
	"K-700(Output)"
};

char eng_menu_access_door[7][MAX_MSG_LEN] = {
	"Pulse Time",
	"Input/Output",
	"Time Delay",
	"Post Time Delay",
	"Alarm",
	"Lock Schedule",
	"Unlock Schedule"
};

char eng_menu_door_config[3][MAX_MSG_LEN] = {
	"Unbundle Door",
	"Normal(Unuse Exit)",
	"Exit"
};

char eng_menu_door_inout[5][MAX_MSG_LEN] = {
	"Lock",
	"Request To Exit",
	"Door Status",
	"Auxiliary Lock",
	"Lock Type"
};

char eng_menu_lock_type[2][MAX_MSG_LEN] = {
	"EM Lock",
	"Deadbolt"
};

char eng_menu_door_alarm[2][MAX_MSG_LEN] = {
	"Open Too Long",
	"Forced Open"
};

char eng_menu_door_alarm_info[2][MAX_MSG_LEN] = {
	"Enable",
	"Open Too Long Time",
};

char eng_menu_access_point[1][MAX_MSG_LEN] = {
	"Authorization Mode"
};

char eng_menu_authorization_mode[4][MAX_MSG_LEN] = {
	"Authorize",
	"Grant Active",
	"Deny All",
	"Card Grant All"
};

char eng_menu_access_zone[6][MAX_MSG_LEN] = {
	"New",
	"Point Definitions",
	"Passback Setting",
	"Zone From",
	"Zone To",
	"Outside"
};

char eng_menu_passback_setting[4][MAX_MSG_LEN] = {
	"Enable passback",
	"Repeat Access Delay",
	"Reset Passback Time",
	"Reset Passback"
};

char eng_menu_passback[6][MAX_MSG_LEN] = {
	"Delete Passback",
	"Passback Mode",
	"Passback On Exit",
	"Passback Timeout",
	"Hard Passback Schedule",
	"Soft Passback Schedule"
};

char eng_menu_passback_mode[3][MAX_MSG_LEN] = {
	"Disable",
	"Hard",
	"Soft(Alarm Only)"
};


char eng_menu_binary_input[4][MAX_MSG_LEN] = {
	"Polarity",
	"Time Delay",
	"Post Time Delay",
	"Alarm Inhibit"
};

char eng_menu_binary_output[1][MAX_MSG_LEN] = {
	"Polarity"
};

char eng_menu_io_polarity[2][MAX_MSG_LEN] = {
	"Normal(Normal Open)",
	"Reverse(Normal Close)"
};

char eng_menu_armed[4][MAX_MSG_LEN] = {
	"Setup Arm",
	"Armed Mode",
	"Armed",
	"Intrusion"
};

char eng_menu_io_input[5][MAX_MSG_LEN] = {
	"Disable",
	"Input 1(NO)",
	"Input 2(NO)",
	"Input 1(NC)",
	"Input 2(NC)"
};

char eng_menu_alarm[4][MAX_MSG_LEN] = {
	"Fire Zones",
	"Fire Points",
	"Alarm Actions",
	"Output Command"
};

char eng_menu_fire_zone[3][MAX_MSG_LEN] = {
	"Fire Points",
	"Unlock Doors",
	"Alarm Actions"
};

char eng_menu_action_index[12][MAX_MSG_LEN] = {
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

char eng_menu_output_command[3][MAX_MSG_LEN] = {
	"Repeat Count",
	"On Time",
	"Off Time"
};

char eng_menu_readers[3][MAX_MSG_LEN] = {
	"FP Reader",
	"PIN Reader",
	"Card Reader"
};

char eng_menu_card_readers[5][MAX_MSG_LEN] = {
	"Mifare Reader",
	"EM Reader",
	"Wiegand Reader",
	"Serial Reader",
	"IP Reader"
};

char eng_menu_wiegand_reader[2][MAX_MSG_LEN] = {
	"Wiegand Card Reader",
	"Wiegand PIN Reader"
};

char eng_menu_fp_reader[4][MAX_MSG_LEN] = {
	"Exposure",
	"Security Level",
	"Free Scan",
	"Fast Capture"
};

char eng_menu_mifare_reader[4][MAX_MSG_LEN] = {
	"Card Format",
	"Special Card Setting",
	"Mifare Type(A/B)",
	"Reading Card"
};

char eng_menu_mifare_format[3][MAX_MSG_LEN] = {
	"Serial Number",
	"Special Card",
	"T-money"
};

char eng_menu_mifare_type[3][MAX_MSG_LEN] = {
	"Type A",
	"Type B",
	"Type A + Type B"
};

char eng_menu_special_card[6][MAX_MSG_LEN] = {
	"Sector No.",
	"Block No.",
	"Data Offset",
	"Data Length",
	"Data Type",
	"Data Order"
};

char eng_menu_data_type[3][MAX_MSG_LEN] = {
	"Digit",
	"Alpha Numeric",
	"Binary"
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
	"Reader Network Setting",
	"IP Address",
	"Port"
};

char eng_menu_wiegand_26bit[3][MAX_MSG_LEN] = {
	"Normal",
	"Card Number Only",
	"Normal + parity"
};

char eng_menu_duress[2][MAX_MSG_LEN] = {
	"Duress Code",
	"Duress Alarm"
};

char eng_menu_setup[8][MAX_MSG_LEN] = {
	"Administrator",
	"Date & Time",
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

char eng_menu_datetime[2][MAX_MSG_LEN] = {
	"Set Date",
	"Set Time"
};

char eng_menu_monitor[6][MAX_MSG_LEN] = {
	"BackLight Time",
	"LCD Contrast",
	"LCD Brightness",
	"Black/White Mode",
	"Time Notation",
	"Date Notation"
};

char eng_menu_time_notation[2][MAX_MSG_LEN] = {
	"12 hour",
	"24 hour"
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
	"Key Lockout",
	"Tamper Enable"	
};

char eng_menu_cam[7][MAX_MSG_LEN] = {
	"Camera Mode",
	"Camera On/Off",
	"Camera Channel",
	"Camera Direction",
	"Camera View",
	"Event with Camera Image",
	"Assign Channel"
};

char eng_menu_cam_mode[3][MAX_MSG_LEN] = {
	"Diable",
	"Automatic",
	"Manual"
};

char eng_menu_cam_run[2][MAX_MSG_LEN] = {
	"Off",
	"On"
};

char eng_menu_cam_channel[4][MAX_MSG_LEN] = {
	"0 Channel",
	"1 Channel",
	"2 Channel",
	"3 Channel"
};

char eng_menu_cam_direction[2][MAX_MSG_LEN] = {
	"Portrait",
	"Landscape"
};

char eng_menu_doorphone[8][MAX_MSG_LEN] = {
	"Operation Mode",
	"Phone Number",
	"Open Number",
	"Control Volume",
	"Silence Time",
	"Tone",
	"Tone Check",
	"Hardware Setting"
};

char eng_menu_doorphone_op_mode[4][MAX_MSG_LEN] = {
	"Disable",
	"Interphone",
	"Telephone",
	"IP phone"
};

char eng_menu_phone_number[3][MAX_MSG_LEN] = {
	"Dial Mode",
	"Speed Dial Number",
	"IP Phone Number" 
};

char eng_menu_dialing_mode[3][MAX_MSG_LEN] = {
	"Standard",
	"Speed Dial",
	"Single Number" 
};

char eng_menu_ip_phone_number[5][MAX_MSG_LEN] = {
	"Enroll Number",
	"Edit Number",
	"Delete Number",
	"Delete All Number",
	"List Number"
};

char eng_menu_ip_phone_item[1][MAX_MSG_LEN] = {
	"IP Address"
};

char eng_menu_doorphone_volume[3][MAX_MSG_LEN] = {
	"Speaker Volume",
	"Line Volume",
	"Mic Volume"
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
	"Background Noise"
};

char eng_menu_doorphone_bgn[4][MAX_MSG_LEN] = {
	"120 mV",
	"180 mV",
	"240 mV",
	"300 mV"
};

char eng_menu_network[6][MAX_MSG_LEN] = {
	"Device ID",
	"MAC Address",
	"Network Mode",
	"IP Address",
	"Server Setting",
	"Ping test"	
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
	"Dynamic Address",
	"Local IP Address",
	"Subnet Mask",
	"Gateway",
	"Server Host",
	"Server Port",
	"IP Address"
};

char eng_menu_network_mode[3][MAX_MSG_LEN] = {
	"Serial",
	"Dial-UP",
	"TCP/IP"
};

char eng_menu_system[7][MAX_MSG_LEN] = {
	"View All Settings",
	"Initialize",
	"Import from SD memory",
	"Export to SD memory",
	"Safely Remove SD Memory",
	"Communication Statistic",
	"Re-initialize Device"
};

char eng_menu_init_device[3][MAX_MSG_LEN] = {
	"Initialize Event",
	"Initialize Setting",
	"Factory Default"
};

char eng_menu_imexport[6][MAX_MSG_LEN] = {
	"Firmware",
	"Resources(Icons,Voices)",
	"Customer Value",
	"User Files",
	"Setting Files",
	"K-300 Firmware"
};

char eng_menu_comm_stats[2][MAX_MSG_LEN] = {
	"View Statistics",
	"Init Statistics"
};

char eng_menu_downfirmware[5][MAX_MSG_LEN] = {
	"Load Firmware",
	"Start Download",
	"Retry Incompleted Only",
	"Cancel Download",
	"View Download Status"
};

char eng_menu_inuse[2][MAX_MSG_LEN] = {
	"Unuse",
	"Inuse"
};

char eng_menu_yesno[2][MAX_MSG_LEN] = {
	"Yes",
	"No"
};

char eng_menu_noyes[2][MAX_MSG_LEN] = {
	"No",
	"Yes"
};

char eng_menu_okcancel[2][MAX_MSG_LEN] = {
	"OK",
	"Cancel"
};

char eng_menu_enable[MAX_MSG_LEN] = "Enable";

char eng_menu_other[24][MAX_MSG_LEN] = {
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
	"Event Count"
};

char _eng_day_of_weeks[7][4] = {
	"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"
};

char _eng_months[12][4] = {
	"JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
};
