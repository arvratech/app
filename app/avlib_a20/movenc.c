#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "error.h"
#include "avstream.h"
#include "avrecorder.h"
#include "avmem.h"
#include "avstring.h"
#include "avio.h"
#include "mathematics.h"
#include "movenc.h"
#undef NDEBUG
#include <assert.h>


//FIXME support 64 bit variant with wide placeholders
static int64_t updateSize(ByteIOContext *bio, int64_t pos)
{
	int64_t		curpos;

	curpos = url_ftell(bio);
	url_fseek(bio, pos, SEEK_SET);
	put_be32(bio, curpos - pos);		// rewrite size
	url_fseek(bio, curpos, SEEK_SET);
	return curpos - pos;
}

static int mov_write_avcc_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t	pos;

printf("avcC...%d\n", trk->vosLen);
    pos = url_ftell(bio);
	put_be32(bio, 0);
	put_tag(bio, "avcC");
	ff_isom_write_avcc(bio, trk->vosData, trk->vosLen);
	return updateSize(bio, pos);
}

static int mov_write_video_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	char	compressor_name[32];
	int64_t pos;

	pos = url_ftell(bio);
printf("%c%c%c%c...\n", trk->tag & 0xff, (trk->tag >> 8) & 0xff, (trk->tag >> 16) & 0xff, (trk->tag >> 24) & 0xff);
	put_be32(bio, 0);			// size
	put_le32(bio, trk->tag);	// store it byteswapped
	put_be32(bio, 0);			// Reserved
	put_be16(bio, 0);			// Reserved
	put_be16(bio, 1);			// Data-reference index

	put_be16(bio, 0);			// Version
	put_be16(bio, 0);			// Revision level
	put_tag(bio, "arvr");		// Vendor
	put_be32(bio, 0x200);		// Temporal quality = normal
	put_be32(bio, 0x200);		// Spatial quality = normal
	track = trk->format;
	put_be16(bio, track->width);	// Width
	put_be16(bio, track->height);	// Height
	put_be32(bio, 0x00480000);	// Horizontal resolution(72dpi)
	put_be32(bio, 0x00480000);	// Vertical resolution(72dpi0
	put_be32(bio, 0);			// Data size(0)
	put_be16(bio, 1);			// Frame count(1)
	memset(compressor_name, 0, 32);
	strncpy(compressor_name, "avc", 31);
	put_byte(bio, strlen(compressor_name));
	put_buffer(bio, compressor_name, 31);
	put_be16(bio, 24);			// Depth
	put_be16(bio, 0xffff);		// Reserved
	mov_write_avcc_tag(bio, trk);
	return updateSize(bio, pos);
}

