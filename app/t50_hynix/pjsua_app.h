#ifndef _PJSUA_APP_H_
#define _PJSUA_APP_H_


#include "pjsua-lib/pjsua.h"


#define PJSUA_APP_NO_LIMIT_DURATION		0x7FFFFFFF


// call specific data
typedef struct _app_call_data {
    pj_timer_entry      timer;
    pj_bool_t			ringback_on;
    pj_bool_t			ring_on;
	int					call_duration;
} app_call_data;

// video settings
typedef struct _app_vid {
    unsigned            vid_cnt;
    int					vcapture_dev;
    int					vrender_dev;
    pj_bool_t			in_auto_show;
    pj_bool_t			out_auto_transmit;
} app_vid;

// Pjsua application data
typedef struct _pjsua_app_config {
    pjsua_config			cfg;
    pjsua_logging_config	log_cfg;
    pjsua_media_config		media_cfg;
    pjsua_transport_config	udp_cfg;
    pjsua_transport_config	tcp_cfg;
    pjsua_transport_config	rtp_cfg;
    pj_bool_t			no_refersub;
    pj_bool_t			ipv6;
    pj_bool_t			enable_qos;
    pj_bool_t			no_tcp;
    pj_bool_t			no_udp;
    pj_bool_t			use_tls;
    pjsip_redirect_op	redir_op;
    int					srtp_keying;
    unsigned            acc_cnt;
    pjsua_acc_config	acc_cfg[PJSUA_MAX_ACC];
    app_call_data		call_data[PJSUA_MAX_CALLS];
    pj_pool_t			*pool;
	// Compatibility with older pjsua
    unsigned			codec_cnt;
    pj_str_t			codec_arg[32];
    unsigned			codec_dis_cnt;
    pj_str_t			codec_dis[32];
    pj_bool_t			null_audio;
    unsigned			wav_count;
    pj_str_t			wav_files[32];
    unsigned			tone_count;
    pjmedia_tone_desc	tones[32];
    pjsua_conf_port_id	tone_slots[32];
    pjsua_player_id		wav_id;
    pjsua_conf_port_id	wav_port;
    pj_bool_t			auto_play;
    pj_bool_t			auto_play_hangup;
    pj_timer_entry		auto_hangup_timer;
	pj_bool_t			auto_loop;
	pj_bool_t			auto_conf;
	pj_str_t			rec_file;
	pj_bool_t           auto_rec;
	pjsua_recorder_id       rec_id;
	pjsua_conf_port_id      rec_port;
	unsigned            auto_answer;
	unsigned            duration;
    float				mic_level, speaker_level;
    int					capture_dev, playback_dev;
    unsigned			capture_lat, playback_lat;
    pj_bool_t			no_tones;
    int					ringback_slot;
    int					ringback_cnt;
    pjmedia_port		*ringback_port;
    int					ring_slot;
    int					ring_cnt;
    pjmedia_port		*ring_port;
    app_vid				vid;
    unsigned            aud_cnt;
} pjsua_app_config;


extern pjsua_app_config		*pjs_cfg;


void pjsuaAppConfigDefault(pjsua_app_config *cfg);
void on_call_state(pjsua_call_id call_id, pjsip_event *e);
void on_call_media_state(pjsua_call_id call_id);


#endif

