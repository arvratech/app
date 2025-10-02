#include <stdio.h>
#include <string.h>
#include "log.h"
#include "as3911.h"
#include "as3911_stream.h"
#include "sc_iso15693_2.h"
#include "sc_iso15693_3.h"


#define ISO15693_NUM_UID_BITS	64		// number of UID bits
#define ISO15693_BUFFER_SIZE	64		// length of iso15693 general purpose buffer/


int iso15693Open(iso15693PhyConfig_t *config)
{
	return iso15693PhyOpen(config);
}

void iso15693Close(void)
{
	iso15693PhyClose();
}

void reverse_memcpy(unsigned char *d, unsigned char *s, int len)
{
	register	int	i;
	
	for(i = 0;i < len;i++) d[i] = s[len-1-i];
}

int iso15693InventoryPresence(iso15693Card_t *card)
{
	unsigned char	buf[32];
	int		rval, len, colBits;

	buf[0] = ISO15693_REQ_FLAG_INVENTORY | ISO15693_REQ_FLAG_1_SLOT;
	buf[1] = ISO15693_CMD_INVENTORY;
    buf[2] = 0;
	rval = iso15693PhyTransceiveInventoryFrame(buf, 3, buf, &len, &colBits);
	if(rval == 0) {
printf("len=%d [%02x", len, (int)buf[0]); for(colBits = 1;colBits < len;colBits++) printf("-%02x", (int)buf[colBits]); printf("\n");
		if(len >= 10) {
			card->flags = buf[0]; card->dsfid = buf[1]; reverse_memcpy(card->uid, buf+2, ISO15693_UID_LENGTH);
		} else	rval = ERR_PROTOCOL;
	}
	return rval;
}

static int _InventoryOneSlot(int maskBitLen, unsigned char *mask, iso15693Card_t *cards, int *cardCount, int maxCards)
{
	iso15693Card_t *card;
	unsigned char	val, buf[32];
	int		rval, len, colBits;

	if(*cardCount >= maxCards) return 0;
	buf[0] = ISO15693_REQ_FLAG_INVENTORY | ISO15693_REQ_FLAG_1_SLOT;
	buf[1] = ISO15693_CMD_INVENTORY;
    buf[2] = maskBitLen;
	len = (maskBitLen + 7) >> 3;
	if(len) reverse_memcpy(buf+3, mask, len);
	rval = iso15693PhyTransceiveInventoryFrame(buf, 3+len, buf, &len, &colBits);
	if(rval == 0) {
printf("len=%d [%02x", len, (int)buf[0]); for(val = 1;val < len;val++) printf("-%02x", (int)buf[val]); printf("\n");
		if(len >= 10) {
			len = *cardCount;
			card = &cards[len];
			card->flags = buf[0]; card->dsfid = buf[1]; reverse_memcpy(card->uid, buf+2, ISO15693_UID_LENGTH);
			(*cardCount) = len + 1;
		} else	rval = ERR_PROTOCOL;
	} else if(rval == ERR_COLLISION) {
		colBits -= 16;
		// we provided a mask but received fewer bytes which is actually impossible. exit at this point with collision error
		if(colBits >= maskBitLen) {
			maskBitLen = colBits; len = (maskBitLen + 7) >> 3;
			val = maskBitLen & 7;
			if(val)  {
				reverse_memcpy(mask, buf+2, len);
				mask[0] &= 0xff >> (8 - val);
			} else {
				reverse_memcpy(mask+1, buf+2, len);
				mask[0] = 0; len++;
			}
			maskBitLen++;
			_InventoryOneSlot(maskBitLen, mask, cards, cardCount, maxCards);
			mask[0] |= 0x01 << val;
			_InventoryOneSlot(maskBitLen, mask, cards, cardCount, maxCards);
		}
	}
	return rval;
}

int iso15693InventoryOneSlot(iso15693Card_t *cards, int *cardCount, int maxCards)
{
	unsigned char	buf[4], mask[8];
	int		rval, reg;

	as3911GetReg(AS3911_REG_RX_CONF2, buf); reg = buf[0];
	as3911SetReg(AS3911_REG_RX_CONF2, reg & ~AS3911_REG_RX_CONF2_agc_en);
	*cardCount = 0;
	rval = _InventoryOneSlot(0, mask, cards, cardCount, maxCards);
    as3911SetReg(AS3911_REG_RX_CONF2, reg);
	return rval;
}