/*
static unsigned int descrLength(unsigned int len)
{
	int		i;

	for(i = 1;len >> (7*i);i++) ;
	return len + 1 + i;
}

static void putDescr(ByteIOContext *bio, int tag, unsigned int size)
{
	int		i;

	i = descrLength(size) - size - 2;
	put_byte(bio, tag);
	for( ;i > 0;i--) put_byte(bio, (size>>(7*i)) | 0x80);
	put_byte(bio, size & 0x7f);
}

static int mov_write_esds_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	int64_t		pos;
	int		decoderSpecificInfoLen;

	pos = url_ftell(bio);
	decoderSpecificInfoLen = trk->vosLen ? descrLength(trk->vosLen):0;
	put_be32(bio, 0);		// size
	put_tag(bio, "esds");
	put_be32(bio, 0);		// Version & flag
	// Elemetary Stream Descriptor
	putDescr(bio, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) + descrLength(1));
	put_be16(bio, trk->trkID);
	put_byte(bio, 0x00); // flags (= no flags)
	// DecoderConfig descriptor
	putDescr(bio, 0x04, 13 + decoderSpecificInfoLen);
	// Object type indication
	put_byte(bio, ff_codec_get_tag(ff_mp4_obj_type, trk->enc->codec_id));
	// the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
	// plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
	track = trk->format;
	if(tractrack->mediaType == AV_MEDIA_TYPE_AUDIO)
		put_byte(bio, 0x15); // flags (= Audiostream)
	else
		put_byte(bio, 0x11); // flags (= Visualstream)
	put_byte(bio,  trk->enc->rc_buffer_size>>(3+16));    // Buffersize DB (24 bits)
	put_be16(bio, (trk->enc->rc_buffer_size>>3)&0xFFFF); // Buffersize DB
	put_be32(bio, FFMAX(trk->enc->bit_rate, trk->enc->rc_max_rate)); // maxbitrate (FIXME should be max rate in any 1 sec window)
	if(trk->enc->rc_max_rate != trk->enc->rc_min_rate || trk->enc->rc_min_rate==0)
		put_be32(bio, 0); // vbr
	else
		put_be32(bio, trk->enc->rc_max_rate); // avg bitrate
	if(trk->vosLen) {
		// DecoderSpecific info descriptor
		putDescr(bio, 0x05, trk->vosLen);
		put_buffer(bio, trk->vosData, trk->vosLen);
	}
	// SL descriptor
	putDescr(bio, 0x06, 1);
	put_byte(bio, 0x02);
	return updateSize(bio, pos);
}

static int mov_write_audio_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t		pos = url_ftell(bio);
    uint32_t	tag = trk->tag;
    int		version = 0;

	pos = url_ftell(bio);
    if(trk->mode == MODE_MOV) {
        if(trk->timescale > UINT16_MAX) {
            version = 2;
        } else if(trk->audio_vbr) {
            version = 1;
        }
    }
    put_be32(bio, 0);		// size
    put_le32(bio, tag);		// store it byteswapped
    put_be32(bio, 0);		// Reserved 
    put_be16(bio, 0);		// Reserved
    put_be16(bio, 1);		// Data-reference index, XXX  == 1
	// SoundDescription
    put_be16(bio, version);	// Version
    put_be16(bio, 0);		// Revision level
    put_be32(bio, 0);		// Reserved
    if(version == 2) {
        put_be16(bio, 3);
        put_be16(bio, 16);
        put_be16(bio, 0xfffe);
        put_be16(bio, 0);
        put_be32(bio, 0x00010000);
        put_be32(bio, 72);
        put_be64(bio, av_dbl2int(trk->timescale));
        put_be32(bio, trk->enc->channels);
        put_be32(bio, 0x7F000000);
        put_be32(bio, av_get_bits_per_sample(trk->enc->codec_id));
        put_be32(bio, mov_get_lpcm_flags(trk->enc->codec_id));
        put_be32(bio, trk->sampleSize);
        put_be32(bio, trk->enc->frame_size);
    } else {
        if(trk->mode == MODE_MOV) {
            put_be16(bio, trk->enc->channels);
            if(trk->enc->codec_id == CODEC_ID_PCM_U8 ||
                trk->enc->codec_id == CODEC_ID_PCM_S8)
                put_be16(bio, 8);	// bits per sample
            else
                put_be16(bio, 16);
            put_be16(bio, trk->audio_vbr ? -2 : 0); // compression ID
        } else {		// reserved for mp4/3gp
            put_be16(bio, 2);
            put_be16(bio, 16);
            put_be16(bio, 0);
        }
        put_be16(bio, 0);	// packet size (= 0)
        put_be16(bio, trk->timescale);	// Time scale
        put_be16(bio, 0);	// Reserved
    }
    if(version == 1) {		// SoundDescription V1 extended info
        put_be32(bio, trk->enc->frame_size);	// Samples per packet
        put_be32(bio, trk->sampleSize / trk->enc->channels);	// Bytes per packet
        put_be32(bio, trk->sampleSize);		// Bytes per frame
        put_be32(bio, 2);	// Bytes per sample
    }
//	if(trk->tag == MKTAG('m','p','4','a'))
    mov_write_esds_tag(bio, trk);
    return updateSize(bio, pos);
}
*/

