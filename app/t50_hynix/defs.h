#ifndef _DEFS_H_
#define _DEFS_H_


#define STORED_TMPL_SZ			512
#define STORED_TMPL_DOUBLE_SZ	1024
#define STORED_TMPL_SHIFT		9

#ifdef _USER_ID_EX
#define	MAX_USERID_SZ			10
#define MAX_USERID_HALF_SZ		5
#else
#define	MAX_USERID_SZ			8
#define MAX_USERID_HALF_SZ		4
#endif
#define	MAX_PHONENO_SZ			8
#define MAX_PHONENO_HALF_SZ		4

#define MIN_PASSWORD_SIZE		4
#define MAX_PASSWORD_SIZE		12
#define RF_INPUT_TIMEOUT		8000L
#define KEY_BEEP_DELAY			30
#define MENT_BEEP_DELAY			200
#define RF_BEEP_DELAY			80

#define MAX_SVRBUF_SZ			2096	// buffer size
#define MAX_SVRBUF_QTR_SZ		516		// buffer quarter size

// Request definitions
#define G_CARD_OPEN_REQUESTED	0x01
#define G_CARD_CLOSE_REQUESTED	0x02
#define G_CARD_REQUESTED		0x03
#define G_SERIAL_SPEED_CHANGED	0x04
#define G_TCP_IP_CHANGED		0x08
#define G_NETWORK_OPEN_DELAY	0x10
#define G_DATE_TIME_UPDATED		0x20
#define G_TAMPER_CFG_CHANGED	0x40

// User Access Mode definitions
#define	USER_FP					0x01	// 0-bit
#define	USER_CARD				0x02	// 1-bit
#define	USER_PIN				0x04	// 2-bit
#define	USER_CARD_FP			0x08	// 3-bit
#define	USER_CARD_PIN			0x10	// 4-bit
#define	USER_CARD_FP_PIN		0x20	// 5-bit
#define	USER_FP_PIN				0x40	// 6-bit USER_PIN_FP in original BK5000 

// Device Option definitions
#define CR_TAMPER				0x80
#define BATTERY					0x40
#define BATTERY_LOCK			0x20
#define ADMPWD_DIGITS_ONLY		0x10
#define INTRUSION_ALARM			0x08
#define IP_PHONE				0x04

// Terminal Option definitions
#define	FP_IDENTIFY				0x02	// 1-bit
#define	FAST_CAPTURE			0x04	// 2-bit
//#define TAMPER				0x10	// 4-bit

// Media Option definitions
#define	MEDIA_KEYPAD_TONE		0x80
#define	MEDIA_TOUCH_SOUNDS		0x40

// dateNotion
#define DISPLAY_DATETIME		0x80
#define TIME_NOTATION			0x40

// Terminal Access Module definitions
#define	TERM_FP_SENSOR			0x01	// 0-bit
#define	TERM_CARD_READER		0x02	// 1-bit
#define	TERM_PIN_KEYPAD			0x04	// 2-bit

// Event Option definitions
#define	MASK_LOCK_EVENT			0x01	// 0-bit
#define	MASK_ACCESS_EVENT		0x02	// 1-bit
#define	MASK_OTHER_EVENT		0x04	// 2-bit
#define	BACKUP_EVENT			0x10	// 4-bit

// TCPIPOption definitions
#define USE_DHCP				0x01
#define USE_WIFI				0x02

// Network Type definitions
#define NETWORK_SERIAL			1
#define NETWORK_DIALUP			2
#define NETWORK_TCPIP			3

// Terminal Status definitions
#define	LOCK_STATE				0x07	// 2-0bit
#define	DOOR_CONTACT_STATUS		0x08	// 3-bit
#define REQUEST_TO_EXIT_STATUS	0x10	// 4-bit
#define TAMPER_STATUS			0x20	// 5-bit

// Capture Purpose
#define CAPTURE_ENROLL			1
#define CAPTURE_VERIFY			2
#define CAPTURE_IDENTIFY		3

#define FP_CAPTURE_X			0
#define FP_CAPTURE_Y			43

// Lock State definitions
#define S_NORMAL_LOCKED			0	
#define S_NORMAL_UNLOCKED		1	
#define S_FORCED_LOCKED			2	
#define S_FORCED_UNLOCKED		3	
#define S_SCHEDULED_LOCKED		4	
#define S_SCHEDULED_UNLOCKED	5	
#define S_LOCK_DOWNED			6	
#define S_LOCK_OPENED			7	

// Network Status definitions
#define S_NULL					0
#define S_IDLE					1
#define S_WAIT_DIAL				2
#define S_DIALING				3
#define S_CONNECTED				4
#define S_DISCONNECTING			5

// USB State definitions
#define S_USB_IDLE				0
#define S_USB_CONNECTED			1
#define S_USB_CONNECTED_TO_HOST	2
#define S_USB_ADMIN_LOCK		3
#define S_USB_ADMIN_AUTH		4
#define S_USB_ADMIN				5

