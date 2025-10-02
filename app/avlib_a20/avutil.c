#include <strings.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "avmem.h"
#include "avstring.h"
#include "avutil.h"
#undef NDEBUG
#include <assert.h>


void *av_fast_realloc(void *ptr, unsigned int *size, unsigned int min_size)
{
	if(min_size < *size)
	return ptr;
	*size = FFMAX(17*min_size/16 + 32, min_size);
	ptr = av_realloc(ptr, *size);
	if(!ptr) //we could set this to the unmodified min_size but this is safer if the user lost the ptr and uses NULL now
		*size= 0;
    return ptr;
}

void av_fast_malloc(void *ptr, unsigned int *size, unsigned int min_size)
{
	void	**p;

	p = ptr;
	if(min_size < *size)
		return;
	*size = FFMAX(17*min_size/16 + 32, min_size);
	av_free(*p);
	*p = av_malloc(*size);
	if(!*p) *size = 0;
}

int av_add_index_entry(AV_STREAM_TRACK *track, int64_t pos, int64_t timestamp, int size, int distance, int flags)
{
	AVIndexEntry	*entries, *ie;
	int		index;

	if((unsigned)track->nb_index_entries + 1 >= UINT_MAX / sizeof(AVIndexEntry))
		return -1;
    entries = av_fast_realloc(track->index_entries, &track->index_entries_allocated_size,
							(track->nb_index_entries + 1) * sizeof(AVIndexEntry));
	if(!entries)
		return -1;
	track->index_entries = entries;
	index = av_index_search_timestamp(track, timestamp, AVSEEK_FLAG_ANY);
	if(index < 0) {
		index = track->nb_index_entries++;
		ie = &entries[index];
		assert(index==0 || ie[-1].timestamp < timestamp);
	} else {
		ie = &entries[index];
		if(ie->timestamp != timestamp) {
			if(ie->timestamp <= timestamp)
				return -1;
			memmove(entries+index+1, entries+index, sizeof(AVIndexEntry)*(track->nb_index_entries-index));
			track->nb_index_entries++;
		} else if(ie->pos == pos && distance < ie->min_distance) //do not reduce the distance
			distance = ie->min_distance;
	}
	ie->pos = pos;
	ie->timestamp = timestamp;
	ie->min_distance = distance;
	ie->size = size;
	ie->flags = flags;
	return index;
}

int av_index_search_timestamp(AV_STREAM_TRACK *track, int64_t wanted_timestamp, int flags)
{
	AVIndexEntry	*entries;
	int64_t		timestamp;
	int		a, b, m, nb_entries;

	nb_entries = track->nb_index_entries;
	entries = track->index_entries;
	a = - 1;
	b = nb_entries;
	//optimize appending index entries at the end
	if(b && entries[b-1].timestamp < wanted_timestamp) a = b-1;
	while(b - a > 1) {
		m = (a + b) >> 1;
		timestamp = entries[m].timestamp;
		if(timestamp >= wanted_timestamp) b = m;
		if(timestamp <= wanted_timestamp) a = m;
	}
	m= (flags & AVSEEK_FLAG_BACKWARD) ? a : b;
	if(!(flags & AVSEEK_FLAG_ANY)) {
		while(m>=0 && m<nb_entries && !(entries[m].flags & AVINDEX_KEYFRAME)){
			m += (flags & AVSEEK_FLAG_BACKWARD) ? -1 : 1;
		}
	}
	if(m == nb_entries) return -1;
	return  m;
}

int64_t av_gettime(void)
{
	struct timeval tv;

	gettimeofday(&tv, NULL);
	return (int64_t)tv.tv_sec * 1000000 + tv.tv_usec;
}

uint64_t ff_ntp_time(void)
{
  return (av_gettime() / 1000) * 1000 + NTP_OFFSET_US;
}

/*
void av_set_pts_info(AVStream *s, int pts_wrap_bits, unsigned int pts_num, unsigned int pts_den)
{
    s->pts_wrap_bits = pts_wrap_bits;

    if(av_reduce(&s->time_base.num, &s->time_base.den, pts_num, pts_den, INT_MAX)){
        if(s->time_base.num != pts_num)
            av_log(NULL, AV_LOG_DEBUG, "st:%d removing common factor %d from timebase\n", s->index, pts_num/s->time_base.num);
    }else
        av_log(NULL, AV_LOG_WARNING, "st:%d has too large timebase, reducing\n", s->index);

    if(!s->time_base.num || !s->time_base.den)
        s->time_base.num= s->time_base.den= 0;
}
*/

