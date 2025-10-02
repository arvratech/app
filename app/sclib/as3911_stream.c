#include "as3911.h"
#include "as3911_stream.h"


int as3911OpenStream(as3911StreamConfig_t *config)
{
	unsigned char	buf[4];
	int		smd;

	as3911GetReg(AS3911_REG_OP_CONTROL, &opControl);
    buf[0] = (opControl | AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en) & ~(AS3911_REG_OP_CONTROL_wu);
	buf[2] = 0x00;		// Bit Rate definition Register: 106 kBit/s both direction
	buf[3] = 0x00;      // ISO14443A and NFC 106kb/s Setting register
	if(config->useBPSK) {
		buf[1] = AS3911_REG_MODE_om_bpsk_stream;		// Mode Definition Register: BPSK stream mode
		if(config->din < 2 || config->din > 4) return ERR_PARAM;	// not in fc/4 .. fc/16
		smd = (4 - config->din) << AS3911_REG_STREAM_MODE_shift_scf;
	} else {
		buf[1] = AS3911_REG_MODE_om_subcarrier_stream;	// Mode Definition Register: Sub-carrier stream mode	
		if(config->din < 3 || config->din > 6) return ERR_PARAM;	// not in fc/8 .. fc/64
		smd = (6 - config->din) << AS3911_REG_STREAM_MODE_shift_scf;
		if(config->report_period_length == 0) return ERR_PARAM;
	}
	if(config->dout < 1 || config->dout > 7) return ERR_PARAM;		// not in fc/2 .. fc/128
    smd |= (7 - config->dout) << AS3911_REG_STREAM_MODE_shift_stx;
	if(config->report_period_length > 3) return ERR_PARAM;
	smd |= config->report_period_length << AS3911_REG_STREAM_MODE_shift_scp;
	as3911SetRegs(AS3911_REG_OP_CONTROL, buf, 4);	// set Sub-carrier stream mode, lowest Rx and TX rate. Enable Rx and Tx
	as3911SetReg(AS3911_REG_STREAM_MODE, smd);
//printf("%02x-%02x-%02x-%02x\n", (int)buf[0], (int)buf[1], (int)buf[2], (int)buf[3]);
//as3911GetReg(AS3911_REG_STREAM_MODE, buf);
//printf("STREAM_MODE=%02x\n", (int)buf[0]);
	as3911ModReg(AS3911_REG_AUX, 0, AS3911_REG_AUX_no_crc_rx);
//printf("as3911OpenStream...\n");
	return 0;
}

void as3911CloseStream(void)
{
	opControl &= ~(AS3911_REG_OP_CONTROL_en | AS3911_REG_OP_CONTROL_tx_en | AS3911_REG_OP_CONTROL_rx_en);
	as3911SetReg(AS3911_REG_OP_CONTROL, (int)opControl);
//printf("as3911CloseStream...\n");
}

