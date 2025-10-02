#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "emv_standard.h"
#include "as3911.h"

unsigned char	opControl;

int as3911Open(void)
{
	unsigned long	status;
	unsigned char	val;
	int		rval, vdd_mV;

	// first, reset the as3911, clear op control to have clean startup
	as3911SetReg(AS3911_REG_OP_CONTROL, 0x00);
	as3911ExecuteCommand(AS3911_CMD_SET_DEFAULT);
	as3911ModReg(AS3911_REG_IO_CONF2, 0, AS3911_REG_IO_CONF2_miso_pd2 | AS3911_REG_IO_CONF2_miso_pd1);	// enable pull downs on miso line
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);	// after reset all interrupts are enabled. so disable them at first
	as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);		// and clear them, just to be sure...
	as3911SetReg(AS3911_REG_ANT_CAL_TARGET, 0x80);	// trim settings for most of our boards is 90degrees
	as3911EnableInterrupts(AS3911_IRQ_MASK_OSC);	// enable oscillator frequency stable interrupt
	as3911ModReg(AS3911_REG_OP_CONTROL, 0, AS3911_REG_OP_CONTROL_en);	// enable oscillator and regulator output
	// wait for the oscillator interrupt, max. 10ms
	status = as3911WaitInterrupts(AS3911_IRQ_MASK_OSC, 20);
	if(!status) printf("as3911WaitInterrupt: zero\n");
	as3911DisableInterrupts(AS3911_IRQ_MASK_OSC);
	if(!status) return ERR_TIMEOUT;
	// measure vdd and set sup3V bit(0:5V 1:3.3V supply) accordingly
	vdd_mV = as3911MeasureVoltage(AS3911_REG_REGULATOR_CONTROL_mpsv_vdd);
printf("MeasureVoltage: vdd_mv=%d\n", vdd_mV);
	if(vdd_mV < 4000) as3911ModReg(AS3911_REG_IO_CONF2, 0, AS3911_REG_IO_CONF2_sup3V);
	else	as3911ModReg(AS3911_REG_IO_CONF2, AS3911_REG_IO_CONF2_sup3V, 0);
	// TODO: this should be adapted for the different boards
	as3911SetFirstStageGainReduction(6, 6);	// Gain redution: AM channel=0 PM channel=6
//as3911SetRxChannel(1);			// AM channel only enabled
	as3911ModReg(AS3911_REG_OP_CONTROL, 0, AS3911_REG_OP_CONTROL_rx_man);	// No automatic channel selection, select AM(default), most often better
	rval = as3911AdjustRegulators(&vdd_mV);		// Adjust the regulators first
printf("AdjustRegulators=%d: vdd_mV=%d\n", rval, vdd_mV);
	as3911CalibrateAntenna(&val);			// Calibrating the antenna might change the power consumption
	rval = as3911AdjustRegulators(&vdd_mV);	// Adjust the regulators again since the power consumption may have changed
printf("AdjustRegulators=%d: vdd_mV=%d\n", rval, vdd_mV);
	return 0;
}

void as3911Close(void)
{
//  AS3911_IRQ_OFF();
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
	as3911SetReg(AS3911_REG_OP_CONTROL, 0x00);
}

// Sanity timeout during an AS3911 transmit operation (milliseconds).
// If no interrupt is signaled by the AS3911 within this timeframe during a tranmsit
// operation then a severe software or hardware failure has to be asumed.
#define AS3911_TX_IRQ_SANITY_TIMEOUT		10

// Sanity timeout during an AS3911 receive operation (milliseconds).
// If no interrupt is signaled by the AS3911 within this timeframe during a receive
// operation then a severe software or hardware failure has to be asumed.
#define AS3911_RX_IRQ_SANITY_TIMEOUT		6000

// Sanity timeout during AS3911 receive operation if rxs was already detected.
// After 10ms@106kbps we must have more than filled the FIFO and water level
// must have been signaled 
#define AS3911_RX_IRQ_SANITY_TIMEOUT_AFTER_RXS	10

// Time offset between the start of a PICC response and the start of receive 
// interrupt (carrier cycles).
#define AS3911_SOR_IRQ_OFFSET				640

// Maximum size of a continuous FIFO read operation inside the irq handler.
// Limiting the fifo read size is necessary to enable proper handling of EMV 
// transmission error recovery. A complete FIFO read time would take to long and
// exceed the maximum response time to a transmission error as defined by the EMV standard.
#define AS3911_IRQ_FIFO_READ_CHUNK_SIZE		10

// Bitmask for the rxon bit of the auxiliary display register of the AS3911
#define AS3911_AUX_DISPLAY_RXON_BIT			0x08

// Minimum delay between a received PICC message and the next PCD message (carrier cycles).
static u32 as3911FrameDelayTime = 0;

// Indicates whether a reception is currently in progress (TRUE) or not (FALSE).
static volatile bool_t as3911ReceptionInProgress = FALSE;

// \ToDo: Remove this variable. It is no longer used.
static volatile bool_t as3911TransmissionInProgress = FALSE;

/*******************************************************************************
 * \brief Enable exception processing according to the EMV standard.
 *
 * If \a as3911EmvExceptionProcessing is set to TRUE, then the receiver will be
 * reenabled  and the received data ignored if certain combinations of receive
 * errors occure.
 ******************************************************************************/
static bool_t as3911EmvExceptionProcessing = FALSE;

/*******************************************************************************
 * This variable is only considered if emv_exception_processing is enabled.
 *
 * If a frame is received with a data encoding (parity), or crc error
 * AND the length of the frame is <=  \a as3911TransmissionErrorThreshold then
 * the receieved frame is asumed to be noise and the receiver is reseted to
 * receive another frame.
 *********************************************************************/
