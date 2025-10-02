#ifndef _EMV_STANDARD_H
#define _EMV_STANDARD_H


#define EMV_T_P_MIN				5	// Minimum value of t_p(milliseconds). note The standard defines t_p min to be 5.1 ms
#define EMV_T_P_MAX				10	// Maximum value of t_p(ms)
#define EMV_T_P					6	// Value of t_p used(in ms)

#define EMV_T_RESET_MIN			5	// Minimum value of t_reset(operating field reset time) (ms). note The standard defines t_reset min to be 5.1 ms.
#define EMV_T_RESET_MAX			10	// Maximum value of t_reset(ms)
#define EMV_T_RESET				6	// Value of t_reset used(in ms)
#define EMV_T_POWEROFF_MIN		15	// Minimum value of t_poweroff used(in ms)
#define EMV_T_POWEROFF			16	// Value of t_poweroff used(in ms)
#define EMV_T_RESETDELAY		1	// Value of t_resetdelay used, between 0 and 33(in ms). 0 sometimes fails TA305_3.

#define EMV_FWT_ACTIVATION_PCD  71680UL	// Frame wait time of RATS for the PCD(microseconds
#define EMV_FWT_ACTIVATION_PICC 65536UL	// Frame wait time of RATS for the PICC(microseconds)

#define EMV_EGT_PCD_MIN			0	 // Minimum allowed extra guard time for the PCD(carrier cycles)
#define EMV_EGT_PCD_MAX			752	 // Maximum allowed extra guard time for the PCD(carrier cycles)
#define EMV_EGT_PICC_MIN		0	 // Minimum allowed extra guard time for the PICC(carrier cycles)
#define EMV_EGT_PICC_MAX		272	 // Maximum allowed extra guard time for the PICC(carrier cycles)
#define EMV_FWT_ATQB_PCD		7680 // Frame wait time of ATQB for the PCD(carrier cycles)
#define EMV_FWT_ATQB_PICC		7296 // Frame wait time of ATQB for the PICC(carrier cycles)
#define EMV_FDT_PCD_MIN			6780 // Minimum frame delay time for the PCD(carrier cycles)

#define EMV_FSDI_MIN_PCD		8	// Minimum PCD frame size integer supported by the PCD
#define EMV_FSDI_MIN_PICC		8	// Minimum PCD frame size integer supported by the PICC
#define EMV_FSD_MIN_PCD			256	// Minimum PCD frame size supported by the PCD
#define EMV_FSD_MIN_PICC		256	// Minimum PCD frame size supported by the PICC
#define EMV_FSCI_MIN_PCD		0	// Minimum PICC frame size integer supported by the PCD
#define EMV_FSCI_MIN_PICC		2	// Minimum PICC frame size integer supporte by the PICC
#define EMV_FSCI_DEFAULT		2	// Default PICC frame size integer. This value has to be used if the ATS has no T0 byte
#define EMV_FSC_MIN_PCD			16	// Minimum PICC frame size supported by the PCD
#define EMV_FSC_MIN_PICC		32	// Minimum PICC frame size supported by the PICC

#define EMV_FWI_MAX_PCD			14	// Maximum frame wait integer supported by the PCD
#define EMV_FWI_MAX_PICC		7	// Maximum frame wait integer allowed for the PICC
#define EMV_FWT_MAX_PCD			67108864UL	// Maximum frame wait time supported by the PCD(carrier cycles)
#define EMV_FWT_MAX_PICC		524288UL	// Maximum frame wait time allowed for the PICC(carrier cycles)

#define EMV_SFGI_MAX_PCD		14	// Maximum frame guard time integer supported by the PCD
#define EMV_SFGI_MAX_PICC		8	// Maximum frame guard time integer supported by the PICC


#define EMV_D_PICC_PCD_DEFAULT	0	// Default value for the PICC->PCD datarate bits if TA(1) is not transmitted in the ATS (ISO14443-A)
#define EMV_D_PCD_PICC_DEFAULT	0	// Default value for the PCD->PICC datarate bits if TA(1) is not transmitted in the ATS (ISO14443-A).
#define EMV_SFGI_DEFAULT		0	// Default value for the frame guard time integer if TB(1) is not transmitted in the ATS (ISO14443-A).
#define EMV_FWI_DEFAULT			4	// Default value for the frame wait integer if TB(1) is not transmitted in the ATS (ISO14443-A).


#endif
