#include "pjsua_app.h"

#define THIS_FILE	"app"

#define SIP_DOMAIN	"example.com"
#define SIP_USER	"alice"
#define SIP_PASSWD	"secret"

pjsua_app_config	*pjs_cfg, _pjs_cfg;


#ifdef PJSUA_HAS_VIDEO

void app_config_init_video(pjsua_acc_config *acc_cfg)
{
	acc_cfg->vid_in_auto_show		= pjs_cfg->vid.in_auto_show;
	acc_cfg->vid_out_auto_transmit	= pjs_cfg->vid.out_auto_transmit;
	// Note that normally GUI application will prefer a borderless window.
	acc_cfg->vid_wnd_flags			= PJMEDIA_VID_DEV_WND_BORDER | PJMEDIA_VID_DEV_WND_RESIZABLE;
	acc_cfg->vid_cap_dev			= pjs_cfg->vid.vcapture_dev;
	acc_cfg->vid_rend_dev			= pjs_cfg->vid.vrender_dev;
}

#else

void app_config_init_video(pjsua_acc_config *acc_cfg)
{
	PJ_UNUSED_ARG(acc_cfg);
}

#endif

int pjsuaInit(void (*on_call_state)(pjsua_call_id call_id, pjsip_event *e), void (*on_media_state)(pjsua_call_id call_id))
{
	pjsua_acc_config	acc_cfg;
	pjsua_transport_id	transport_id;
    pjsua_acc_id		acc_id;
	pjmedia_tone_desc	tone[2];
	pj_pool_t		*tmp_pool;
    pj_str_t		name;
	pj_status_t		status;
	unsigned int	samples_per_frame;
	int		i;

	status = pjsua_create();
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_create() error", status);
		return -1;
	}
	pjs_cfg = &_pjs_cfg;
    pjsua_config_default(&pjs_cfg->cfg);
    pjs_cfg->pool = pjsua_pool_create("pjsua-app", 1000, 1000);
    tmp_pool = pjsua_pool_create("tmp-pjsua", 1000, 1000);
	pjsuaAppConfigDefault(pjs_cfg);
	pjs_cfg->cfg.cb.on_incoming_call	= NULL;
	pjs_cfg->cfg.cb.on_call_media_state	= on_media_state;
	pjs_cfg->cfg.cb.on_call_state		= on_call_state;
	pjs_cfg->media_cfg.snd_rec_latency	= 64;
	pjs_cfg->media_cfg.snd_play_latency	= 20;
	pjs_cfg->log_cfg.console_level		= 4;
	pjs_cfg->vid.vid_cnt				= 1;
	pjs_cfg->vid.in_auto_show			= PJ_FALSE;
	pjs_cfg->vid.out_auto_transmit		= PJ_TRUE;
printf("### pjs_init()...\n");
	status = pjsua_init(&pjs_cfg->cfg, &pjs_cfg->log_cfg, &pjs_cfg->media_cfg);
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_init() error", status);
		pj_pool_release(tmp_pool);
		pjsua_destroy();
		return -1;
	}
printf("### pjs_init() end\n");
	// create ringback tones
    if(pjs_cfg->no_tones == PJ_FALSE) {
		samples_per_frame = pjs_cfg->media_cfg.audio_frame_ptime * pjs_cfg->media_cfg.clock_rate *
									pjs_cfg->media_cfg.channel_count / 1000;
		name = pj_str("ringback");
		status = pjmedia_tonegen_create2(pjs_cfg->pool, &name,
                     pjs_cfg->media_cfg.clock_rate, pjs_cfg->media_cfg.channel_count, samples_per_frame,
                     16, PJMEDIA_TONEGEN_LOOP, &pjs_cfg->ringback_port);
		if(status != PJ_SUCCESS) {
			pjsua_perror(THIS_FILE,  "pjmedia_tonegen_create2() error", status);
			pj_pool_release(tmp_pool);
			pjsua_destroy();
			return -1;
		}
		tone[0].freq1		= 440;
		tone[0].freq2		= 480;
		tone[0].on_msec		= 1000;
		tone[0].off_msec	= 2000;
		pjmedia_tonegen_play(pjs_cfg->ringback_port, 1, tone, PJMEDIA_TONEGEN_LOOP);
		status = pjsua_conf_add_port(pjs_cfg->pool, pjs_cfg->ringback_port, &pjs_cfg->ringback_slot);
	}