static int as3911TransmissionErrorThreshold = 0;

void as3911EnableEmvExceptionProcessing(bool_t enableEmvExceptionProcessing)
{
    as3911EmvExceptionProcessing = enableEmvExceptionProcessing;
}

bool_t as3911EmvExceptionProcessingIsEnabled()
{
    return as3911EmvExceptionProcessing;
}

void as3911SetTransmissionErrorThreshold(int transmissionErrorThreshold)
{
    as3911TransmissionErrorThreshold = transmissionErrorThreshold;
}

int as3911GetTransmissionErrorThreshold()
{
    return as3911TransmissionErrorThreshold;
}

void as3911SetMaskReceiveTime(int maskReceiveTime)
{
	unsigned char	regMode;
	int		mrtValue;

	as3911GetReg(AS3911_REG_MODE, &regMode);
	if((regMode & AS3911_REG_MODE_mask_om) == AS3911_REG_MODE_om_nfc) {	// NFCIP1 mode
		mrtValue = (maskReceiveTime >> 9) & 0xff;
		if(maskReceiveTime & 0x01ff) mrtValue++;
	} else {	// Non NFCIP1 mode
		mrtValue = (maskReceiveTime >> 6) & 0xff;
		if(maskReceiveTime & 0x003f) mrtValue++;
	}
	as3911SetReg(AS3911_REG_MASK_RX_TIMER, mrtValue);
}

/// Unit: 1/fc=1/13.564=0.07372us
void as3911SetNoResponseTime(int noResponseTime)
{
	int		nrtValue;

 	if(noResponseTime > 0x003fffc0) {
		nrtValue = noResponseTime >> 12;	// Use 1:4096 prescaler
		if(noResponseTime & 0x0fff) nrtValue++;
		as3911ModReg(AS3911_REG_GPT_CONTROL, AS3911_REG_GPT_CONTROL_nrt_emv, AS3911_REG_GPT_CONTROL_nrt_step);
	} else {
		nrtValue = noResponseTime >> 6;		// Use 1:64 prescaler
		if(noResponseTime & 0x03f) nrtValue++;
		as3911ModReg(AS3911_REG_GPT_CONTROL, AS3911_REG_GPT_CONTROL_nrt_emv | AS3911_REG_GPT_CONTROL_nrt_step, 0);
	}
	as3911SetReg(AS3911_REG_NO_RESPONSE_TIMER1, nrtValue >> 8);
	as3911SetReg(AS3911_REG_NO_RESPONSE_TIMER2, nrtValue & 0xff);
}

/// Unit: 1/fc=1/13.564MHz=0.07372us
void as3911SetFrameDelayTime(int frameDelayTime)
{
	int		gptValue;

//	as3911FrameDelayTime = frameDelayTime;
	as3911ModReg(AS3911_REG_GPT_CONTROL, AS3911_REG_GPT_CONTROL_gptc_mask, AS3911_REG_GPT_CONTROL_gptc_no_trigger);
	gptValue = frameDelayTime >> 3;
	as3911SetReg(AS3911_REG_GPT1, gptValue >> 8);
	as3911SetReg(AS3911_REG_GPT2, gptValue & 0xff);
}

static int _as3911FdtIsRunning()
{
    unsigned char	val;

    as3911GetReg(AS3911_REG_REGULATOR_RESULT, &val);
    if(val & AS3911_REG_REGULATOR_RESULT_gpt_on) return 1;	// General purpose timer is running
    else	return 0;
}

//    Fucntion            no_ta_par  no_rx_par  anctl  no_crc_rx  crc_2_fifo       Command
// TransceiveBitData          0          0        1        1          0      TRANSMIT_WITHOUT_CRC
// TransceiveRawBitData       1          1        0        1          0      TRANSMIT_WITHOUT_CRC
// TransceiveData             0          0        0        0          0      TRANSMIT_WITH_CRC

static int _as3911TransmitBitData(unsigned char *buf, int bitLen)
{
	unsigned long	status;	
	int		val, len, size, count, mask;
 
 	as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);
	as3911ExecuteCommand(AS3911_CMD_CLEAR_RX_GAIN);
	as3911SetReg(AS3911_REG_NUM_TX_BYTES1, (bitLen >> 8) & 0xff);
	as3911SetReg(AS3911_REG_NUM_TX_BYTES2, bitLen & 0xff);
	len = (bitLen + 7) >> 3;
	if(len > AS3911_FIFO_SIZE) count = AS3911_FIFO_SIZE;
	else	count = len;
	as3911WriteFifo(buf, count);
	size = count;
	as3911ExecuteCommand(AS3911_CMD_TRANSMIT_WITHOUT_CRC);
	while(1) {
		mask = AS3911_IRQ_MASK_TXE;
        count = len - size;
		if(count > 0) mask |= AS3911_IRQ_MASK_WL;
		status = as3911WaitInterrupts(mask, AS3911_TX_IRQ_SANITY_TIMEOUT);
        if(!(status & AS3911_IRQ_MASK_WL)) break;
        if(count > 0) {
			val = AS3911_FIFO_SIZE - AS3911_FIFO_TRANSMIT_WL0;
			if(count > val) count = val;
			as3911WriteFifo(buf+size, count);
			size += count;
    	}
    }
    if(status) val = 0;
    else	val = ERR_CHIP;
    return val;	
}

