#include <stdio.h>
#include <string.h>
#include "log.h"
#include "as3911.h"
#include "as3911_stream.h"
#include "sc_mifare_crc.h"
#include "sc_iso15693_2.h"


#define ISO15693_DAT_SOF_1_4		0x21	// LSB constants
#define ISO15693_DAT_EOF_1_4		0x04
#define ISO15693_DAT_00_1_4			0x02
#define ISO15693_DAT_01_1_4			0x08
#define ISO15693_DAT_10_1_4			0x20
#define ISO15693_DAT_11_1_4			0x80

#define ISO15693_DAT_SOF_1_256		0x81
#define ISO15693_DAT_EOF_1_256		0x04
#define ISO15693_DAT_SLOT0_1_256	0x02
#define ISO15693_DAT_SLOT1_1_256	0x08
#define ISO15693_DAT_SLOT2_1_256	0x20
#define ISO15693_DAT_SLOT3_1_256	0x80

#define ISO15693_PHY_DAT_MANCHESTER_1	0xaaaa

// size of the receiving buffer. Might be adjusted if longer datastreams are expected.
#define ISO15693_PHY_BIT_BUF_SZ		1000

static iso15693PhyConfig_t iso15693PhyConfig; // current phy configuration


int iso15693PhyOpen(const iso15693PhyConfig_t *config)
{
	as3911StreamConfig_t	streamcfg;
	int		rval, val;
unsigned char	buf[4];

	// make a copy of the configuration
	memcpy(&iso15693PhyConfig, config, sizeof(iso15693PhyConfig_t));
	switch(config->mi) {
	case ISO15693_MODULATION_INDEX_10:	val = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_10percent; break;
	case ISO15693_MODULATION_INDEX_14:	val = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_14percent; break;
	case ISO15693_MODULATION_INDEX_20:	val = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_20percent; break;
	case ISO15693_MODULATION_INDEX_30:	val = AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_30percent; break;
	case ISO15693_MODULATION_INDEX_OOK: val = AS3911_REG_AM_MOD_DEPTH_CONTROL_am_s; break;

	}
	as3911SetReg(AS3911_REG_AM_MOD_DEPTH_CONTROL, val);		// AM Modulation Depth Control Register
	if(config->mi == ISO15693_MODULATION_INDEX_OOK) {
//		as3911SetReg( AS3911_REG_RFO_AM_ON_LEVEL, 0x00);	// RFO AM Modulated Level Definition Register
		as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_tr_am, 0);	// disable AM
	} else {
		as3911ModReg(AS3911_REG_AUX, 0, AS3911_REG_AUX_tr_am);	// enable AM
	}
	// Stream mode does not have analog preset, use settings from table 9:
	// "Recommended for 424/484 kHz sub-carrier"
	as3911ModReg(AS3911_REG_RX_CONF1, 0x3f, 0x0c);
	// finally, execute calibrate modulation depth */
	as3911CalibrateModulationDepth(buf);
	as3911SetMaskReceiveTime(ISO15693_MASK_RECEIVE_TIME);
	as3911SetFrameDelayTime(ISO15693_FRAME_DELAY_TIME);		
	streamcfg.useBPSK = 0;	// 0: subcarrier, 1:BPSK
	streamcfg.din = 5;		// 2^5*fc =  32*fc = 423750Hz : divider for the in subcarrier frequency
	streamcfg.dout = 7;		// 2^7*fc = 128*fc = 105937Hz : divider for the out subcarrier frequency
	streamcfg.report_period_length = 3;	// 8=2^3 the length of the reporting period
	rval = as3911OpenStream(&streamcfg);
	return rval;
}

void iso15693PhyClose(void)
{
	as3911CloseStream();
}

void iso15693PhyGetConfiguration(iso15693PhyConfig_t *config)
{
   memcpy(config, &iso15693PhyConfig, sizeof(iso15693PhyConfig_t));
}

static int _Iso15693DecodeCollisionFrame(unsigned char *bitBuf, int bitLen, unsigned char *buf, int *len, int *colBitLen);
static int _Iso15693DecodeFrame(unsigned char *bitBuf, int bitLen, unsigned char *buf, int *len);

