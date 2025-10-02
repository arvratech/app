#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "avutil.h"
#include "avmem.h"
#include "avstring.h"
#include "avio.h"
#include "mpeg4audio.h"
#include "avstream.h"
#include "avcodecs.h"
#include "avplayer.h"
#include "isom.h"
#include "mov.h"
#undef NDEBUG
#include <assert.h>


MOVStreamContext	_movsc[8];

// links atom IDs to parse functions
typedef struct MOVParseTableEntry {
    uint32_t type;
    int (*parse)(MOVContext *mov, ByteIOContext *bio, MOVAtom atom);
} MOVParseTableEntry;

static const MOVParseTableEntry mov_default_parse_table[];

static int mov_metadata_trkn(MOVContext *mov, ByteIOContext *bio, unsigned len)
{
	char buf[16];

	get_be16(bio); // unknown
	snprintf(buf, sizeof(buf), "%d", get_be16(bio));
//	av_metadata_set2(&mov->fc->metadata, "track", buf, 0);
	get_be16(bio); // total tracks
	return 0;
}

static const uint32_t mac_to_unicode[128] = {
    0x00C4,0x00C5,0x00C7,0x00C9,0x00D1,0x00D6,0x00DC,0x00E1,
    0x00E0,0x00E2,0x00E4,0x00E3,0x00E5,0x00E7,0x00E9,0x00E8,
    0x00EA,0x00EB,0x00ED,0x00EC,0x00EE,0x00EF,0x00F1,0x00F3,
    0x00F2,0x00F4,0x00F6,0x00F5,0x00FA,0x00F9,0x00FB,0x00FC,
    0x2020,0x00B0,0x00A2,0x00A3,0x00A7,0x2022,0x00B6,0x00DF,
    0x00AE,0x00A9,0x2122,0x00B4,0x00A8,0x2260,0x00C6,0x00D8,
    0x221E,0x00B1,0x2264,0x2265,0x00A5,0x00B5,0x2202,0x2211,
    0x220F,0x03C0,0x222B,0x00AA,0x00BA,0x03A9,0x00E6,0x00F8,
    0x00BF,0x00A1,0x00AC,0x221A,0x0192,0x2248,0x2206,0x00AB,
    0x00BB,0x2026,0x00A0,0x00C0,0x00C3,0x00D5,0x0152,0x0153,
    0x2013,0x2014,0x201C,0x201D,0x2018,0x2019,0x00F7,0x25CA,
    0x00FF,0x0178,0x2044,0x20AC,0x2039,0x203A,0xFB01,0xFB02,
    0x2021,0x00B7,0x201A,0x201E,0x2030,0x00C2,0x00CA,0x00C1,
    0x00CB,0x00C8,0x00CD,0x00CE,0x00CF,0x00CC,0x00D3,0x00D4,
    0xF8FF,0x00D2,0x00DA,0x00DB,0x00D9,0x0131,0x02C6,0x02DC,
    0x00AF,0x02D8,0x02D9,0x02DA,0x00B8,0x02DD,0x02DB,0x02C7,
};

static int mov_read_mac_string(MOVContext *mov, ByteIOContext *bio, int len, char *dst, int dstlen)
{
	char	*p = dst;
	char	*end = dst+dstlen-1;
	int		i;

	for(i = 0; i < len; i++) {
		uint8_t t, c = get_byte(bio);
		if(c < 0x80 && p < end) *p++ = c;
		else	PUT_UTF8(mac_to_unicode[c-0x80], t, if (p < end) *p++ = t;);
	}
	*p = 0;
    return p - dst;
}

static int mov_read_udta_string(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
#ifdef MOV_EXPORT_ALL_METADATA
    char tmp_key[5];
#endif
    char str[1024], key2[16], language[4] = {0};
    const char *key = NULL;
    uint16_t str_size, langcode = 0;
    uint32_t data_type = 0;
    int (*parse)(MOVContext*, ByteIOContext*, unsigned) = NULL;

    switch (atom.type) {
    case MKTAG(0xa9,'n','a','m'): key = "title";     break;
    case MKTAG(0xa9,'a','u','t'):
    case MKTAG(0xa9,'A','R','T'): key = "artist";    break;
    case MKTAG(0xa9,'w','r','t'): key = "composer";  break;
    case MKTAG( 'c','p','r','t'):
    case MKTAG(0xa9,'c','p','y'): key = "copyright"; break;
    case MKTAG(0xa9,'c','m','t'):
    case MKTAG(0xa9,'i','n','f'): key = "comment";   break;
    case MKTAG(0xa9,'a','l','b'): key = "album";     break;
    case MKTAG(0xa9,'d','a','y'): key = "date";      break;
    case MKTAG(0xa9,'g','e','n'): key = "genre";     break;
    case MKTAG(0xa9,'t','o','o'):
    case MKTAG(0xa9,'e','n','c'): key = "encoder";   break;
    case MKTAG( 'd','e','s','c'): key = "description";break;
    case MKTAG( 'l','d','e','s'): key = "synopsis";  break;
    case MKTAG( 't','v','s','h'): key = "show";      break;
    case MKTAG( 't','v','e','n'): key = "episode_id";break;
    case MKTAG( 't','v','n','n'): key = "network";   break;
    case MKTAG( 't','r','k','n'): key = "track";
        parse = mov_metadata_trkn; break;
    }
    if(mov->itunes_metadata && atom.size > 8) {
        int data_size = get_be32(bio);
        int tag = get_le32(bio);
        if(tag == MKTAG('d','a','t','a')) {
            data_type = get_be32(bio); // type
            get_be32(bio); // unknown
            str_size = data_size - 16;
            atom.size -= 16;
        } else return 0;
    } else if(atom.size > 4 && key && !mov->itunes_metadata) {
        str_size = get_be16(bio); // string length
        langcode = get_be16(bio);
        ff_mov_lang_to_iso639(langcode, language);
        atom.size -= 4;
    } else
        str_size = atom.size;
#ifdef MOV_EXPORT_ALL_METADATA
    if(!key) {
        snprintf(tmp_key, 5, "%.4s", (char*)&atom.type);
        key = tmp_key;
    }
#endif
    if(!key)
        return 0;
    if(atom.size < 0)
        return -1;
    str_size = FFMIN3(sizeof(str)-1, str_size, atom.size);
    if(parse) parse(mov, bio, str_size);
    else {
        if(data_type == 3 || (data_type == 0 && langcode < 0x800)) { // MAC Encoded
            mov_read_mac_string(mov, bio, str_size, str, sizeof(str));
        } else {
            get_buffer(bio, str, str_size);
            str[str_size] = 0;
        }
//		av_metadata_set2(&mov->fc->metadata, key, str, 0);
        if(*language && strcmp(language, "und")) {
            snprintf(key2, sizeof(key2), "%s-%s", key, language);
//			av_metadata_set2(&mov->fc->metadata, key2, str, 0);
        }
    }
#ifdef DEBUG_METADATA
    printf("lang \"%3s\" ", language);
    printf("tag \"%s\" value \"%s\" atom \"%.4s\" %d %lld\n",
			           key, str, (char*)&atom.type, str_size, atom.size);
#endif
    return 0;
}