static int _as3911ReceiveBitData(unsigned char *buf, int maxLen, int *bitLen)
{
	unsigned long	status;
	unsigned char	*p, fifoStatus[2], tmpbuf[64];	
    int		rval, count, size, timeout, lastbits, overflow;

	size = 0; overflow = 0; timeout = 90;
	while(1) {
		status = as3911WaitInterrupts(AS3911_IRQ_MASK_WL | AS3911_IRQ_MASK_RXE | AS3911_IRQ_MASK_NRE, timeout);	
		if(status != AS3911_IRQ_MASK_WL) break;
		as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
		if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
		count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
		if(size + count > maxLen) p = tmpbuf;
		else	p = buf + size;
		as3911ReadFifo(p, count);
		if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
		size += count;
		timeout = 10;
	}
	if(status & AS3911_IRQ_MASK_RXE) {
		as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
		if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
		lastbits = 0;
		count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
		if(count) {
			if(size + count > maxLen) p = tmpbuf;
			else	p = buf + size;
			as3911ReadFifo(p, count);
			if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
			size += count;
			if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ncp) {
				lastbits = (fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_mask_fifo_lb) >> AS3911_REG_FIFO_RX_STATUS2_shift_fifo_lb;
				size--;
			}
		}
		*bitLen = (size << 3) + lastbits;
		if(overflow) rval = ERR_OVERFLOW;
		else if(size > maxLen) rval = ERR_DATA_OVER;		
		else	rval = 0;
	} else if(status & AS3911_IRQ_MASK_NRE) {
		rval= ERR_TIMEOUT;
	} else { 	
		rval = ERR_CHIP;
	}
	return rval; 
}

int as3911TransceiveBitData(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout)
{
	unsigned long	status;
	int		rval;

	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_crc_2_fifo, AS3911_REG_AUX_no_crc_rx);
	as3911ModReg(AS3911_REG_ISO14443A_NFC, 0, AS3911_REG_ISO14443A_NFC_antcl);
//	as3911AdjustModulationLevel();	// Adjust modulation level
	while(_as3911FdtIsRunning()) ;	// Wait for the frame delay time to pass
    as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
    as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
	if(maxLen > 0) as3911SetNoResponseTime(timeout);
	rval = _as3911TransmitBitData(txBuf, txBitLen);
	if(!rval && maxLen > 0) {
		if(maxLen > 0) {
	    	rval = _as3911ReceiveBitData(rxBuf, maxLen, rxBitLen);
			if(rval != ERR_CHIP && rval != ERR_TIMEOUT) {
				as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD
				status = as3911GetInterrupts(AS3911_IRQ_MASK_COL | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
				if(!rval) {
					if(status & AS3911_IRQ_MASK_COL) rval = ERR_COLLISION;
					else if(status & AS3911_IRQ_MASK_PAR) rval = ERR_PARITY;
					else if(status & AS3911_IRQ_MASK_ERR1) rval = ERR_HARD_FRAMING;
					else if(status & AS3911_IRQ_MASK_ERR2) rval = ERR_SOFT_FRAMING;
					else if(*rxBitLen > (maxLen << 3)) rval = ERR_OVERFLOW;
				}
			}
		} else {
			as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD			
		}
	}
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
    return rval;
}

int as3911TransceiveRawBitData(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout)
{
 	unsigned long	status;
	int		rval;

	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, 0);
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_crc_2_fifo, AS3911_REG_AUX_no_crc_rx);
	as3911ModReg(AS3911_REG_ISO14443A_NFC, 0, AS3911_REG_ISO14443A_NFC_no_tx_par | AS3911_REG_ISO14443A_NFC_no_rx_par);
//	as3911AdjustModulationLevel();	// Adjust modulation level
	while(_as3911FdtIsRunning()) ;	// Wait for the frame delay time to pass
    as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
    as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
    if(maxLen > 0) as3911SetNoResponseTime(timeout);	
	rval = _as3911TransmitBitData(txBuf, txBitLen);
	if(!rval) {
		if(maxLen > 0) {
	    	rval = _as3911ReceiveBitData(rxBuf, maxLen, rxBitLen);
			if(rval != ERR_CHIP && rval != ERR_TIMEOUT) {
				as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD
				status = as3911GetInterrupts(AS3911_IRQ_MASK_COL | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
				if(!rval) {
					if(status & AS3911_IRQ_MASK_COL) rval = ERR_COLLISION;
					else if(status & AS3911_IRQ_MASK_ERR1) rval = ERR_HARD_FRAMING;
					else if(status & AS3911_IRQ_MASK_ERR2) rval = ERR_SOFT_FRAMING;
					else if(*rxBitLen > (maxLen << 3)) rval = ERR_OVERFLOW;
				}
			}
		} else {
			as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD			
		}
	}		
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_no_tx_par | AS3911_REG_ISO14443A_NFC_no_rx_par, 0);
    return rval;
}

int as3911TransceiveStream(unsigned char *txBuf, int txBitLen, unsigned char *rxBuf, int maxLen, int *rxBitLen, int timeout)
{
	unsigned long	status;
	int		rval;

	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, 0);
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_crc_2_fifo, AS3911_REG_AUX_no_crc_rx);
	as3911ModReg(AS3911_REG_ISO14443A_NFC, 0, AS3911_REG_ISO14443A_NFC_no_tx_par | AS3911_REG_ISO14443A_NFC_no_rx_par);