int iso15693PhyTransceiveEof(unsigned char *rxBuf, int *rxLen, int *colBitLen)
{
	unsigned char	txBitBuf[4], rxBitBuf[ISO15693_PHY_BIT_BUF_SZ];
	int		rval, bitLen;

	if(iso15693PhyConfig.coding == ISO15693_VCD_CODING_1_4) txBitBuf[0] = ISO15693_DAT_EOF_1_4;
	else	txBitBuf[0] = ISO15693_DAT_EOF_1_256;
	rval = as3911TransceiveStream(txBitBuf, 8, rxBitBuf, 26, &bitLen, ISO15693_NO_RESPONSE_TIME);	// SOF + (flags+DSFID+UID:8+CRC:2)*2 + EOF
printf("EOF transceive=%d\n", rval);
	if(!rval) rval = _Iso15693DecodeCollisionFrame(rxBitBuf, bitLen, rxBuf, rxLen, colBitLen);
	return rval;
}

static int _Iso15693EncodeFrame(unsigned char *buf, int len, unsigned char *bitBuf, iso15693VcdCoding_t coding);

int iso15693PhyTransceiveInventoryFrame(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int *rxLen, int *colBitLen)
{
	unsigned char	txBitBuf[ISO15693_PHY_BIT_BUF_SZ], rxBitBuf[ISO15693_PHY_BIT_BUF_SZ];
	int		rval, bitLen;
 
	txBuf[0] |= ISO15693_REQ_FLAG_HIGH_DATARATE;	// set high datarate flag
	txBuf[0] &= ~ISO15693_REQ_FLAG_TWO_SUBCARRIERS;	// clear sub-carrier flag - we only support single sub-carrier
//printf("Tx %d [%02x", txLen, txBuf[0]); for(i = 1;i < txLen;i++) printf("-%02x", txBuf[i]); printf("]\n"); 
    bitLen = _Iso15693EncodeFrame(txBuf, txLen, txBitBuf, iso15693PhyConfig.coding);
	rval = as3911TransceiveStream(txBitBuf, bitLen, rxBitBuf, 26, &bitLen, ISO15693_NO_RESPONSE_TIME);	// SOF + (flags+DSFID+UID:8+CRC:2)*2 + EOF
	if(!rval) rval = _Iso15693DecodeCollisionFrame(rxBitBuf, bitLen, rxBuf, rxLen, colBitLen);
    return rval;
}

int iso15693PhyTransceiveFrame(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	txBitBuf[ISO15693_PHY_BIT_BUF_SZ], rxBitBuf[ISO15693_PHY_BIT_BUF_SZ];
	int		rval, bitLen;
 
	txBuf[0] |= ISO15693_REQ_FLAG_HIGH_DATARATE;	// set high datarate flag
	txBuf[0] &= ~ISO15693_REQ_FLAG_TWO_SUBCARRIERS;	// clear sub-carrier flag - we only support single sub-carrier
 //printf("Tx %d [%02x", txLen, txBuf[0]); for(i = 1;i < txLen;i++) printf("-%02x", txBuf[i]); printf("]\n"); 
    bitLen = _Iso15693EncodeFrame(txBuf, txLen, txBitBuf, iso15693PhyConfig.coding);
	rval = as3911TransceiveStream(txBitBuf, bitLen, rxBitBuf, ((maxLen+2)<< 1)+2, &bitLen, ISO15693_NO_RESPONSE_TIME);	// SOF + (data+CRC)*2 + EOF
    if(!rval) rval = _Iso15693DecodeFrame(rxBitBuf, bitLen, rxBuf, rxLen);
    return rval;
}

int iso15693PhyTransmitFrame(unsigned char *buf, int len)
{
	unsigned char	bitBuf[ISO15693_PHY_BIT_BUF_SZ];
	int		rval, bitLen;
 
	buf[0] |= ISO15693_REQ_FLAG_HIGH_DATARATE;		// set high datarate flag
	buf[0] &= ~ISO15693_REQ_FLAG_TWO_SUBCARRIERS;	// clear sub-carrier flag - we only support single sub-carrier
 //printf("Tx %d [%02x", txLen, txBuf[0]); for(i = 1;i < txLen;i++) printf("-%02x", txBuf[i]); printf("]\n"); 
    bitLen = _Iso15693EncodeFrame(buf, len, bitBuf, iso15693PhyConfig.coding);
	rval = as3911TransceiveStream(bitBuf, bitLen, NULL, 0, NULL, 0);	// transmit only
    return rval;
}

