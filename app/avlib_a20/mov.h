#ifndef AV_MOV_H
#define AV_MOV_H


/* the QuickTime file format is quite convoluted...
 * it has lots of index tables, each indexing something in another one...
 * Here we just use what is needed to read the chunks
 */

typedef struct {
    int count;
    int duration;
} MOVStts;

typedef struct {
    int first;
    int count;
    int id;
} MOVStsc;

typedef struct {
    uint32_t type;
    char *path;
    char *dir;
    char volume[28];
    char filename[64];
    int16_t nlvl_to, nlvl_from;
} MOVDref;

typedef struct {
    uint32_t type;
    int64_t size; /* total size (excluding the size and type fields) */
} MOVAtom;

struct MOVParseTableEntry;

typedef struct {
    unsigned track_id;
    uint64_t base_data_offset;
    uint64_t moof_offset;
    unsigned stsd_id;
    unsigned duration;
    unsigned size;
    unsigned flags;
} MOVFragment;

typedef struct {
    unsigned track_id;
    unsigned stsd_id;
    unsigned duration;
    unsigned size;
    unsigned flags;
} MOVTrackExt;

typedef struct MOVStreamContext {
    ByteIOContext *pb;
    int ffindex;          ///< AVStream index
    int next_chunk;
    unsigned int chunk_count;
    int64_t *chunk_offsets;
    unsigned int stts_count;
    MOVStts *stts_data;
    unsigned int ctts_count;
    MOVStts *ctts_data;
    unsigned int stsc_count;
    MOVStsc *stsc_data;
    unsigned int stps_count;
    unsigned *stps_data;  ///< partial sync sample for mpeg-2 open gop
    int ctts_index;
    int ctts_sample;
    unsigned int sample_size;
    unsigned int sample_count;
    int *sample_sizes;
    unsigned int keyframe_count;
    int *keyframes;
    int time_scale;
    int time_offset;      ///< time offset of the first edit list entry
    int current_sample;
    unsigned int bytes_per_frame;
    unsigned int samples_per_frame;
    int dv_audio_container;
    int pseudo_stream_id; ///< -1 means demux all ids
    int16_t audio_cid;    ///< stsd audio compression id
    unsigned drefs_count;
    MOVDref *drefs;
    int dref_id;
    int wrong_dts;        ///< dts are wrong due to huge ctts offset (iMovie files)
    int width;            ///< tkhd width
    int height;           ///< tkhd height
    int dts_shift;        ///< dts shift when ctts is negative
} MOVStreamContext;

typedef struct MOVContext {
	void		*player;
	void		*format;
	int			time_scale;
	int64_t		duration;		///< duration of the longest track
	int			found_moov;		///< 'moov' atom has been found
	int			found_mdat;		///< 'mdat' atom has been found
	int			isom;			///< 1 if file is ISO Media (mp4/3gp)
	MOVFragment	fragment; ///< current fragment in moof atom
	MOVTrackExt	*trex_data;
	unsigned	trex_count;
	int			itunes_metadata;  ///< metadata are itunes style
	int			chapter_track;
} MOVContext;


#endif