static int _Inventory16Slot(iso15693Card_t *cards, int *cardCount, int maxCards)
{
	iso15693Card_t		*card;
	unsigned char	val, mask[8], buf[ISO15693_BUFFER_SIZE+2], slots[16];
	int		rval, i, len, maskBitLen, colBits;

	buf[0] = ISO15693_REQ_FLAG_INVENTORY;
	buf[1] = ISO15693_CMD_INVENTORY;
	buf[2] = 0;
	memset(slots, 0, 16);
	rval = iso15693PhyTransceiveInventoryFrame(buf, 3, buf, &len, &colBits);
	for(i = 0;i < 16;i++) {
		if(*cardCount >= maxCards) break;
		if(rval == 0) {
printf("len=%d [%02x", len, (int)buf[0]); for(val = 1;val < len;val++) printf("-%02x", (int)buf[val]); printf("\n");
			if(len >= 10) {
				len = *cardCount;
				card = &cards[len];
				card->flags = buf[0]; card->dsfid = buf[1]; reverse_memcpy(card->uid, buf+2, ISO15693_UID_LENGTH);
				(*cardCount) = len + 1;
			} else	rval = ERR_PROTOCOL;
		} else if(rval == ERR_COLLISION) {
			colBits -= 16;
			if(colBits >= 4) {
				slots[i] = colBits; rval = 0;
			} 	
		}
		if(rval) break;
		rval = iso15693PhyTransceiveEof(buf, &len, &colBits);
	}
	if(rval) return rval;
	for(i = 0;i < 16;i++) {
		if(*cardCount >= maxCards) break;
		if(slots[i]) {
			maskBitLen = slots[i]; len = (maskBitLen + 7) >> 3;
			val = maskBitLen & 7;
			if(val) {
				reverse_memcpy(mask, buf+2, len);
				mask[0] &= 0xff >> (8 - val);
			} else {
				reverse_memcpy(mask+1, buf+2, len);
				mask[0] = 0; 
				len++;
			}
			maskBitLen++;
			rval = _InventoryOneSlot(maskBitLen, mask, cards, cardCount, maxCards);
			mask[0] |= 0x01 << val;
			rval = _InventoryOneSlot(maskBitLen, mask, cards, cardCount, maxCards);
		}
	}
    return rval;
}

int iso15693Inventory16eSlot(iso15693Card_t *cards, int *cardCount, int maxCards)
{
	unsigned char	buf[4];
	int		rval, reg;

	as3911GetReg(AS3911_REG_RX_CONF2, buf); reg = buf[0];
	as3911SetReg(AS3911_REG_RX_CONF2, reg & ~AS3911_REG_RX_CONF2_agc_en);
	*cardCount = 0;
	rval = _Inventory16Slot(cards, cardCount, maxCards);
    as3911SetReg(AS3911_REG_RX_CONF2, reg);
	return rval;
}

int iso15693StayQuiet(unsigned char *uid)
{
	unsigned char	buf[12];
	int		rval, len;

	if(uid) {
//printf("StayQuiet UID=[%02x", (int)uid[0]); for(len = 1;len < 8;len++) printf("-%02x", (int)uid[len]); printf("\n");
		buf[0] = ISO15693_REQ_FLAG_ADDRESS;
		reverse_memcpy(buf+2, uid, ISO15693_UID_LENGTH);
		len = 2 + ISO15693_UID_LENGTH;
    } else {	// uid is NULL which means that selected PICC(using #iso15693SelectPicc) is used
		buf[0] = ISO15693_REQ_FLAG_SELECT;
        len = 2;
    }
	buf[1] = ISO15693_CMD_STAY_QUIET;
	rval = iso15693PhyTransmitFrame(buf, len);
	return rval;
}

static int iso15693Tranceive(int cmd, int flags, unsigned char *uid, unsigned char *data, int dataLen, unsigned char *rxBuf, int maxLen, int *rxLen);

int iso15693ResetToReady(unsigned char *uid)
{
	unsigned char	buf[8];
	int		rval, len;

	rval = iso15693Tranceive(ISO15693_CMD_RESET_TO_READY, 0, uid, NULL, 0, buf, 4, &len);
    return rval;
}

int iso15693SelectPicc(iso15693Card_t *card)
{
	unsigned char	buf[8];
	int		rval, len;

	rval = iso15693Tranceive(ISO15693_CMD_SELECT, 0, card->uid, NULL, 0, buf, 4, &len);
    return rval;
}

int iso15693GetPiccSystemInformation(iso15693Card_t *card, iso15693PiccSystemInformation_t *sysInfo)
{
	unsigned char	buf[24];
	int		rval, len, offset;

	rval = iso15693Tranceive(ISO15693_CMD_GET_SYSTEM_INFORMATION, 0, card->uid, NULL, 0, buf, 15, &len);
	if(rval) return rval;
	sysInfo->dsfid = 0;
	sysInfo->afi = 0;
	sysInfo->memNumBlocks = 0;
	sysInfo->memBlockSize = 0;
	sysInfo->icReference = 0;
	if(!rval) {
		if(len > 9) {
			// copy first 10 bytes which are fixed
			memcpy(sysInfo, buf, 10);
			offset = 0;
			// evaluate infoFlags field
			if(sysInfo->infoFlags & 0x01) {	// dsfid field present
				sysInfo->dsfid = buf[10]; offset++;
			}
			if(sysInfo->infoFlags & 0x02) {	// afi field present
				sysInfo->afi = buf[10+offset]; offset++;
			}
			if(sysInfo->infoFlags & 0x04) {	// memory size field present
				sysInfo->memNumBlocks = buf[10+offset];
				sysInfo->memBlockSize = buf[11+offset];
				offset += 2;
			}
			if(sysInfo->infoFlags & 0x08) {	// ic reference field present
				sysInfo->icReference = buf[10+offset];
			}
		} else	rval = ERR_PROTOCOL;	
	}
	return rval;
}

