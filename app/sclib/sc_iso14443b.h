#ifndef _SC_ISO_14443_B_H
#define _SC_ISO_14443_B_H


#define ISO14443B_PUPI_LENGTH       4
#define ISO14443B_APPDATA_LENGTH    4
#define ISO14443B_PROTINFO_LENGTH   3


// PCD command set.
typedef enum {
    ISO14443B_CMD_REQB = 0x00,      /*!< command REQB */
    ISO14443B_CMD_WUPB = 0x08,      /*!< command WUPB */
    ISO14443B_CMD_HLTB = 0x50,      /*!< command HLTB */
    ISO14443B_CMD_ATTRIB = 0x1D,    /*!< command ATTRIB */
} iso14443BCommand_t;

// slot count (N parameter) used for iso14443b anti collision
typedef enum {
    ISO14443B_SLOT_COUNT_1 = 0,
    ISO14443B_SLOT_COUNT_2 = 1,
    ISO14443B_SLOT_COUNT_4 = 2,
    ISO14443B_SLOT_COUNT_8 = 3,
    ISO14443B_SLOT_COUNT_16 = 4,
} iso14443BSlotCount_t;


void iso14443BOpen(void);
void iso14443BClose(void);
int  iso14443BSendRequest(iso14443BCommand_t cmd, int AFI);
int  iso14443BReceiveRequest(unsigned char *pupi);
int  iso14443BAntiCollsion(iso14443BCommand_t cmd, iso14443BSlotCount_t slotCount, int afi, unsigned char *pupi);
int  iso14443BSlotMarker(int slotNumber, unsigned char *pupi);
int  iso14443BHalt(unsigned char *pupi);
int  iso14443BEnterProtocolMode(unsigned char *pupi);


#endif

