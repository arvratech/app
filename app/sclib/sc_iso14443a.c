#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "as3911_modulation_adjustment.h"
#include "emv_standard.h"
#include "sc_iso14443a.h"

// AS3911 operation control register transmit enable bit mask
#define EMV_HAL_REG_OPCONTROL_TXEN_BIT    0x08

/*******************************************************************************
 * Receiver dead time (in carrier cycles) for the ISO14443A protocol.
 * See [CCP v2.01, PCD 4.8.1.3].
 * Calculation:
 *
 * 1108: Deaftime if the last transmitted data bis was logic '1'
 *
 *  276: Time from the rising pulse of the pause of the logic '1'
 *       (i.e. the timepoint to measure the deaftime from), to the actual end
 *       of the EOF sequence (the point where the MRT starts).
 *       Please note that the AS3911 uses the ISO14443-2 definition where the
 *       EOF consists of logic '0' followed by sequence Y.
 * 
 *   64: Adjustment for the MRT timer jitter. Currently the MRT timer will have
 *       any timeout between the set timeout and the set timout + 64 cycles.
 ******************************************************************************/
#define EMV_HAL_ISO14443A_RECEIVER_DEADTIME		(1108 - 276 - 64)

/*******************************************************************************
 * Adjustment from the timeout requested by the caller to the timeout
 * value which has to be used for the AS3911.
   2장 이상에서 860 이하이면 타임아웃 발생됨
 ******************************************************************************/
#define EMV_HAL_TYPE_A_TIMEOUT_ADJUSTMENT		970		// 512

/*******************************************************************************
 * Frame wait time for n = 9 frames of the ISO14443A protocol.
 * 
 * The frame wait time depends on the last transmitted bit, therefore
 * the higher value (last transmitted bit is asumed to be one) has been 
 * taken (1236 carrier cycles) with an additional 14 cycles safety margin.
 ******************************************************************************/
#define EMV_TYPEA_FDT_9				1250

#define EMV_ATQA_UID_SIZE_MASK		0xc0	// Bit mask for the UID size field of the first ATQA byte
#define EMV_ATQA_UID_SIZE_SINGLE	0x00	// ATQA UID size field value for a single size UID
#define EMV_ATQA_UID_SIZE_DOUBLE    0x40	// ATQA UID size field value for a double size UID
#define EMV_ATQA_UID_SIZE_TRIPLE    0x80	// ATQA UID size field value for a triple size UID

#define EMV_ATQA_UID_SIZE_INVALID	0xC0	// Invalid ATQA UID size field value. A standard conforming card should never return this value.

#define EMV_SEL_CL1					0x93	// Value of the SEL byte of a cascade level 1 anticollision or select request			
#define EMV_SEL_CL2					0x95	// Value of the SEL byte of a cascade level 2 anticollision or select request
#define EMV_SEL_CL3					0x97	// Value of the SEL byte of a cascade level 3 anticollision or select request

#define EMV_ANTICOLLISION_NVB		0x20	// NVB (number of valid bits) byte value for an ISO14443-A anticollision request
#define EMV_SELECT_NVB				0x70	// NVB (number of valid bits) byte value for an ISO14443-A select request

#define EMV_CASCADE_TAG				0x88	// ISO14434-A anticollision response cascade tag value

#define EMV_SAK_CASCADE_BIT_MASK	0x04	// Mask for the cascade bit of an ISO14443-A SAK

#define EMV_SAK_ISO144434_COMPLIANT_BIT_MASK  0x20	// Mask for the ISO14443-4 compliance bit fo an ISO14443-A SAK

#define EMV_HLTA_FDT				1250	// Frame delay time (timeout) used for ISO14443-A HLTA commands

static void * emvHalLowAGainTable = NULL;
static void * emvHalNormAGainTable = NULL;

ModulationLevelAutomatic_t	mla = { 0x80, 0x80 };

