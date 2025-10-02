#ifndef _PKT_DEFS_H_
#define _PKT_DFFS_H_


#define MAX_BUF_SZ			3840
#define CAM_IMAGE_SZ		115200

// Packet Type definitions
#define P_LOGIN					0xff
#define P_PING					0xfe
#define P_AUTH					0xf0
#define P_STORE					0x10
#define P_GET					0x11
#define P_GET_EX				0x1a
#define P_SET					0x12
#define P_DELETE				0x13
#define P_EXPORT				0x14
#define P_IMPORT				0x15
#define P_XFER_GET				0x26
#define P_XFER_PUT				0x27
#define P_XFER_EVENT			0x28
#define P_XFER_RRQ				0x21
#define P_XFER_WRQ				0x22
#define P_XFER_DATA				0x23
#define P_XFER_ACK				0x24
#define P_XFER_ERROR			0x25
#define P_ACCESS_CONTROL		0x30
#define P_OUTPUT_CONTROL		0x31
#define P_GET_STATUS			0x40
#define P_CONNECT_DIALUP		0x42
#define P_DISCONNECT_DIALUP		0x43
#define P_EVENT_OLD				0x50
#define P_EVENT					0x51
#define P_EVENT_EX				0x52
#define P_GET_EVENT				0x55
#define P_VERIFY_MATCH			0x60
#define P_IDENTIFY_MATCH		0x61

#define P_CAM_CONTROL			0x70
#define P_CAM_IDLE				0x71
#define P_CAM_IMAGE_OLD			0x72
#define P_GET_CAM_IMAGE			0x73
#define P_CAM_IMAGE				0x74
#define P_CAM_IMAGE_NEW			0x78
#define P_EVENT_CAM_IMAGE		0x75
#define P_EVENT_CAM_IMAGE_NEW	0x76
#define P_RESET_PASSBACK		0x77
#define P_ENROLL_USER			0x78
#define P_AUTHORIZE_USER		0x78
#define P_AUTH_RESULT			0x81

// Authentication packet type definitions
#define PIN0					0x13861686
#define AUTH_RAND1				0x5c
#define AUTH_RAND2				0x5d
#define AUTH_AUTH1				0xa1
#define AUTH_AUTH2				0xa2
#define AUTH_AUTH3				0xa3
#define AUTH_AUTH_REQ			0x97
#define AUTH_AUTH_RSP			0x48
#define AUTH_PING				0x58
#define AUTH_ISSUE_CERT			0x49


#endif
