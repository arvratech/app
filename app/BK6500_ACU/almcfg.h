#ifndef _ALM_CFG_H_
#define _ALM_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


#define MAX_OUTPUT_COMMAND_SZ		12


// AlarmAction data structure
typedef struct _ALARM_ACTION_CFG {
	unsigned char	AlarmOutput;
	unsigned char	OutputCommands[MAX_OUTPUT_COMMAND_SZ];	// OutputCommand ID
} ALARM_ACTION_CFG;


#define MAX_OUTPUT_CADENCE_SZ		4

// OutputCommand data structure
typedef struct _OUTPUT_COMMAND_CFG {
	unsigned char	RepeatCount;
	unsigned char	reserve[3];
	unsigned char	OutputCadences[MAX_OUTPUT_CADENCE_SZ][2];	// [][0]=OnTime [][1]=OffTime
} OUTPUT_COMMAND_CFG;


_EXTERN_C void actSetDefault(ALARM_ACTION_CFG *cfg);
_EXTERN_C int  actEncode(ALARM_ACTION_CFG *cfg, void *buf);
_EXTERN_C int  actDecode(ALARM_ACTION_CFG *cfg, void *buf);
_EXTERN_C int  actValidate(ALARM_ACTION_CFG *cfg);
_EXTERN_C int  ocEncodeID(long nID, void *buf);
_EXTERN_C int  ocDecodeID(long *pID, void *buf);
_EXTERN_C int  ocValidateID(long nID);
_EXTERN_C void ocSetDefault(OUTPUT_COMMAND_CFG *cfg);
_EXTERN_C int  ocEncode(OUTPUT_COMMAND_CFG *cfg, void *buf);
_EXTERN_C int  ocDecode(OUTPUT_COMMAND_CFG *cfg, void *buf);
_EXTERN_C int  ocValidate(OUTPUT_COMMAND_CFG *cfg);


#endif	/* _ALM_CFG_H_ */