static int mov_read_chpl(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    return 0;
}

static int mov_read_default(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	MOVAtom		a;
	int64_t		size;
	int		i;

	if(atom.size < 0) atom.size = INT64_MAX;
//printf("mov_read_default: %lld\n", atom.size);
	size = 0;
	while(size + 8 < atom.size && !url_feof(bio)) {
		int (*parse)(MOVContext*, ByteIOContext*, MOVAtom) = NULL;
		a.size = atom.size;
		a.type = 0;
		if(atom.size >= 8) {
			a.size = get_be32(bio);
			a.type = get_le32(bio);
		}
		size += 8;
//printf("type: %08x  %.4s  sz: %"PRIx64"  %"PRIx64"   %"PRIx64"\n",
//					a.type, (char*)&a.type, a.size, atom.size, size);
		if(a.size == 1) {	// 64 bit extended size
			a.size = get_be64(bio) - 8;
			size += 8;
		}
		if(a.size == 0) {
			a.size = atom.size - size;
			if(a.size <= 8) break;
		}
		a.size -= 8;
		if(a.size < 0) break;
		a.size = FFMIN(a.size, atom.size - size);
		for(i = 0;mov_default_parse_table[i].type;i++)
			if(mov_default_parse_table[i].type == a.type) {
				parse = mov_default_parse_table[i].parse;
				break;
			}
		// container is user data
		if(!parse && (atom.type == MKTAG('u','d','t','a') || atom.type == MKTAG('i','l','s','t')))
			parse = mov_read_udta_string;
		if(!parse) {	// skip leaf atoms data
			url_fskip(bio, a.size);
		} else {
			int64_t start_pos = url_ftell(bio);
			int64_t left;
			int err = parse(mov, bio, a);
			if(err < 0) return err;
			if(mov->found_moov && mov->found_mdat && (url_is_streamed(bio) || start_pos + a.size == url_fsize(bio)))
				return 0;
			left = a.size - url_ftell(bio) + start_pos;
			if(left > 0) // skip garbage at atom end 
				url_fskip(bio, left);
		}
		size += a.size;
	}
	if(size < atom.size && atom.size < 0x7ffff)
		url_fskip(bio, atom.size - size);
	return 0;
}

static int mov_read_dref(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    return 0;
}

static int mov_read_hdlr(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
    uint32_t	type, ctype;
	int		count;

	player = mov->player;
	track   = mov->format;
    get_be32(bio);			// version + flags
	// component type
    ctype = get_le32(bio);
    type  = get_le32(bio);	// component subtype 
    get_be32(bio);	// component  manufacture
    get_be32(bio);	// component flags
    get_be32(bio);	// component flags mask
	if(track) {
		if(type == MKTAG('v','i','d','e')) track->mediaType = AV_MEDIA_TYPE_VIDEO;
		else if(type == MKTAG('s','o','u','n')) track->mediaType = AV_MEDIA_TYPE_AUDIO;
		else if(type == MKTAG('s','u','b','p')) track->mediaType = AV_MEDIA_TYPE_SUBTITLE;
		else	track->mediaType = AV_MEDIA_TYPE_UNKNOWN;
printf("hdlr: mediaType=%d\n", track->mediaType);
	} else {
printf("hdlr: unexpected\n");
	}

    return 0;
}

static int mp4_read_descr_len(ByteIOContext *bio)
{
	int		c, len, count;

	len = 0; count = 4;
	while(count--) {
		c = get_byte(bio);
		len = (len << 7) | (c & 0x7f);
		if(c < 0x80) break;
	}
	return len;
}

#define ESDescrTag				0x03
#define DecoderConfigDescrTag	0x04
#define DecSpecificInfoTag		0x05
#define SLConfigDescrTag		0x06

int ff_mov_read_esds(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MPEG4AudioConfig	cfg;
	uint64_t	pos;
	int		size, tag, len, obj_type;

	player = mov->player;
	track   = mov->format;
    get_be32(bio);				// Version
	size = atom.size - 4;		// already atom.size is substracted 8
	// Elementary Stream descriptor
	while(size > 1) {
		pos = url_ftell(bio);
		tag = get_byte(bio);	// Tag	
		len = mp4_read_descr_len(bio);	// Length
//rintf("tag=%02x len=%d size=%d\n", tag, len, size);
		switch(tag) {
		case ESDescrTag:
			get_be16(bio);		// ES_ID
			get_byte(bio);		// StreamDepenenceFlag...
			break;
		case DecoderConfigDescrTag:
			obj_type = get_byte(bio);	// ObjectTypeIndication
			get_byte(bio);		// StreamType...
			get_be24(bio);		// BufferSizeDB
			get_be32(bio);		// MaxBitrate
			get_be32(bio);		// AvgBitrate
			// 0x40		MPEG-4 audio
			// 0x66		MPEG-4 ADTS main
			// 0x67		MPEG-4 ADTS Low Complexity = 103
			// 0x68		MPEG-4 ADTS Scalable Sampling Rate = 104
			if(obj_type != 0x40 && obj_type != 0x69 && (obj_type < 0x66 || obj_type > 0x68)) {
						printf("codec=0x%02x not supported\n", obj_type);
				return -1;
			}
			break;
		case DecSpecificInfoTag:
			if((uint64_t)len > (1<<30)) return -1;
			get_buffer(bio, track->extradata, len);
			track->extradataLen = len;
			ff_mpeg4audio_get_config(&cfg, track->extradata, track->extradataLen);
			track->channels	= cfg.channels;
			track->sampleRate	= cfg.sample_rate;
			break;
		case SLConfigDescrTag:
			get_byte(bio);		// Predefined
			break;
		default:
			printf("esds: unknown tag: 0x%02x\n", tag);
            url_fskip(bio, len);
		}
        size -= url_ftell(bio) - pos;
	}
printf("esds: codec=0x%02x 0x%02x\n", obj_type, cfg.object_type);
printf("esds: ch=%d sr=%d\n", track->channels, track->sampleRate);
    return 0;
}

static int mov_read_esds(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    return ff_mov_read_esds(mov, bio, atom);
}

static int mov_read_pasp(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	// aspect ratio
    return 0;
}

// this atom contains actual media data 
static int mov_read_mdat(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	if(atom.size == 0) {
		printf("wrong one (MP4)\n");
		return 0;
	}
	mov->found_mdat = 1;
	return 0;	// now go for moov
}

// read major brand, minor version and compatible brands and store them as metadata
static int mov_read_ftyp(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	return 0;
}

// this atom should contain all header atoms
static int mov_read_moov(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	if(mov_read_default(mov, bio, atom) < 0)
		return -1;
	// we parsed the 'moov' atom, we can terminate the parsing as soon as we find the 'mdat'
	// so we don't parse the whole file if over a network 
	mov->found_moov = 1;
    return 0;	// now go for mdat
}

static int mov_read_moof(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	mov->fragment.moof_offset = url_ftell(bio) - 8;
//dprintf(mov->fc, "moof offset %llx\n", mov->fragment.moof_offset);
	return mov_read_default(mov, bio, atom);
}