//	as3911AdjustModulationLevel();	// Adjust modulation level
	while(_as3911FdtIsRunning()) ;	// Wait for the frame delay time to pass
    as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
    as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
    if(maxLen > 0) as3911SetNoResponseTime(timeout);	
	rval = _as3911TransmitBitData(txBuf, txBitLen);
	if(!rval) {
		if(maxLen > 0) {
			rval = _as3911ReceiveBitData(rxBuf, maxLen, rxBitLen);
			if(rval != ERR_CHIP && rval != ERR_TIMEOUT) {
				as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD
				status = as3911GetInterrupts(AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
				if(!rval && *rxBitLen > (maxLen << 3)) rval = ERR_OVERFLOW;
			}
		} else {
			as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD			
		}
	}
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_no_tx_par | AS3911_REG_ISO14443A_NFC_no_rx_par, 0);
    return rval;
}

static int _as3911TransmitData(unsigned char *buf, int len)
{
	unsigned long	status;
	int		val, size, count, mask;

	as3911ExecuteCommand(AS3911_CMD_CLEAR_FIFO);
	as3911ExecuteCommand(AS3911_CMD_CLEAR_RX_GAIN);
	as3911SetReg(AS3911_REG_NUM_TX_BYTES1, (len >> 5) & 0xff);
	as3911SetReg(AS3911_REG_NUM_TX_BYTES2, (len << 3) & 0xff);
	if(len > AS3911_FIFO_SIZE) count = AS3911_FIFO_SIZE;
	else	count = len;
	as3911WriteFifo(buf, count);
	size = count;
	as3911ExecuteCommand(AS3911_CMD_TRANSMIT_WITH_CRC);	// AS3911_CMD_TRANSMIT_WITHOUT_CRC;
	while(1) {
		mask = AS3911_IRQ_MASK_TXE;
		count = len - count;
		if(count > 0) mask |= AS3911_IRQ_MASK_WL;
		status = as3911WaitInterrupts(mask, AS3911_TX_IRQ_SANITY_TIMEOUT);
        if(!(status & AS3911_IRQ_MASK_WL)) break;
        if(count > 0) {
			val = AS3911_FIFO_SIZE - AS3911_FIFO_TRANSMIT_WL0;
			if(count > val) count = val;
			as3911WriteFifo(buf+size, count);
			size += count;
    	}
    }
    if(status) val = 0;
    else	val = ERR_CHIP;
    return val;
}

static int _as3911ReceiveData(unsigned char *buf, int maxLen, int *len, int timeout)
{
	unsigned long	status, timer;
	unsigned char	*p, fifoStatus[2], tmpbuf[64];
	int		rval, size, count, overflow, ms, duration;

	ms = (timeout + 13563) / 13564;
	ms += 5;
	size = 0; overflow = 0;
	timer = SC_TIMER;
	while(1) {
		count = SC_TIMER - timer;
		count = ms - count;
		if(count < 1) break;
		status = as3911WaitInterrupts(AS3911_IRQ_MASK_WL | AS3911_IRQ_MASK_RXE | AS3911_IRQ_MASK_NRE, count);
		if(status != AS3911_IRQ_MASK_WL) break;
		as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
		if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
		count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
		if(size + count > maxLen) p = tmpbuf;
		else	p = buf + size;
		as3911ReadFifo(p, count);
		if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
		size += count;
	}
	if(status & AS3911_IRQ_MASK_RXE) {
		as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
		if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
		count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
		if(count) {
			if(size + count > maxLen) p = tmpbuf;
			else	p = buf + size;
			as3911ReadFifo(p, count);
			if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
			size += count;
		}
		*len = size;
		if(overflow) rval = ERR_OVERFLOW;
		else if(fifoStatus[1] & (AS3911_REG_FIFO_RX_STATUS2_fifo_ncp | AS3911_REG_FIFO_RX_STATUS2_np_lb)) rval = ERR_SOFT_FRAMING;
		else if(size > maxLen) rval = ERR_DATA_OVER;
		else	rval = 0;
	} else if(status & AS3911_IRQ_MASK_NRE) {
		rval= ERR_TIMEOUT;
	} else {
		rval = ERR_CHIP;
	}
	return rval; 
}

int as3911TransceiveData(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int maxLen, int *rxLen, int timeout)
{
	unsigned long	status;
	int		rval;
//unsigned char	val[4];
	
	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, 0);
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx | AS3911_REG_AUX_crc_2_fifo, 0);
//	as3911AdjustModulationLevel();	// Adjust modulation level
	while(_as3911FdtIsRunning()) ;	// Wait for the frame delay time to pass
    as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
    as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
	as3911SetNoResponseTime(timeout);
	rval = _as3911TransmitData(txBuf, txLen);
	if(!rval && maxLen > 0) {
    	rval = _as3911ReceiveData(rxBuf, maxLen, rxLen, timeout);
		if(rval != ERR_CHIP && rval != ERR_TIMEOUT) {
			as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD
			status = as3911GetInterrupts(AS3911_IRQ_MASK_CRC | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
			if(!rval) {
				if(status & AS3911_IRQ_MASK_ERR1) rval = ERR_HARD_FRAMING;
				else if(status & AS3911_IRQ_MASK_CRC) rval = ERR_CRC;
				else if(status & AS3911_IRQ_MASK_PAR) rval = ERR_PARITY;
				else if(status & AS3911_IRQ_MASK_ERR2) rval = ERR_SOFT_FRAMING;
				else if(*rxLen > maxLen) rval = ERR_OVERFLOW;
			}
		}
	}
//as3911GetRegs(AS3911_REG_RSSI_RESULT, val, 2);
//printf("RSSI=%02x GainRedu=%02x\n", (int)val[0], (int)val[1]);
	as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
    return rval;
}