static int mov_write_stsd_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	int64_t		pos;

printf("stsd...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0);		// size
	put_tag(bio, "stsd");
	put_be32(bio, 0);		// version & flags
	put_be32(bio, 1);		// entry count
	track = trk->format;
    if(track->mediaType == AV_MEDIA_TYPE_VIDEO)
		mov_write_video_tag(bio, trk);
//	else if(track->mediaType == AV_MEDIA_TYPE_AUDIO)
//		mov_write_audio_tag(bio, trk);
	return updateSize(bio, pos);
}

// Time to sample atom
static int mov_write_stts_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int		duration;

	put_be32(bio, 24);		// size
	put_tag(bio, "stts");
	put_be32(bio, 0);		// version & flags
	put_be32(bio, 1);		// entry count
	put_be32(bio, trk->entry);	// sample count
	duration = trk->trackDuration/trk->entry;
	put_be32(bio, duration);	// sample duration
//put_be32(bio, av_rescale_rnd(trk->trackDuration, MOV_TIMESCALE, trk->timescale, AV_ROUND_UP));
	return 24;
}

// Sync sample atom
static int mov_write_stss_tag(ByteIOContext *bio, MOVTrack *trk)
{
    int64_t		pos, curpos, entryPos;
    int		i, entry;

    pos = url_ftell(bio);
    put_be32(bio, 0);		// size
    put_tag(bio, "stss");
    put_be32(bio, 0);		// version & flags
    entryPos = url_ftell(bio);
    put_be32(bio, 0);		// number of entries
    for(i = entry = 0;i < trk->entry;i++) {
        if(trk->cluster[i].flags & MOV_SYNC_SAMPLE) {
            put_be32(bio, i+1);
            entry++;
        }
    }
    curpos = url_ftell(bio);
    url_fseek(bio, entryPos, SEEK_SET);
    put_be32(bio, entry);	// rewrite  nuumber of entries
    url_fseek(bio, curpos, SEEK_SET);
    return updateSize(bio, pos);
}

// Sample to chunk atom
static int mov_write_stsc_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t		pos;

printf("stsc...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0);		// size
	put_tag(bio, "stsc");
	put_be32(bio, 0);		// version & flags
	put_be32(bio, 1);		// number of entries
	put_be32(bio, 1);		// first chunk
	put_be32(bio, 1);		// samples per chunk
	put_be32(bio, 0x1);		// sample description index
	return updateSize(bio, pos);
}

// Sample size atom
static int mov_write_stsz_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t		pos;
	int		i;

	pos = url_ftell(bio);
printf("stsz...entry=%d\n", trk->entry);
	put_be32(bio, 0);		// size
	put_tag(bio, "stsz");
	put_be32(bio, 0);		// version & flags
	put_be32(bio, 0);		// sample size
	put_be32(bio, trk->entry);	// sample count
	for(i = 0;i < trk->entry;i++) {
		put_be32(bio, trk->cluster[i].size);
	}
	return updateSize(bio, pos);
}

// Chunk offset atom
static int mov_write_stco_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t		pos;
	int		i, mode64;

	pos = url_ftell(bio);
printf("stco...entry=%d\n", trk->entry);
	put_be32(bio, 0);		// size
	if(pos > UINT32_MAX) {
		mode64 = 1;
		put_tag(bio, "co64");
	} else {
		put_tag(bio, "stco");
		mode64 = 0;
	}
	put_be32(bio, 0);		// version & flags
	put_be32(bio, trk->entry); // entry count
	for(i = 0;i < trk->entry;i++) {
        if(mode64) put_be64(bio, trk->cluster[i].pos);
        else	   put_be32(bio, trk->cluster[i].pos);
	}
	return updateSize(bio, pos);
}

typedef struct {
	int		count;
	int		duration;
} MOVStts;