void iso14443AOpen(void)
{
    unsigned char	buf[4];

	as3911GetReg(AS3911_REG_OP_CONTROL, &opControl);
	buf[0] = (opControl | AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en) & ~AS3911_REG_OP_CONTROL_wu;
	buf[1] = AS3911_REG_MODE_om_iso14443a;	// Mode Definition Register: ISO14443A
	buf[2] = 0x00;		// Bit Rate definition Register: 106kbps both direction
	buf[3] = 0x00;      // ISO14443A and NFC 106kb/s Setting register
						// Disable transmit without parity, disable receive without parity&crc, set to default pulse width, and disable special anticollision mode. 
    as3911SetRegs(AS3911_REG_OP_CONTROL, buf, 4);
	// no tolerant processing of the first byte, AS3911_REG_AUX_tr_am=0(OOK) set by ANALOG_PRESET command
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx | AS3911_REG_AUX_crc_2_fifo | AS3911_REG_AUX_rx_tol, 0);	
			
    as3911SetMaskReceiveTime(EMV_HAL_ISO14443A_RECEIVER_DEADTIME);
	as3911SetFrameDelayTime(EMV_FDT_PCD_MIN);	
	as3911ExecuteCommand(AS3911_CMD_ANALOG_PRESET);
	//as3911SetGainTables(emvHalLowAGainTable, emvHalNormAGainTable);	// Set proper gain table for used technology
	//as3911SetModulationLevelMode(ML_FIXED, NULL);   // Disable dynamic adjustment of the modulation level
//printf("iso14443AOpen...\n");
}

void iso14443AClose(void)
{
//	opControl &= ~(AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
	opControl &= ~(AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
	as3911SetReg(AS3911_REG_OP_CONTROL, (int)opControl);
//printf("iso14443AClose...\n");
}

//	xxxxrrtt 
void iso14443ASetBitRate(int bitRate)
{
	int		val;

	//			tx rate					rx rate
	val = ((bitRate & 0x03) << 4) | ((bitRate & 0x0c) >> 2);
	as3911SetReg(AS3911_REG_BIT_RATE, val);
	as3911ExecuteCommand(AS3911_CMD_ANALOG_PRESET);
}

unsigned long	cmdTimer;
long	cmdTimeout;

#include "as3911_def.h"
#include "as3911_io.h"

void iso14443ASendRequest(iso14443ACommand_t cmd)
{
	int		dcmd;
	
	// first disable CRC while receiving since ATQA has no CRC included
	as3911ModReg(AS3911_REG_AUX, 0, AS3911_REG_AUX_no_crc_rx);
	// Enable antcl to recognize collision in first byte of ATQA
    as3911ModReg(AS3911_REG_ISO14443A_NFC, 0, AS3911_REG_ISO14443A_NFC_antcl);
//	if(as3911TxrxOnCsx) as3911WriteTestRegister(0x1, 0x0a); // digital modulation on pin CSI
	as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);
	as3911ExecuteCommand(AS3911_CMD_CLEAR_RX_GAIN);
	as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
	as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
	// Chip bug: Clear nbtx bits before sending WUPA/REQA - otherwise AS3911 will report parity error
	as3911SetReg(AS3911_REG_NUM_TX_BYTES2, 0);
    // now send either WUPA or REQA. All affected tags will backscatter ATQA and change to READY state
	if(cmd == ISO14443A_CMD_WUPA) dcmd = AS3911_CMD_TRANSMIT_WUPA; // All
	else	dcmd = AS3911_CMD_TRANSMIT_REQA;	// Idle
	as3911SetNoResponseTime(EMV_TYPEA_FDT_9+EMV_HAL_TYPE_A_TIMEOUT_ADJUSTMENT);
	as3911ExecuteCommand(dcmd);
	cmdTimer = SC_TIMER; cmdTimeout = 10;
}

