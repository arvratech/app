#include "iso15693_2.h"
#include "iso15693_3.h"
#include "delay.h"
#include "logger.h"



#define ISO15693_NUM_UID_BITS	64		// number of UID bits

#define ISO15693_BUFFER_SIZE	64		// length of iso15693 general purpose buffer/

static unsigned char iso15693DirMarker[ISO15693_NUM_UID_BITS];	// direction marker used during inventory for binary tree search.
																// values: 0, 1, 2. 0 means no collision at current bit position.
																// 1 means that left path is tried and 2 means right path is tried.
static unsigned char iso15693DefaultSendFlags;		// default flags used for iso15693SendRequest

static int iso15693SendRequest(int cmd, int flags, const iso15693Card_t* card, unsigned char* data, int dataLen,
							 		unsigned char *rxBuf, int maxLen, int *rxLen);


int iso15693Open(iso15693PhyConfig_t *config)
{
	return iso15693PhyInitialize(config);
}

void iso15693Close(void)
{
	iso15693PhyClose();
}

int iso15693Inventory(iso15693NumSlots_t slotNum, int maskBitLen, unsigned char *mask, iso15693Card_t* cards, int maxCards, int *cardsFound)
{
    iso15693PhyConfig_t	cfg;
    iso15693Card_t		*card;
	unsigned char	val, buf[ISO15693_BUFFER_SIZE+2];
	unsigned long long	bitCollisions;		// 64 bit long marker holding all unresolved collisions within 64bit UID
	int		rval, i, bitLen, maskBitLen, maskLen, slot, cardIdx, bitColPos, slotNumPos, slotCollisions, slotColPos, timeout;

	as3911GetReg(AS3911_REG_RX_CONF2, &val);
	as3911SetReg(AS3911_REG_RX_CONF2, val & ~AS3911_REG_RX_CONF2_agc_en);
	iso15693PhyGetConfiguration(&cfg);
	// depending on set modulation index and datarate, there has to be a
	// timeout before sending EOF for slot count = 16!
	if(cfg.mi != ISO15693_MODULATION_INDEX_OOK) timeout = 5;
 	else	timeout = 1;
	memset(iso15693DirMarker, 0, ISO15693_NUM_UID_BITS);
    buf[0] = ISO15693_REQ_FLAG_INVENTORY;	// // set inventory flag
	// if number of slot is 1 then set the appropriate flag
    if(slotNum == ISO15693_NUM_SLOTS_1) buf[0] |= ISO15693_REQ_FLAG_1_SLOT;
    buf[1] = ISO15693_CMD_INVENTORY;
    buf[2] = maskBitLen;
    maskLen = (maskBitLen + 7) >> 3;
    if(maskLen > 8) return ERR_PARAM;
    i = maskBitLen & 7;
    if(i) mask[maskLen-1] &= 0xff >> (8-i); // mask out unneeded bits
 	memcpy(buf+3, mask, maskLen);
    slotNumPos = i;
   
    card = cards;
    cardIdx = 0;
    bitCollisions = 0; bitColPos = 0;
    slotCollisions = 0; slotColPos = -1;
	// this outer loop iterates as long as there are unresolved collisions including slots with unresolved collisions
	do {
   		// inner loop iterates once over all slots (in case of slot count 16)
		// or only one iteration in case of slot count 1.
		// After first 16 slot iterations (slot count 16) slot count is changed to 1
		for(slot = 0;i < slot < 16;slot++) {
            if(slot == 0) {
                // send the inventory request. Note: CRC is appended by physical layer.
				iso15693PhyTransmitFrame(buf, 3 + maskLen, 1, 1);
            } else {
                // in case if slot count 16 slot is incremented by just sending EOF
                delayNMilliSeconds(timeout);
                iso15693PhyTransmitEof();
            }
            // receive data ignoring collisions in first 2 bytes
           	rval = iso15693PhyReceiveFrame((unsigned char *)card, sizeof(iso15693Card_t), &bitLen, 16);
            if(rval == ERR_COLLISION) {
                if(bitLen < 16) {
                    // collision before actually receiving UID!!! This should not happen
                    // since we ignored collisions in these bytes.
                    err = ERR_COLLISION;
                    goto out;
                }
                if((bitLen - 16) < maskBitLen) {
                    // we provided a mask but received fewer bytes which is actually
                    // impossible. exit at this point with collision error
					rval = ERR_COLLISION;
                    goto out;
                }
                if(slotNum == ISO15693_NUM_SLOTS_1) {	// in case slot count is 1 collision needs to be resolved
                    // find position of collision within received UID and
                    // update mask and mask length appropriately
                    maskBitLen = bitLen - 16 + 1;
                    if(maskBitLen > ISO15693_NUM_UID_BITS) maskBitLen = ISO15693_NUM_UID_BITS;
				    buf[2] = maskBitLen;
					maskLen = (maskBitLen + 7) >> 3;
					memcpy(buf+3, card->uid, maskLen);	// copy received UID to mask
					bitColPos = maskBitLen - 1;
					bitCollisions |= ((u64)1 << (u64)bitColPos);              
				    i = bitColPos & 7;
    				buf[2+maskLen] &= 0xff >> (8 - i); // clear bit where collision happened which means try left branch of the tree first
                    if(iso15693DirMarker[bitColPos] == 1) {
                        // if left branch has been tried out before (dirMarker set to 1)
                        // the set the bit where collision happened to 1, i.e. try right branch
                        buf[2+maskLen] |= 1 << i;
                    }
                    // in any case increment dirMarker to indicate the way we chose 
                    iso15693DirMarker[bitColPos]++;
                } else {
                    // in case of slot count 16 just mark that there is a collision
                    // within this slot. Resolve it later when switching back to slot count 1
                    slotCollisions |= 1 << slot;
                }
		} else {
            if(!rval) {	// received all bytes without collision - store UID
                cardIdx++;
                if(cardIdx >= maxCards) {
                    goto out_max_cards;
                }
                card++;
            }
            if(slotNum == ISO15693_NUM_SLOTS_1) {
                i = ISO15693_NUM_UID_BITS;
                // a collisions has been resolved. Go back in the tree to find next collision
                while(i--) {
                    if(bitCollisions & ((u64)1 << (u64)i)) {
                        if(iso15693DirMarker[i] > 1) {
                            // dirMarker 2 means that both paths(left and right) have been
                            // tried (=resolved). Remove this collision
                            bitCollisions &= ~((u64)((u64)1 << (u64)i));
                            iso15693DirMarker[i] = 0;
                            if(slotColPos >= 0) {
                                // if this collision was within a slot unmark also this slot
                                slotCollisions &= ~(1 << slotColPos);
                                slotColPos = -1;
                            }
                        } else {
                            // update collision position. dirMarker 1 also means that left
                            // branch was tried before. Switch to right branch now.
                            bitColPos = i;
                            buf[2] = bitColPos + 1;
                            maskByteLen = (bitColPos >> 3) + 1;
                            buf[2+maskByteLen] |= (1 << (bitColPos & 7));
                            iso15693DirMarker[bitColPos]++;
                            break;
                        }
                    }
                }
                if(slotColPos >= 0 && iso15693DirMarker[bitColPos] == 0) {
                    // a slot where a collision was found before has been processed
                    // with no collision. So unmark this slot
                    slotCollisions &= ~(1 << slotColPos);
                    slotColPos = -1;
                }
            }
	        if(slotNum == ISO15693_NUM_SLOTS_1) break;    
		}
		// after 16 iterations switch back to slot count 0 which means a normal binary tree search
		if(slotNum == ISO15693_NUM_SLOTS_16) {
            slotNum = ISO15693_NUM_SLOTS_1;
            buf[0] |= ISO15693_REQ_FLAG_1_SLOT;
        }
        if(!bitCollisions && slotNum == ISO15693_NUM_SLOTS_1) {
			// if all collisions are resolved check the slots for open collisions
            for(i = 0;i < 16;i++) {
                if(slotCollisions & (1 << i)) {
                    // found a slot with unresolved collision.
                    // Reset mask length to original value and append slot number to mask
                    maskLen = (maskBitLen + 7) >> 3;
                    if(slotNumPos == 0) {
                        // add an additional byte in case slot number starts at LSB
                        maskLen++;
                    }
                    if(slotNumPos > 4) {
                        // also if slot number would overlap add an additional byte
                        maskLen++;
                        // add slot number to mask
                        buf[2+maskLen] &= ~((1 << (8 - slotNumPos)) - 1);
                        buf[2+maskLen] |= i >> (8 - slotNumPos);
                        buf[1+maskLen] &= (1 << slotNumPos) - 1;
                        buf[1+maskLen] |= (i << slotNumPos);
                    } else {
                        // add slot number to mask
                        buf[2+maskLen] &= (1 << slotNumPos) - 1;
                        buf[2+maskLen] |= (i << slotNumPos);
                    }
                    // in any case number of mask bits needs to be incremented by 4
                    buf[2] = maskBitLen + 4;
                    ISO_15693_DEBUG("checking col slot 0x%x\n", i);
                    slotColPos = i;
                    break;
                }
            }

        }
        // do not stop before all collisions in all slots are resolved
    } while(bitCollisions || slotCollisions);

out_max_cards:
    err = ERR_NONE;
out:
    *cardsFound = cardIdx;
    if(*cardsFound == 0) err = ERR_NOTFOUND;
    as3911SetReg(AS3911_REG_RX_CONF2, reg);
    return err;
}