int as3911TransmitData(unsigned char *buf, int len, int timeout)
{
    int		rval;
	
	as3911ModReg(AS3911_REG_ISO14443A_NFC, AS3911_REG_ISO14443A_NFC_antcl, 0);
	as3911ModReg(AS3911_REG_AUX, AS3911_REG_AUX_no_crc_rx | AS3911_REG_AUX_crc_2_fifo, 0);
//	as3911AdjustModulationLevel();	// Adjust modulation level
	while(_as3911FdtIsRunning()) ;	// Wait for the frame delay time to pass
    as3911ClearInterrupts(AS3911_IRQ_MASK_ALL);
    as3911EnableInterrupts(AS3911_IRQ_MASK_ALL);
	as3911SetNoResponseTime(timeout);
	rval = _as3911TransmitData(buf, len);
    return rval;
}

// Return 	-1:Continue	 0:OK  >=1:Error
int as3911PollData(unsigned char *buf, int maxLen, int *len)
{
	unsigned long	status;
	unsigned char	*p, fifoStatus[2], tmpbuf[64];
	int		rval, size, count, overflow;

   	overflow = 0;
	status = as3911WaitInterrupts(AS3911_IRQ_MASK_WL | AS3911_IRQ_MASK_RXE | AS3911_IRQ_MASK_NRE, 90);
	rval = -1;
	if(status) {
		size = *len;
		if(status == AS3911_IRQ_MASK_WL) {
			as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
			if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
			count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;
			if(size + count > maxLen) p = tmpbuf;
			else	p = buf + size;
			as3911ReadFifo(p, count);
			if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
			size += count;
		} else if(status & AS3911_IRQ_MASK_RXE) {	
			as3911GetRegs(AS3911_REG_FIFO_RX_STATUS1, fifoStatus, 2);
			if(fifoStatus[1] & AS3911_REG_FIFO_RX_STATUS2_fifo_ovr) overflow = 1;
			count = fifoStatus[0] & AS3911_REG_FIFO_RX_STATUS1_mask_fifo_b;	// bug fix: inserted at 2021.10.12
			if(count) {
				if(size + count > maxLen) p = tmpbuf;
				else	p = buf + size;
				as3911ReadFifo(p, count);
				if(size < maxLen && size + count > maxLen) memcpy(buf+size, p, maxLen-size);
				size += count;
			}
			*len = size;
			if(overflow) rval = ERR_OVERFLOW;
			else if(fifoStatus[1] & (AS3911_REG_FIFO_RX_STATUS2_fifo_ncp | AS3911_REG_FIFO_RX_STATUS2_np_lb)) rval = ERR_SOFT_FRAMING;
			else if(size > maxLen) rval = ERR_DATA_OVER;
			else {
				status = as3911GetInterrupts(AS3911_IRQ_MASK_CRC | AS3911_IRQ_MASK_PAR | AS3911_IRQ_MASK_ERR1 | AS3911_IRQ_MASK_ERR2);
				if(status & AS3911_IRQ_MASK_ERR1) rval = ERR_HARD_FRAMING;
				else if(status & AS3911_IRQ_MASK_CRC) rval = ERR_CRC;
				else if(status & AS3911_IRQ_MASK_PAR) rval = ERR_PARITY;
				else if(status & AS3911_IRQ_MASK_ERR2) rval = ERR_SOFT_FRAMING;
				else	rval = 0;
			}
			as3911ExecuteCommand(AS3911_CMD_START_GP_TIMER);	// Start GPT timer used for FDT_PCD
		} else if(status & AS3911_IRQ_MASK_NRE) {
			rval = ERR_TIMEOUT;
		}
		if(rval >= 0) as3911DisableInterrupts(AS3911_IRQ_MASK_ALL);
	} else {
		rval = ERR_CHIP;
	}
	return rval;
}

void as3911ExecuteCommandAndGetResult(int cmd, int resreg, int sleeptime, unsigned char *result)
{
	unsigned long	irqs;

	if(cmd == AS3911_CMD_ADJUST_REGULATORS) {
		as3911ExecuteCommand(cmd);
//		sleepMilliseconds(sleeptime);
	} else if((cmd >= AS3911_CMD_INITIAL_RF_COLLISION && cmd <= AS3911_CMD_RESPONSE_RF_COLLISION_0)
					|| (cmd == AS3911_CMD_MEASURE_AMPLITUDE)
					|| (cmd >= AS3911_CMD_ADJUST_REGULATORS && cmd <= AS3911_CMD_MEASURE_PHASE)
					|| (cmd >= AS3911_CMD_CALIBRATE_C_SENSOR && cmd <= AS3911_CMD_MEASURE_VDD)
					|| (cmd >= 0xFD && cmd <= 0xFE)) {
			as3911EnableInterrupts(AS3911_IRQ_MASK_DCT);
			irqs = as3911GetInterrupts(AS3911_IRQ_MASK_DCT);
			as3911ExecuteCommand(cmd);
			irqs = as3911WaitInterrupts(AS3911_IRQ_MASK_DCT, sleeptime);
			as3911DisableInterrupts(AS3911_IRQ_MASK_DCT);
	} else {
		as3911ExecuteCommand(cmd);
//		sleepMilliseconds(sleeptime);
    }
	// read out the result if the pointer is not NULL
	if(result) as3911GetReg(resreg, result);
}

void as3911OnField(void)
{
	as3911ModReg(AS3911_REG_OP_CONTROL, 0, AS3911_REG_OP_CONTROL_tx_en);
	as3911AdjustModulationLevel();
 }

