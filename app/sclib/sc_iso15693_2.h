#ifndef SC_ISO_15693_2_H
#define SC_ISO_15693_2_H


// enum holding possible VICC datarates
typedef enum {
	ISO15693_VCD_CODING_1_4,
	ISO15693_VCD_CODING_1_256
} iso15693VcdCoding_t;

// enum holding VICC modulation indeces
typedef enum {
	ISO15693_MODULATION_INDEX_10,
	ISO15693_MODULATION_INDEX_14,
	ISO15693_MODULATION_INDEX_20,
	ISO15693_MODULATION_INDEX_30,
	ISO15693_MODULATION_INDEX_OOK
} iso15693ModulationIndex_t;

// Configuration parameter used by #iso15693PhyInitialize.
typedef struct {
	iso15693VcdCoding_t			coding;	// desired VCD coding
	iso15693ModulationIndex_t	mi;		// modulation index
} iso15693PhyConfig_t;

#define ISO15693_MASK_RECEIVE_TIME			3968		// t1min = 4192
#define ISO15693_NO_RESPONSE_TIME           6304+3000		// t1max + seof = 4256 + 2048
#define ISO15693_FRAME_DELAY_TIME			4192		// t2min0 = 4192

#define ISO15693_REQ_FLAG_TWO_SUBCARRIERS	0x01
#define ISO15693_REQ_FLAG_HIGH_DATARATE		0x02


int  iso15693PhyOpen(const iso15693PhyConfig_t *config);
void iso15693PhyClose(void);
void iso15693PhyGetConfiguration(iso15693PhyConfig_t *config);
int  iso15693PhyTransceiveEof(unsigned char *rxBuf, int *rxLen, int *colBitLen);
int  iso15693PhyTransceiveInventoryFrame(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int *rxLen, int *colBitLen);
int  iso15693PhyTransceiveFrame(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int maxLen, int *rxLen);
int  iso15693PhyTransmitFrame(unsigned char *txBuf, int txLen);


#endif

