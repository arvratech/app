#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <linux/types.h>
#include <sys/soundcard.h>
#include "rtc.h"
#include "uv.h"
#include "vox.h"

#define VOX_STATE_IDLE			0
#define VOX_STATE_BUSY			1
#define VOX_STATE_DONE			2
#define VOX_STATE_STOPPING		3

static unsigned char voxBuf[8192], tmpBuf[4096];

typedef struct _VOX {
	int		dsp;
	int		mix;
	timer_t	tid;
	int		channelNo;
	int		bitsPerSample;
	int		sampleRate;
	int		state;
	FILE	*fp;
} VOX;

static VOX	*vox, _vox;

static _VoxTimerStart(int usec)
{
	struct itimerspec *it, _it;
	long	val;
	int		rval;

	it = &_it;
	val = usec;
	if(val < 1000000) it->it_value.tv_sec  = 0;
	else {
		it->it_value.tv_sec = val / 1000000;
		val = val % 1000000;
	}
	it->it_value.tv_nsec = val * 1000;
	it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	rval = timer_settime(vox->tid, 0, it, NULL);
	if(rval < 0) {
		perror("timerStartEx: timer_settime");
		return -1;
	}
	return 0;
}

static void _VoxTimerStop(void)
{
	struct itimerspec *it, _it;
	long	val;
	int		rval;

	it = &_it;
	it->it_value.tv_sec  = 0; it->it_value.tv_nsec = 0;
	it->it_interval.tv_sec = 0; it->it_interval.tv_nsec = 0;
	rval = timer_settime(vox->tid, 0, it, NULL);
	if(rval < 0) {
		perror("timerStop: timer_settime");
	}
}

void audioGet(void);

void SigTimerHandler(int signo, siginfo_t *si, void *uc)
{
	audio_buf_info	info;			
	unsigned char	*s, *d;
//	timer_t *tid;
	int		i, size, delay, fragSize;

//	tid = (void **)(si->si_value.sival_ptr);
	i = signo - SIGRTMIN;
	while(vox->state == VOX_STATE_BUSY) {
		ioctl(vox->dsp, SNDCTL_DSP_GETOSPACE, &info);
		// Fixed: info.fragstotal=2 info.fragsize=8192
		// First: info.fragments=0 info.bytes=16384
		//	if(stopReq) break;
		fragSize = info.fragsize;
		if(info.bytes < fragSize) {
			delay = (fragSize-info.bytes) * 250000 / vox->sampleRate;	// 25000=1000000/2*2  2:16bit 2:2ChannelNo
			if(delay <= 0) delay = 1;
			_VoxTimerStart(delay);
			break;
		} else {
			size = info.bytes;
			if(size > fragSize) size = fragSize;
			if(vox->channelNo >= 2) {
				size = fread(voxBuf, 1, size, vox->fp);
				if(size <= 0) vox->state = VOX_STATE_DONE;
			} else {
				s = tmpBuf; d = voxBuf;
				size = fread(s, 1, size>>1, vox->fp);
				if(size <= 0) vox->state = VOX_STATE_DONE;
				else {
					for(i = 0;i < size;i += 2) {
						*d = *(d+2) = *s++;
						*(d+1) = *(d+3) = *s++;
						d += 4;
					}
					size <<= 1;
				}
			}
			if(size > 0) write(vox->dsp, voxBuf, size);
		}
	}
	if(vox->state == VOX_STATE_DONE) {
		if(vox->fp) {
			fclose(vox->fp); vox->fp = NULL;
		}
		ioctl(vox->dsp, SNDCTL_DSP_GETOSPACE, &info);
		fragSize = info.fragsize * info.fragstotal;
		size = fragSize - info.bytes;
		ioctl(vox->dsp, SNDCTL_DSP_GETODELAY, &size);
		if(size <= 0) {
			vox->state = VOX_STATE_STOPPING;
			delay = 1000;
		} else {
			delay = (size >> 1) * 250000 / vox->sampleRate;
//printf("Remain: size=%d %d delay=%d\n", size, delay);
			if(delay < 500) delay = 500;
		}
		_VoxTimerStart(delay);
	} else if(vox->state > VOX_STATE_DONE) {
		if(vox->fp) {
			fclose(vox->fp); vox->fp = NULL;
		}
		ioctl(vox->dsp, SNDCTL_DSP_RESET, NULL);
		vox->state = VOX_STATE_IDLE;
		audioGet();
	}
}

