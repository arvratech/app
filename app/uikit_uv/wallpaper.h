#ifndef _WALL_PAPER_H_
#define _WALL_PAPER_H_


void wallpaperInit(int id);
void wallpaperDarken(void);
int  wallpaperNext(int id);
int  wallpaperChange(int wallpaperId);
int  wallpaperAnimate(int wallpaperId, int alpha);
void wallpaperDraw(CGRect *rect);
void wallpaperSwap(char *fileName);
void wallpaperSwapBitmap(UIColor color, char *fileName, char *text, char *text2);
void wallpaperPut(void);


#endif