// Event definitions
// Lock Event
#define E_NORMAL_UNLOCKED_ACCESS_GRANTED		100
#define E_NORMAL_UNLOCKED_REQUEST_TO_EXIT		101
#define E_NORMAL_UNLOCKED_MOMEMTARY_UNLOCK		102
#define E_NORMAL_LOCKED_UNLOCK_TIMEOUT			103
#define E_NORMAL_LOCKED_CLEAR					104
#define E_NORMAL_LOCKED_LOCK_DOWN_RELEASE		105
#define E_NORMAL_LOCKED_LOCK_OPEN_RELEASE		106
#define E_FORCED_LOCKED							107
#define E_FORCED_LOCKED_LOCK_DOWN_RELEASE		108
#define E_FORCED_LOCKED_LOCK_OPEN_RELEASE		109
#define E_NORMAL_LOCKED_FORCED_LOCK_RELEASE		110
#define E_FORCED_UNLOCKED						111
#define E_FORCED_UNLOCKED_LOCK_DOWN_RELEASE		112
#define E_FORCED_UNLOCKED_LOCK_OPEN_RELEASE		113
#define E_NORMAL_LOCKED_FORCED_UNLOCK_RELEASE	114
#define E_SCHEDULED_LOCKED						115
#define E_SCHEDULED_LOCKED_LOCK_DOWN_RELEASE	116
#define E_SCHEDULED_LOCKED_LOCK_OPEN_RELEASE	117
#define E_NORMAL_LOCKED_SCHEDULED_LOCK_END		118
#define E_SCHEDULED_UNLOCKED					119
#define E_SCHEDULED_UNLOCKED_LOCK_DOWN_RELEASE	120
#define E_SCHEDULED_UNLOCKED_LOCK_OPEN_RELEASE	121
#define E_NORMAL_LOCKED_SCHEDULED_UNLOCK_END	122
#define E_LOCK_DOWNED							123
#define E_LOCK_OPENED							124

// Access Event
#define E_ACCESS_GRANTED_FP						130
#define E_ACCESS_GRANTED_FP_PIN					130		// not exist GRANTED_FP_PIN 
#define E_ACCESS_GRANTED_CARD					131
#define E_ACCESS_GRANTED_PIN					132
#define E_ACCESS_GRANTED_CARD_FP				133
#define E_ACCESS_GRANTED_CARD_PIN				134
#define E_ACCESS_DENIED_USER_NOT_ACTIVATED		135
#define E_ACCESS_DENIED_USER_EXPIRED			136
#define E_ACCESS_DENIED_ACCESS_CONTROL			137
#define E_ACCESS_DENIED_ACCESS_RIGHTS			138
#define E_ACCESS_DENIED_BAD_FP					139
#define E_ACCESS_DENIED_BAD_CARD				140
#define E_ACCESS_DENIED_BAD_PIN					141
#define E_ACCESS_DENIED_GOOD_CARD_BAD_FP		142
#define E_ACCESS_DENIED_GOOD_CARD_BAD_PIN		143
#define E_ACCESS_DENIED_UNREGISTERED_FP			144
#define E_ACCESS_DENIED_UNREGISTERED_CARD		145
#define E_ACCESS_DENIED_LOST_CARD				146
#define E_ACCESS_DENIED_ACCESS_MODE				147
#define E_ACCESS_DENIED_PASSBACK				148			// Appended 2009.8.28
#define E_ACCESS_DURESS							149
#define E_ACCESS_GRANTED_CARD_FP_PIN			150			// Card+FP+PIN
#define E_ACCESS_DENIED_GOOD_CARD_BAD_FP_PIN	151			// Card+FP+PIN
#define E_ACCESS_DENIED_GOOD_CARD_FP_BAD_PIN	152			// Card+FP+PIN
#define E_ACCESS_DENIED_GOOD_CARD_BAD_FP_BAD_PIN 153		// Card+FP or Card+PIN
#define E_ACCESS_DENIED_ALL						154
#define E_ACCESS_GRANTED_ALL					155
#define E_DENIED_MEAL_OVERLAP					156			// Appended 2010.3.4
#define E_DENIED_NEGATIVE_BALANCE				157			// Appended 2012.7.13
#define E_DENIED_CARD_RW_ERROR					158			// Appended 2012.7.13
#define E_DENIED_FAIL_SERVER					159
#define E_PASSBACK_DETECTED						160
#define E_ACCESS_GRANTED_EMV_CARD				171
#define E_ACCESS_DENIED_UNREGISTERED_EMV_CARD	175
#define E_ACCESS_GRANTED_ALL_EMV				177