static int mov_read_mdhd(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
	unsigned lang;
    int		version;
    char language[4] = {0};

	player = mov->player;
	track   = mov->format;
    sc = track->priv_data;
	version = get_byte(bio);
	if(version > 1)
		return -1;		// unsupported
	get_be24(bio);		// flags
    if(version) {
		get_be64(bio);
		get_be64(bio);
	} else {
		get_be32(bio);	// creation time
		get_be32(bio);	// modification time
    }
    sc->time_scale = get_be32(bio);
    if(version) track->duration = get_be64(bio);
	else		track->duration = get_be32(bio);	// duration
	lang = get_be16(bio);	// language
//	if(ff_mov_lang_to_iso639(lang, language))
//		av_metadata_set2(&st->metadata, "language", language, 0);
    get_be16(bio);		// quality
    return 0;
}

static int mov_read_mvhd(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	int		count, version;

	version = get_byte(bio);	// version
    get_be24(bio);				// flags */
    if(version) {
        get_be64(bio);
        get_be64(bio);
    } else {
        get_be32(bio);			// creation time
        get_be32(bio);			// modification time
    }
    mov->time_scale = get_be32(bio);	// time scale
//dprintf(mov->fc, "time scale = %i\n", mov->time_scale);
	if(version) mov->duration = get_be64(bio);
	else		mov->duration = get_be32(bio);	/// duration
    get_be32(bio);				// preferred scale
    get_be16(bio);				// preferred volume
    url_fskip(bio, 10);			// reserved
    url_fskip(bio, 36);			// display matrix
    get_be32(bio);				// preview time
    get_be32(bio);				// preview duration
    get_be32(bio);				// poster time
    get_be32(bio);				// selection time
    get_be32(bio);				// selection duration
    get_be32(bio);				// current time 
    get_be32(bio);				// next track ID
    return 0;
}

static int mov_read_smi(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	return 0;
}

static int mov_read_enda(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	return 0;
}

// FIXME modify qdm2/svq3/h264 decoders to take full atom as extradata 
static int mov_read_extradata(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
    unsigned char	*p;
    uint64_t	size;

	player = mov->player;
	track   = mov->format;
    size = (uint64_t)track->extradataLen + atom.size + 8 + FF_INPUT_BUFFER_PADDING_SIZE;
    if(size > INT_MAX || (uint64_t)atom.size > INT_MAX)
        return -1;
    p = track->extradata;
    p += track->extradataLen;
    track->extradataLen = size - FF_INPUT_BUFFER_PADDING_SIZE;
    AV_WB32(p, atom.size + 8);
    AV_WL32(p+4, atom.type);
    get_buffer(bio, p+8, atom.size);
printf("read extradata: size=%d\n", track->extradataLen);
    return 0;
}

static int mov_read_wave(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	return 0;
}

// This function reads atom content and puts data in extradata without tag
// nor size unlike mov_read_extradata.
static int mov_read_glbl(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;

	player = mov->player;
	track   = mov->format;
	if((uint64_t)atom.size > (1<<30))
		return -1;
	track->extradataLen = atom.size;
	get_buffer(bio, track->extradata, atom.size);
	return 0;
}

/**
 * An strf atom is a BITMAPINFOHEADER struct. This struct is 40 bytes itself,
 * but can have extradata appended at the end after the 40 bytes belonging
 * to the struct.
 */
static int mov_read_strf(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;

	player = mov->player;
	track  = mov->format;
    if(atom.size <= 40)
        return 0;
    if((uint64_t)atom.size > (1<<30))
        return -1;
    track->extradataLen = atom.size - 40;
    url_fskip(bio, 40);
    get_buffer(bio, track->extradata, atom.size - 40);
printf("strf: extradata=%d\n", track->extradataLen);
    return 0;
}

static int mov_read_stco(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext *sc;
	int		i, entries;

	player = mov->player;
	track  = mov->format;
	sc = track->priv_data;
    get_be32(bio);			// version + flags
	entries = get_be32(bio);
	if(entries >= UINT_MAX/sizeof(int64_t))
		return -1;
	sc->chunk_offsets = av_malloc(entries * sizeof(int64_t));
	if(!sc->chunk_offsets)
		return AVERROR(ENOMEM);
	sc->chunk_count = entries;
	if(atom.type == MKTAG('s','t','c','o')) {
        for(i = 0;i < entries;i++)
            sc->chunk_offsets[i] = get_be32(bio);
	} else if(atom.type == MKTAG('c','o','6','4')) {
        for(i = 0;i < entries;i++)
            sc->chunk_offsets[i] = get_be64(bio);
	} else
        return -1;
    return 0;
}

