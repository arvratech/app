#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "emv_standard.h"
#include "sc_iso14443b.h"


/*******************************************************************************
 * Receiver dead time (in carrier cycles) for the ISO14443B protocol.
 * See [CCP v2.3.1, PCD 4.8.1.3].
 *
 * 1008: TR0_MIN
 *
 *  340: Time from the rising edge of the EoS to the starting point of the MRT
 *       timer (sometime after the final high part of the EoS is completed).
 * 
 *   64: Adjustment for the MRT timer jitter. Currently the MRT timer will have
 *       any timeout between the set timeout and the set timout + 64 cycles.
 ******************************************************************************/
#define EMV_HAL_ISO14443B_RECEIVER_DEADTIME		(1008 - 340 - 64)

/*******************************************************************************
 * Adjustment from the timeout requested by the caller to the timeout
 * value which has to be used for the AS3911.
 ******************************************************************************/
#define EMV_HAL_TYPE_B_TIMEOUT_ADJUSTMENT		6360


// local definitions
#define ISO14443B_PARAM_APF 			0x5
#define ISO14443B_MAX_AC_LOOP_COUNT 	5

// Calculating with TR0min = TR1min = 16*16/fc => 8*64/fc
#define ISO14443B_MASK_RECEIVE_TIME			512 

// iso14443 TR0 + TR1 is 456/fs = (456*16)/fc = 114*64/fc
//#define ISO14443B_ACTIVATION_WAITING_TIME	150
#define ISO14443B_ACTIVATION_WAITING_TIME	3900

// iso14443 max FDT is ~5secs. However this is much too large for our USB 
// communication. Limit to 100ms = 21186 * 64/fc
#define ISO14443B_FRAME_DELAY_TIME  		21186

static ModulationLevelMode_enum emvHalTypeBModulationLevelMode = ML_FIXED;
static void *emvHalTypeBModulationLevelModeData = NULL;
static void * emvHalLowBGainTable = NULL;
static void * emvHalNormBGainTable = NULL;


void iso14443BOpen(void)
{
    unsigned char	buf[4];

	buf[0] = (opControl | AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en) & ~AS3911_REG_OP_CONTROL_wu;
   	buf[1] = AS3911_REG_MODE_om_iso14443b;	// Mode Definition Register: ISO14443B
	buf[2] = 0x00;		// Bit Rate definition Register: 106 kBit/s both direction
   	as3911SetRegs(AS3911_REG_OP_CONTROL, buf, 3);
	// AS3911_REG_AUX_tr_am=1(AM) set by ANALOG_PRESET command
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx | AS3911_REG_AUX_crc_2_fifo, 0);
/*
    // Set proper gain table for used technology
    as3911SetGainTables(emvHalLowBGainTable, emvHalNormBGainTable);
    // Enable dynamic adjustment of the modulation level
    as3911SetModulationLevelMode(emvHalTypeBModulationLevelMode, emvHalTypeBModulationLevelModeData);
    as3911AdjustModulationLevel();
*/
	as3911SetReg(AS3911_REG_AM_MOD_DEPTH_CONTROL, AS3911_REG_AM_MOD_DEPTH_CONTROL_mod_10percent);  // Set modulation depth to 10% 
	as3911CalibrateModulationDepth(buf);
	as3911SetMaskReceiveTime(EMV_HAL_ISO14443B_RECEIVER_DEADTIME);
	as3911SetFrameDelayTime(EMV_FDT_PCD_MIN);
	as3911ExecuteCommand(AS3911_CMD_ANALOG_PRESET);
//printf("iso14443BOpen...\n");
}

