#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <linux/types.h>
#include <sys/soundcard.h>
#include "rtc.h"
#include "vox.h"

static int	dsp, mix;

typedef struct _PCM {
	int		channelNo;
	int		bitsPerSample;
	int		sampleRate;
	int		dataSize;
} PCM;

static PCM	*pcm, _pcm;

static int _VoxOpen(void)
{
	dsp = open("/dev/dsp", O_RDWR);
	if(dsp < 0) {
		perror("open(/dev/dsp)");
		mix = -1;
		return -1;
	}
	mix = open("/dev/mixer", O_RDWR);
	if(mix < 0 ){
		perror("open(/dev/mixer)");
		close(dsp); dsp = -1;
		return -1;
	}
	pcm = &_pcm;
printf("voxOpen: %d %d\n", dsp, mix);
	return 0;
}

static int _VoxClose(void)
{
printf("voxClose...%d %d\n", dsp, mix);
	if(dsp >= 0) {
		close(dsp); dsp = -1;
	}
	if(mix >= 0) {
		close(mix); mix = -1;
	}
	return 0;	
}

int voxOpen(void)
{
#ifdef _VOX_CLOSE_ 
	return 0;
#else
	return _VoxOpen();
#endif
}

int voxClose(void)
{
#ifdef _VOX_CLOSE_ 
	return 0;
#else
	return _VoxClose();	
#endif
}

unsigned short _Reverse2Short(unsigned char *data)
{
	unsigned short	val;

	val = *data | *(data+1) << 8;
	return val;
}

unsigned long _Reverse2Long(unsigned char *data)
{
	unsigned long	val;

	val = *data | *(data+1) << 8 | *(data+2) << 16 | *(data+3) << 24;
	return val;
}

void voxSetVolume(int volume)
{
	int		val;

	val = (volume) << 8 | volume;
	ioctl(mix, MIXER_WRITE(SOUND_MIXER_PCM), &val);
//	ioctl(mix, MIXER_READ(SOUND_MIXER_PCM), &val);	
//printf("get volume=%d-%d\n", val >> 8, val & 0xff);	
//	val = 0x6464;
//	ioctl(mix, MIXER_WRITE(SOUND_MIXER_VOLUME), &val);	
//	ioctl(mix, MIXER_WRITE(SOUND_MIXER_SPEAKER), &val);	
}

static unsigned char voxBuf[8192], tmpBuf[4096];
static int	stopReq;

int voxPlayWave(char *fileName, int volume)
{
	FILE	*fp;
	unsigned char	*s, *d;
	audio_buf_info	info;			
	int		i, rval, data, size, count, fragSize, delay;	
	
	stopReq = 0;
	fp = fopen(fileName, "r+");
    if(!fp) {
    	printf("voxPlayWave: fopen error: %s\n", fileName);
    	return -1;
    }
	s = tmpBuf;
	rval = fread(s, 1, 44, fp);
	if(rval < 44) {
		printf("voxPlayWave: %s format error\n", fileName);
		fclose(fp);
		return -1;
	}
#ifdef _VOX_DEBUG
printf("voxPlayWave: [%s] vol=%d...\n", fileName, volume);
#endif
	s += 12;
	pcm->channelNo	   = _Reverse2Short(s + 10);
	pcm->sampleRate	   = _Reverse2Long(s + 12);
	pcm->bitsPerSample = _Reverse2Short(s + 22);
	count = _Reverse2Long(s + 4);
	s += count + 8;
	if(s[0] != 'd') {
		count = _Reverse2Long(s + 4);
		s += count + 8;	
	}	
	if(s[0] != 'd') {
//printf("wave file format error...\n");
		fclose(fp);
		return -1;
	}
#ifdef _VOX_CLOSE_ 
	rval = _VoxOpen();
	if(rval < 0) {
		fclose(fp);
		return -1; 
	}
#endif
	pcm->dataSize	= _Reverse2Long(s + 4);
//printf("[%s] %dHz %dbit %dCH Size=%d\n", fileName, pcm->sampleRate, pcm->bitsPerSample, pcm->channelNo, pcm->dataSize);
	s += 8;
	data = AFMT_S16_LE;		// standard 16bit little endian format, support this format only
	ioctl(dsp, SNDCTL_DSP_SETFMT, &data);			// Select the sample format
	ioctl(dsp, SNDCTL_DSP_SPEED, &pcm->sampleRate);	// Set the sampling rate
	data = 2;
	ioctl(dsp, SNDCTL_DSP_CHANNELS, &data);			// Set the number of audio channels
	ioctl(dsp, SNDCTL_DSP_GETBLKSIZE, &fragSize);
	voxSetVolume(volume);
	while(1) {
		while(1) {
			ioctl(dsp , SNDCTL_DSP_GETOSPACE , &info);
			// Fixed: info.fragstotal=2 info.fragsize=8192
			// First: info.fragments=0 info.bytes=16384
			if(info.bytes >= fragSize) break;
			if(stopReq) break;
			delay = (fragSize-info.bytes) * 250000 / pcm->sampleRate;	// 25000=1000000/2*2  2:16bit 2:2ChannelNo
			if(delay <= 0) delay = 1;
//printf("s=%d d=%d\n", info.bytes, delay);	
			usleep(delay);
		}
		if(stopReq) break;
		size = info.bytes;
		if(size > fragSize) size = fragSize;
		if(pcm->channelNo >= 2) {
			size = fread(voxBuf, 1, size, fp);
			if(size <= 0) break;
		} else {
			s = tmpBuf; d = voxBuf;
			size = fread(s, 1, size>>1, fp);
			if(size <= 0) break;
			for(i = 0;i < size;i += 2) {
				*d = *(d+2) = *s++;
				*(d+1) = *(d+3) = *s++;
				d += 4;
			}
			size <<= 1;
		}
		write(dsp, voxBuf, size);
//printf("wr=%d\n", size);	
	}
	fclose(fp);
	if(!stopReq) {
		fragSize *= info.fragstotal;	
		while(1) {	
			ioctl(dsp, SNDCTL_DSP_GETOSPACE , &info);
			data = fragSize - info.bytes;
			ioctl(dsp, SNDCTL_DSP_GETODELAY, &size);
			if(size <= 0) break;
			if(stopReq) break;
			delay = (size >> 1) * 250000 / pcm->sampleRate;
//printf("Remain: size=%d %d delay=%d\n", size, data, delay);
			if(delay < 500) delay = 500;
			usleep(delay);
		}
	}
	if(!stopReq) usleep(1000);
	ioctl(dsp, SNDCTL_DSP_RESET, NULL);
//printf("vox END.......\n");
#ifdef _VOX_CLOSE_ 
	 _VoxClose();
#endif
#ifdef _VOX_DEBUG
printf("voxPlayWave: [%s] end %d\n", fileName, stopReq);
#endif
	return 0;
}

void voxStopWave(void)
{
	stopReq = 1;
}