static int mov_read_stsd(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext *sc;
	char	temp[32];
	int		j, entries, stream_id;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);			// version + flags
	entries = get_be32(bio);
	// parsing Sample description table
	for(stream_id = 0;stream_id < entries;stream_id++) {
        int64_t		start_pos;
        uint32_t	format;
        int		id, size, dref_id = 1;
        MOVAtom a = { 0 };

        start_pos = url_ftell(bio);
        size = get_be32(bio);	// size
        format = get_le32(bio);	// data format		avc1/mp4a
        if(size >= 16) {
            get_be32(bio);		// reserved
            get_be16(bio);		// reserved
            dref_id = get_be16(bio);
        }
        sc->pseudo_stream_id = -1;
        sc->dref_id = dref_id;
        track->codecTag = format;
		if(track->mediaType == AV_MEDIA_TYPE_VIDEO) {
			if(format != MKTAG('a','v','c','1')) {
printf("stsd: video is not 'avc1'\n");
			}
		} else if(track->mediaType == AV_MEDIA_TYPE_AUDIO) {
			if(format != MKTAG('m','p','4','a')) {
printf("stsd: audio is not 'mp4a'\n");
			}
		} else {
printf("stsd: '%c%c%c%c' isn't audio/video\n", format&0xff, format>>8&0xff, format>>16&0xff, format>>24&0xff);
        }
        if(track->mediaType == AV_MEDIA_TYPE_VIDEO) {
            unsigned int color_depth, len;
            int color_greyscale;
			//track->codecId	= id;
			track->mediaFormat	= AV_VIDEO_YUV_MB32_420;
			track->codecId		= AV_CODEC_AVC;
			get_be16(bio);				// Version
            get_be16(bio);				// Revision level
            get_be32(bio);				// Vendor
            get_be32(bio);				// Temporal quality
            get_be32(bio);				// Spatial quality
            track->width  = get_be16(bio);	// Width
            track->height = get_be16(bio);	// Height
            get_be32(bio);				// Horizontal resolution
            get_be32(bio);				// Vertical resolution
            get_be32(bio);				// Data size - always 0
            get_be16(bio);				// FrameCount
            len = get_byte(bio);		// Compressor name - 32-byte Pascal string
            if(len > 31) len = 31;
            mov_read_mac_string(mov, bio, len, temp, 32);
            if(len < 31) url_fskip(bio, 31 - len);
            // codec_tag YV12 triggers an UV swap in rawdec.c
            track->bitsPerSample = get_be16(bio);		// Depth
            get_be16(bio);				// Color table ID
            // figure out the palette situation 
 printf("stsd.avc1: w=%d h=%d\n", track->width, track->height);
        } else if(track->mediaType == AV_MEDIA_TYPE_AUDIO) {
            int		bits_per_sample, flags, version;
			track->mediaFormat	= AV_AUDIO_PCM;
			track->codecId		= AV_CODEC_AAC;
            version = get_be16(bio);	// Version
            get_be16(bio);				// Revision level
            get_be32(bio);				// Vendor
			if(version == 2) {
				get_be16(bio);			// alwaye3
				get_be16(bio);			// alwaye16
				get_be16(bio);			// alwaysMinus-2
				get_be16(bio);			// always0
            	get_be32(bio);			// always65536
				get_be32(bio);			// sizeofSructOnly
				track->sampleRate = av_int2dbl(get_be64(bio)); // audioSampleRate - 64-bit float
				track->channels	= get_be32(bio);		// numAudioChannels
				get_be32(bio);			// always7F000000 
				track->bitsPerSample = get_be32(bio);		// constBitsPerChannel
				flags = get_be32(bio);					// formatSpecificFlags - LPCM flag values
				sc->bytes_per_frame = get_be32(bio);	// constBytesPerAudioPacket
				sc->samples_per_frame = get_be32(bio);	// constLPCMFramesPerAudioPAcket
			} else {
				track->channels = get_be16(bio);		// Number of channels
				track->bitsPerSample = get_be16(bio);	// Sample size(bits) - 8/16
				sc->audio_cid = get_be16(bio);		// Compression ID
				get_be16(bio);						// Ppacket size - 0
				track->sampleRate = ((get_be32(bio) >> 16));	// Sample rate - 32-bit unsigned fixed-point number(16.16)
                if(version == 1) {
                    sc->samples_per_frame = get_be32(bio);	// Samples per packet
                    get_be32(bio);		// Bytes per packet
                    sc->bytes_per_frame = get_be32(bio);	// Bytes per frame
                    get_be32(bio);		// Bytes per sample
                }
            }
printf("stsd.mp4a: ch=%d sr=%d bits=%d\n", track->channels, track->sampleRate, track->bitsPerSample);
        } else {
            // other codec type, just skip (rtp, mp4s, tmcd ...)
            url_fskip(bio, size - (url_ftell(bio) - start_pos));
        }
        // this will read extra atoms at the end (wave, alac, damr, avcC, SMI ...) */
        a.size = size - (url_ftell(bio) - start_pos);
        if(a.size > 8) {
            if(mov_read_default(mov, bio, a) < 0)
                return -1;
        } else if(a.size > 0)
            url_fskip(bio, a.size);
    }
    if(track->mediaType==AV_MEDIA_TYPE_AUDIO && track->sampleRate==0 && sc->time_scale>1)
        track->sampleRate= sc->time_scale;
    // special codec parameters handling 

    return 0;
}

static int mov_read_stsc(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
	int		i, entries;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
    entries = get_be32(bio);
//dprintf(mov->fc, "track[%i].stsc.entries = %i\n", c->fc->nb_streams-1, entries);
    if(entries >= UINT_MAX / sizeof(*sc->stsc_data))
        return -1;
    sc->stsc_data = av_malloc(entries * sizeof(*sc->stsc_data));
    if (!sc->stsc_data)
        return AVERROR(ENOMEM);
    sc->stsc_count = entries;
    for(i = 0;i < entries;i++) {
        sc->stsc_data[i].first = get_be32(bio);
        sc->stsc_data[i].count = get_be32(bio);
        sc->stsc_data[i].id    = get_be32(bio);
    }
    return 0;
}

static int mov_read_stps(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
	int		i, entries;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
	entries = get_be32(bio);
	if(entries >= UINT_MAX / sizeof(*sc->stps_data))
		return -1;
	sc->stps_data = av_malloc(entries * sizeof(*sc->stps_data));
	if(!sc->stps_data)
		return AVERROR(ENOMEM);
	sc->stps_count = entries;
	for(i = 0;i < entries;i++) {
        sc->stps_data[i] = get_be32(bio);
//dprintf(mov->fc, "stps %d\n", sc->stps_data[i]);
	}
	return 0;
}

static int mov_read_stss(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
	int		i, entries;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
	entries = get_be32(bio);
	if(entries >= UINT_MAX / sizeof(int))
		return -1;
	sc->keyframes = av_malloc(entries * sizeof(int));
	if(!sc->keyframes)
		return AVERROR(ENOMEM);
	sc->keyframe_count = entries;
    for(i = 0;i < entries;i++) {
		sc->keyframes[i] = get_be32(bio);
//dprintf(mov->fc, "keyframes[]=%d\n", sc->keyframes[i]);
	}
	return 0;
}

static int mov_read_stsz(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
    int		i, entries, sample_size, field_size, num_bytes;
    GetBitContext gb;
    unsigned char	*buf;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
    if(atom.type == MKTAG('s','t','s','z')) {
		sample_size = get_be32(bio);
		if(!sc->sample_size) sc->sample_size = sample_size;	// do not overwrite value computed in stsd
		field_size = 32;
	} else {
		sample_size = 0;
		get_be24(bio);	// reserved
        field_size = get_byte(bio);
    }
	entries = get_be32(bio);
//dprintf(mov->fc, "sample_size = %d sample_count = %d\n", sc->sample_size, entries);
	sc->sample_count = entries;
	if(sample_size)
		return 0;
	if(field_size != 4 && field_size != 8 && field_size != 16 && field_size != 32) {
		printf("stsz: Invalid sample field size %d\n", field_size);
		return -1;
	}
	if(entries >= UINT_MAX / sizeof(int) || entries >= (UINT_MAX - 4) / field_size)
		return -1;
	sc->sample_sizes = av_malloc(entries * sizeof(int));
	if(!sc->sample_sizes)
		return AVERROR(ENOMEM);
	num_bytes = (entries*field_size+4)>>3;
	buf = av_malloc(num_bytes+FF_INPUT_BUFFER_PADDING_SIZE);
	if(!buf) {
		av_freep(&sc->sample_sizes);
		return AVERROR(ENOMEM);
	}
	if(get_buffer(bio, buf, num_bytes) < num_bytes) {
		av_freep(&sc->sample_sizes);
		av_free(buf);
		return -1;
	}
	init_get_bits(&gb, buf, 8*num_bytes);
	for(i = 0;i < entries;i++)
		sc->sample_sizes[i] = get_bits_long(&gb, field_size);
	av_free(buf);
	return 0;
}

static int mov_read_stts(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
    int		i, entries, sample_count, sample_duration;
    int64_t duration=0;
    int64_t total_sample_count=0;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
	entries = get_be32(bio);
//dprintf(mov->fc, "track[%i].stts.entries = %i\n", c->fc->nb_streams-1, entries);
	if(entries >= UINT_MAX / sizeof(*sc->stts_data))
		return -1;
	sc->stts_data = av_malloc(entries * sizeof(*sc->stts_data));
	if(!sc->stts_data)
		return AVERROR(ENOMEM);
	sc->stts_count = entries;
	for(i = 0;i < entries;i++) {
		sample_count = get_be32(bio);
		sample_duration = get_be32(bio);
		sc->stts_data[i].count= sample_count;
		sc->stts_data[i].duration= sample_duration;
//dprintf(mov->fc, "sample_count=%d, sample_duration=%d\n",sample_count,sample_duration);
		duration += (int64_t)sample_duration*sample_count;
		total_sample_count += sample_count;
	}
	track->frameRateNum = sc->time_scale;
	track->frameRateDen = sc->stts_data[0].duration;
printf("stts: frameRate=%d/%d\n", track->frameRateNum, track->frameRateDen);
    track->frameCount = total_sample_count;
    if(duration) track->duration = duration;
    return 0;
}

