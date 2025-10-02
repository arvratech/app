#ifndef _OBJ_DEFS_H_
#define _OBJ_DFFS_H_


// Object Type definitions
#define OT_SERVER					0x10
#define OT_DEVICE					0x11
#define OT_UNIT_SETTING				0x12
#define OT_NETWORK_PORT				0x13
#define OT_AC_UNIT					0x1a
#define OT_IO_UNIT					0x1b
#define OT_ACCESS_DOOR				0x30
#define OT_ACCESS_POINT				0x31
#define OT_ACCESS_ZONE				0x32
#define OT_BINARY_INPUT				0x14
#define OT_BINARY_OUTPUT			0x15
#define OT_CREDENTIAL_READER		0x33
#define OT_FIRE_ZONE				0x1c
#define OT_FIRE_POINT				0x1d
#define OT_ALARM_ACTION				0x17
#define OT_OUTPUT_COMMAND			0x18
#define OT_CALENDAR					0x20
#define OT_SCHEDULE					0x21
#define OT_USER						0x34
#define OT_ACCESS_RIGHTS			0x36
#define OT_PHONE_NO					0x41
#define OT_DEVICE_FILE				0x40

// Device Object Property definitions
#define OP_LOCAL_DATE_TIME			0x30
#define OP_DEVICE_PASSWORD			0x31
#define OP_MAKER					0x32
#define OP_MODEL					0x33
#define OP_TA_TEXT					0x34
#define OP_PASSBACK					0x37
#define OP_UNIT_FIRMWARE_STATUS		0x38
#define OP_CAMERA					0x41
#define OP_CAM_IMAGE				0x42

// AccessDoor Object Property definitions
#define OP_PRESENT_VALUE			0x30
#define OP_SILENCED					0x31

// CredentialReader Object Property definitions
#define OP_FP_READER				0x30
#define OP_PIN_READER				0x31
#define OP_MIFARE_READER			0x32
#define OP_EM_READER				0x33
#define OP_WIEGAND_READER			0x34
#define OP_SERIAL_READER			0x35
#define OP_IP_READER				0x36
#define OP_SMART_CARD_KEY			0x37
#define OP_FP_IMAGE					0x38
#define OP_TA_TEXTS					0x39
#define OP_IP_ADDRESS				0x3a
#define OP_SEND_CAM_IMAGE			0x3b
#define OP_INTRUSION_PRESENT_VALUE	0x40
#define OP_BLE_CONF					0x43
#define OP_NETWORK_PORT				0x44
#define OP_DEVICE					0x45

// User Object Property definitions
#define OP_FP_AUTHENTICATION_FACTOR				0x30
#define OP_SECOND_CARD_AUTHENTICATION_FACTOR	0x31
#define OP_USER_EX					0x32
#define OP_USER_PHOTO				0x33
#define OP_USER_ACCESS_RIGHTS		0x34
#define OP_RESET_PASSBACK			0x35

// DeviceFile Object Property definitions
#define OP_FILE_BODY				0x30


#endif	/* _OBJ_DEFS_H_ */