void as3911OffField(void)
{
	unsigned char	reg;

	as3911GetReg(AS3911_REG_RFO_AM_OFF_LEVEL, &reg);
	as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, 0xff);
	as3911ModReg(AS3911_REG_OP_CONTROL, AS3911_REG_OP_CONTROL_tx_en, 0);
	// Softly re-enable drivers avoid microscopic ringing
	as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, 0xfe);
	as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, 0xfc);
	// Restore previous value of AS3911_REG_RFO_AM_OFF_LEVEL
	as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, reg);
}

// rfoLevel:  0  1  2  3  4  5  6  7  8  9 10 11 12 13 14
//			 ff fe fc f8 f0 e0 c0 80 40 20 10 08 04 02 01
void as3911SetRfoNormalLevel(int rfoLevel)
{
	int		val;

	val = rfoLevel;
	if(val < 8) val = (0xff << val) & 0xff;
	else	val = 0x40 >> (val-8);
printf("rfoLevel=%d 0x%02x\n", rfoLevel, val);
	as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, val);
}

int as3911AdjustRegulators(int *result_mV)
{
    unsigned char	result, io_conf2;

	// first check the status of the reg_s bit in AS3911_REG_VREG_DEF register.
	// if this bit is set adjusting the regulators is not allowed
	as3911GetReg(AS3911_REG_REGULATOR_CONTROL, &result);
	if(result & AS3911_REG_REGULATOR_CONTROL_reg_s) return ERR_REQUEST;
	as3911ExecuteCommandAndGetResult(AS3911_CMD_ADJUST_REGULATORS, AS3911_REG_REGULATOR_RESULT, 5, &result);
	as3911GetReg(AS3911_REG_IO_CONF2, &io_conf2);
	result >>= AS3911_REG_REGULATOR_RESULT_shift_reg;
	result -= 5;
	if(result_mV) {
		if(io_conf2 & AS3911_REG_IO_CONF2_sup3V) {
			*result_mV = 2400;
			*result_mV += result * 100;
		} else {
			*result_mV = 3900;
			*result_mV += result * 120;
		}
	}
	return 0;
}

void as3911MeasureRF(unsigned char *result)
{
	as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_AMPLITUDE, AS3911_REG_AD_RESULT, 10, result);
printf("[%d]", (int)*result);
	
//	as3911GetReg(AS3911_REG_AMPLITUDE_MEASURE_RESULT, buf);
//printf("[%d]", (int)buf[0]);
//	as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_AMPLITUDE, AS3911_REG_AD_RESULT, 10, result);
//	as3911GetReg(0x35, buf+1);
//printf("Amplitude: %d %d\n", (int)*result, (int)buf[1]);
}

void as3911MeasureAntennaResonance(unsigned char *result)
{
	as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_PHASE, AS3911_REG_AD_RESULT, 10, result);
printf("measure Phase: 0x%02x\n", (int)*result);
}

void as3911CalibrateAntenna(unsigned char *result)
{
	unsigned char	val;
	int		err;

	// ERRATA ID 14: Run Calibrate Antenna always twice
	as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_ANTENNA, AS3911_REG_ANT_CAL_RESULT, 10, &val);
if(val & 0x08) err = 1; else err = 0;
printf("Calibrate Antenna Result: trim=%01x trim_err=%d\n", (int)(val >> 4), err);
	as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_ANTENNA, AS3911_REG_ANT_CAL_RESULT, 10, &val);
if(val & 0x08) err = 1; else err = 0;
printf("Calibrate Antenna Result: trim=%01x trim_err=%d\n", (int)(val >> 4), err);
	*result = val;
}

void as3911CalibrateModulationDepth(unsigned char *result)
{
    as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_MODULATION, AS3911_REG_AM_MOD_DEPTH_RESULT, 10, result);
//printf("Calibrate Modulation Depth Result: 0x%02x\n", (int)*result);
}

int as3911MeasureVoltage(int mpsv)
{
	unsigned char	result;
	int		mV;

    mpsv &= AS3911_REG_REGULATOR_CONTROL_mask_mpsv;
    as3911ModReg(AS3911_REG_REGULATOR_CONTROL, AS3911_REG_REGULATOR_CONTROL_mask_mpsv, mpsv);
    as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_VDD, AS3911_REG_AD_RESULT, 100, &result);
    mV = result * 23;
    mV += ((result * 438) + 500) / 1000;
    return mV;
}

// VoltageVal	 5    6    7    8    9    10   11   12   13   14   15
// 5.0V Mode	3.90 4.02 4.14 4.26 4.38 4.50 4.62 4.74 4.86 4.98 5.10
// 3.3V Mode	2.4  2.5  2.6  2.7  2.8  2.9  3.0  3.1  3.2  3.3  3.4  
void as3911SetVoltage(int VoltVal)
{
	as3911ModReg(AS3911_REG_REGULATOR_CONTROL, 0xf8, 0x80 | (VoltVal << 3));
}	

// 0:Both 1:AM  2:PM
void as3911SetRxChannel(int Mode)
{
	int		val;
	
	if(Mode) val = AS3911_REG_OP_CONTROL_rx_chn; else val = 0;
	as3911ModReg(AS3911_REG_OP_CONTROL, AS3911_REG_OP_CONTROL_rx_chn | AS3911_REG_OP_CONTROL_rx_man, val);
	if(Mode == 2) val = AS3911_REG_RX_CONF1_ch_sel; else val = 0;
	as3911ModReg(AS3911_REG_RX_CONF1, AS3911_REG_RX_CONF1_ch_sel, val);
}

//     0     1    2   3   4    5    6       7
//          2.5  5.0 7.5 10.0 12.5 15.0   
// Full gain     Gain reduction(dB)    Boost +5.5dB
void as3911SetFirstStageGainReduction(int AMVal, int PMVal)
{
	as3911ModReg(AS3911_REG_RX_CONF3, 0xfc, ((AMVal & 0x07) << 5) | ((PMVal & 0x07) << 2));
}

