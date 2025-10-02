#ifndef _SC_CFG_H_
#define _SC_CFG_H_


int  CrScReaderEnable(void);
int  CrScReaderFormat(void);
int  CrIsEnableHynixEmvReader(void);
int  CrRfoNormalLevel(void);
void CrSetScReader(int enable);
int  CrUidFormatCardType(void);
int  CrBlockFormatCardType(void);
int  CrBlockFormatCardApp(void);
int  CrIso7816FormatCardType(void);
int  CrIso7816FormatCardApp(void);
unsigned char *CrMifareKey(void);
int  CrBlockNumber(void);
int  CrBlockDataLength(void);
unsigned char *CrAID(void);
int  CrAIDlength(void);
int  CrIso7816SFI(void);
int  CrAFI(void);
int  CrIso7816RecordNumber(void);
int  CrIso7816ReadLength(void);
int  CrIso7816DataOffset(void);
int  CrIso7816DataLength(void);
int  CrIso7816EnableSelectMF(void);


#endif