int iso14443AReceiveRequest(unsigned char *atqa)
{
	unsigned long	status;
	unsigned char	fifoStatus[2], buf[100];
	int		rval,  count, bitlen, lastbits;

	atqa[0] = atqa[1] = 0; 
	status = as3911GetInterrupts(AS3911_IRQ_MASK_RXE | AS3911_IRQ_MASK_NRE);
	rval = -1;
	if(status & AS3911_IRQ_MASK_RXE) {
		as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
		count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
		if(count) {
			as3911ReadFifo(buf, count);
			if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ncp) {
				lastbits = (fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_mask_fifo_lb) >> AS3911_REG_FIFO_RX_STATUS2_shift_fifo_lb;
				bitlen = ((count - 1) << 3) + lastbits;  			
			} else	bitlen = count << 3;
		} else	bitlen = 0;
		status = as3911GetInterrupts(AS3911_IRQ_MASK_COL | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
		if(status & AS3911_IRQ_MASK_COL) rval = ERR_COLLISION;
		else if(status & AS3911_IRQ_MASK_PAR) rval = ERR_PARITY;
		else if(status & AS3911_IRQ_MASK_ERR1) rval = ERR_SOFT_FRAMING;
		else if(status & AS3911_IRQ_MASK_ERR2) rval = ERR_SOFT_FRAMING;
		else	rval = 0;
	} else if(status & AS3911_IRQ_MASK_NRE) {
		rval = ERR_TIMEOUT;
	} else if((MS_TIMER-cmdTimer) > cmdTimeout) { 
printf("timeout: %lu - %lu > %d\n", MS_TIMER, cmdTimer, cmdTimeout);
		as3911GetInterrupts(AS3911_IRQ_MASK_ALL);
		rval = ERR_CHIP;
	}	
	if(rval >= 0) {
		as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
 		if(!rval) {
			if(bitlen == 16) {
				atqa[0] = buf[0]; atqa[1] = buf[1];
				if(!(atqa[0] & 0x1f)) rval = ERR_NOTSUPP;	// Select/anticollision not supported
			} else 	rval = ERR_PROTOCOL;
		}
		as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx, 0);
		as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, 0);
		as3911SetFrameDelayTime(EMV_FDT_PCD_MIN);
//if(!rval) printf("ReceiveRequest: OK %02x-%02x %d-%02x\n", (int)atqa[0], (int)atqa[0], count, (int)fifoStatus[1]);
//else if(rval == ERR_COLLISION) printf("ReceiveRequest: Collision %d-%02x\n", bitlen, (int)fifoStatus[1]);
//else if(rval == ERR_PARITY) printf("ReceiveRequest: Parity %d-%02x\n", count, (int)fifoStatus[1]);
//else if(rval == ERR_HARD_FRAMING) printf("ReceiveRequest: HardFraming %d-%02x\n", count, (int)fifoStatus[1]);
//else if(rval == ERR_SOFT_FRAMING) printf("ReceiveRequest: SoftFraming %d-%02x\n", count, (int)fifoStatus[1]);
//else if(rval == ERR_TIMEOUT) ; // printf("%lu ReceiveRequest: Timeout\n", MS_TIMER);
//else	printf("ReceiveRequest: %d\n", rval);
    }
	return rval;
}

#define COMPLETE_UID_BITS 	40

