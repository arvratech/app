#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avio.h"
#include "avcodecs.h"
#include "wav.h"

static int64_t		data_end;


void pcmSingle2Stereo(unsigned char *src, int srcLen, unsigned char *dst)
{
	unsigned char	*s, *d;
	int		i;

	s = src; d = dst;
	for(i = 0;i < srcLen;i += 2) {
		*d = *(d+2) = *s++;
		*(d+1) = *(d+3) = *s++;
		d += 4;
	}
}

int wavProbe(unsigned char *buf)
{
	int		score;

	if(!memcmp(buf, "RIFF", 4) && !memcmp(buf+8, "WAVE", 4)) {
		score = 100;
	} else {
		score = 0;
	}
	return score;
}

// return the size of the found tag
static int64_t find_tag(ByteIOContext *bio, uint32_t tag1)
{
	unsigned int	tag;
	int64_t		size;

	while(1) {
		if(url_feof(bio)) return -1;
		tag  = get_le32(bio);
		size = get_le32(bio);
		if(tag == tag1) break;
		url_fseek(bio, size, SEEK_CUR);
	}
	return size;
}

/* We could be given one of the three possible structures here:
 * WAVEFORMAT, PCMWAVEFORMAT or WAVEFORMATEX. Each structure
 * is an expansion of the previous one with the fields added
 * at the bottom. PCMWAVEFORMAT adds 'WORD wBitsPerSample' and
 * WAVEFORMATEX adds 'WORD  cbSize' and basically makes itself
 * an openended structure.
 */
void ff_get_wav_header(ByteIOContext *bio, AV_STREAM_TRACK *trk, int size)
{
	int		id, cbSize;

	id = get_le16(bio);
    trk->mediaType = AV_MEDIA_TYPE_AUDIO;
	//froamt->codec_tag = id;
    trk->channels	= get_le16(bio);
    trk->sampleRate	= get_le32(bio);
	trk->bitRate	= get_le32(bio) * 8;
	trk->blockAlign	= get_le16(bio);
	if(size == 14) {	// We're dealing with plain vanilla WAVEFORMAT
		trk->bitsPerSample = 8;
	} else {
		trk->bitsPerSample = get_le16(bio);
	}
	if(size >= 18) {	// We're obviously dealing with WAVEFORMATEX
		cbSize = get_le16(bio);	// cbSize
		size -= 18;
		cbSize = FFMIN(size, cbSize);
		if(cbSize >= 22 && id == 0xfffe) {	// WAVEFORMATEXTENSIBLE
			trk->bitsPerSample	= get_le16(bio);
			get_le32(bio);		// dwChannelMask
			id = get_le32(bio);	// 4 first bytes of GUID 
			url_fskip(bio, 12);	// skip end of GUID
            cbSize -= 22;
            size -= 22;
        }
		trk->extradataLen = cbSize;
		if(cbSize > 0) {
			get_buffer(bio, trk->extradata, trk->extradataLen);
			size -= cbSize;
		}
		// It is possible for the chunk to contain garbage at the end
		if(size > 0) url_fskip(bio, size);
    }
	trk->codecId = AV_CODEC_WAV;
	//codec->codecId = ff_wav_codec_get_id(id, codec->bits_per_coded_sample);
}

int wavReadHeader(AV_PLAYER *player)
{
	AV_STREAM_TRACK	*trk, _trk;
	ByteIOContext	*bio;
	unsigned int	tag, tag1;
    int64_t		size;

	bio = player->bio;
	trk = &_trk;
	memset(trk, 0, sizeof(AV_STREAM_TRACK));
	// check RIFF header
	tag = get_le32(bio);
	if(tag == MKTAG('R', 'F', '6', '4')) {
		printf("unsupported 64-bit wave\n");
		return -1;
	}
    if(tag != MKTAG('R', 'I', 'F', 'F')) return -1;
    get_le32(bio);		// file size
    tag = get_le32(bio);
    if(tag != MKTAG('W', 'A', 'V', 'E')) return -1;
    // parse fmt header
    size = find_tag(bio, MKTAG('f', 'm', 't', ' '));
    if(size < 0) return -1;
    ff_get_wav_header(bio, trk, size);
    size = find_tag(bio, MKTAG('d', 'a', 't', 'a'));
    if(size < 0) return -1;
    if(!size) data_end = INT64_MAX;
    else	data_end = url_ftell(bio) + size;
	avStreamAddTrack(player, trk);
	return 0;
}

int wavReadData(AV_PLAYER *player, unsigned char *buf, int size)
{
	ByteIOContext		*bio;
	AV_STREAM_TRACK		*trk;
	int64_t		left;
	int		rval;

	bio = player->bio;
	trk = avStreamTrack(player, AV_MEDIA_TYPE_AUDIO);
    left = data_end - url_ftell(bio);
	if(left <= 0){
		left = find_tag(bio, MKTAG('d', 'a', 't', 'a'));
		if(left < 0) return -1;
		data_end = url_ftell(bio) + left;
    }
    if(trk->blockAlign > 1) {
		if(size < trk->blockAlign) size = trk->blockAlign;
		size = (size / trk->blockAlign) * trk->blockAlign;
    }
	size = FFMIN(size, left);
	rval = get_buffer(bio, buf, size);
    return rval;
}

int wavDecOpen(AV_CODEC *codec)
{
	codec->encoder = 0;	
	codec->codecId = AV_CODEC_WAV;
	strcpy(codec->codecName, "WAVE");
	return  0;
}

void wavDecClose(AV_CODEC *codec)
{
}

int wavDecStart(AV_CODEC *codec)
{
	return 0;
}

void wavDecStop(AV_CODEC *codec)
{
}

int wavDecDecodeInputBuffer(AV_CODEC *codec, AV_BUFFER *buffer, unsigned char *outBuf)
{
	AV_STREAM_TRACK	*trk;
	unsigned char	*p;
	int		len;

	trk = avCodecOutputTrack(codec);
	if(trk->channels < 2) {
		pcmSingle2Stereo(buffer->data, buffer->size, outBuf);
		len = buffer->size << 1;
	} else {
		memcpy(outBuf, buffer->data, buffer->size);
		len = buffer->size;
	}
	return len;
}

int wavEncOpen(AV_CODEC *codec)
{
	codec->encoder = 1;	
	codec->codecId = AV_CODEC_WAV;
	strcpy(codec->codecName, "WAVE");
	return  0;
}

void wavEncClose(AV_CODEC *codec)
{
}

int wavEncStart(AV_CODEC *codec)
{
	return 0;
}

void wavEncStop(AV_CODEC *codec)
{
}