static int _Iso15693PhyEncode1Of4(unsigned char *buf, int len, unsigned char *bitBuf)
{
	unsigned char	*p, val, val2;
	int		i, j;

 	p = bitBuf;
	for(i = 0;i < len;i++) {
 		val = buf[i];
		for(j = 0;j < 4;j++) {
			switch(val & 0x3) {
 			case 0:	val2 = ISO15693_DAT_00_1_4; break;
 			case 1: val2 = ISO15693_DAT_01_1_4; break;
 			case 2: val2 = ISO15693_DAT_10_1_4; break;
 			case 3: val2 = ISO15693_DAT_11_1_4; break;
   			}
   			val >>= 2;
   			*p++ = val2;
   		}
	}
	return p - bitBuf;
}

static int _Iso15693PhyEncode1Of256(unsigned char *buf, int len, unsigned char *bitBuf)
{
	unsigned char	*p, val, val2;
	int		i, j;

 	p = bitBuf;
	for(i = 0;i < len;i++) {
 		val = buf[i];
		for(j = 0;j < 64;j++) {
			switch(val) {
			case 0:	 val2 = ISO15693_DAT_SLOT0_1_256; break;
			case 1:	 val2 = ISO15693_DAT_SLOT1_1_256; break;
			case 2:	 val2 = ISO15693_DAT_SLOT2_1_256; break;
			case 3:	 val2 = ISO15693_DAT_SLOT3_1_256; break;
			default: val2 = 0;
    		}
			val -= 4;
 			*p++ = val2;
	 	}
	}
	return p - bitBuf;
}

static int _Iso15693EncodeFrame(unsigned char *buf, int len, unsigned char *bitBuf, iso15693VcdCoding_t coding)
{
	unsigned char	*p, tmpbuf[4];
	unsigned short	crc;

	crc = ~crcCalculateCcitt(0xffff, buf, len);
	tmpbuf[0] = crc & 0xff; tmpbuf[1] = (crc >> 8) & 0xff;
	p = bitBuf;	
	*p++ = 0;	// Errata 11: First TX byte is ignored in Subcarrier stream mode
	if(coding == ISO15693_VCD_CODING_1_4) {
		*p++ = ISO15693_DAT_SOF_1_4;
		p += _Iso15693PhyEncode1Of4(buf, len, p);
		p += _Iso15693PhyEncode1Of4(tmpbuf, 2, p);
		*p++ = ISO15693_DAT_EOF_1_4;
	} else {
		*p++ = ISO15693_DAT_SOF_1_256;
		p += _Iso15693PhyEncode1Of256(buf, len, p);
		p += _Iso15693PhyEncode1Of256(tmpbuf, 2, p);
		*p++ = ISO15693_DAT_EOF_1_256;
	}
	return (p - bitBuf) << 3;
}