static int _PiccCascAnticoll(unsigned char sel_code, unsigned char *uid)
{
	unsigned char	val, txbuf[16], rxbuf[16];
	int		rval, i, bitcount, nbytes, nbits, rbits, offset;

#ifdef SC_DEBUG
printf("_PiccCasAnticoll: SEL=%02x...\n", (int)sel_code);
#endif
	rval = 0; bitcount = 0;
	while(!rval && bitcount < COMPLETE_UID_BITS) {
		nbytes = bitcount >> 3; nbits = bitcount & 0x7;
		txbuf[0] = sel_code;
		txbuf[1] = (unsigned char)(0x20 + (nbytes << 4) + nbits);
		if(nbits) nbytes++;
		for(i = 0;i < nbytes;i++) txbuf[2+i] = uid[i];
		rval = as3911TransceiveBitData(txbuf, bitcount+16, rxbuf, 16, &rbits, EMV_TYPEA_FDT_9+EMV_HAL_TYPE_A_TIMEOUT_ADJUSTMENT);
		if(!rval || rval == ERR_COLLISION) {
			if(rval == ERR_COLLISION) {
				if(rbits & 0x07) { val = 0x01; val <<= rbits & 0x07; rxbuf[rbits>>3] |= val; }
				else	rxbuf[rbits>>3] = 0x80;
				rbits++;
			}
			if(rbits > nbits) {
				rbits -= nbits;
#ifdef SC_DEBUG
printf("_PiccCasAnticoll=%d: BitCount=%d(%d+%d)\n", rval, bitcount+rbits, bitcount, rbits);
#endif
				bitcount += rbits;
				if(bitcount > COMPLETE_UID_BITS) rval = ERR_PROTOCOL;	// UID0+UID1+UID2+UID3+BCC(5Bytes=40bits)
				else {
					offset = 0;
					if(nbits) {
						val = 0xff; val >>= (8-nbits);	// if nbits=3, mask=b00000111
						uid[nbytes-1] = (uid[nbytes-1] & val) | (rxbuf[0] & ~val);
						offset++;
					}
					for(i = 0;i < 5-nbytes;i++) uid[nbytes+i] = rxbuf[i+offset];
					rval = 0;
					if(bitcount >= COMPLETE_UID_BITS) {
						val = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];
						if(val != uid[4]) rval = ERR_PROTOCOL;
					}
				}
			} else {
#ifdef SC_DEBUG
printf("_PiccCasAnticoll=%d: BitCount=%d %d<=%d\n", rval, bitcount, rbits, nbits);
#endif
				rval = 0;
			}
		}
	}
#ifdef SC_DEBUG
if(rval) printf("_PiccCasAnticoll=%d\n", rval);
#endif
	return rval;
}

static int _PiccCascSelect(unsigned char sel_code, unsigned char *uid, unsigned char *sak)
{
	unsigned char	txbuf[16], rxbuf[16];
	int		rval, rlen;
 
#ifdef SC_DEBUG
printf("_PiccCasSelect: SEL=%02x UID=[%02x%02x%02x%02x]...\n", (int)sel_code, (int)uid[0], (int)uid[1], (int)uid[2], (int)uid[3]);
#endif
	txbuf[0] = sel_code;
	txbuf[1] = 0x70;
	memcpy(txbuf+2, uid, 4);
	txbuf[6] = uid[0] ^ uid[1] ^ uid[2] ^ uid[3];	// BCC
	rval = as3911TransceiveData(txbuf, 7, rxbuf, 16, &rlen, EMV_TYPEA_FDT_9+EMV_HAL_TYPE_A_TIMEOUT_ADJUSTMENT);
	if(!rval && rlen != 1) rval = ERR_PROTOCOL;
	if(rval) sak[0] = 0;
	else	sak[0] = rxbuf[0];
#ifdef SC_DEBUG
if(!rval) printf("_PiccCascSelect: SAK=%02x\n", (int)sak[0]);
else if(rval == ERR_PROTOCOL) printf("_PiccCasSelect: ERR_PROTOCOL Len=%d\n", rlen);
else	 printf("_PiccCascSelect=%d\n", rval);
#endif
	return rval;
}

