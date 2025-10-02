#ifndef _AVVE_H_
#define _AVVE_H_

#include <linux/videodev2.h>
#include "v4l.h"

typedef struct _VE_CONTEXT {
	int				ve;
	unsigned char	state;
	int				width, height;
	int				fps;
	void			*stream;
	unsigned long	sampleCount;
} VE_CONTEXT;


#define VE_STATE_NULL			0
#define VE_STATE_IDLE			1
#define VE_STATE_BUSY			2


int  veOpen(VE_CONTEXT *ve, void *av);
void veClose(VE_CONTEXT *ve);
int  veRecord(VE_CONTEXT *ve);
void veStop(VE_CONTEXT *ve);
int  veOpenCodec(VE_CONTEXT *ve);
void veCloseCodec(VE_CONTEXT *ve);
void veEncodeFrame(VE_CONTEXT *ve, V4L_PICT *pict);


#endif

