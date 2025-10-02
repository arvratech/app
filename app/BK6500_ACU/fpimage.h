#ifndef _FP_IMG_H
#define _FP_IMG_H 

void FPImageInitialize(void);
void FPImageDrawBoundary(int x, int y, int Color);
void FPImageDraw(int x, int y);
void FPImageDrawImage(int x, int y, unsigned char *Buffer);
void FPImageDrawPrompt(int x, int y);
void FPImageGet(unsigned char *Buffer);
void FPImagePut(int x, int y, unsigned char *Buffer);


#endif /* _FP_IMG_H */