void iso14443BClose(void)
{
//	opControl &= ~(AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
	opControl &= ~(AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
	as3911SetReg(AS3911_REG_OP_CONTROL, (int)opControl);
//printf("iso14443BClose...\n");
}

int iso14443BSendRequest(iso14443BCommand_t cmd, int AFI)
{
	unsigned char	buf[4];
    int     rval;
 
	buf[0] = ISO14443B_PARAM_APF;	// APf: AntiCollision Prefix
	buf[1] = AFI;					// AFI:	Application Family Identifier
	buf[2] = cmd;           		// PARAM
	rval = as3911TransmitData(buf, 3, EMV_FWT_ATQB_PCD+6000);
//printf("SendRequest: %d\n", AFI);
    return rval;
}

extern unsigned char    fsci, sfgi, fwi, bno;

int iso14443BReceiveRequest(unsigned char *pupi)
{
	unsigned char	buf[12];
	int		rval, len;

	len = 0;
	rval = as3911PollData(buf, 12, &len);
	if(rval == 0) {
		if(len != 12 || buf[0] != 0x50) rval = ERR_PROTOCOL;
		else {
			memcpy(pupi, buf+1, 4);
            fsci = buf[10] >> 4; fwi = buf[11] >> 4;
		}
	}
if(!rval) printf("ReceiveRequest: OK %02x-%02x-%02X-%02X\n", (int)pupi[0], (int)pupi[1], (int)pupi[2], (int)pupi[3]);
else if(rval == ERR_COLLISION) printf("ReceiveRequest: Collision\n");
else if(rval == ERR_PARITY) printf("ReceiveRequest: Parity\n");
else if(rval == ERR_HARD_FRAMING) printf("ReceiveRequest: HardFraming\n");
else if(rval == ERR_SOFT_FRAMING) printf("ReceiveRequest: SoftFraming\n");
else if(rval == ERR_TIMEOUT) ; // printf("ReceiveRequest: Timeout\n");
else if(rval > 0) printf("ReceiveRequest: %d\n", rval);
	return rval;
}

int iso14443BAntiCollsion(iso14443BCommand_t cmd, iso14443BSlotCount_t slotCount, int afi, unsigned char *pupi)
{
	unsigned char	txbuf[4], rxbuf[16];
	int		rval, len;

	txbuf[0] = ISO14443B_PARAM_APF;	// APf: AntiCollision Prefix
	txbuf[1] = afi;					// AFI:	Application Family Identifier
	txbuf[2] = slotCount | cmd;		// PARAM
printf("[%02x-%02x-%02x]\n", (int)txbuf[0], (int)txbuf[1], (int)txbuf[2]);
	rval = as3911TransceiveData(txbuf, 3, rxbuf, 12, &len, EMV_FWT_ATQB_PCD+9000);
	if(!rval) {
		if(len != 12 || rxbuf[0] != 0x50) rval = ERR_PROTOCOL;
		else {
			memcpy(pupi, rxbuf+1, 4);
            fsci = rxbuf[10] >> 4; fwi = rxbuf[11] >> 4;
		}
	}
	return rval;
}

// err = iso14443BSendApfAndGetResult(cmd, card, afi, ISO14443B_SLOT_COUNT_1);
int iso14443BSlotMarker(int slotNumber, unsigned char *pupi)
{
	unsigned char	txbuf[8], rxbuf[16];
	int		rval, len;
 
	
	txbuf[0] = ISO14443B_PARAM_APF | ((slotNumber-1) << 4);
	rval = as3911TransceiveData(txbuf, 1, rxbuf, 12, &len, EMV_FWT_ATQB_PCD);
	if(!rval) {
		if(len != 12 || rxbuf[0] != 0x50) rval = ERR_PROTOCOL;
		else {
			memcpy(pupi, rxbuf+1, 4);
            fsci = rxbuf[10] >> 4; fwi = rxbuf[11] >> 4;
		}
	}
	return rval;
}

int iso14443BHalt(unsigned char *pupi)
{
	unsigned char	txbuf[8], rxbuf[8];
	int		rval, rlen;
 
	
	txbuf[0] = ISO14443B_CMD_HLTB;
	memcpy(txbuf+1, pupi, 4);
	txbuf[1] = 0x70;
	rval = as3911TransceiveData(txbuf, 6, rxbuf, 1, &rlen, ISO14443B_ACTIVATION_WAITING_TIME);
	if(!rval && (rlen != 1 || rxbuf[0])) rval = ERR_PROTOCOL;
	return rval;
}


int iso14443BEnterProtocolMode(unsigned char *pupi)
{
	unsigned char	txbuf[12], rxbuf[12];
	int		rval, len;    

	txbuf[0] = ISO14443B_CMD_ATTRIB;
	memcpy(txbuf+1, pupi, 4);
	txbuf[5] = 0x00;		// Param 1
	txbuf[6] = 0x08;		// Param 2  maximum frame size that can be received by PCD : 8=256
	txbuf[7] = 0x01;    	// Param 3
	txbuf[8] = 0x00;		// CID
	rval = as3911TransceiveData(txbuf, 9, rxbuf, 1, &len, ISO14443B_ACTIVATION_WAITING_TIME);
	if(!rval && len != 1) rval = ERR_PROTOCOL;
	return rval;
}