static int mov_write_ctts_tag(ByteIOContext *bio, MOVTrack *trk)
{
    MOVStts *ctts_entries;
    uint32_t entries = 0;
    uint32_t atom_size;
    int i;

    ctts_entries = av_malloc((trk->entry + 1) * sizeof(*ctts_entries)); // worst case
    ctts_entries[0].count = 1;
    ctts_entries[0].duration = trk->cluster[0].cts;
    for(i = 1;i < trk->entry;i++) {
        if (trk->cluster[i].cts == ctts_entries[entries].duration) {
            ctts_entries[entries].count++; // compress
        } else {
            entries++;
            ctts_entries[entries].duration = trk->cluster[i].cts;
            ctts_entries[entries].count = 1;
        }
    }
    entries++;		// last one
    atom_size = 16 + (entries * 8);
    put_be32(bio, atom_size);	// size
    put_tag(bio, "ctts");
    put_be32(bio, 0);			// version & flags
    put_be32(bio, entries); 	// entry count
    for(i = 0;i < entries;i++) {
        put_be32(bio, ctts_entries[i].count);
        put_be32(bio, ctts_entries[i].duration);
    }
    av_free(ctts_entries);
    return atom_size;
}

static int mov_write_stbl_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	int64_t		pos;

printf("stbl...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0);	// size
	put_tag(bio, "stbl");
	mov_write_stsd_tag(bio, trk);
	mov_write_stts_tag(bio, trk);
	track = trk->format;
	if(track->mediaType == AV_MEDIA_TYPE_VIDEO)
		mov_write_stss_tag(bio, trk);
    if(track->mediaType == AV_MEDIA_TYPE_VIDEO && trk->flags & MOV_TRACK_CTTS)
		mov_write_ctts_tag(bio, trk);
	mov_write_stsc_tag(bio, trk);
	mov_write_stsz_tag(bio, trk);
	mov_write_stco_tag(bio, trk);
	return updateSize(bio, pos);
}

static int mov_write_dref_tag(ByteIOContext *bio)
{
printf("dref...\n");
	put_be32(bio, 28);		// size
	put_tag(bio, "dref");
	put_be32(bio, 0);		// version & flags
	put_be32(bio, 1);		// entry count
printf("url ...\n");
	put_be32(bio, 0xc);		// size
	put_tag(bio, "url ");
	put_be32(bio, 1);		// version & flags
    return 28;
}

static int mov_write_dinf_tag(ByteIOContext *bio)
{
	int64_t		pos;

printf("dinf...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0);	// size
	put_tag(bio, "dinf");
	mov_write_dref_tag(bio);
    return updateSize(bio, pos);
}

static int mov_write_smhd_tag(ByteIOContext *bio)
{
	put_be32(bio, 16);	// size
	put_tag(bio, "smhd");
	put_be32(bio, 0);	// version & flags
	put_be16(bio, 0);	// reserved (balance, normally = 0)
	put_be16(bio, 0);	// reserved
    return 16;
}

static int mov_write_vmhd_tag(ByteIOContext *bio)
{
printf("vmhd...\n");
	put_be32(bio, 0x14);	// size (always 0x14)
	put_tag(bio, "vmhd");
	put_be32(bio, 0x01);	// version & flags
	put_be64(bio, 0);	// reserved (graphics mode = copy)
	return 0x14;
}

static int mov_write_hdlr_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	char	hdlr[8], hdlr_type[8], descr[32];
	int64_t	pos;

printf("hdlr...\n");
	pos = url_ftell(bio);
	track = trk->format;
    if(!trk) {	// no media --> data handler
		strcpy(hdlr, "dhlr");
		strcpy(hdlr_type, "url ");
		strcpy(descr, "DataHandler");
    } else {
		strcpy(hdlr, "\0\0\0\0");	// mhlr
		if(track->mediaType == AV_MEDIA_TYPE_VIDEO) {
			strcpy(hdlr_type, "vide");
			strcpy(descr, "VideoHandler");
		} else if(track->mediaType == AV_MEDIA_TYPE_AUDIO) {
			strcpy(hdlr_type, "soun");
			strcpy(descr, "SoundHandler");
		}
	}
//printf("%s-%s-%s\n", hdlr, hdlr_type, descr);
	put_be32(bio, 0);			// size
	put_tag(bio, "hdlr");
	put_be32(bio, 0);			// Version & flags
	put_buffer(bio, hdlr, 4);	// handler
	put_tag(bio, hdlr_type);	// handler type
	put_be32(bio, 0);			// reserved
	put_be32(bio, 0);			// reserved
	put_be32(bio, 0);			// reserved
	put_buffer(bio, descr, strlen(descr));	// handler description
	put_byte(bio, 0);			// c null terminated string
	return updateSize(bio, pos);
}

