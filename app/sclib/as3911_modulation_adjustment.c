#include <stdio.h>
#include <limits.h>
#include "as3911.h"
#include "log.h"

// Sanity timeout for the AS3911 direct command completed interrupt
#define AS3911_DCT_IRQ_SANITY_TIMEOUT   5	

#define DELTA(A,B) (((A)>(B))? ((A)-(B)) : ((B)-(A)))

// Active modulation level adjustment mode
static ModulationLevelMode_enum modulationLevelMode = ML_FIXED;
static unsigned short modLastAmp = SHRT_MAX;
// Lookup table for the active modulation level adjustment mode
static ModulationLevelTable_t *modulationLevelTable = NULL;
static ModulationLevelAutomatic_t *modulationLevelAutomatic = NULL;
static unsigned char lowXs[2] = { 0x5e, 0x6c };
static unsigned char lowYs[2] = { 0xce, 0xae };
static ModulationLevelTable_t lowModulationLevelTable = { 2, lowXs, lowYs };
static OutputLevels_t outputLevels = { 0, 0, 0, 0 };
static GainTable_t *lowGainTable = NULL;
static GainTable_t *normGainTable = NULL;


void as3911SetOutputLevels(OutputLevels_t *levels)
{
	outputLevels = *levels;
}

void as3911SetModulationLevelMode(ModulationLevelMode_enum mode, void *data)
{
	modulationLevelMode = mode;
	if(mode == ML_FIXED) ;
	else if(mode == ML_AUTOMATIC) modulationLevelAutomatic = data;
	else if(mode == ML_FROM_PHASE || mode == ML_FROM_AMPLITUDE) modulationLevelTable = data;
}

void as3911GetModulationLevelMode(ModulationLevelMode_enum *mode, void *data)
{
	*mode = modulationLevelMode;
	if(modulationLevelMode == ML_FIXED) data = NULL;
	else if(modulationLevelMode == ML_AUTOMATIC) data = modulationLevelAutomatic;
	else if(modulationLevelMode == ML_FROM_PHASE || modulationLevelMode == ML_FROM_AMPLITUDE) data = modulationLevelTable;
}

int as3911GetInterpolatedValue(int x1, int y1, int x2, int y2, int xi);

