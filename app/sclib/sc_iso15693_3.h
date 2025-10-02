#ifndef SC_ISO_15693_3_H
#define SC_ISO_15693_3_H


#define ISO15693_UID_LENGTH					8
#define ISO15693_MAX_MEMORY_BLOCK_SIZE		32

#define ISO15693_CMD_INVENTORY				0x01
#define ISO15693_CMD_STAY_QUIET				0x02
#define ISO15693_CMD_READ_SINGLE_BLOCK		0x20
#define ISO15693_CMD_WRITE_SINGLE_BLOCK		0x21
#define ISO15693_CMD_READ_MULTIPLE_BLOCKS	0x23
#define ISO15693_CMD_SELECT					0x25
#define ISO15693_CMD_RESET_TO_READY			0x26
#define ISO15693_CMD_GET_SYSTEM_INFORMATION	0x2b

#define ISO15693_REQ_FLAG_TWO_SUBCARRIER	0x01
#define ISO15693_REQ_FLAG_HIGH_DATA_RATE	0x02
#define ISO15693_REQ_FLAG_INVENTORY			0x04
#define ISO15693_REQ_FLAG_PROT_EXTENSION	0x08
// with INVENTORY flag set we have these bits :
#define ISO15693_REQ_FLAG_SELECT			0x10
#define ISO15693_REQ_FLAG_ADDRESS			0x20
// with INVENTORY flag not set we have these bits :
#define ISO15693_REQ_FLAG_AFI				0x10
#define ISO15693_REQ_FLAG_1_SLOT			0x20
// OPTION flag is the same for both:
#define ISO15693_REQ_FLAG_OPTION			0x40

#define ISO15693_RESP_FLAG_ERROR			0x01
#define ISO15693_RESP_FLAG_PROT_EXTENSION	0x08


// structure representing an ISO15693 PICC
typedef struct {
	unsigned char	flags;			// flag byte of response
	unsigned char	dsfid;			// Data Storage Format Identifier
	unsigned char	uid[ISO15693_UID_LENGTH];	// UID of the PICC
    unsigned char   reserve[2];
} iso15693Card_t;

// tructure holding the information returned by #iso15693GetPiccSystemInformation
typedef struct {
	unsigned char	flags;			// flag byte of response
	unsigned char	infoFlags;		// info flags
	unsigned char	uid[ISO15693_UID_LENGTH];	// UID of the PICC
	unsigned char	dsfid;			// Data Storage Format Identifier
	unsigned char	afi;			// Application Family Identifier
	unsigned char	memNumBlocks;	// number of blocks available
    unsigned char	memBlockSize;	// number of bytes per block
    unsigned char	icReference;	// IC reference field
} iso15693PiccSystemInformation_t;

// structure representing a memory block
typedef struct {
	unsigned char	flags;
	unsigned char	errorCode;
	unsigned char	securityStatus;	// security status byte
	unsigned char	reserve;
	unsigned char	data[ISO15693_MAX_MEMORY_BLOCK_SIZE];	// the content
} iso15693PiccBlock_t;

// structure representing a memory block
typedef struct {
	unsigned char	flags;
	unsigned char	errorCode;
	unsigned char	securityStatus;	// security status byte
	unsigned char	reserve;
	unsigned char	data[256];		// the content
} iso15693PiccMultipleBlock_t;


// enum holding possible slot count values used by inventory command.
typedef enum {
	ISO15693_NUM_SLOTS_1,	// 1 slot
	ISO15693_NUM_SLOTS_16	// 16 slots
} iso15693NumSlots_t;


int  iso15693Open(iso15693PhyConfig_t *config);
void iso15693Close(void);
int  iso15693InventoryPresence(iso15693Card_t *card);
int  iso15693InventoryOneSlot(iso15693Card_t *cards, int *cardCount, int maxCards);
int  iso15693Inventory16eSlot(iso15693Card_t *cards, int *cardCount, int maxCards);
int  iso15693StayQuiet(unsigned char *uid);
int  iso15693ResetToReady(unsigned char *uid);
int  iso15693SelectPicc(iso15693Card_t *card);
int  iso15693GetPiccSystemInformation(iso15693Card_t *card, iso15693PiccSystemInformation_t *sysInfo);
int  iso15693ReadSingleBlock(iso15693Card_t *card, int blockNo, int blockSize, iso15693PiccBlock_t *block);
int  iso15693ReadMultipleBlocks(iso15693Card_t *card, int blockNo, int blockSize, int blockCount, iso15693PiccMultipleBlock_t *block);
int  iso15693WriteSingleBlock(iso15693Card_t *card, int flags, int blockNo, int blockSize, iso15693PiccBlock_t *block);


#endif