static int mov_write_minf_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	int64_t		pos;

    pos = url_ftell(bio);
printf("minf...\n");
	put_be32(bio, 0);	// size
	put_tag(bio, "minf");
	track = trk->format;
	if(track->mediaType == AV_MEDIA_TYPE_VIDEO) {
		mov_write_vmhd_tag(bio);
	} else if(track->mediaType == AV_MEDIA_TYPE_AUDIO) {
		mov_write_smhd_tag(bio);
	}
	mov_write_dinf_tag(bio);
	mov_write_stbl_tag(bio, trk);
	return updateSize(bio, pos);
}

static int mov_write_mdhd_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int		version, rval;

	version = trk->trackDuration < INT32_MAX ? 0 : 1;
	if(version) put_be32(bio, 44);
	else		put_be32(bio, 32);	// size
printf("mdhd...%d\n", version);
	put_tag(bio, "mdhd");
	put_byte(bio, version);
	put_be24(bio, 0);	// flags
    if(version) {
		put_be64(bio, trk->time);
		put_be64(bio, trk->time);
	} else {
		put_be32(bio, trk->time);	// creation time
		put_be32(bio, trk->time);	// modification time
	}
	put_be32(bio, trk->timescale);	// time scale (sample rate for audio)
	if(version) put_be64(bio, trk->trackDuration);
	else		put_be32(bio, trk->trackDuration);	// duration
    put_be16(bio, trk->language);	// language
    put_be16(bio, 0);				// reserved (quality)
	if(version) rval = 44;
	else		rval = 32;
    return rval;
}

static int mov_write_mdia_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t pos;

printf("mdia...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0); /* size */
    put_tag(bio, "mdia");
	mov_write_mdhd_tag(bio, trk);
	mov_write_hdlr_tag(bio, trk);
	mov_write_minf_tag(bio, trk);
	return updateSize(bio, pos);
}

static int mov_write_tkhd_tag(ByteIOContext *bio, MOVTrack *trk)
{
	AV_STREAM_TRACK	*track;
	int64_t		duration;
    int		version, rval;

	duration = av_rescale_rnd(trk->trackDuration, MOV_TIMESCALE, trk->timescale, AV_ROUND_UP);
printf("tkhd: duration=%lld dur=%lld\n", trk->trackDuration, duration);
    version = duration < INT32_MAX ? 0 : 1;
printf("tkhd...%d\n", version);
	if(version) put_be32(bio, 104);
	else		put_be32(bio, 92);	// size
	put_tag(bio, "tkhd");
	put_byte(bio, version);
	put_be24(bio, 0xf);		// flags (track enabled)
	if(version) {
		put_be64(bio, trk->time);
		put_be64(bio, trk->time);
	} else {
		put_be32(bio, trk->time);	// creation time
		put_be32(bio, trk->time);	// modification time
	}
	put_be32(bio, trk->trackID);	// track-id
	put_be32(bio, 0);				// reserved
	if(version) put_be64(bio, duration);
	else		put_be32(bio, duration);
	put_be32(bio, 0);		// reserved
	put_be32(bio, 0);		// reserved
	put_be32(bio, 0x0);		// reserved (Layer & Alternate group)
	track = trk->format;
	// Volume, only for audio
	if(track->mediaType == AV_MEDIA_TYPE_AUDIO)
		put_be16(bio, 0x0100);
	else
		put_be16(bio, 0);
	put_be16(bio, 0);	// reserved
	// Matrix structure
	put_be32(bio, 0x00010000);	// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x00010000);	// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x0);			// reserved
	put_be32(bio, 0x40000000);	// reserved
	// Track width and height, for visual only
	if(track->mediaType == AV_MEDIA_TYPE_VIDEO) {
		put_be32(bio, track->width*0x10000);
		put_be32(bio, track->height*0x10000);
	} else {
		put_be32(bio, 0);
		put_be32(bio, 0);
    }
	if(version == 1) rval = 104;
	else	rval = 92;
	return rval;
}