pjs_cfg->no_udp = 0;
pjs_cfg->no_tcp = 1;
    if(!pjs_cfg->no_udp) {
		// Add UDP transport
		pjsua_transport_config_default(&pjs_cfg->udp_cfg);
		pjs_cfg->udp_cfg.port = 5060;
		status = pjsua_transport_create(PJSIP_TRANSPORT_UDP, &pjs_cfg->udp_cfg, &transport_id);
		if(status != PJ_SUCCESS) {
			pjsua_perror(THIS_FILE,  "pjsua_transport_create() error", status);
			pjsua_destroy();
			return -1;
		}
		status = pjsua_acc_add_local(transport_id, PJ_TRUE, &acc_id);
		if(status != PJ_SUCCESS) {
			pjsua_perror(THIS_FILE,  "pjsua_acc_add_local() error", status);
			pjsua_destroy();
			return -1;
		}
        pjsua_acc_get_config(acc_id, tmp_pool, &acc_cfg);
        app_config_init_video(&acc_cfg);
        acc_cfg.rtp_cfg = pjs_cfg->rtp_cfg;
        pjsua_acc_modify(acc_id, &acc_cfg);
		pjsua_acc_set_online_status(pjsua_acc_get_default(), PJ_TRUE);
	}
	if(!pjs_cfg->no_tcp) {
		status = pjsua_transport_create(PJSIP_TRANSPORT_TCP, &pjs_cfg->tcp_cfg, &transport_id);
		if(status != PJ_SUCCESS) {
			pjsua_perror(THIS_FILE,  "pjsua_acc_add_local() error", status);
			pjsua_destroy();
			return -1;
		}
		pjsua_acc_add_local(transport_id, PJ_TRUE, &acc_id);
		pjsua_acc_get_config(acc_id, tmp_pool, &acc_cfg);
		app_config_init_video(&acc_cfg);
		acc_cfg.rtp_cfg = pjs_cfg->rtp_cfg;
		pjsua_acc_modify(acc_id, &acc_cfg);
		pjsua_acc_set_online_status(pjsua_acc_get_default(), PJ_TRUE);
	}
	// add accounts
	for(i = 0;i < pjs_cfg->acc_cnt;i++) {
		pjs_cfg->acc_cfg[i].rtp_cfg = pjs_cfg->rtp_cfg;
		pjs_cfg->acc_cfg[i].reg_retry_interval = 300;
		pjs_cfg->acc_cfg[i].reg_first_retry_interval = 60;
		app_config_init_video(&pjs_cfg->acc_cfg[i]);
		status = pjsua_acc_add(&pjs_cfg->acc_cfg[i], PJ_TRUE, NULL);
		if(status != PJ_SUCCESS) break;
		pjsua_acc_set_online_status(pjsua_acc_get_default(), PJ_TRUE);
    }
printf("### pasua_start()...\n");
	// Initialization is done, now start pjsua
    status = pjsua_start();
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_start() error", status);
		pjsua_destroy();
		return -1;
	}
printf("### pasua_start() end\n");
/*
	// Register to SIP server by creating SIP account
	pjsua_acc_config_default(&acc_cfg);
	acc_cfg.id			= pj_str("sip:" SIP_USER "@" SIP_DOMAIN);
	acc_cfg.reg_uri		= pj_str("sip:" SIP_DOMAIN);
	acc_cfg.cred_count	= 1;
	acc_cfg.cred_info[0].realm		= pj_str(SIP_DOMAIN);
	acc_cfg.cred_info[0].scheme		= pj_str("digest");
	acc_cfg.cred_info[0].username	= pj_str(SIP_USER);
	acc_cfg.cred_info[0].data_type	= PJSIP_CRED_DATA_PLAIN_PASSWD;
	acc_cfg.cred_info[0].data		= pj_str(SIP_PASSWD);
	status = pjsua_acc_add(&acc_cfg, PJ_TRUE, &acc_id);
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_acc_add() error", status);
		pjsua_destroy();
		return -1;
	}
*/
	return 0;
}

void pjsuaExit(void)
{
    pjsua_destroy();
}

pjsua_call_setting  call_opt;

int pjsuaMakeCall(char *url)
{
	pj_status_t		status;

printf("pjsuaMakeCall: %s\n", url);
	status = pjsua_verify_url(url);
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_verify_url() error", status);
		return  -1;
	}
	pj_str_t uri = pj_str(url);
	pjsua_call_setting_default(&call_opt);
    call_opt.aud_cnt	= pjs_cfg->aud_cnt;
	call_opt.vid_cnt	= pjs_cfg->vid.vid_cnt;
	call_opt.flag		= PJSUA_CALL_INCLUDE_DISABLED_MEDIA;
	status = pjsua_call_make_call(pjsua_acc_get_default(), &uri, &call_opt, NULL, NULL, NULL);
	if(status != PJ_SUCCESS) {
		pjsua_perror(THIS_FILE,  "pjsua_call_make_call() error", status);
		return  -1;
	}
	return  0;
}

extern pjsua_call_id	current_call;

int pjsuaHangupCall(void)
{
printf("pjsuaHangupCall: call_id=%d\n", current_call);
	pjsua_call_hangup(current_call, 0, NULL, NULL);
	return 0;
}

int pjsuaCallDuration(void)
{
	app_call_data	*cd;
	
	cd = &pjs_cfg->call_data[current_call];
	return cd->call_duration;
}