static int _Iso15693DecodeCollisionFrame(unsigned char *bitBuf, int bitLen, unsigned char *buf, int *len, int *colBitLen)
{
	unsigned char	val, smask, dmask;
    unsigned short  crc;
	int		rval, sbits, dbits, man, cbits, collision;
	
	// first check for valid SOF. Since it starts with 3 unmodulated pulses it is 0x17
	if(bitLen < 15 || (bitBuf[0] & 0x1f) != 0x17) {
printf("SOF error: bitLen=%d\n", bitLen);
		return ERR_HARD_FRAMING;
	}
	sbits = 5; smask = 0x20;	// 5 bits were SOF, now manchester starts: 2 bits per payload bit
	dbits = 0; buf[0] = 0; dmask = 0x01;
	cbits = 0; collision = 0; rval = ERR_HARD_FRAMING;
	while(sbits < bitLen-9) {
		val = bitBuf[sbits >> 3]; 
		if(val & smask) man = 1;
		else	man = 0;
		sbits++; smask <<= 1;
		if(!smask) {
			smask = 0x01; val = bitBuf[sbits >> 3];
		}
		if(val & smask) man |= 2;
		sbits++; smask <<= 1; if(!smask) smask = 0x01;
		if(man == 0) {
printf("Frame error: %d\n", dbits);
			rval = ERR_HARD_FRAMING;
			break;
		} else if(man == 3) {	// 3 collision
			if(dbits >= 16 && dbits < 80) {
				if(!cbits) cbits = dbits;
//printf("%d col=%d\n", collision, dbits-16);
			}
			collision++;
			man = 1;	// collision leave as 0
		}
		if(man == 2) {
			if(dmask == 0x01) buf[dbits >> 3] = dmask;
			else	 buf[dbits >> 3] |= dmask;
		} else {
			if(dmask == 0x01) buf[dbits >> 3] = 0;
		}
		dbits++; dmask <<= 1; if(!dmask) dmask = 0x01; 
		if(dmask == 0x01 && (bitBuf[sbits>>3] & 0xe0) == 0xa0 && bitBuf[(sbits>>3)+1] == 0x03) {
			rval = 0;
			break;	// Now we know that it was 10111000 = EOF
		}
	}
if(rval == ERR_HARD_FRAMING) printf("Frame error: bitLen=%d sbits=%d dbits=%d\n", bitLen, sbits, dbits);
//printf("bitLen=%d sbits=%d dbits=%d\n", bitLen, sbits, dbits);
	if(!rval) {
		if(collision) {
printf("collision=%d %d\n", collision, cbits);
			*colBitLen = cbits;
			rval = ERR_COLLISION;
		} else {
			man = dbits >> 3;
 			crc = ~crcCalculateCcitt(0xffff, buf, man - 2);
			if((crc & 0xff) == buf[man-2] && ((crc >> 8) & 0xff) == buf[man-1]) (*len) = man - 2;
			else	rval = ERR_CRC;
		}
    }
    return rval;
}

static int _Iso15693DecodeFrame(unsigned char *bitBuf, int bitLen, unsigned char *buf, int *len)
{
	unsigned char	val, smask, dmask;
    unsigned short  crc;
	int		rval, sbits, dbits, man;

	// first check for valid SOF. Since it starts with 3 unmodulated pulses it is 0x17
	if(bitLen < 15 || (bitBuf[0] & 0x1f) != 0x17) return ERR_HARD_FRAMING;
	sbits = 5; smask = 0x20;	// 5 bits were SOF, now manchester starts: 2 bits per payload bit
	dbits = 0; buf[0] = 0; dmask = 0x01;
	rval = ERR_HARD_FRAMING;
	while(sbits < bitLen-9) {
		val = bitBuf[sbits >> 3]; 
		if(val & smask) man = 1;
		else	man = 0;
		sbits++; smask <<= 1;
		if(!smask) {
			smask = 0x01; val = bitBuf[sbits >> 3];
		}
		if(val & smask) man |= 2;
		sbits++; smask <<= 1; if(!smask) smask = 0x01;
		if(man == 2) {
			if(dmask == 0x01) buf[dbits >> 3] = dmask;
			else	 buf[dbits >> 3] |= dmask;
		} else if(man == 1) {
			if(dmask == 0x01) buf[dbits >> 3] = 0;
		} else  {
			rval = ERR_HARD_FRAMING;
			break;
		}
		dbits++; dmask <<= 1; if(!dmask) dmask = 0x01; 
		if(dmask == 0x01 && (bitBuf[sbits>>3] & 0xe0) == 0xa0 && bitBuf[(sbits>>3)+1] == 0x03) {
			rval = 0;
			break;	// Now we know that it was 10111000 = EOF
		}
	}
	if(!rval) {
		man = dbits >> 3;
		crc = ~crcCalculateCcitt(0xffff, buf, man - 2);
		if((crc & 0xff) == buf[man-2] && ((crc >> 8) & 0xff) == buf[man-1]) (*len) = man - 2;
		else	rval = ERR_CRC;
    }
    return rval;
}
