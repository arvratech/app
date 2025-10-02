#ifndef AVFORMAT_MOVENC_H
#define AVFORMAT_MOVENC_H


#define MOV_INDEX_CLUSTER_SIZE	16384
#define MOV_TIMESCALE			1000

#define RTP_MAX_PACKET_SIZE		1450

#define MODE_MP4				0x01
#define MODE_MOV				0x02

typedef struct MOVIentry {
	unsigned int size;
	uint64_t     pos;
    unsigned int samplesInChunk;
	unsigned int entries;
	int          cts;
	int64_t      dts;
#define MOV_SYNC_SAMPLE         0x0001
#define MOV_PARTIAL_SYNC_SAMPLE 0x0002
	uint32_t     flags;
} MOVIentry;


typedef struct MOVIndex {
    int         mode;
    int         entry;
    unsigned    timescale;
    uint64_t    time;
    int64_t     trackDuration;
    long        sampleCount;
    long        sampleSize;
    int         hasKeyframes;
#define MOV_TRACK_CTTS         0x0001
#define MOV_TRACK_STPS         0x0002
    uint32_t    flags;
    int         language;
    int         trackID;
    int         tag; ///< stsd fourcc
    int         vosLen;
    uint8_t     *vosData;
    MOVIentry   *cluster;
    int         audio_vbr;
    uint32_t    tref_tag;
    int         tref_id; ///< trackID of the referenced track
    int         hint_track;   ///< the track that hints this track, -1 if no hint track is set
    int         src_track;    ///< the track that this hint track describes
    uint32_t    prev_rtp_ts;
	void		*format;
} MOVTrack;

typedef struct MOVMuxContext {
	int			mode;
	int64_t		time;
	int			nb_streams;
	int			chapter_track; ///< qt chapter track number
	int64_t		mdat_pos;
	uint64_t	mdat_size;
	MOVTrack	tracks[2];
} MOVMuxContext;


#endif

