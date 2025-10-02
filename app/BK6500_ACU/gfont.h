#ifndef _GFONT_H_
#define _GFONT_H_


#define FONT_TYPE_16		16
#define FONT_TYPE_24		24
#define FONT_TYPE_32		32
#define FONT_TYPE_48		48


void InitFont(void);
void SetLanguage(int lang);
int  GetLanguage(void);
int  GetBidi(void);
int  GetFontHeight(void);
int  GetFontSize(void);
void SetFontSize(int FontSize);
void RestoreFontSize(void);
void _GetEngFont(int c, unsigned char **font, int *w);
int  GetFont(char *st, unsigned char **font, int *w);
int  GetStringWidth(char *string);
unsigned char *GetCJKDayFont(void);
unsigned char *GetCJKMonthFont(void);


#endif	/* _GFONT_H */