// This box seems important for the psp playback ... without it the movie seems to hang
static int mov_write_edts_tag(ByteIOContext *bio, MOVTrack *trk)
{
printf("edts...\n");
	put_be32(bio, 0x24);		// size 
	put_tag(bio, "edts");
	put_be32(bio, 0x1c);		// size
    put_tag(bio, "elst");
	put_be32(bio, 0x0);			// version(1)+flag(3)
	put_be32(bio, 0x1);			// number of edit list entries
	put_be32(bio, av_rescale_rnd(trk->trackDuration, MOV_TIMESCALE, trk->timescale, AV_ROUND_UP));
	put_be32(bio, trk->cluster[0].cts);	// first pts is cts since dts is 0
	put_be32(bio, 0x00010000);
    return 0x24;
}

static int mov_write_trak_tag(ByteIOContext *bio, MOVTrack *trk)
{
	int64_t		pos;

printf("trak...\n");
	pos = url_ftell(bio);
	put_be32(bio, 0);	// size
	put_tag(bio, "trak");
	mov_write_tkhd_tag(bio, trk);
	mov_write_edts_tag(bio, trk);
	mov_write_mdia_tag(bio, trk);
	return updateSize(bio, pos);
}

static int mov_write_mvhd_tag(ByteIOContext *bio, MOVMuxContext *mov)
{
	MOVTrack	*trk;
    int64_t maxTrackDur, duration;
    int		i, version, maxTrackID;

printf("mvhd...\n");
	maxTrackDur = 0; maxTrackID = 1;
	for(i = 0;i < mov->nb_streams;i++) {
		trk = &mov->tracks[i];
		if(trk->entry > 0) {
			duration = av_rescale_rnd(trk->trackDuration, MOV_TIMESCALE, trk->timescale, AV_ROUND_UP);
printf("dur=%lld itimescale=%d = %d\n", trk->trackDuration, trk->timescale, duration);
			if(maxTrackDur < duration) maxTrackDur = duration;
			if(maxTrackID < trk->trackID) maxTrackID = mov->tracks[i].trackID;
        }
    }
	version = maxTrackDur < UINT32_MAX ? 0 : 1;
	if(version) put_be32(bio, 120);
	else		put_be32(bio, 108);	// size
	put_tag(bio, "mvhd");
	put_byte(bio, version);
	put_be24(bio, 0);	// flags
	if(version) {
		put_be64(bio, mov->time);
		put_be64(bio, mov->time);
	} else {
		put_be32(bio, mov->time);	// creation time
		put_be32(bio, mov->time);	// modification time
	}
	put_be32(bio, MOV_TIMESCALE);
	if(version) put_be64(bio, maxTrackDur);
	else	    put_be32(bio, maxTrackDur);	// duration of longest track
	put_be32(bio, 0x00010000);		// reserved (preferred rate) 1.0 = normal
	put_be16(bio, 0x0100);			// reserved (preferred volume) 1.0 = normal
	put_be16(bio, 0);				// reserved
	put_be32(bio, 0);				// reserved
	put_be32(bio, 0);				// reserved
	// Matrix structure
	put_be32(bio, 0x00010000);		// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x00010000);		// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x0);				// reserved
	put_be32(bio, 0x40000000);		// reserved
	// Predefines
	put_be32(bio, 0);				// reserved (preview time)
	put_be32(bio, 0);				// reserved (preview duration)
	put_be32(bio, 0);				// reserved (poster time)
	put_be32(bio, 0);				// reserved (selection time)
	put_be32(bio, 0);				// reserved (selection duration)
	put_be32(bio, 0);				// reserved (current time)
	put_be32(bio, maxTrackID+1);	// Next track id
	if(version)  i = 120;
	else	i = 108;
	return i;
}

