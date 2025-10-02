#ifndef _CT_FONT_H_
#define _CT_FONT_H_


typedef struct _CT_FONT {
	signed char		x;
	signed char		y;
	unsigned char	width;
	unsigned char	height;
	unsigned char	pitch;
	unsigned char	reserve[3];
} CT_FONT;


int  ftInit(char* fontPath);
void ftExit(void);
void ftSetSize(int point);
int  ftPixelSize(void);
int  ftSize(void);
int  ftPixelHeight(void);
int  ftGetPixelHeight(int size);
int  ftHeight(void);
int  ftGetTextPixelWidth(int size, char *string);
int  ftTextPixelWidth(char *string);
int  ftTextWidth(char *string);
int  ftTextPixelPosition(char *string, int textPosition);
int  ftTextPositionFromTouched(char *string, int pixelPosition, int touchPosition);


#endif