static int mov_read_ctts(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
    int		i, count, entries, duration;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
	entries = get_be32(bio);
//dprintf(mov->fc, "track[%i].ctts.entries = %i\n", c->fc->nb_streams-1, entries);
	if(entries >= UINT_MAX / sizeof(*sc->ctts_data))
		return -1;
	sc->ctts_data = av_malloc(entries * sizeof(*sc->ctts_data));
	if(!sc->ctts_data)
		return AVERROR(ENOMEM);
	sc->ctts_count = entries;
    for(i = 0;i < entries;i++) {
		count    = get_be32(bio);
		duration = get_be32(bio);
        sc->ctts_data[i].count   = count;
        sc->ctts_data[i].duration= duration;
        if(duration < 0) sc->dts_shift = FFMAX(sc->dts_shift, -duration);
	}
//dprintf(mov->fc, "dts shift %d\n", sc->dts_shift);
	return 0;
}

static void mov_build_index(MOVContext *mov, AV_STREAM_TRACK *track)
{
    MOVStreamContext	*sc;
    int64_t		current_offset, current_dts, stream_size;
	int		i, j, rescaled, stts_index, stsc_index, stss_index, stps_index;

    sc = track->priv_data;
    current_dts = stream_size = 0;
	stts_index = stsc_index = stss_index = stps_index = 0;
	// adjust first dts according to edit list
	if(sc->time_offset) {
        rescaled = sc->time_offset < 0 ? av_rescale(sc->time_offset, sc->time_scale, mov->time_scale) : sc->time_offset;
        current_dts = -rescaled;
        if(sc->ctts_data && sc->ctts_data[0].duration / sc->stts_data[0].duration > 16) {
            // more than 16 frames delay, dts are likely wrong
            // this happens with files created by iMovie
            sc->wrong_dts = 1;
            track->has_b_frames = 1;
        }
    }
	// only use old uncompressed audio chunk demuxing when stts specifies it
    if (!(track->mediaType == AV_MEDIA_TYPE_AUDIO && sc->stts_count == 1 && sc->stts_data[0].duration == 1)) {
        unsigned int current_sample = 0;
        unsigned int stts_sample = 0;
        unsigned int sample_size;
        unsigned int distance = 0;
        int key_off = sc->keyframes && sc->keyframes[0] == 1;

        current_dts -= sc->dts_shift;
        if(sc->sample_count >= UINT_MAX / sizeof(*track->index_entries))
            return;
        track->index_entries = av_malloc(sc->sample_count*sizeof(*track->index_entries));
        if(!track->index_entries)
            return;
        track->index_entries_allocated_size = sc->sample_count*sizeof(*track->index_entries);
        for(i = 0;i < sc->chunk_count;i++) {
            current_offset = sc->chunk_offsets[i];
            if(stsc_index + 1 < sc->stsc_count &&
					i + 1 == sc->stsc_data[stsc_index + 1].first)
                stsc_index++;
            for(j = 0;j < sc->stsc_data[stsc_index].count;j++) {
                int keyframe = 0;
                if(current_sample >= sc->sample_count) {
                    printf("wrong sample count\n");
                    return;
                }
                if(!sc->keyframe_count || current_sample+key_off == sc->keyframes[stss_index]) {
                    keyframe = 1;
                    if(stss_index + 1 < sc->keyframe_count) stss_index++;
                } else if(sc->stps_count && current_sample+key_off == sc->stps_data[stps_index]) {
                    keyframe = 1;
                    if(stps_index + 1 < sc->stps_count) stps_index++;
                }
                if(keyframe) distance = 0;
                sample_size = sc->sample_size > 0 ? sc->sample_size : sc->sample_sizes[current_sample];
                if(sc->pseudo_stream_id == -1 || sc->stsc_data[stsc_index].id - 1 == sc->pseudo_stream_id) {
                    AVIndexEntry *e = &track->index_entries[track->nb_index_entries++];
                    e->pos = current_offset;
                    e->timestamp = current_dts;
                    e->size = sample_size;
                    e->min_distance = distance;
                    e->flags = keyframe ? AVINDEX_KEYFRAME : 0;
//dprintf(mov->fc, "AVIndex stream %d, sample %d, offset %"PRIx64", dts %"PRId64", "
//							"size %d, distance %d, keyframe %d\n", st->index, current_sample,
//							current_offset, current_dts, sample_size, distance, keyframe);
                }
                current_offset += sample_size;
                stream_size += sample_size;
                current_dts += sc->stts_data[stts_index].duration;
                distance++;
                stts_sample++;
                current_sample++;
                if(stts_index + 1 < sc->stts_count && stts_sample == sc->stts_data[stts_index].count) {
                    stts_sample = 0;
                    stts_index++;
                }
            }
        }
        if(track->duration > 0) track->bitRate = stream_size*8*sc->time_scale/track->duration;
    } else {
        unsigned chunk_samples, total = 0;

        // compute total chunk count
        for(i = 0;i < sc->stsc_count;i++) {
            unsigned count, chunk_count;
            chunk_samples = sc->stsc_data[i].count;
            if(sc->samples_per_frame && chunk_samples % sc->samples_per_frame) {
                printf("error unaligned chunk\n");
                return;
            }
            if(sc->samples_per_frame >= 160) { // gsm
                count = chunk_samples / sc->samples_per_frame;
            } else if(sc->samples_per_frame > 1) {
                unsigned samples = (1024/sc->samples_per_frame)*sc->samples_per_frame;
                count = (chunk_samples+samples-1) / samples;
            } else {
                count = (chunk_samples+1023) / 1024;
            }
            if(i < sc->stsc_count - 1)
                chunk_count = sc->stsc_data[i+1].first - sc->stsc_data[i].first;
            else
                chunk_count = sc->chunk_count - (sc->stsc_data[i].first - 1);
            total += chunk_count * count;
        }
//dprintf(mov->fc, "chunk count %d\n", total);
        if(total >= UINT_MAX / sizeof(*track->index_entries))
            return;
        track->index_entries = av_malloc(total*sizeof(*track->index_entries));
        if(!track->index_entries)
            return;
        track->index_entries_allocated_size = total*sizeof(*track->index_entries);
        // populate index
        for(i = 0;i < sc->chunk_count;i++) {
            current_offset = sc->chunk_offsets[i];
            if(stsc_index + 1 < sc->stsc_count &&
                i + 1 == sc->stsc_data[stsc_index + 1].first)
                stsc_index++;
            chunk_samples = sc->stsc_data[stsc_index].count;
            while(chunk_samples > 0) {
                AVIndexEntry	*e;
                unsigned size, samples;
                if(sc->samples_per_frame >= 160) { // gsm
                    samples = sc->samples_per_frame;
                    size = sc->bytes_per_frame;
                } else {
                    if (sc->samples_per_frame > 1) {
                        samples = FFMIN((1024 / sc->samples_per_frame)* sc->samples_per_frame, chunk_samples);
                        size = (samples / sc->samples_per_frame) * sc->bytes_per_frame;
                    } else {
                        samples = FFMIN(1024, chunk_samples);
                        size = samples * sc->sample_size;
                    }
                }
                if(track->nb_index_entries >= total) {
                    printf("wrong chunk count %d\n", total);
                    return;
                }
                e = &track->index_entries[track->nb_index_entries++];
                e->pos			= current_offset;
                e->timestamp	= current_dts;
                e->size			= size;
                e->min_distance	= 0;
                e->flags		= AVINDEX_KEYFRAME;
//dprintf(mov->fc, "AVIndex stream %d, chunk %d, offset %"PRIx64", dts %"PRId64", "
//						"size %d, duration %d\n", st->index, i, current_offset, current_dts, size, samples);

                current_offset += size;
                current_dts += samples;
                chunk_samples -= samples;
            }
        }
    }
}

