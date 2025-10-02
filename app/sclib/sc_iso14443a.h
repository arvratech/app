#ifndef _SC_ISO_14443_A_H
#define _SC_ISO_14443_A_H


#define ISO14443A_MAX_UID_LENGTH        10
#define ISO14443A_MAX_CASCADE_LEVELS    3
#define ISO14443A_CASCADE_LENGTH        7
#define ISO14443A_RESPONSE_CT           0x88

// ISO 14443-4
#define COMPLIANT_WITH_ISO14443_4       0x20


// PCD command set
typedef enum {
    ISO14443A_CMD_REQA = 0x26,          /*!< command REQA */
    ISO14443A_CMD_WUPA = 0x52,          /*!< command WUPA */
    ISO14443A_CMD_SELECT_CL1 = 0x93,    /*!< command SELECT cascade level 1 */
    ISO14443A_CMD_SELECT_CL2 = 0x95,    /*!< command SELECT cascade level 2 */
    ISO14443A_CMD_SELECT_CL3 = 0x97,    /*!< command SELECT cascade level 3 */
    ISO14443A_CMD_HLTA = 0x50,          /*!< command HLTA */
    ISO14443A_CMD_PPSS = 0xd0,          /*!< command PPSS */
    ISO14443A_CMD_RATS = 0xe0,          /*!< command RATS */
} iso14443ACommand_t;


void iso14443AOpen(void);
void iso14443AClose(void);
void iso14443ASetBitRate(int bitRate);
void iso14443ASendRequest(iso14443ACommand_t cmd);
int  iso14443AReceiveRequest(unsigned char *atqa);
int  iso14443AActivate(unsigned char *uid, unsigned char *sak);
int  iso14443ASelect(unsigned char *uid, unsigned char *sak);
int  iso14443AHalt(void);
int  iso14443AActivateCard(unsigned char *uid, unsigned char *sak);
int  iso14443AToIdle(void);


#endif