// Other Event
#define E_DEVICE_RESET							200
#define E_DEVICE_TAMPER_NORMAL					201
#define E_CALL_BUTTON_PRESSED					202
#define E_DEVICE_TAMPER_ALARM					203
#define E_DOOR_NORMAL							204
#define E_DOOR_OPEN_TOO_LONG					205
#define E_DOOR_FORCED_OPEN						206
#define E_FIRE_ALARM							208
#define E_FIRE_NORMAL							209
#define E_MAKE_CALL								210
#define E_INPUT_ALARM							211
#define E_INPUT_NORMAL							212
#define E_READER_TAMPER_ALARM					213
#define E_READER_TAMPER_NORMAL					214
#define E_INTRUSION_ALARM						215
#define E_INTRUSION_NORMAL						216
#define E_UNIT_DISCONNECTED						219
#define E_UNIT_CONNECTED						220
#define E_BEACON_DETECTED						221
#define E_BEACON_DETECTED2						222
#define E_AC_POWER_NORMAL						223
#define E_AC_POWER_ALARM						224
#define E_POWER_FAIL							225
#define E_POE_PRESENT                           226
#define E_POE_NOT_PRESENT                       227
#define E_BATTERY_POWER                         228

// Result definitions
#define	R_SUCCESS					0x00
#define R_PACKET_FORMAT_ERROR		0x01
#define R_PACKET_ADDRESS_ERROR		0x02
#define R_PACKET_CHECKSUM_ERROR		0x03
#define R_UNKNOWN_COMMAND			0x04	
#define R_PARAMETER_ERROR			0x05
#define R_R_SYSTEM_ERROR			0x06
#define R_OBJECT_NOT_FOUND			0x07
#define R_OBJECT_IS_FULL			0x08
#define R_ACTION_NOT_ALLOWED		0x09
#define R_XFER_PROTOCOL_ERROR		0x0c
#define R_XFER_ACCESS_VIOLATION		0x0d
#define R_XFER_FILE_NOT_FOUND		0x0e
#define R_XFER_SYSTEM_ERROR			0x0f

// Network cause efinitions
#define R_NETWORK_CLEAR				0
#define R_LINK_DOWN					1
#define R_NO_DHCP_SERVER			2
#define R_NO_HOST_SERVER			3
#define R_AUTH_SYSTEM_ERROR			4
#define R_AUTH_PACKET_FORMAT_ERROR	5
#define R_AUTH_DISCONNECTED			6
#define R_AUTH_TIMEOUT				7
#define R_AUTH_PROTOCOL_ERROR		8
#define R_NOT_ACTIVE_PERIOD			9
#define R_AUTH_DEVICE_NOT_FOUND		10
#define R_AUTH_DEVICE_BUSY			11
#define R_AUTH_DEVICE_FULL			12
#define R_AUTH_CERT_FAILED			13


// ID definitions
#define ID_UNKNOWN					999
#define TAG_BACK_BTN				1
#define TAG_CONFIG_BTN				2
#define TAG_KEYPAD_BTN				3
#define TAG_PHONE_BTN				4
#define TAG_PHONE_HANGUP_BTN		5
#define TAG_FUNC_LBL				6				
#define TAG_FUNC_BTN				7				
#define TAG_OK_BTN					11
#define TAG_CANCEL_BTN				12
#define TAG_ADD_BTN					13
#define TAG_EDIT_BTN				14
#define TAG_DELETE_BTN				15
#define TAG_DIALOG_BTN				16
#define TAG_VIEW_KEYPAD				50
#define TAG_KEYPAD_BASE_BTN			51
#define TAG_KEYPAD_FUNC_BTN			80
#define TAG_TEXT_INPUT				90
#define TAG_PICKERVIEW				91

#define TAG_RESULT_TIME				101
#define TAG_RESULT_PHOTO			102
#define TAG_RESULT_USERNAME			103	
#define TAG_RESULT_POSICOMP			104
#define TAG_RESULT_DEPARTMENT		105
#define TAG_RESULT_MSG				106

#define TAG_LOCK_ICON				110
#define TAG_DOOR_ICON				111
#define TAG_SVRNET_ICON				112
#define TAG_SLVNET_TEXT				113
#define TAG_WIFI_ICON				114
#define TAG_CAM_ICON				115
#define TAG_BLE_ICON				116
#define TAG_CARD_CAP_ICON			117
#define TAG_POWER_TEXT				118
#define TAG_KEYPAD_TEXT				119
#define TAG_PHONE_DIAL				120
#define TAG_PHONE_NO_NAME			121
#define TAG_PHONE_NO_URL			122
#define TAG_PHONE_CALL_STATE		123

#define TAG_CLOCK_VIEW				120
#define TAG_CLOCK_BASE				121

#define TAG_ADM_LABEL				160
#define TAG_ADM_LABEL2				161
#define TAG_ADM_IMAGEVIEW1			162
#define TAG_ADM_IMAGEVIEW2			163


#endif