// This needs to be more general
static int mov_write_ftyp_tag(ByteIOContext *bio, AV_RECORDER *recorder)
{
    MOVMuxContext	*mov;
    int64_t		pos;

    mov = recorder->priv_data;
    pos = url_ftell(bio);
	put_be32(bio, 0);
	put_tag(bio, "ftyp");
	put_tag(bio, "isom");
    put_be32(bio, 0x200);
	put_tag(bio, "isom");
	put_tag(bio, "iso2");
	put_tag(bio, "avc1");
	put_tag(bio, "mp41");
    return updateSize(bio, pos);
}

static int mov_write_moov_tag(ByteIOContext *bio, AV_RECORDER *recorder)
{
    MOVMuxContext	*mov;
	MOVTrack		*trk;
	int64_t		pos;
	int		i;

printf("moov...\n"); 
    mov = recorder->priv_data;
	pos = url_ftell(bio);
	put_be32(bio, 0);	// size placeholder
	put_tag(bio, "moov");
	mov_write_mvhd_tag(bio, mov);
    for(i = 0;i < mov->nb_streams;i++) {
		trk = &mov->tracks[i];
		if(trk->entry > 0) {
			mov_write_trak_tag(bio, trk);
		}
    }
//	mov_write_udta_tag(bio, mov, recorder);
	return updateSize(bio, pos);
}

static int mov_write_mdat_tag(ByteIOContext *bio, MOVMuxContext *mov)
{
printf("free...\n"); 
    put_be32(bio, 8);    // placeholder for extended size field (64 bit)
    put_tag(bio, "free");
printf("mdat...\n");
    mov->mdat_pos = url_ftell(bio);
    put_be32(bio, 0);	// size placeholder
    put_tag(bio, "mdat");
    return 0;
}

int mov_write_packet(AV_RECORDER *recorder, int formatIndex, AV_BUFFER *buffer)
{
	MOVMuxContext	*mov;
	ByteIOContext	*bio;
	MOVTrack		*trk;
	AV_STREAM_TRACK	*track;
	int		size;

	bio = recorder->bio; mov = recorder->priv_data;
	trk = &mov->tracks[formatIndex];
	track = avStreamTrackAt(recorder, formatIndex);
	size = buffer->size;
	if(!size) return 0; /* Discard 0 sized packets */
	if(url_is_streamed(bio)) return 0; /* Can't handle that */
    // copy extradata if it exists 
	if(trk->vosLen == 0 && track->extradataLen > 0) {
		trk->vosLen = track->extradataLen;
		trk->vosData = av_malloc(trk->vosLen);
		memcpy(trk->vosData, track->extradata, trk->vosLen);
printf("mov_write_packet: extradata=%d\n", track->extradataLen);
    }
/*
	if(enc->codec_id == CODEC_ID_H264 && pkt->vosData[0] != 1) {
		// from x264 or from bytestream h264, nal reformating needed
		size = ff_avc_parse_nal_units(bio, pkt->data, pkt->size);
	} else {
		put_buffer(bio, pkt->data, size);
	}
*/
//printf("write_packet: size=%d entry=%d\n", buffer->size, trk->entry);
	put_buffer(bio, buffer->data, size);
    if(!(trk->entry % MOV_INDEX_CLUSTER_SIZE)) {
		trk->cluster = av_realloc(trk->cluster, (trk->entry + MOV_INDEX_CLUSTER_SIZE) * sizeof(*trk->cluster));
		if(!trk->cluster)
            return -1;
    }
	trk->cluster[trk->entry].pos			= url_ftell(bio) - size;
	trk->cluster[trk->entry].samplesInChunk	= 1;
	trk->cluster[trk->entry].size			= size;
	trk->cluster[trk->entry].entries		= 1;
	trk->cluster[trk->entry].dts			= buffer->dts;
	trk->trackDuration = buffer->dts - trk->cluster[0].dts + buffer->duration;
//printf("%x duration=%lld dts=%lld %lld\n", buffer->flags, trk->trackDuration, buffer->dts, trk->cluster[0].dts);
	if(buffer->pts == AV_NOPTS_VALUE) {
		//av_log(s, AV_LOG_WARNING, "pts has no value\n");
		buffer->pts = buffer->dts;
	}
	if(buffer->dts != buffer->pts) trk->flags |= MOV_TRACK_CTTS;
	trk->cluster[trk->entry].cts = buffer->pts - buffer->dts;
	trk->cluster[trk->entry].flags = 0;
	if(buffer->flags & AV_FLAG_KEY_FRAME) {
		trk->cluster[trk->entry].flags = MOV_SYNC_SAMPLE;
		trk->hasKeyframes++;
	}
	trk->entry++;
	trk->sampleCount++;
	mov->mdat_size += size;
	put_flush_packet(bio);
    return 0;
}

