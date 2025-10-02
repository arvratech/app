#ifndef _IMG_H
#define _IMG_H 

#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


/* Image type definitions */
#define T_GRAY_IMAGE	0
#define T_BW_IMAGE		1
#define T_COLOR_IMAGE	2

/* Image Structure */
typedef struct _IMAGE {
	short	Width, Height;
	unsigned char	Type;
	unsigned char	Quality;
	short	reserve;	
	int		Offset;
	unsigned char *Buffer;
} IMAGE;

/* image both border definition */
#define IMG_SKIP_SZ		32

_EXTERN_C void imgInitialize(IMAGE *img);
_EXTERN_C void imgSet(IMAGE *img, int Type, int Width, int Height, void *Buffer);
_EXTERN_C unsigned char *imgGet(IMAGE *img);
_EXTERN_C int  imgGetType(IMAGE *img);
_EXTERN_C void imgSetType(IMAGE *img, int Type);
_EXTERN_C int  imgGetWidth(IMAGE *img);
_EXTERN_C int  imgGetHeight(IMAGE *img);
_EXTERN_C unsigned char *imgGetBuffer(IMAGE *img);
_EXTERN_C void imgSetBuffer(IMAGE *img, void *Buffer);
_EXTERN_C int  imgGetTotalWidth(IMAGE *img);
_EXTERN_C int  imgGetTotalHeight(IMAGE *img);
_EXTERN_C int  imgGetBufferSize(IMAGE *img);
_EXTERN_C int  imgCalculateBufferSize(int Width, int Height);
_EXTERN_C int  imgCalculateBufferOffset(int Width);
_EXTERN_C void imgSetBorder(IMAGE *img, int C);
_EXTERN_C void imgSetOneBorder(IMAGE *img, int C);
_EXTERN_C void imgPadOneBorder(IMAGE *img);
_EXTERN_C void imgSetConstant(IMAGE *img, int C);
_EXTERN_C void imgAddConstant(IMAGE *imgD, IMAGE *imgS, int C);
_EXTERN_C void imgInverse(IMAGE *imgD, IMAGE *imgS);
_EXTERN_C void imgAdd(IMAGE *imgD, IMAGE *imgS1, IMAGE *imgS2);
_EXTERN_C void imgSubstract(IMAGE *imgD, IMAGE *imgS1, IMAGE *imgS2, int AbsoluteFlag);
_EXTERN_C void imgDialate(IMAGE *imgD, IMAGE *imgS1, IMAGE *imgS2);
_EXTERN_C void imgCopy(IMAGE *imgD, IMAGE *imgS, void *Buffer);
_EXTERN_C void imgCopyFromRaw(IMAGE *img, unsigned char *pImage, int Width, int Height, void *Buffer);
_EXTERN_C void imgCopyToRaw(IMAGE *img, unsigned char *pImage);
_EXTERN_C void imgCopyPart(IMAGE *imgD, int x, int y, IMAGE *imgS);
_EXTERN_C void imgCrop(IMAGE *imgD, IMAGE *imgS, int x, int y);
_EXTERN_C void imgAdjust(IMAGE *imgD, IMAGE *imgS, int low1, int high1, int low2, int high2);
_EXTERN_C void imgResize(IMAGE *imgD, IMAGE *imgS, int Method);
_EXTERN_C void mtxReduceExp2FromImage(MATRIX *mtxD, IMAGE *imgS, int x_q, int y_q, void *Buffer);
_EXTERN_C void imgGlobalStretching(IMAGE *img, int CutPercentage);


#endif /* _IMG_H */