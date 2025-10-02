#include <stdio.h>
#include "defs.h"
#include "NSEnum.h"
#include "gactivity.h"
#include "gapp.h"
#include "appact.h"
#include "pjsualib.h"
#include "pjsua_app.h"

#define THIS_FILE	"app"

pjsua_call_id	current_call = PJSUA_INVALID_ID;


void pjsuaAppConfigDefault(pjsua_app_config *cfg)
{
    char	tmp[80];
	int		i;

    pj_ansi_sprintf(tmp, "PJSUA v%s %s", pj_get_version(), pj_get_sys_info()->info.ptr);
    pj_strdup2_with_null(cfg->pool, &cfg->cfg.user_agent, tmp);
    pjsua_logging_config_default(&cfg->log_cfg);
    pjsua_media_config_default(&cfg->media_cfg);
    pjsua_transport_config_default(&cfg->udp_cfg);
    cfg->udp_cfg.port		= 5060;
    pjsua_transport_config_default(&cfg->rtp_cfg);
    cfg->rtp_cfg.port		= 4000;
    cfg->redir_op			= PJSIP_REDIRECT_ACCEPT_REPLACE;
    cfg->duration			= PJSUA_APP_NO_LIMIT_DURATION;
    cfg->wav_id				= PJSUA_INVALID_ID;
    cfg->rec_id				= PJSUA_INVALID_ID;
    cfg->wav_port			= PJSUA_INVALID_ID;
    cfg->rec_port			= PJSUA_INVALID_ID;
    cfg->mic_level			= cfg->speaker_level = 1.0;
    cfg->capture_dev		= PJSUA_INVALID_ID;
    cfg->playback_dev		= PJSUA_INVALID_ID;
    cfg->capture_lat		= PJMEDIA_SND_DEFAULT_REC_LATENCY;
    cfg->playback_lat		= PJMEDIA_SND_DEFAULT_PLAY_LATENCY;
    cfg->ringback_slot		= PJSUA_INVALID_ID;
    cfg->ring_slot			= PJSUA_INVALID_ID;
    for(i = 0;i < PJ_ARRAY_SIZE(cfg->acc_cfg);i++)
		pjsua_acc_config_default(&cfg->acc_cfg[i]);
    cfg->vid.vcapture_dev	= PJMEDIA_VID_DEFAULT_CAPTURE_DEV;
	cfg->vid.vrender_dev	= PJMEDIA_VID_DEFAULT_RENDER_DEV;
    //cfg->vid.vrender_dev = PJSUA_INVALID_ID;
    cfg->aud_cnt = 1;
}

static void ringback_start(pjsua_call_id call_id)
{
printf("### ringback start: call_id=%d on=%d\n", call_id, pjs_cfg->call_data[call_id].ringback_on);
	if(!pjs_cfg->no_tones && !pjs_cfg->call_data[call_id].ringback_on) {
		pjs_cfg->call_data[call_id].ringback_on = PJ_TRUE;
		if(pjs_cfg->ringback_slot != PJSUA_INVALID_ID)
			pjsua_conf_connect(pjs_cfg->ringback_slot, 0);
	}
}

static void ringback_stop(pjsua_call_id call_id)
{
printf("### ringback stop: call_id=%d on=%d\n", call_id, pjs_cfg->call_data[call_id].ringback_on);
	if(!pjs_cfg->no_tones && pjs_cfg->call_data[call_id].ringback_on) {
		pjs_cfg->call_data[call_id].ringback_on = PJ_FALSE;
		if( pjs_cfg->ringback_slot != PJSUA_INVALID_ID) {
			pjsua_conf_disconnect(pjs_cfg->ringback_slot, 0);
			pjmedia_tonegen_rewind(pjs_cfg->ringback_port);
		}
    }
}

