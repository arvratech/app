#ifndef _OUTPUT_COMMAND_CFG_H_
#define _OUTPUT_COMMAND_CFG_H_

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


#define MAX_OUTPUT_CADENCE_SZ		4

// OutputCommand data structure
typedef struct _OUTPUT_COMMAND_CFG {
	unsigned char	RepeatCount;
	unsigned char	reserve[3];
	unsigned char	OutputCadences[MAX_OUTPUT_CADENCE_SZ][2];	// [][0]=OnTime [][1]=OffTime
} OUTPUT_COMMAND_CFG;


_EXTERN_C int  ocEncodeID(long nID, void *buf);
_EXTERN_C int  ocDecodeID(long *pID, void *buf);
_EXTERN_C int  ocValidateID(long nID);

int  ocEncode(OUTPUT_COMMAND *oc, void *buf);
int  ocDecode(OUTPUT_COMMAND *oc, void *buf);
int  ocValidate(OUTPUT_COMMAND *oc);


#endif	/* _OUTPUT_COMMAND_CFG_H_ */