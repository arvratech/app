#ifndef _PKT_DEFS_H_
#define _PKT_DFFS_H_


#define MAX_BUF_SZ			3480
#define CAM_IMAGE_SZ		115200

// Ascii char define
#define ASCII_STX		0x02
#define ASCII_ETX		0x03
#define ASCII_EOT		0x04
#define ASCII_ENQ		0x05
#define ASCII_ACK		0x06
#define ASCII_BEL       0x07
#define ASCII_BS        0x08
#define ASCII_LF        0x0A
#define ASCII_CR        0x0D
#define ASCII_XON       0x11
#define ASCII_XOFF      0x13
#define ASCII_NAK       0x15
#define ASCII_CAN       0x18
#define ASCII_ESC		0x1b

// Packet Type definitions
#define P_READ_OBJECT			0x60
#define P_READ_PROPERTY			0x61
#define P_READ_STATUS			0x62
#define P_WRITE_OBJECT			0x63
#define P_WRITE_PROPERTY		0x64
#define P_DELETE_OBJECT			0x65
#define P_READ_OBJECT_LIST		0x66
#define P_READ_PROPERTY_LIST	0x67
#define P_READ_STATUS_LIST		0x68
#define P_WRITE_OBJECT_LIST		0x69
#define P_WRITE_PROPERTY_LIST	0x6a
#define P_EVENT_NOTIFICATION	0x6b
#define P_STATUS_NOTIFICATION	0x6c
#define P_READ_FILE				0x6d
#define P_WRITE_FILE			0x6e

#define P_EXPORT_OBJECT			0x66
#define P_EXPORT_PROPERTY		0x67
#define P_IMPORT_OBJECT			0x68
#define P_IMPORT_PROPERTY		0x69

#define P_LOGIN					0xff
#define P_PING					0xfe
#define P_RELOAD_SERVER			0xfd
#define P_QUERY_DEVICE			0xfc
#define P_XFER_GET				0x26
#define P_XFER_PUT				0x27
#define P_XFER_EVENT			0x28
#define P_XFER_RRQ				0x21
#define P_XFER_WRQ				0x22
#define P_XFER_DATA				0x23
#define P_XFER_ACK				0x24
#define P_XFER_ERROR			0x25

#define P_SUBSCRIBE_STATUS		0x47
#define P_NOTIFY_STATUS			0x48
#define P_UNSUBSCRIBE_STATUS	0x49
#define P_CONNECT_DIALUP		0x42
#define P_DISCONNECT_DIALUP		0x43
#define P_EVENT_OLD				0x50
#define P_EVENT					0x51
#define P_EVENT_EX				0x52
#define P_EVENT2				0x53
#define P_GET_EVENT				0x55
#define P_READ_EVENT			0x56
#define P_SUBSCRIBE_EVENT		0x57
#define P_NOTIFY_EVENT			0x58
#define P_UNSUBSCRIBE_EVENT		0x59
#define P_SUBSCRIBE_CAM			0x5a
#define P_NOTIFY_CAM			0x5b
#define P_UNSUBSCRIBE_CAM		0x5c


#define P_CAM_CONTROL			0x70
#define P_CAM_IDLE				0x71
#define P_CAM_IMAGE_OLD			0x72
#define P_GET_CAM_IMAGE			0x73
#define P_CAM_IMAGE				0x74
#define P_CAM_IMAGE_NEW			0x78
#define P_EVENT_CAM_IMAGE		0x75
#define P_EVENT_CAM_IMAGE_NEW	0x76
#define P_RESET_PASSBACK		0x77
#define P_ISSUE_CERTIFICATE		0x80
#define P_AUTH_RESULT			0x81
#define P_REINITIALIZE_DEVICE	0x82
#define P_REINITIALIZE_CREDENTIAL_READER	0x83

// Appended Packet Type definitions
#define P_GET_OBJECT			0x35
#define P_GET_PROPERTY			0x36
#define P_FIND_OBJECT			0x37
#define P_FIND_PROPERTY			0x38
#define P_DELETE_PROPERTY		0x6a
#define P_CR_READ_EVENT			0x39


#endif	/* _PKT_DEFS_H_ */