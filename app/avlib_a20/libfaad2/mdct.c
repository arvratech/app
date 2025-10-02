#include "common.h"
#include "structs.h"

#include <stdlib.h>
#ifdef _WIN32_WCE
#define assert(x)
#else
#include <assert.h>
#endif

#include "cfft.h"
#include "mdct.h"
//#include "mdct_tab.h"

#if 1
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include "w55fa92_aac.h"

static int aacHandle = -1;
#endif

#if 0
static unsigned long g_totaltime;
static unsigned long g_number;
#endif


void faad_mdct_init(int aac)
{
	aacHandle = aac;
}

void faad_imdct(int N, real_t *X_in, real_t *X_out)
{
	aac_dec_ctx_s	decoder;

	decoder.i32Size	= N;
	decoder.inbuf	= (int *)X_in;
	decoder.outbuf	= (int *)X_out;
	ioctl(aacHandle, AAC_IOCSDEC, &decoder);
}

#ifdef LTP_DEC
void faad_mdct(int N, real_t *X_in, real_t *X_out)
{
}
#endif