// uid : uid[0]=length  UID[1..n]
int iso14443AActivate(unsigned char *uid, unsigned char *psak)
{
	unsigned char *p, reg_rx_conf2, sel_code, sak, temp[8];
	int		rval, len, level, no_complete;

#ifdef SC_DEBUG
	printf("iso14443AActivate...\n");
#endif
	as3911GetReg(AS3911_REG_RX_CONF2, &reg_rx_conf2);
//	as3911SetReg(AS3911_REG_RX_CONF2, reg_rx_conf2 & ~AS3911_REG_RX_CONF2_agc_en);
	rval = 0;
	p = uid + 1;
	level = len = 0;
	do {
		sel_code = ISO14443A_CMD_SELECT_CL1 + level + level;	// 0x93, 0x95, 0x97	
		rval = _PiccCascAnticoll(sel_code, temp);
		if(!rval) {
			rval = _PiccCascSelect(sel_code, temp, &sak);
			if(!rval) {
				level++;
				no_complete = sak & 0x04;
				if(no_complete) {	// cascaded bit(bit2)=1: UID not complete
					//this UID is cascaded, remove the cascaded tag that is 0x88 as first of the 4 byte received
					memcpy(p, temp+1, 3); len += 3; p += 3;
				} else {			// cascaded bit(bit2)=0: UID complete
					memcpy(p, temp, 4); len += 4; p += 4;
				}
			}
		}
	} while(!rval && no_complete && level < 3);
    as3911SetReg(AS3911_REG_RX_CONF2, reg_rx_conf2);		// restore agc setting
	// if cascade level is triple and sak indicates another cascase level.
	if(!rval && level == 3 && no_complete) rval = ERR_PROTOCOL;
	if(rval) len = 0;
	else	*psak = sak;
	uid[0] = len;
#ifdef SC_DEBUG
if(!rval) {
	printf("iso14443AActivate: Length=%d UID=[", len);
	for(level = 0;level < len;level++) printf("%02x", (int)uid[level+1]);
	printf("]\n");
} else	printf("iso14443AActivate=%d\n", rval);
#endif
	return rval;
}

// uid : uid[0]=length  UID[1..n]
int iso14443ASelect(unsigned char *uid, unsigned char *psak)
{
	unsigned char	*p, sel_code, sak, temp[8];
	int		rval, count, level, no_complete;;

#ifdef SC_DEBUG
	printf("iso14443ASelect: UID=["); for(status = 0;status < uid[0];status++) printf("%02x", (int)uid[status+1]); printf("]...\n");
#endif
	p = uid+1; count = uid[0];
	level = 0;
	do {
		sel_code = ISO14443A_CMD_SELECT_CL1 + level + level;	// 0x93, 0x95, 0x97	
		if(count == 4) {
			memcpy(temp, p, 4); count -= 4;
		} else {
			temp[0] = 0x88; memcpy(temp+1, p, 3); count -= 3;
		}
		rval = _PiccCascSelect(sel_code, temp, &sak);
		if(!rval) {
			level++;
			no_complete = sak & 0x04;
		}
	} while(!rval && count > 0 && no_complete && level < 3) ;
 	// if cascade level is triple and sak indicates another cascase level.
	if(!rval && (count || level == 3 && no_complete))  rval = ERR_PROTOCOL;
	if(!rval) *psak = sak;
	return rval;
}

int iso14443AHalt(void)
{
	unsigned char	buf[16];
	int		rval;

	buf[0] = ISO14443A_CMD_HLTA;
	buf[1] = 0;
	rval = as3911TransceiveData(buf, 2, NULL, 0, NULL, 0);
	// according to ISO14443-3 we should wait here for 1.1ms.
	// If any PICC responds within this time, HLTA command shall be interpreted as not acknowledged
//printf("iso14443AHalt=%d\n", rval);
	return rval;
}

int iso14443AActivateCard(unsigned char *uid, unsigned char *sak)
{
	unsigned char	buf[12];
	int		rval;

	iso14443ASendRequest(ISO14443A_CMD_REQA);
	do {
		rval = iso14443AReceiveRequest(buf);
		if(rval == ERR_CHIP || rval == ERR_TIMEOUT) break;
		else if(rval >= 0) rval = 0;
	} while(rval < 0) ;
	if(rval == 0) rval = iso14443AActivate(uid, sak);
	return rval;
}

int iso14443AToIdle(void)
{
	unsigned char	buf[8];
	int		rval;

	buf[0] = 0x60;		// unknown command
	buf[1] = 0;
	rval = as3911TransceiveData(buf, 2, NULL, 0, NULL, 0);
	return rval;
}