static int mov_open_dref(ByteIOContext **bio, char *src, MOVDref *ref)
{
	return 0;
}

static int mov_read_trak(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track, _track;
	MOVStreamContext	*sc;
	int		rval;

	player = mov->player;
printf("trak: index=%d\n", avStreamTrackCount(player));
	track = &_track;
	memset(track, 0, sizeof(AV_STREAM_TRACK));
	track->index = avStreamTrackCount(player);
	mov->format = track;
	sc = &_movsc[track->index];
	memset(sc, 0, sizeof(MOVStreamContext));
	track->priv_data = sc;
    sc->ffindex	= track->index;
    rval = mov_read_default(mov, bio, atom);
    if(rval < 0) return rval;
    // sanity checks
    if(sc->chunk_count && (!sc->stts_count || !sc->stsc_count ||
                            (!sc->sample_size && !sc->sample_count))) {
        printf("format %d, missing mandatory atoms, broken header\n", track->index);
        return 0;
    }
//printf("read_trak: index=%d type=%d\n", stream->formatCount, track->mediaType);
	track = avStreamAddTrack(player, track);
	mov->format = NULL;
    if(!sc->time_scale) {
        printf("format %d, timescale not set\n", track->index);
        sc->time_scale = mov->time_scale;
        if(!sc->time_scale) sc->time_scale = 1;
    }
//	av_set_pts_info(st, 64, 1, sc->time_scale);
    if(track->mediaType == AV_MEDIA_TYPE_AUDIO && !track->frameSize && sc->stts_count == 1) {
        track->frameSize = av_rescale(sc->stts_data[0].duration, track->sampleRate, sc->time_scale);
//dprintf(mov->fc, "frame size %d\n", st->codec->frame_size);
    }
    mov_build_index(mov, track);
	sc->pb = player->bio;	// check KDK

	// Do not need those anymore.
    av_freep(&sc->chunk_offsets);
    av_freep(&sc->stsc_data);
    av_freep(&sc->sample_sizes);
    av_freep(&sc->keyframes);
    av_freep(&sc->stts_data);
    av_freep(&sc->stps_data);

    return 0;
}

static int mov_read_ilst(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	int		rval;

	mov->itunes_metadata = 1;
	rval = mov_read_default(mov, bio, atom);
	mov->itunes_metadata = 0;
	return rval;
}

static int mov_read_meta(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	uint32_t	tag;

	while(atom.size > 8) {
		tag = get_le32(bio);
		atom.size -= 4;
		if(tag == MKTAG('h','d','l','r')) {
			url_fseek(bio, -8, SEEK_CUR);
			atom.size += 8;
			return mov_read_default(mov, bio, atom);
		}
	}
	return 0;
}

static int mov_read_tkhd(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
	MOVStreamContext	*sc;
	int64_t		disp_transform[2];
	int		display_matrix[3][2];
	int		i, width, height, version;

	player = mov->player;
	track  = mov->format;
	sc = track->priv_data;
	version = get_byte(bio);
	get_be24(bio);		// flags
    /*
    MOV_TRACK_ENABLED 0x0001
    MOV_TRACK_IN_MOVIE 0x0002
    MOV_TRACK_IN_PREVIEW 0x0004
    MOV_TRACK_IN_POSTER 0x0008
    */
	if(version) {
		get_be64(bio);
		get_be64(bio);
    } else {
		get_be32(bio);		// creation time
		get_be32(bio);		// modification time
    }
	track->trackId = get_be32(bio);	// track id (NOT 0 !)
	get_be32(bio);			// reserved
	// highlevel (considering edits) duration in movie timebase
	if(version) get_be64(bio);
	else		get_be32(bio);
    get_be32(bio);			// reserved
    get_be32(bio);			// reserved
    get_be16(bio);			// layer
    get_be16(bio);			// alternate group
    get_be16(bio);			// volume
    get_be16(bio);			// reserved
	// read in the display matrix (outlined in ISO 14496-12, Section 6.2.2)
	// they're kept in fixed point format through all calculations
	// ignore u,v,z b/c we don't need the scale factor to calc aspect ratio
    for(i = 0;i < 3;i++) {
		display_matrix[i][0] = get_be32(bio);   // 16.16 fixed point
		display_matrix[i][1] = get_be32(bio);   // 16.16 fixed point
		get_be32(bio);           // 2.30 fixed point (not used)
	}
	width = get_be32(bio);       // 16.16 fixed point track width
	height = get_be32(bio);      // 16.16 fixed point track height
	sc->width = width >> 16;
	sc->height = height >> 16;
	// transform the display width/height according to the matrix
	// skip this if the display matrix is the default identity matrix
	// or if it is rotating the picture, ex iPhone 3GS
	// to keep the same scale, use [width height 1<<16]
	if(width && height &&
        ((display_matrix[0][0] != 65536  ||
          display_matrix[1][1] != 65536) &&
         !display_matrix[0][1] &&
         !display_matrix[1][0] &&
         !display_matrix[2][0] && !display_matrix[2][1])) {
        for(i = 0;i < 2;i++)
            disp_transform[i] =
                (int64_t)  width  * display_matrix[0][i] +
                (int64_t)  height * display_matrix[1][i] +
                ((int64_t) display_matrix[2][i] << 16);

        //sample aspect ratio is new width/height divided by old width/height
//		track->sampleAspectRatio = av_d2q(((double)disp_transform[0]*height)
//									/ ((double)disp_transform[1]*width), INT_MAX);
    }
    return 0;
}