// Callback called by the library when call's state has changed
void on_call_state(pjsua_call_id call_id, pjsip_event *e)
{
	pjsua_call_info		call_info;
	pjsua_call_vid_strm_op_param	vid_strm_param;
	pjsua_stream_info	si;
	app_call_data		*cd;
	pjsip_endpoint		*endpt;
	pj_time_val		delay;
	pj_str_t		reason;
	pjsip_msg		*msg;
    unsigned char   amsg[12];
	int				code, vid_idx;

	PJ_UNUSED_ARG(e);
	pjsua_call_get_info(call_id, &call_info);
//PJ_LOG(3,(THIS_FILE, "call: id=%d %.*s", call_id, call_info.state_text.slen, call_info.state_text.ptr));
printf("##### call: %d %.*s #####\n", call_id, call_info.state_text.slen, call_info.state_text.ptr);
	switch(call_info.state) {
	case PJSIP_INV_STATE_NULL:
		break;
    case PJSIP_INV_STATE_CALLING:
		break;
    case PJSIP_INV_STATE_INCOMING:
		break;
    case PJSIP_INV_STATE_EARLY:
		// This can only occur because of TX or RX message
		pj_assert(e->type == PJSIP_EVENT_TSX_STATE);
		if(e->body.tsx_state.type == PJSIP_EVENT_RX_MSG) {
			msg = e->body.tsx_state.src.rdata->msg_info.msg;
		} else {
			msg = e->body.tsx_state.src.tdata->msg;
		}
		code = msg->line.status.code;
		reason = msg->line.status.reason;
		// Start ringback for 180 for UAC unless there's SDP in 180
		if(call_info.role==PJSIP_ROLE_UAC && code==180 && !msg->body
							&& call_info.media_status==PJSUA_CALL_MEDIA_NONE) {
			ringback_start(call_id);
		}
		break;	
    case PJSIP_INV_STATE_CONNECTING:
		break;
    case PJSIP_INV_STATE_CONFIRMED:
		vid_idx = pjsua_call_get_vid_stream_idx(call_id);
printf("### vid_index=%d\n", vid_idx);
		/*
		pjsua_call_vid_strm_op_param_default(&vid_strm_param);
		vid_strm_param.med_idx = vid_idx;
		//if(pjsua_call_get_stream_info(call_id, vid_strm_param.med_idx, &si) ||
	//			si.type != PJMEDIA_TYPE_VIDEO) {
	//		PJ_LOG(3,(THIS_FILE, "Error: invalid stream!"));
	//	}
		//vid_strm_param.dir = si.info.vid.dir & PJMEDIA_DIR_ENCODING;
		vid_strm_param.dir = PJMEDIA_DIR_ENCODING;
 		pjsua_call_set_vid_strm(call_id, PJSUA_CALL_VID_STRM_CHANGE_DIR, &vid_strm_param);
		*/
		if(pjs_cfg->duration != PJSUA_APP_NO_LIMIT_DURATION) {
			// Schedule timer to hangup call after the specified duration
			cd = &pjs_cfg->call_data[call_id];
			endpt = pjsua_get_pjsip_endpt();
			cd->timer.id = call_id;
			delay.sec = pjs_cfg->duration;
			delay.msec = 0;
			pjsip_endpt_schedule_timer(endpt, &cd->timer, &delay);
printf("schedule duration timer...%d sec\n", delay.sec);
		}
		break;
    case PJSIP_INV_STATE_DISCONNECTED:
		ringback_stop(call_id);
		cd = &pjs_cfg->call_data[call_id];
		cd->call_duration = call_info.connect_duration.sec;
		// Cancel duration timer, if any */
		if(cd->timer.id != PJSUA_INVALID_ID) {
printf("cancel duration timer...\n");
			endpt = pjsua_get_pjsip_endpt();
			cd->timer.id = PJSUA_INVALID_ID;
			pjsip_endpt_cancel_timer(endpt, &cd->timer);
		}
		break;
	}
	amsg[0] = GM_CALL_STATE_CHANGED; amsg[1] = call_info.state; memset(amsg+2, 8, 0);
	appPostMessage(amsg);
    // Rewind play file when hangup automatically, since file is not looped
	if(pjs_cfg->auto_play_hangup) {
		pjsua_player_set_pos(pjs_cfg->wav_id, 0);
PJ_LOG(3,(THIS_FILE, "Call %d is DISCONNECTED [reason=%d (%.*s)]", call_id, call_info.last_status,
					call_info.last_status_text.slen, call_info.last_status_text.ptr));
    }
//	if(current_call==PJSUA_INVALID_ID) current_call = call_id;
	current_call = call_id;
}

