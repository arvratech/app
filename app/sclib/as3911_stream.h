#ifndef AS3911_STREAM_H
#define AS3911_STREAM_H


// Parameters how the stream mode should work
typedef struct _as3911StreamConfig {
	unsigned char	useBPSK;	// 0:subcarrier  1:BPSK
	unsigned char	din;		// the divider for the in subcarrier frequency: fc/2^din
	unsigned char	dout;		// the divider for the out subcarrier frequency fc/2^dout
	unsigned char	report_period_length;	// the length of the reporting period 2^report_period_length
} as3911StreamConfig_t;


int  as3911OpenStream(as3911StreamConfig_t *config);
void as3911CloseStream(void);


#endif

