#ifndef _OC_H_
#define _OC_H_


#define MAX_OC_SZ		12


typedef struct _OUTPUT_COMMAND {
	unsigned char	repeatCount;
	unsigned char	onTime;
	unsigned char	offTime;
	unsigned char	reserve[1];
} OUTPUT_COMMAND;


void ocSetDefault(OUTPUT_COMMAND *oc);
int  ocEncode(OUTPUT_COMMAND *oc, void *buf);
int  ocDecode(OUTPUT_COMMAND *oc, void *buf);
int  ocValidate(OUTPUT_COMMAND *oc);

int  ocGetEnable(int ocID);
void ocSetEnableDefault(int ocID);
int  ocGetRepeatCount(int ocID);
void ocSetRepeatCount(int ocID, int repeatCount);
int  ocGetOnTime(int ocID);
void ocSetOnTime(int ocID, int onTime);
int  ocGetOffTime(int ocID);
void ocSetOffTime(int ocID, int offTime);


#endif