int mov_write_header(AV_RECORDER *recorder)
{
	ByteIOContext	*bio;
	MOVMuxContext	*mov;
	AV_STREAM_TRACK	*track;
	MOVTrack		*trk;
    int		i, hint_track = 0;

	bio = recorder->bio; mov = recorder->priv_data;
	memset(mov, 0, sizeof(MOVMuxContext));
	mov->mode = MODE_MP4;
	mov->nb_streams = avStreamTrackCount(recorder);
	trk = &mov->tracks[0];
	track = avStreamTrack(recorder, AV_MEDIA_TYPE_VIDEO);
	trk->format = track;
	//AVMetadataTag *lang = av_metadata_get(st->metadata, "language", NULL, 0);
	trk->time		= mov->time;
	trk->trackID	= 1;
	trk->language	= 0;
	trk->mode		= mov->mode;
	trk->tag		=  MKTAG('a','v','c','1');
	trk->hint_track = -1;
	trk->timescale	= track->frameRateNum;
    trk->sampleSize = 0;
	trk->entry = 0;
	trk->sampleCount = 0;
//	av_set_pts_info(st, 64, 1, track->timescale);
	mov->time		= recorder->timestamp + 0x7C25B080;	// 1970 based -> 1904 based
	mov->mdat_size	= 0;
	mov_write_ftyp_tag(bio, recorder);
	mov_write_mdat_tag(bio, mov);
    put_flush_packet(bio);
    return 0;
}

int mov_write_trailer(AV_RECORDER *recorder)
{
	ByteIOContext	*bio;
	MOVMuxContext	*mov;
    int64_t		moov_pos;
    int		i;

	bio = recorder->bio; mov = recorder->priv_data;
printf("write_trailer...%d\n", mov->mdat_size);
    moov_pos = url_ftell(bio);
	// Write size of mdat tag
	if(mov->mdat_size+8 <= UINT32_MAX) {
		url_fseek(bio, mov->mdat_pos, SEEK_SET);
		put_be32(bio, mov->mdat_size+8);
    } else {
		// overwrite 'wide' placeholder atom
		url_fseek(bio, mov->mdat_pos - 8, SEEK_SET);
		put_be32(bio, 1);	// special value: real atom size will be 64 bit value after tag field
		put_tag(bio, "mdat");
		put_be64(bio, mov->mdat_size+16);
	}
	url_fseek(bio, moov_pos, SEEK_SET);
	mov_write_moov_tag(bio, recorder);
	for(i = 0;i < mov->nb_streams;i++) {
        if(mov->tracks[i].vosLen) av_free(mov->tracks[i].vosData);
    }
	put_flush_packet(bio);
    return 0;
}

