#ifndef _ENG_16_H_
#define _ENG_16_H_


void _GetEng16Font(int c, unsigned char **font, int *w);
int  bmpcPutEng16Char(BMPC *bmpD, int x, int y, int c, int Color);
int  bmpcPutEng16Text(BMPC *bmpD, int x, int y, char *string, int Color);
int  lcdPutEng16Char(int x, int y, int c, int Color);
int  lcdPutEng16Text(int x, int y, char *string, int Color);


#endif	/* _ENG_16_H_ */