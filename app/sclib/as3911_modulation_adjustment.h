#ifndef	AS3911_MODULATION_ADJUSTMENT_H
#define	AS3911_MODULATION_ADJUSTMENT_H

// Available modulation	level adjustment modes
typedef	enum _ModulationLevelMode_ {
	ML_FIXED,
	ML_AUTOMATIC,	
	ML_FROM_AMPLITUDE,
	ML_FROM_PHASE
} ModulationLevelMode_enum;

// Lookup table	for	modulation level adjustment. This table consists of
// x and y values	as separate	arrays.	So x[i], y[i] define the
// (x,y) coordinates of point i of the lookup table.	
// Note: The x values must be monotonically	increasing to produce correct
//		 results during	linear interpolation.
typedef	struct _ModulationLevelTable_ {	
	unsigned char	length;	// Length of the x and y point arrays
	unsigned char	*x;		// X data points of	the	modulation level table
	unsigned char	*y;		// Y data points of	the	modulation level table
} ModulationLevelTable_t;

typedef	struct _ModulationLevelAutomatic_ {	
	unsigned char	targetValue;// Target value	for the AS3911 automatic modulation depth adjustment
	unsigned char	hysteresis;	// Hysteresis for doing adjustment again
} ModulationLevelAutomatic_t;

typedef	struct OutputLevels	{
	unsigned char	lowRed;		// low level reduction value for reg 0x27
	unsigned char	highRed;	// high	level reduction	value for reg 0x27
	unsigned char	decThresh;	// amplitude threshold	for	going high->low	
	unsigned char	incThresh;	// amplitude threshold for going low->high
} OutputLevels_t;

typedef struct _GainTableVal_	{
	unsigned char	amp;
	unsigned char	phase;
	unsigned char	reg02;
	unsigned char	regA;
	unsigned char	regB;
	unsigned char	regC;
	unsigned char	regD;
} GainTableVal_t;

typedef struct _GainTable_ {
	int		num_vals;
	GainTableVal_t  table[16];
	int		used_id;
	char	*text;
} GainTable_t;

// Set the modualtion level	adjustment mode	and	configure additional data
// needed by some of the adjustment modes. There are four modes available:	
//   ML_FIXED: no dynamic adjustment of the modulation level is performed.
//		The modulationLevelTable parameter will be ignored.
//   ML_AUTOMATIC: Use the AS3911 automatic modulation depth adjustment to
//		set the modulation depth.
//   ML_FROM_AMPLITUDE: The modulation level is adjusted based on the RF
//		amplitude and the lookup table values in modulationLevelModeData.
//   ML_FROM_PHASE: The modulation level is adjusted based on the phase
//		difference between RFI and RFO signals and the lookup table values
//		in a modulationLevelModeData.
// Note: Internally only a pointer to the modulationLevelModeData will be
//      stored. The data structure pointed to by modulationLevelModeData
//		should therefore not be modified while in use by this module.
// [in]	Mode: Set the modulation level adjustment mode.
// [in]	Data: Additional data for the selected modulation level adjustment
//			  mode. This must be of the following datatype:
//		  ML_FIXED: no additional data required, can be NULL
//		  ML_AUTOMATIC: Pointer to ModulationLevelAutomatic_t	
//		  ML_FROM_AMPLITUDE: Pointer to ModulationLevelTable_t
//		  ML_FROM_PHASE: Pointer to ModulationLevelTable_t
void as3911SetModulationLevelMode(ModulationLevelMode_enum mode, void *data);

// Get the active modulation level adjustment mode and modulation level
// adjustment table.
void as3911GetModulationLevelMode(ModulationLevelMode_enum *mode, void *data);

void as3911SetOutputLevels(OutputLevels_t *levels);

// Adjust the modualtion level based on the active modulation level
// adjustment mode and the modulation level adjustment table.
// Note: This method may block for up to 50 microseconds depending on the
//		 active modulation level adjustment mode.
void as3911AdjustModulationLevel(void);	

// Provide gain table which will be applied when adjusting modulation
void as3911SetGainTables(GainTable_t *lownTable, GainTable_t *normTable);


#endif