int iso15693ReadSingleBlock(iso15693Card_t *card, int blockNo, int blockSize, iso15693PiccBlock_t *block)
{
	unsigned char	tmpbuf[4], buf[ISO15693_BUFFER_SIZE];
	int		rval, len;

	tmpbuf[0] = blockNo;
	rval = iso15693Tranceive(ISO15693_CMD_READ_SINGLE_BLOCK, 0, card->uid, tmpbuf, 1, buf, ISO15693_BUFFER_SIZE, &len);
	if(rval) return rval;
	rval = ERR_PROTOCOL;
	if(len >= 2) {
		block->flags = buf[0]; len--;
		if(block->flags & ISO15693_RESP_FLAG_ERROR) {
			block->errorCode = buf[1];
			rval = 0;
		} else if(len == blockSize) {
			block->errorCode = 0;
			reverse_memcpy(block->data, buf+1, blockSize);
			rval = 0;
		}
	}
	return rval;
}

int iso15693ReadMultipleBlocks(iso15693Card_t *card, int blockNo, int blockSize, int blockCount, iso15693PiccMultipleBlock_t *block)
{
	unsigned char	*s, *d, tmpbuf[4], buf[ISO15693_BUFFER_SIZE];
	int		rval, i, len;

	tmpbuf[0] = blockNo; tmpbuf[1] = blockCount - 1;
	rval = iso15693Tranceive(ISO15693_CMD_READ_MULTIPLE_BLOCKS, 0, card->uid, tmpbuf, 2, buf, ISO15693_BUFFER_SIZE, &len);
	if(rval) return rval;
	rval = ERR_PROTOCOL;
	if(len >= 2) {
		block->flags = buf[0]; len--;
		if(block->flags & ISO15693_RESP_FLAG_ERROR) {
			block->errorCode = buf[1];
			rval = 0;
		} else if(len == blockSize * blockCount) {
			s = buf+1; d = block->data;
			for(i = 0;i < blockCount;i++) {
				reverse_memcpy(d, s, blockSize);
printf("[%02x-%02x-%02x-%02x]\n", (int)s[0], (int)s[1], (int)s[2], (int)s[3]);
				s += blockSize; d += blockSize;
			}
			rval = 0;
		}
	}
	return rval;
}

int iso15693WriteSingleBlock(iso15693Card_t *card, int flags, int blockNo, int blockSize,iso15693PiccBlock_t *block)
{
	unsigned char	tmpbuf[36], buf[ISO15693_BUFFER_SIZE];
	int		rval, bitLen, colBits;

	if(!(flags & ISO15693_REQ_FLAG_OPTION)) {
//		iso15693PhySetNoResponseTime_64fcs(4238); /* ~ 20ms */
	}
	tmpbuf[0] = blockNo;
	reverse_memcpy(tmpbuf+1, block->data, blockSize);
	// just send the request and wait separately for the answer
    rval = iso15693Tranceive(ISO15693_CMD_WRITE_SINGLE_BLOCK, flags, card->uid, tmpbuf, blockSize+1, buf, 32, &bitLen);
	if(rval) return rval;
	if(flags & ISO15693_REQ_FLAG_OPTION) {
//		delayNMilliSeconds(20);		// according to the standard wait 20ms before sending EOF
		rval = iso15693PhyTransceiveEof(buf, &bitLen, &colBits);
		if(rval) return rval;
	}
	if(bitLen > 24) rval = ERR_NOTSUPP;
	if(!(flags & ISO15693_REQ_FLAG_OPTION)) {
//		iso15693PhySetNoResponseTime_64fcs(ISO15693_NO_RESPONSE_TIME);
	}
	return rval;
}

static int iso15693Tranceive(int cmd, int flags, unsigned char *uid, unsigned char *data, int dataLen, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	buf[256];
	int		rval, len;

	buf[0] = flags;
 	// sanity checks first
	if((ISO15693_UID_LENGTH + dataLen + 2) > ISO15693_BUFFER_SIZE) return ERR_PARAM;
	if(uid) {
		buf[0] |= ISO15693_REQ_FLAG_ADDRESS;
		reverse_memcpy(buf+2, uid, ISO15693_UID_LENGTH);
		len = 2 + ISO15693_UID_LENGTH;
    } else {	// uid is NULL which means that selected PICC(using #iso15693SelectPicc) is used
		buf[0] |= ISO15693_REQ_FLAG_SELECT;
        len = 2;
    }
	buf[1] = cmd;
	memcpy(buf+len, data, dataLen); len += dataLen;
	rval = iso15693PhyTransceiveFrame(buf, len, rxBuf, maxLen, rxLen);
	if(rval) return rval;
	return rval;
}
