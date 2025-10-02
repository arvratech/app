#ifndef _LCD_H_
#define _LCD_H_

#include "bmpc.h"

// LCD video structure
typedef struct _LCD_VIDEO {
	unsigned char	mode;
	unsigned char	reserve[3];
	CGSize			size;
	CGRect			src;
	CGRect			dst;
	unsigned long	paddrY;
	unsigned long	paddrC;
} LCD_VIDEO;

// g2d buffer structure
typedef struct _G2D_BUFFER {
	unsigned char	type;		// 0:LCD  1:other
	unsigned char	index;
	unsigned char	bpp;
	unsigned char	reserve[1];
	unsigned long	paddr;
	void			*addr;
	int				width;
	int				height;
	int				size;
} G2D_BUFFER;

extern int	_lcdNumerator, _lcdDenominator;

#define _scale(v)				(v) * _lcdNumerator / _lcdDenominator
#define _scaleCeil(v)			((v) * _lcdNumerator + _lcdDenominator - 1) / _lcdDenominator
#define _scale24_6(v)			((v) * _lcdNumerator << 6) / _lcdDenominator
#define _reverseScale(v)		(v) * _lcdDenominator / _lcdNumerator
#define _reverseScaleCeil(v)	((v) * _lcdDenominator + _lcdNumerator - 1) / _lcdNumerator

int  lcdOpen(void);
void lcdClose(void);
unsigned long lcdPhysFrameBuffer(void);
void lcdChangePage(void);
void lcdSwapPage(void);

int  lcdOpenVideo(void);
void lcdCloseVideo(int handle);
void lcdStartVideo(int handle, LCD_VIDEO *video);
void lcdUpdateVideo(int handle, unsigned long paddrY, unsigned long paddrC);

void lcdOnBacklight(void);
void lcdOffBacklight(void);
void lcdSetContrast(int val);
void lcdSetBrightness(int val);
void lcdResetDisplay(void);

G2D_BUFFER *g2dLcdBuffer(void);
G2D_BUFFER *g2dMemoryBuffer(int index);
void g2dSetSize(G2D_BUFFER *g2d, int width, int height);
void g2dClipToRect(G2D_BUFFER *g2d, CGRect *rect);
void g2dSetDefaultClipRect(G2D_BUFFER *g2d);
void g2dFill(G2D_BUFFER *g2d, int lx, int ly, int width, int height, unsigned long color);
void g2dBlit(G2D_BUFFER *g2d, int lx, int ly, int width, int height, G2D_BUFFER *g2dS, int alpha);
void g2dMove(G2D_BUFFER *g2d, CGRect *rect, int dy);

int  lcdDirection(void);
void lcdSetDirection(int landscape);
int  lcdRealWidth(void);
int  lcdRealHeight(void);
int  lcdWidth(void);
int  lcdHeight(void);
int  lcdPixelWidth(void);
int  lcdPixelHeight(void);
int  lcdScale(void);
void lcdClipToRect(CGRect *rect);
void lcdSetDefaultClipRect(void);
void lcdPointToPixelRect(CGRect *rect);
void lcdPixelClipToRect(CGRect *rect, CGRect *clipRect);
void lcdClear(UIColor  color);
void lcdFillRect(int x, int y, int width, int height, UIColor color);
void lcdDimm(int x, int y, int width, int height, int value);
void lcdRect(int x, int y, int width, int height, UIColor color);
void lcdHorizontalLine(int x, int y, int width, UIColor color);
void lcdVerticalLine(int x, int y, int height, UIColor color);
void lcdPixel(int x, int y, UIColor color);
void lcdFadeoutBackgroundScreen(int alpha);
void lcdPutBackgroundScreen(int x, int y, G2D_BUFFER *g2d);
void lcdPutBmpFile(int x, int y, int width, int height, char *fileName);
void lcdPutAlphaMaskFile(int x, int y, int width, int height, UIColor  color, char *fileName);
void lcdPutAlphaMaskFileEx(int x, int y, int width, int height, UIColor  color, char *fileName);
void lcdPutJpeg(int x, int y, int width, int height, unsigned char *jpegBuffer, int jpegSize);
void lcdPutJpegFile(int x, int y, int width, int height, char *fileName);
int  lcdPutText(int x, int y, char *string, UIColor color);


#endif

