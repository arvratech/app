#ifndef _CF_H_
#define _CF_H_


#define MAX_CF_SZ		7

#define CF_SC_READER_UID		0
#define CF_SC_READER_BLOCK		1
#define CF_SC_READER_FILE		2
#define CF_EM_READER			3
#define CF_WIEGAND_READER		4
#define CF_SERIAL_READER		5
#define CF_IP_READER			6

// AuthenticationFactor formatType definition
#define FORMAT_TYPE_BINARY		0
#define FORMAT_TYPE_BCD			1
#define FORMAT_TYPE_ASCII		2

// encoding definition
#define CF_ENCODING_BINARY		0
#define CF_ENCODING_BCD			1
#define CF_ENCODING_ASCII		2


// Card Format structure
typedef struct _CARD_FORMAT {
	unsigned char	formatType;
	unsigned char	reader;
	unsigned char	scReaderMode;
	unsigned char	startPosition;
	unsigned char	length;
	unsigned char	encoding;
	unsigned char	reverseBits;
	unsigned char	reserve[1];
} CARD_FORMAT;


void cfSetDefault(CARD_FORMAT *cf, int index);
int  cfEncode(CARD_FORMAT *cf, void *buf);
int  cfDecode(CARD_FORMAT *cf, void *buf);
int  cfValidate(CARD_FORMAT *cf);
int  cfGetFormatType(CARD_FORMAT *cf);
void cfSetFormatType(CARD_FORMAT *cf, int formatType);
int  cfGetStartPosition(CARD_FORMAT *cf);
void cfSetStartPosition(CARD_FORMAT *cf, int startPosition);
int  cfGetLength(CARD_FORMAT *cf);
void cfSetLength(CARD_FORMAT *cf, int length);
int  cfGetEncoding(CARD_FORMAT *cf);
void cfSetEncoding(CARD_FORMAT *cf, int encoding);


#endif