void as3911SetSecondStageGainReduction(int AMVal, int PMVal)
{
	as3911SetReg(AS3911_REG_RX_CONF4, ((AMVal & 0x0f) << 4) | (PMVal & 0x0f));
    as3911ExecuteCommand(AS3911_CMD_CLEAR_RX_GAIN);
}

#define EMV_MAX_RETRY				2		// Maximum number of retransmission for the layer 4 protocol
#define EMV_MAX_CONSECUTIVE_SWTX_AFTER_RETX_REQUESTS  2	 // Maximum number of S(WTX) requests allowed for the PICC during a single layer-4 block transfer
#define EMV_PCB_IBLOCK_CHAINED		0x12	// ISO14443-4 I-block PCB with the chaining bit se
#define EMV_PCB_IBLOCK_UNCHAINED	0x02	// ISO14443-4 I-block PCB with the chaining bit cleared
#define EMV_PCB_RACK				0xA2	// ISO14443-4 acknowledge R-block PCB. This constant needs to be xored with the current block number
#define EMV_PCB_RNAK				0xB2	// ISO14443-4 not acknowledge R-block PCB. This constant needs to be xored with the current block number
#define EMV_PCB_SWTX				0xF2	// ISO14443-4 WTX request or response S-block PCB. This must not be combined with the current block number
#define EMV_PCB_CHAIN_MASK			0x10	// Bitmask for the chaining bit of an ISO14443-4 I-block PCB
#define EMV_PCB_BNO_MASK			0x01	// Bitmask for the block number of an ISO14443-4 R-block or I-block PCB
#define EMV_WTXM_MASK				0x3F	// Bitmask for the WTXM bits of the inf byte of an WTX request or response
/*!
 *****************************************************************************
 * Maximum allowed value for the WTXM of an WTX request. If a value above
 * EMV_MAX_WTXM is requested, then EMV_MAX_WTXM will be used instead of the
 * requested value.
 *****************************************************************************
 */
#define EMV_MAX_WTXM                59

static unsigned char	bn;


void emvInitLayer4(void)
{
	bn = 0;
}

extern unsigned char dPiccPcd, dPcdPicc, fsci, sfgi, fwi;

static int _EmvTransceiveBlock(unsigned char pcb, unsigned char *inf, int infLen, unsigned char *rxBuf, int maxLen, int *rxLen, int txBlock);

int emvTransceiveApdu(unsigned char *apdu, int apduLen, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned short	fsc;
	unsigned char	*p, pcb, buf[EMV_FSD_MIN_PCD];
	int		rval, val, len, count, retry;

 	switch(fsci) {
	case 0:		fsc = 16;  break;
 	case 1:		fsc = 24;  break;
	case 2:		fsc = 32;  break;
	case 3:		fsc = 40;  break;
 	case 4:		fsc = 48;  break;
 	case 5:		fsc = 64;  break;
	case 6:		fsc = 96;  break;
 	case 7:		fsc = 128; break;
	case 8:		fsc = 256; break;
 	default:	fsc = 256; break;
	}
	fsc -= 3;
	p = apdu; count = apduLen;
    retry = 0;
    while(count > 0) {
      	if(count > fsc) {
    		pcb = EMV_PCB_IBLOCK_CHAINED | bn;
    		len = fsc; val = 1;
    	} else {
    		pcb = EMV_PCB_IBLOCK_UNCHAINED | bn;
    		len = count; val = 0;
    	}
    	rval = _EmvTransceiveBlock(pcb, p, len, buf, EMV_FSD_MIN_PCD, &len, 1);
        if(rval) break;
        if(len < 1) rval = ERR_PROTOCOL;
		else if((buf[0] & ~EMV_PCB_BNO_MASK) == EMV_PCB_RACK) {	// The received block is R(ACK)
	        if(len != 1) rval = ERR_PROTOCOL;
			else if((buf[0] & EMV_PCB_BNO_MASK) != bn) {
				if(retry < EMV_MAX_RETRY) retry++;
				else	rval = ERR_PROTOCOL;
			} else if(!val) {	// Received an R(ACK) with correct block number as response to an unchained I-block. This is a protocol error.
				rval = ERR_PROTOCOL; 
			} else {			// Received proper R(ACK), toggle block number
				bn ^= 0x01;
				p += len; count -= len;
				retry = 0;
 			}
		} else if(val) rval = ERR_PROTOCOL;	// Response must be a R(ACK)
		else	break;		// handling of the response I-block.
        if(rval) break;
	}
	if(rval) return rval;
	// Handle the response of the card to the APDU
	p = rxBuf; count = 0;
	do {
		if(len < 1) rval = ERR_PROTOCOL;	// An I-block must have a size > 0
		else {
			val = buf[0] & ~EMV_PCB_BNO_MASK;
			if(val != EMV_PCB_IBLOCK_UNCHAINED && val != EMV_PCB_IBLOCK_CHAINED ||	// The received block must be a proper I-block
					(buf[0] & EMV_PCB_BNO_MASK) != bn) rval = ERR_PROTOCOL;	// The received block must have a correct block number
        }
		if(rval) break;
		bn ^= 0x01;	// Received proper I-block, toggle block number
		if(count + len > maxLen) {
			memcpy(p, buf, maxLen-count); *rxLen = maxLen;
			rval = ERR_DATA_TOO_BIG;
		} else {
			len--;
			memcpy(p, buf+1, len);
			p += len; count += len;
			if(val == EMV_PCB_IBLOCK_CHAINED) {
				// Chaining bit set. Send R(ACK) and receive next I-block
				pcb = EMV_PCB_RACK | bn;
				rval = _EmvTransceiveBlock(pcb, NULL, 0, buf, EMV_FSD_MIN_PCD, &len, 0);
			} else {
				*rxLen = count;
				break;
			}
		}
	} while(!rval) ;
	return rval;
}

