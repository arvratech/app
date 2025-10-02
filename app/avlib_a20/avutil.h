#ifndef AVUTIL_AVUTIL_H
#define AVUTIL_AVUTIL_H

#include "common.h"
#include "error.h"
#include "internal.h"
#include "bswap.h"
#include "mathematics.h"
#include "rational.h"
#include "intfloat_readwrite.h"
#include "avstream.h"


#define NTP_OFFSET		2208988800ULL
#define NTP_OFFSET_US	(NTP_OFFSET * 1000000ULL)


void *av_fast_realloc(void *ptr, unsigned int *size, unsigned int min_size);
void av_fast_malloc(void *ptr, unsigned int *size, unsigned int min_size);
int  av_add_index_entry(AV_STREAM_TRACK *track, int64_t pos, int64_t timestamp, int size, int distance, int flags);
int  av_index_search_timestamp(AV_STREAM_TRACK *track, int64_t wanted_timestamp, int flags);
int64_t av_gettime(void);
uint64_t ff_ntp_time(void);


#endif