void as3911AdjustModulationLevel()
{
    ModulationLevelTable_t  *p;
	GainTableVal_t	*gtv;
    GainTable_t		*gt;
 	unsigned char	measurementCommand, amplitudePhase, phase, antennaDriverStrength, am_off_level;
    int		i, j;

    p = modulationLevelTable;
	gt = NULL;
    as3911GetReg(AS3911_REG_RFO_AM_OFF_LEVEL, &am_off_level);
    for (i = 0;i < 2;i++) {
        as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_AMPLITUDE, AS3911_REG_AD_RESULT, AS3911_DCT_IRQ_SANITY_TIMEOUT, &amplitudePhase);
        //printf("i=%x, am_off_lvl=%x, amp=%x\n",i,am_off_level, amplitudePhase);
        if(i == 0) {
            if(outputLevels.lowRed == outputLevels.highRed) {
                gt = normGainTable;
                break;
            }
            if(outputLevels.highRed == am_off_level) {
                if(amplitudePhase > outputLevels.decThresh) { // amplitude is fine, output power and current table are o.k.
                    gt = normGainTable;
                    break;
                }
                // Reduce output power, redo measuremnt, change table
                j = outputLevels.highRed;
                do {
                    j++;
                    as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, j);
                } while(j < outputLevels.lowRed);
printf("%x <= %x : reduced AM_OFF_LEVEL -> 0x%x\n", amplitudePhase, outputLevels.decThresh, j);
                modLastAmp = SHRT_MAX; // Force triggering of automatic mod depth calibration if used
                p = &lowModulationLevelTable;
            } else {
                if(amplitudePhase < outputLevels.incThresh) { // Output power was reduced, amplitude is still fine, use low table
                    p = &lowModulationLevelTable;
                    gt = lowGainTable;
                    break;
                }
                // Output power was reduced, increase again, redo measuremnt, use normal table
                j = am_off_level;
                do {
                	j--;
                    as3911SetReg(AS3911_REG_RFO_AM_OFF_LEVEL, j);
                } while(j > outputLevels.highRed);
                gt = normGainTable;  
printf("%x >= %x: increased AM_OFF_LEVEL -> 0x%x\n",amplitudePhase, outputLevels.incThresh, j);
                modLastAmp = SHRT_MAX;	// Force triggering of automatic mod depth calibration if used
            }
        }
    }
    if(gt) {
        gtv = NULL;
        i = 0;
        while (i+1 < gt->num_vals) {
            if(DELTA(gt->table[i+1].amp,amplitudePhase) > DELTA(gt->table[i].amp,amplitudePhase)) break;
            // If amplitude delta gets bigger, we can break since table is first order sorted by amp
            i++;
        }
        //printf("highest index of closest amp: %d\n", i);
        // We found now highest index with the closest amplitude.
        // Now go down to find the one with the closest phase
        if((i-1 >= 0) && (gt->table[i-1].amp == gt->table[i].amp)) { // Need to measure phase and find the one with closest phase
            as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_PHASE, AS3911_REG_AD_RESULT, AS3911_DCT_IRQ_SANITY_TIMEOUT, &phase);
            while(i-1 >= 0) {
                if(gt->table[i-1].amp != gt->table[i].amp) { // Only continue if amp is still the same
                    //printf("break amp not equal %d\n", i);
                    break;
                }
                if(DELTA(gt->table[i-1].phase,phase) > DELTA (gt->table[i].phase, phase)) {
					// If phase delta gets bigger, we can break since table is second order sorted by phase
                    //printf("break phase delta too low %d\n", i);
				}
                i--;
            }
        }
        if(i < gt->num_vals)  gtv = gt->table + i;
        if(gtv) {
            if(gt->used_id != i)  {
                char *text = "";
                // FIXME Remove const, as used_id is solely used for printing in here. Will not work with gain tables in ROM!
                ((GainTable_t *)gt)->used_id = i;
                if(gt->text) text = gt->text;
printf("DGT %s adjusted gains: %02hx %02hx -> %02hx %02hx %02hx %02hx %02hx\n",text , amplitudePhase, phase, gtv->reg02, gtv->regA, gtv->regB, gtv->regC, gtv->regD);
            }
            as3911ModReg(AS3911_REG_OP_CONTROL, AS3911_REG_OP_CONTROL_rx_man | AS3911_REG_OP_CONTROL_rx_chn, gtv->reg02 & 0x30);
            as3911SetRegs(AS3911_REG_RX_CONF1, &gtv->regA, 4);
        }
    }
	measurementCommand = 0;
    // Done with OFF level, now handle ON level if needed
    if(modulationLevelMode == ML_FIXED) return;
    else if(modulationLevelMode == ML_AUTOMATIC) {
        as3911ExecuteCommandAndGetResult(AS3911_CMD_MEASURE_AMPLITUDE, AS3911_REG_AD_RESULT, AS3911_DCT_IRQ_SANITY_TIMEOUT, &amplitudePhase);
		if(abs(((s32)amplitudePhase - (s32)modLastAmp)) > modulationLevelAutomatic->hysteresis) {
            modLastAmp = amplitudePhase;
            as3911ExecuteCommandAndGetResult(AS3911_CMD_CALIBRATE_MODULATION, AS3911_REG_AM_MOD_DEPTH_RESULT, AS3911_DCT_IRQ_SANITY_TIMEOUT, NULL);
printf("Recalibrated mod depth\n");
            // delay some time after this pulse
            for (i = 0;i < 1000;i++) ;	// FIXME wild approximation
		}
    } else if(modulationLevelMode == ML_FROM_AMPLITUDE) measurementCommand = AS3911_CMD_MEASURE_AMPLITUDE;
    else if(modulationLevelMode == ML_FROM_PHASE) measurementCommand = AS3911_CMD_MEASURE_PHASE;
    else	return;		// ToDo: enter debug code here
    // Measurement based modulation strength adjustment requires a modulation
    // level table with at least 2 entries to perform interpolation.
    if(!p || p->length < 2) return;		// ToDo: enter debug code here.
    as3911ExecuteCommandAndGetResult(measurementCommand, AS3911_REG_AD_RESULT, AS3911_DCT_IRQ_SANITY_TIMEOUT, &amplitudePhase);
    for(i = 0;i < p->length;i++)
        if(amplitudePhase <= p->x[i])  break;
    // Use the last interpolation level dataset for any values outside the highest. x-value from the datasets.
    if(i == p->length) i--;
    if(i == 0)	antennaDriverStrength = as3911GetInterpolatedValue(p->x[i], p->y[i], p->x[i+1], p->y[i+1], amplitudePhase);
    else		antennaDriverStrength = as3911GetInterpolatedValue(p->x[i-1], p->y[i-1], p->x[i], p->y[i], amplitudePhase);
    as3911SetReg(AS3911_REG_RFO_AM_ON_LEVEL, antennaDriverStrength);
printf("as3911AdjustModulationLevel end\n");
}

void as3911SetGainTables(GainTable_t *lowTable, GainTable_t *normTable)
{
    lowGainTable = lowTable;
    normGainTable = normTable;
}

// brief Calculate a linear interpolated value.
// note  x1 must be <= x2.
// Calculate the linear interpolated value at xi. The curve for the linear interpolation is 
// defined by (x1, y1) and (x2, y2).
// For values xi below x1 the function will return y1, and for values  xi above  x2 the function
// will return y2. For any intermediate value the function will return y1 + (y2 - y1) * (xi - x1) / (x2 - x1).
// (x1, y1) first point   (x2, y2) second point   xi X-coordinate of the interpolation point
int as3911GetInterpolatedValue(int x1, int y1, int x2, int y2, int xi)
{
	int		v;

	if(xi <= x1) v = y1;
	else if(xi >= x2) v = y2;
	else	v = y1 + (((long) y2 - y1) * (xi - x1)) / (x2 - x1);
	return v;
}