// General processing for media state. "mi" is the media index
static void on_call_generic_media_state(pjsua_call_info *ci, int mi, pj_bool_t *has_error)
{
    const char *status_name[] = {
        "None",
        "Active",
        "Local hold",
        "Remote hold",
        "Error"
    };

	PJ_UNUSED_ARG(has_error);
	pj_assert(ci->media[mi].status <= PJ_ARRAY_SIZE(status_name));
	pj_assert(PJSUA_CALL_MEDIA_ERROR == 4);
//PJ_LOG(4,(THIS_FILE, "Call %d media %d [type=%s], status is %s",
//			ci->id, mi, pjmedia_type_name(ci->media[mi].type), status_name[ci->media[mi].status]));
printf("##### media generic: %d type=%s status=%s #####\n", mi, pjmedia_type_name(ci->media[mi].type),
						status_name[ci->media[mi].status]);
}

// Process audio media state. "mi" is the media index.
static void on_call_audio_state(pjsua_call_info *ci, int mi, pj_bool_t *has_error)
{
	PJ_UNUSED_ARG(has_error);
	ringback_stop(ci->id);
printf("##### media audio: %d #####\n", ci->media_status);
	// Connect ports appropriately when media status is ACTIVE or REMOTE HOLD,
	// otherwise we should NOT connect the ports.
	if(ci->media[mi].status == PJSUA_CALL_MEDIA_ACTIVE
			|| ci->media[mi].status == PJSUA_CALL_MEDIA_REMOTE_HOLD) {
		pj_bool_t connect_sound = PJ_TRUE;
		pj_bool_t disconnect_mic = PJ_FALSE;
		pjsua_conf_port_id	call_conf_slot;
		call_conf_slot = ci->media[mi].stream.aud.conf_slot;
		// Loopback sound, if desired
		if(pjs_cfg->auto_loop) {
			pjsua_conf_connect(call_conf_slot, call_conf_slot);
			connect_sound = PJ_FALSE;
		}
		// Automatically record conversation, if desired 
		if(pjs_cfg->auto_rec && pjs_cfg->rec_port != PJSUA_INVALID_ID) {
			pjsua_conf_connect(call_conf_slot, pjs_cfg->rec_port);
		}
		//Stream a file, if desired
		if((pjs_cfg->auto_play || pjs_cfg->auto_play_hangup) && pjs_cfg->wav_port != PJSUA_INVALID_ID) {
		    pjsua_conf_connect(pjs_cfg->wav_port, call_conf_slot);
		    connect_sound = PJ_FALSE;
		}
		// Put call in conference with other calls, if desired
		if(pjs_cfg->auto_conf) {
			pjsua_call_id call_ids[PJSUA_MAX_CALLS];
			unsigned call_cnt=PJ_ARRAY_SIZE(call_ids);
			unsigned i;
			// Get all calls, and establish media connection between
			// this call and other calls.
			pjsua_enum_calls(call_ids, &call_cnt);
			for(i = 0;i < call_cnt;i++) {
				if(call_ids[i] == ci->id) continue;
				if(!pjsua_call_has_media(call_ids[i])) continue;
				pjsua_conf_connect(call_conf_slot, pjsua_call_get_conf_port(call_ids[i]));
				pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]), call_conf_slot);
				// Automatically record conversation, if desired 
				if(pjs_cfg->auto_rec && pjs_cfg->rec_port != PJSUA_INVALID_ID) {
					pjsua_conf_connect(pjsua_call_get_conf_port(call_ids[i]), pjs_cfg->rec_port);
				}
			}
			// Also connect call to local sound device 
			connect_sound = PJ_TRUE;
		}
		// Otherwise connect to sound device 
		if(connect_sound) {
			pjsua_conf_connect(call_conf_slot, 0);
			if(!disconnect_mic) pjsua_conf_connect(0, call_conf_slot);
			// Automatically record conversation, if desired 
			if(pjs_cfg->auto_rec && pjs_cfg->rec_port != PJSUA_INVALID_ID) {
				pjsua_conf_connect(call_conf_slot, pjs_cfg->rec_port);
				pjsua_conf_connect(0, pjs_cfg->rec_port);
			}
		}
    }
}

// Process video media state. "mi" is the media index
static void on_call_video_state(pjsua_call_info *ci, int mi, pj_bool_t *has_error)
{
printf("##### media video: %d #####\n", ci->media_status);
	if(ci->media_status != PJSUA_CALL_MEDIA_ACTIVE)
		return;
	//arrange_window(ci->media[mi].stream.vid.win_in);
	PJ_UNUSED_ARG(has_error);
}

 // The action may connect the call to sound device, to file, or  to loop the call.