static int mov_read_tfhd(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	MOVFragment	*frag;
	MOVTrackExt	*trex;
    int		flags, track_id, i;

    frag = &mov->fragment;
    get_byte(bio);		// version
    flags = get_be24(bio);
    track_id = get_be32(bio);
    if(!track_id) return -1;
    frag->track_id = track_id;
    trex = NULL;
    for(i = 0;i < mov->trex_count;i++)
        if(mov->trex_data[i].track_id == frag->track_id) {
            trex = &mov->trex_data[i];
            break;
        }
    if(!trex) {
        printf("could not find corresponding trex\n");
        return -1;
    }
    if(flags & 0x01) frag->base_data_offset = get_be64(bio);
    else             frag->base_data_offset = frag->moof_offset;
    if(flags & 0x02) frag->stsd_id          = get_be32(bio);
    else             frag->stsd_id          = trex->stsd_id;
    frag->duration = flags & 0x08 ? get_be32(bio) : trex->duration;
    frag->size     = flags & 0x10 ? get_be32(bio) : trex->size;
    frag->flags    = flags & 0x20 ? get_be32(bio) : trex->flags;
//dprintf(mov->fc, "frag flags 0x%x\n", frag->flags);
    return 0;
}

static int mov_read_chap(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    mov->chapter_track = get_be32(bio);
    return 0;
}

static int mov_read_trex(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    MOVTrackExt *trex;

    if((uint64_t)mov->trex_count+1 >= UINT_MAX / sizeof(*mov->trex_data))
        return -1;
    trex = av_realloc(mov->trex_data, (mov->trex_count+1)*sizeof(*mov->trex_data));
    if(!trex)
        return AVERROR(ENOMEM);
    mov->trex_data = trex;
    trex = &mov->trex_data[mov->trex_count++];
    get_byte(bio); /* version */
    get_be24(bio); /* flags */
    trex->track_id = get_be32(bio);
    trex->stsd_id  = get_be32(bio);
    trex->duration = get_be32(bio);
    trex->size     = get_be32(bio);
    trex->flags    = get_be32(bio);
    return 0;
}

static int mov_read_trun(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track, *track2;
    MOVStreamContext	*sc;
	MOVFragment		*frag;
    uint64_t offset;
    int64_t dts;
    int data_offset = 0;
    unsigned entries, first_sample_flags = frag->flags;
	int		count, flags, distance, i;

	player = mov->player;
    frag = &mov->fragment;
	track2 = NULL;
	count = avStreamTrackCount(player);
    for(i = 0; i < count;i++) {
		track = avStreamTrackAt(player, i);
        if(track->trackId == frag->track_id) {
            track2 = track;
            break;
        }
    }
    if(!track2) {
        printf("could not find corresponding track id %d\n", frag->track_id);
        return -1;
    }
    sc = track2->priv_data;
    if(sc->pseudo_stream_id+1 != frag->stsd_id)
        return 0;
    get_byte(bio);	// version
    flags = get_be24(bio);
    entries = get_be32(bio);
//dprintf(mov->fc, "flags 0x%x entries %d\n", flags, entries);
    if(flags & 0x001) data_offset        = get_be32(bio);
    if(flags & 0x004) first_sample_flags = get_be32(bio);
    if(flags & 0x800) {
        MOVStts *ctts_data;
        if ((uint64_t)entries+sc->ctts_count >= UINT_MAX/sizeof(*sc->ctts_data))
            return -1;
        ctts_data = av_realloc(sc->ctts_data,
                               (entries+sc->ctts_count)*sizeof(*sc->ctts_data));
        if(!ctts_data)
            return AVERROR(ENOMEM);
        sc->ctts_data = ctts_data;
    }
    dts = track2->duration;
    offset = frag->base_data_offset + data_offset;
    distance = 0;
//dprintf(mov->fc, "first sample flags 0x%x\n", first_sample_flags);
    for(i = 0;i < entries;i++) {
        unsigned sample_size = frag->size;
        int sample_flags = i ? frag->flags : first_sample_flags;
        unsigned sample_duration = frag->duration;
        int keyframe;

        if(flags & 0x100) sample_duration = get_be32(bio);
        if(flags & 0x200) sample_size     = get_be32(bio);
        if(flags & 0x400) sample_flags    = get_be32(bio);
        if(flags & 0x800) {
            sc->ctts_data[sc->ctts_count].count = 1;
            sc->ctts_data[sc->ctts_count].duration = get_be32(bio);
            sc->ctts_count++;
        }
        if((keyframe = track2->mediaType == AV_MEDIA_TYPE_AUDIO ||
				(flags & 0x004 && !i && !sample_flags) || sample_flags & 0x2000000))
            distance = 0;
        av_add_index_entry(track2, offset, dts, sample_size, distance, keyframe ? AVINDEX_KEYFRAME : 0);
//dprintf(mov->fc, "AVIndex stream %d, sample %d, offset %"PRIx64", dts %"PRId64", "
//					"size %d, distance %d, keyframe %d\n", st->index, sc->sample_count+i,
//					offset, dts, sample_size, distance, keyframe);
        distance++;
        dts += sample_duration;
        offset += sample_size;
    }
    frag->moof_offset = offset;
    track2->duration = dts;
    return 0;
}

// this atom should be null (from specs), but some buggy files put the 'moov' atom inside it...
// like the files created with Adobe Premiere 5.0, for samples see
// http://graphics.tudelft.nl/~wouter/publications/soundtests
static int mov_read_wide(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
    int		rval;

    if(atom.size < 8) return 0; // continue
    if(get_be32(bio)) {		// 0 sized mdat atom... use the 'wide' atom size
        url_fskip(bio, atom.size - 4);
        return 0;
    }
    atom.type = get_le32(bio);
    atom.size -= 8;
    if(atom.type != MKTAG('m','d','a','t')) {
        url_fskip(bio, atom.size);
        return 0;
    }
	rval = mov_read_mdat(mov, bio, atom);
    return rval;
}

static int mov_read_cmov(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	printf("not support compressed format\n");
    return -1;
}

// edit list atom
static int mov_read_elst(MOVContext *mov, ByteIOContext *bio, MOVAtom atom)
{
	AV_PLAYER	*player;
	AV_STREAM_TRACK	*track;
    MOVStreamContext	*sc;
	int		i, edit_count, time, duration;

	player = mov->player;
	track  = mov->format;
    sc = track->priv_data;
	get_be32(bio);		// version + flags
	edit_count = get_be32(bio);	// entries
	if((uint64_t)edit_count*12+8 > atom.size)
		return -1;
	for(i = 0;i < edit_count;i++) {
		duration = get_be32(bio);	// Track duration
		time = get_be32(bio);		// Media time
		get_be32(bio);				// Media rate
		if(i == 0 && time >= -1) {
			sc->time_offset = time != -1 ? time : -duration;
		}
	}
	if(edit_count > 1) printf("multiple edit list entries, a/v desync might occur, patch welcome\n");
//dprintf(mov->fc, "track[%i].edit_count = %i\n", c->fc->nb_streams-1, edit_count);
    return 0;
}