static int _EmvTransceiveBlock(unsigned char pcb, unsigned char *inf, int infLen, unsigned char *rxBuf, int maxLen, int *rxLen, int txBlock)
{
	unsigned char   buf[EMV_FSD_MIN_PCD], wtxm;
    unsigned long	fwt, fwtx;
	int		i, rval, retry, swtxRetry, lastIsRetryRequest;

	buf[0] = pcb; memcpy(buf+1, inf, infLen);
	fwt = (4096L + 384) << fwi;	// Calculate frame wait time
//printf("fwt=%d(0x%x)\n", fwt, fwt);
//printf("EMV Tx %d [%02x", infLen+1, (int)buf[0]); for(i = 1;i < infLen+1;i++) printf("-%02x", (int)buf[i]); printf("]\n");
    rval = as3911TransceiveData(buf, infLen+1, rxBuf, maxLen , rxLen, fwt);
//if(rval) printf("as3911TransceiveData=%d\n", rval);
//else { printf("EMV Rx %d [%02x", *rxLen, (int)rxBuf[0]); for(i = 1;i < *rxLen;i++) printf("-%02x", (int)rxBuf[i]); printf("]\n"); }
	retry = swtxRetry = 0;
	lastIsRetryRequest = 0;
	while(retry < EMV_MAX_RETRY) {
		// Mesages larger than the reported reader buffer size(FSD) must be treated
		// as a protocol error.
		// Note: There have been diverting statements from FIME regarding treatment
		// of recieved block exceeding FSD. One statement was to tread it as
		// a protocol error(which implies that no further retransmissions are
		// requested). Another statement was to treat this as a transmission error
		// which might require a retransmission request to be send.
		// This implementation treats too long messages as protocol error.
		//if(rval == ERR_OVERFLOW) break;
		if(rval == ERR_TIMEOUT || rval == ERR_OVERFLOW) break;
		else if(rval == 0 && rxBuf[0] == EMV_PCB_SWTX) {	// Handle frame wait extension requests			
			// The EMV standard limits the number of consecutive sequences of
			// 'retransmission request followed by S(WTX) request' but there is
			// no limit to the number of S(WTX) requests received after any other block.
			if(lastIsRetryRequest) swtxRetry++;
			else	swtxRetry = 0;
			// According to FIME reception of an S(WTX) request counts as reception of a correct frame.
			// Thus the counter for consecutive transmission errors must be reseted.
			retry = 0;
			if(*rxLen != 2) {	// Check for proper S(WTX) request size(2 bytes)
				rval = ERR_PROTOCOL;
				break;
			}
			// An excess of S(WTX) requests after retransmission requests must
			// be treated as timeout error.
			// See 10.3.5.5. The explanation from this clause also applies to
			// 10.3.5.8 although consecutive S(WTX) requests are not mentioned there.
			if(swtxRetry > EMV_MAX_CONSECUTIVE_SWTX_AFTER_RETX_REQUESTS) {
				rval = ERR_TIMEOUT;
				break;
			}
			wtxm = rxBuf[1] & EMV_WTXM_MASK;
			// Check validity of wtxm value and adjust range
			if(wtxm == 0 || wtxm > EMV_MAX_WTXM) {
				rval = ERR_PROTOCOL;
				break;
			}
//printf("wtxm=%d\n", wtxm);
			// Calculate requested frame wait time in carrier cycles
			fwtx = ((4096L + 384) << fwi) * wtxm;
			// In the EMV standard there is no clear definition on what to do
			// if the requested FWT_TEMP exceeds FWT_MAX.
			// But, according to FIME the PCD shall limit FWT_TEMP to FWT_MAX 
			// if that happens.
			if(fwtx > ((4096L + 384) << EMV_FWI_MAX_PCD)) fwtx = ((4096L + 384) << EMV_FWI_MAX_PCD);
			buf[0] = EMV_PCB_SWTX;
			buf[1] = wtxm;
			lastIsRetryRequest = 0;
			rval = as3911TransceiveData(buf, 2, rxBuf, maxLen, rxLen, fwtx);
			// ToDo: Check whether S(WTX) pairs are counted as retransmissions
		} else if(rval == 0) {
			break;
        } else {        	
            // Check for a timeout after consecutive R(NAK), S(WTX), S(WTX) response chains.
            // See PCD 10.3.5.5 for details.
            // Note: The >= is there for a reason. Please think _twice_ before changing it into >.
            if(rval == ERR_TIMEOUT && swtxRetry >= EMV_MAX_CONSECUTIVE_SWTX_AFTER_RETX_REQUESTS) break;
            // All (other) error conditions lead to a retransmission request
            if(txBlock) buf[0] = EMV_PCB_RNAK | bn;
            else		buf[0] = EMV_PCB_RACK | bn;
            lastIsRetryRequest = 1;
printf("emv retry=%d\n", retry+1); // ResetWDT();
            rval = as3911TransceiveData(buf, 1, rxBuf, maxLen, rxLen, fwt);
            retry++;
        }
    }
 	if(rval == 0 && rxBuf[0] == EMV_PCB_SWTX) rval = ERR_TIMEOUT;	// The last retransmitted block is not allowed to be an S(WTX) request
	return rval;
}