void on_call_media_state(pjsua_call_id call_id)
{
	pjsua_call_info call_info;
	pj_bool_t has_error = PJ_FALSE;
	int		i;

	pjsua_call_get_info(call_id, &call_info);
	for(i = 0;i < call_info.media_cnt;i++) {
		on_call_generic_media_state(&call_info, i, &has_error);
		switch(call_info.media[i].type) {
		case PJMEDIA_TYPE_AUDIO:
			on_call_audio_state(&call_info, i, &has_error);
			break;
		case PJMEDIA_TYPE_VIDEO:
			on_call_video_state(&call_info, i, &has_error);
			break;
		}
	}
	if(has_error) {
		pj_str_t reason = pj_str("Media failed");
		pjsua_call_hangup(call_id, 500, &reason, NULL);
    }
#if PJSUA_HAS_VIDEO
	// Check if remote has just tried to enable video
	if(call_info.rem_offerer && call_info.rem_vid_cnt) {
		// Check if there is active video
		i = pjsua_call_get_vid_stream_idx(call_id);
		if(i == -1 || call_info.media[i].dir == PJMEDIA_DIR_NONE) {
PJ_LOG(3,(THIS_FILE, "Just rejected incoming video offer on call %d, "
				"use \"vid call enable %d\" or \"vid call add\" to "
				"enable video!", call_id, i));
		}
	}
#endif
}

static void on_transport_state(pjsip_transport *tp, pjsip_transport_state state,
			       const pjsip_transport_state_info *info)
{
	char		buf[100], host_port[128];

    pj_addr_str_print(&tp->remote_name.host, tp->remote_name.port, host_port, sizeof(host_port), 1);
	switch(state) {
	case PJSIP_TP_STATE_CONNECTED:
		PJ_LOG(3,(THIS_FILE, "SIP %s transport is connected to %s", tp->type_name, host_port));
		break;
    case PJSIP_TP_STATE_DISCONNECTED:
		snprintf(buf, sizeof(buf), "SIP %s transport is disconnected " "from %s", tp->type_name, host_port);
		pjsua_perror(THIS_FILE, buf, info->status);
		break;
    }
}

static void on_call_media_event(pjsua_call_id call_id, unsigned med_idx, pjmedia_event *event)
{
	pjmedia_event_media_tp_err_data	*err_data;
	pjsua_call_info		ci;
	pjsua_vid_win_id	wid;
	pjmedia_rect_size	size;
	pjsua_vid_win_info	win_info;
    char	event_name[8];

PJ_LOG(4,(THIS_FILE, "Event %s", pjmedia_fourcc_name(event->type, event_name)));
	if(event->type == PJMEDIA_EVENT_MEDIA_TP_ERR) {
	err_data = &event->data.med_tp_err;
PJ_PERROR(3, (THIS_FILE, err_data->status, 
		  "Media transport error event (%s %s %s)",
		  (err_data->type==PJMEDIA_TYPE_AUDIO)?"Audio":"Video",
		  (err_data->is_rtp)?"RTP":"RTCP",
		  get_media_dir(err_data->dir)));
    }
#if PJSUA_HAS_VIDEO
	else if(event->type == PJMEDIA_EVENT_FMT_CHANGED) {
		// Adjust renderer window size to original video size
		pjsua_call_get_info(call_id, &ci);
		if(ci.media[med_idx].type == PJMEDIA_TYPE_VIDEO &&
					ci.media[med_idx].dir & PJMEDIA_DIR_DECODING) {
			wid = ci.media[med_idx].stream.vid.win_in;
			pjsua_vid_win_get_info(wid, &win_info);
			size = event->data.fmt_changed.new_fmt.det.vid.size;
			if(size.w != win_info.size.w || size.h != win_info.size.h) {
				pjsua_vid_win_set_size(wid, &size);
				// Re-arrange video windows
				arrange_window(PJSUA_INVALID_ID);
			}
		}
	}
#else
	PJ_UNUSED_ARG(call_id);
	PJ_UNUSED_ARG(med_idx);    
#endif
}