static const MOVParseTableEntry mov_default_parse_table[] = {
{ MKTAG('a','v','s','s'), mov_read_extradata },
{ MKTAG('c','h','p','l'), mov_read_chpl },
{ MKTAG('c','o','6','4'), mov_read_stco },
{ MKTAG('c','t','t','s'), mov_read_ctts },		// composition time to sample
{ MKTAG('d','i','n','f'), mov_read_default },
{ MKTAG('d','r','e','f'), mov_read_dref },
{ MKTAG('e','d','t','s'), mov_read_default },
{ MKTAG('e','l','s','t'), mov_read_elst },
{ MKTAG('e','n','d','a'), mov_read_enda },
{ MKTAG('f','i','e','l'), mov_read_extradata },
{ MKTAG('f','t','y','p'), mov_read_ftyp },
{ MKTAG('g','l','b','l'), mov_read_glbl },
{ MKTAG('h','d','l','r'), mov_read_hdlr },
{ MKTAG('i','l','s','t'), mov_read_ilst },
{ MKTAG('j','p','2','h'), mov_read_extradata },
{ MKTAG('m','d','a','t'), mov_read_mdat },
{ MKTAG('m','d','h','d'), mov_read_mdhd },
{ MKTAG('m','d','i','a'), mov_read_default },
{ MKTAG('m','e','t','a'), mov_read_meta },
{ MKTAG('m','i','n','f'), mov_read_default },
{ MKTAG('m','o','o','f'), mov_read_moof },
{ MKTAG('m','o','o','v'), mov_read_moov },
{ MKTAG('m','v','e','x'), mov_read_default },
{ MKTAG('m','v','h','d'), mov_read_mvhd },
{ MKTAG('S','M','I',' '), mov_read_smi },		// Sorenson extension ???
{ MKTAG('a','l','a','c'), mov_read_extradata },	// alac specific atom
{ MKTAG('a','v','c','C'), mov_read_glbl },
{ MKTAG('p','a','s','p'), mov_read_pasp },
{ MKTAG('s','t','b','l'), mov_read_default },
{ MKTAG('s','t','c','o'), mov_read_stco },
{ MKTAG('s','t','p','s'), mov_read_stps },
{ MKTAG('s','t','r','f'), mov_read_strf },
{ MKTAG('s','t','s','c'), mov_read_stsc },
{ MKTAG('s','t','s','d'), mov_read_stsd },		// sample description
{ MKTAG('s','t','s','s'), mov_read_stss },		// sync sample
{ MKTAG('s','t','s','z'), mov_read_stsz },		// sample size
{ MKTAG('s','t','t','s'), mov_read_stts },
{ MKTAG('s','t','z','2'), mov_read_stsz },		// compact sample size
{ MKTAG('t','k','h','d'), mov_read_tkhd },		// track header
{ MKTAG('t','f','h','d'), mov_read_tfhd },		// track fragment header
{ MKTAG('t','r','a','k'), mov_read_trak },
{ MKTAG('t','r','a','f'), mov_read_default },
{ MKTAG('t','r','e','f'), mov_read_default },
{ MKTAG('c','h','a','p'), mov_read_chap },
{ MKTAG('t','r','e','x'), mov_read_trex },
{ MKTAG('t','r','u','n'), mov_read_trun },
{ MKTAG('u','d','t','a'), mov_read_default },
{ MKTAG('w','a','v','e'), mov_read_wave },
{ MKTAG('e','s','d','s'), mov_read_esds },
{ MKTAG('w','i','d','e'), mov_read_wide },		// place holder
{ MKTAG('c','m','o','v'), mov_read_cmov },
{ 0, NULL }
};

int mov_probe(unsigned char *buf, int size)
{
	unsigned int	tag;
	int		score, offset;

    // check file header
	score = 0;
    offset = 0;
    for(;;) {
        // ignore invalid offset
        if((offset + 8) > size) return score;
        tag = AV_RL32(buf + offset + 4);
        switch(tag) {
        // check for obvious tags
        case MKTAG('f','t','y','p'):
        case MKTAG('m','o','o','v'):
        case MKTAG('m','d','a','t'):
            return 100;
        case MKTAG(0x82,0x82,0x7f,0x7d):
        case MKTAG('s','k','i','p'):
        case MKTAG('u','u','i','d'):
        case MKTAG('p','r','f','l'):
            offset += AV_RB32(buf+offset);
            // if we only find those cause probedata is too small at least rate them
            score = 50;
            break;
        default:
            // unrecognized tag
            return score;
        }
    }
    return score;
}

int mov_read_header(AV_PLAYER *player)
{
	MOVContext		*mov;
	ByteIOContext	*bio;
	MOVAtom atom = { 0 };
	int64_t		cur_pos;
	int		rval;

	mov = player->priv_data;
	bio = player->bio;
	mov->player = player;
	if(url_is_streamed(bio)) atom.size = INT64_MAX;
	else	atom.size = url_fsize(bio);
	rval = mov_read_default(mov, bio, atom);
	if(rval < 0) {
		printf("header read error: %d\n", rval);
		return rval;
	}
	if(!mov->found_moov) {
		printf("moov atom not found\n");
		return -1;
	}
	return 0;
}

void mov_read_packet2(AV_PLAYER *player, int formatIndex, AV_BUFFER *buffer)
{
	MOVContext		*mov;
	ByteIOContext	*bio;
	AV_STREAM_TRACK	*trk;
    AVIndexEntry	*sample;
	int		rval;

	mov = player->priv_data;
	bio = player->bio;
	trk = avStreamTrackAt(player, formatIndex);
	if(trk->currentSample < trk->nb_index_entries) {
		sample = &trk->index_entries[trk->currentSample];
//av_log(NULL, AV_LOG_INFO, "%d/%d %lld %d\n", trk->currentSample, trk->nb_index_entries, sample->pos, sample->size);
		trk->currentSample++;
        if(url_fseek(bio, sample->pos, SEEK_SET) != sample->pos) {
            //printf("format %d, offset 0x%"PRIx64": partial file\n", movsc->ffindex, sample->pos);
			buffer->status = 3;
            return;
        }
		rval = get_buffer(bio, buffer->data, sample->size);
//printf("mov_read: %x %d-%d/%d => %d\n", formatIndex, trk, trk->currentSample, trk->nb_index_entries, rval);
		if(rval < 0) buffer->status = 2;
		else {
			buffer->size = rval; buffer->status = 0;
			buffer->frameCount = trk->currentSample-1;
			buffer->frameSize = trk->nb_index_entries;
		}
	} else	buffer->status = 1;
}

/*
static int mov_read_close(AVFormatContext *avfc)
{
    MOVContext *mov = avfc->priv_data;
    int i, j;

    for(i = 0;i < avfc->nb_streams;i++) {
        AVStream *st = avfc->streams[i];
        MOVStreamContext *sc = st->priv_data;
        av_freep(&sc->ctts_data);
        for(j = 0;j < sc->drefs_count; j++) {
            av_freep(&sc->drefs[j].path);
            av_freep(&sc->drefs[j].dir);
        }
        av_freep(&sc->drefs);
        if(sc->pb && sc->pb != avfc->pb)
            url_fclose(sc->pb);
        av_freep(&st->codec->palctrl);
    }
    if(mov->dv_demux) {
        for(i = 0;i < mov->dv_fctx->nb_streams;i++) {
            av_freep(&mov->dv_fctx->streams[i]->codec);
            av_freep(&mov->dv_fctx->streams[i]);
        }
        av_freep(&mov->dv_fctx);
        av_freep(&mov->dv_demux);
    }
    av_freep(&mov->trex_data);
    return 0;
}

*/