int voxOpen(void)
{
	struct sigevent   *se, _se;
	struct sigaction  _sa;
	timer_t		tid;
	int		rval, id, dsp, mix;

	dsp = open("/dev/dsp", O_RDWR);
	if(dsp < 0) {
		perror("open(/dev/dsp)");
		return -1;
	}
	mix = open("/dev/mixer", O_RDWR);
	if(mix < 0 ){
		perror("open(/dev/mixer)");
		close(dsp);
		return -1;
	}
	id = 16;
	se = &_se;
	se->sigev_notify = SIGEV_SIGNAL;
	se->sigev_signo = SIGRTMIN + id;
	se->sigev_value.sival_ptr = &tid;
	rval = timer_create(CLOCK_REALTIME, se, &tid);
	if(rval < 0) {
		perror("timerCreate: timer_create");
		close(dsp); close(mix);
		return -1;
	}
	//_sa.sa_handler = SigTimerHandler;
	_sa.sa_sigaction = SigTimerHandler;
	sigemptyset(&_sa.sa_mask);
	_sa.sa_flags = SA_SIGINFO;
	rval = sigaction(SIGRTMIN+id, &_sa, 0);
	if(rval < 0) {
		perror("timerCreate: sigaction");
		timer_delete(tid);
		close(dsp); close(mix);
		return -1;
	}	
	vox = &_vox;
	vox->dsp = dsp; vox->mix = mix; vox->tid = tid;
	vox->state = VOX_STATE_IDLE;
	// unlock the timer signal, so that timer notification can be delivered
printf("voxOpen: OK\n");
	return 0;
}

int voxClose(void)
{
	close(vox->dsp);
	close(vox->mix);
	timer_delete(vox->tid);
	return 0;
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
	ioctl(vox->mix, MIXER_WRITE(SOUND_MIXER_PCM), &val);
//	ioctl(mix, MIXER_READ(SOUND_MIXER_PCM), &val);	
//printf("get volume=%d-%d\n", val >> 8, val & 0xff);	
//	val = 0x6464;
//	ioctl(mix, MIXER_WRITE(SOUND_MIXER_VOLUME), &val);	
//	ioctl(mix, MIXER_WRITE(SOUND_MIXER_SPEAKER), &val);	
}

int voxPlayWave(char *fileName, int volume)
{
	unsigned char	*p;
	int		rval, data, count, fragSize;	

	vox->fp = fopen(fileName, "r+");
    if(!vox->fp) {
    	printf("voxPlayWave: fopen error: %s\n", fileName);
    	return -1;
    }
	p = tmpBuf;
	rval = fread(p, 1, 44, vox->fp);
	if(rval < 44) {
		printf("voxPlayWave: %s format error\n", fileName);
		fclose(vox->fp); vox->fp = NULL;
		return -1;
	}
#ifdef _VOX_DEBUG
printf("voxPlayWave: [%s] vol=%d...\n", fileName, volume);
#endif
	p += 12;
	vox->channelNo	   = _Reverse2Short(p + 10);
	vox->sampleRate	   = _Reverse2Long(p + 12);
	vox->bitsPerSample = _Reverse2Short(p + 22);
	count = _Reverse2Long(p + 4);
	p += count + 8;
	if(p[0] != 'd') {
		count = _Reverse2Long(p + 4);
		p += count + 8;	
	}	
	if(p[0] != 'd') {
//printf("wave file format error...\n");
		fclose(vox->fp); vox->fp = NULL;
		return -1;
	}
//	vox->dataSize	= _Reverse2Long(s + 4);
//printf("[%s] %dHz %dbit %dCH Size=%d\n", fileName, vox->sampleRate, vox->bitsPerSample, vox->channelNo, vox->dataSize);
	p += 8;
	data = AFMT_S16_LE;		// standard 16bit little endian format, support this format only
	ioctl(vox->dsp, SNDCTL_DSP_SETFMT, &data);
	ioctl(vox->dsp, SNDCTL_DSP_SPEED, &vox->sampleRate);
	data = 2;
	ioctl(vox->dsp, SNDCTL_DSP_CHANNELS, &data);
	ioctl(vox->dsp, SNDCTL_DSP_GETBLKSIZE, &fragSize);
	voxSetVolume(volume);
	vox->state = VOX_STATE_BUSY;
	_VoxTimerStart(10);
	return 0;
}

void voxStopWave(void)
{
	if(vox->state) {
		_VoxTimerStop();
		ioctl(vox->dsp, SNDCTL_DSP_RESET, NULL);
		vox->state = VOX_STATE_IDLE;
	}
}

int voxState(void)
{
	return (int)vox->state;
}

