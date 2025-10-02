#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <malloc.h>
#include <alsa/asoundlib.h>
#include "avdevice.h"

char	*card	= "default";
static snd_mixer_t			*mixer;
static snd_mixer_selem_id_t	*sid;
static snd_mixer_elem_t		*elem;
static const char	*selem_name	= "Master";
static int		audioVolume;


int avAudioMixerOpen(void)
{
	int		rval;

	rval = snd_mixer_open(&mixer, 0);
	if(rval < 0) {
		printf("snd_mixer_open() error: %s\n", snd_strerror(rval));
		return -1;
	}
	rval = snd_mixer_attach(mixer, card);
    rval = snd_mixer_selem_register(mixer, NULL, NULL);
    rval = snd_mixer_load(mixer);
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
	elem = snd_mixer_find_selem(mixer, sid);
	if(!elem) {
		printf("snd_mixer_find_selem() error\n");
		return -1;
	}
	audioVolume = -1;
//printf("avAudioMixerOpen: OK\n");
	return 0;
}

void avAudioMixerClose(void)
{
	snd_mixer_detach(mixer, card);
    snd_mixer_close(mixer);
//printf("avAudioMixerClose...\n");
}

void avAudioMixerSetVolume(int volume)
{
//	snd_mixer_elem_t	*elem;
	long	min, max;
	int		val;

	if(volume > 46 && volume < 58) {
		volume += 6;
		if(volume > 58) volume = 58; else volume = 46;
	}
	if(audioVolume != volume) {
		//elem = snd_mixer_find_selem(mixer, sid);	return elem=NULL
		snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
		snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);
		audioVolume = volume;
	}
//printf("##### volume=%d #####\n", volume);
}

