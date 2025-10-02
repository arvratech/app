#ifndef _SC_INFO_H_
#define _SC_INFO_H_


// Smart Card Info structure
typedef struct _SMART_CARD_INFO {
	unsigned char	mifareKey[6];		// Mifare key
	unsigned char	mifareBlockNo;		// Mifare Sector# x 4 + Block#
	unsigned char	mifareBlockCount;	// Mifare block count
	unsigned char	aidLength;			// ISO7816-4 AID length
	unsigned char	aid[10];			// ISO7816-4 AID
	unsigned char	apduMode;			// reserve
	unsigned char	apdu[5];			// ISO7816-4 APDU: INS, P1, P2, Lc, Le
	unsigned char	startPosition;		// Start position of ISO7816-4 file
	unsigned char	dataLength;			// Data length of ISO7816-4 file
	unsigned char	reserve[1];	
} SMART_CARD_INFO;


void scInfoSetDefault(SMART_CARD_INFO *scInfo);
int  scInfoEncode(SMART_CARD_INFO *scInfo, void *buf);
int  scInfoDecode(SMART_CARD_INFO *scInfo, void *buf);
int  scInfoValidate(SMART_CARD_INFO *scInfo);


#endif
