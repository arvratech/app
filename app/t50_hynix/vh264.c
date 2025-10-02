#include <pjmedia-codec/openh264.h>
#include <pjmedia-codec/h264_packetizer.h>
#include <pjmedia/vid_codec_util.h>
#include <pjmedia/errno.h>
#include <pj/log.h>

#define DEFAULT_WIDTH			640
#define DEFAULT_HEIGHT			480
#define DEFAULT_FPS				15
#define DEFAULT_AVG_BITRATE		256000
#define DEFAULT_MAX_BITRATE		256000
#define MAX_RX_WIDTH			1200
#define MAX_RX_HEIGHT			800


static pj_status_t vh264_test_alloc(pjmedia_vid_codec_factory *factory,
                                    const pjmedia_vid_codec_info *info);
static pj_status_t vh264_default_attr(pjmedia_vid_codec_factory *factory,
                                      const pjmedia_vid_codec_info *info,
                                      pjmedia_vid_codec_param *attr);
static pj_status_t vh264_enum_info(pjmedia_vid_codec_factory *factory,
                                   unsigned *count,
                                   pjmedia_vid_codec_info codecs[]);
static pj_status_t vh264_alloc_codec(pjmedia_vid_codec_factory *factory,
                                     const pjmedia_vid_codec_info *info,
                                     pjmedia_vid_codec **p_codec);
static pj_status_t vh264_dealloc_codec(pjmedia_vid_codec_factory *factory,
                                       pjmedia_vid_codec *codec);

// Codec operations
static pj_status_t vh264_codec_init(pjmedia_vid_codec *codec, pj_pool_t *pool );
static pj_status_t vh264_codec_open(pjmedia_vid_codec *codec,
                                    pjmedia_vid_codec_param *param );
static pj_status_t vh264_codec_close(pjmedia_vid_codec *codec);
static pj_status_t vh264_codec_modify(pjmedia_vid_codec *codec,
                                      const pjmedia_vid_codec_param *param);
static pj_status_t vh264_codec_get_param(pjmedia_vid_codec *codec,
                                         pjmedia_vid_codec_param *param);
static pj_status_t vh264_codec_encode_begin(pjmedia_vid_codec *codec,
                                            const pjmedia_vid_encode_opt *opt,
                                            const pjmedia_frame *input,
                                            unsigned out_size,
                                            pjmedia_frame *output,
                                            pj_bool_t *has_more);
static pj_status_t vh264_codec_encode_more(pjmedia_vid_codec *codec,
                                           unsigned out_size,
                                           pjmedia_frame *output,
                                           pj_bool_t *has_more);
static pj_status_t vh264_codec_decode(pjmedia_vid_codec *codec,
                                      pj_size_t count,
                                      pjmedia_frame packets[],
                                      unsigned out_size,
                                      pjmedia_frame *output);

// Definition for H264 codecs operations
static pjmedia_vid_codec_op vh264_codec_op = {
    &vh264_codec_init,
    &vh264_codec_open,
    &vh264_codec_close,
    &vh264_codec_modify,
    &vh264_codec_get_param,
    &vh264_codec_encode_begin,
    &vh264_codec_encode_more,
    &vh264_codec_decode,
    NULL
};

// Definition for H264 codecs factory operations
static pjmedia_vid_codec_factory_op vh264_factory_op = {
    &vh264_test_alloc,
    &vh264_default_attr,
    &vh264_enum_info,
    &vh264_alloc_codec,
    &vh264_dealloc_codec
};

static struct vh264_factory {
    pjmedia_vid_codec_factory	base;
    pjmedia_vid_codec_mgr		*mgr;
    pj_pool_factory             *pf;
    pj_pool_t					*pool;
} vh264_factory;


PJ_DEF(pj_status_t)pjmedia_codec_vh264_vid_init(pjmedia_vid_codec_mgr *mgr,
                                                    pj_pool_factory *pf)
{
    const pj_str_t h264_name = { (char *)"H264", 4};
    pj_status_t status;

    if(vh264_factory.pool) {
		// Already initialized
		return PJ_SUCCESS;
    }
    if(!mgr) mgr = pjmedia_vid_codec_mgr_instance();
    PJ_ASSERT_RETURN(mgr, PJ_EINVAL);
    // Create OpenH264 codec factory. */
	vh264_factory.base.op = &vh264_factory_op;
	vh264_factory.base.factory_data = NULL;
	vh264_factory.mgr = mgr;
	vh264_factory.pf = pf;
    // Registering format match for SDP negotiation
    status = pjmedia_sdp_neg_register_fmt_match_cb(&h264_name,
					&pjmedia_vid_codec_h264_match_sdp);
    if(status != PJ_SUCCESS) return status;
    // Register codec factory to codec manager
    status = pjmedia_vid_codec_mgr_register_factory(mgr,
						    &vh264_factory.base);
    if(status != PJ_SUCCESS) return status;
PJ_LOG(4,(THIS_FILE, "H264 codec initialized"));
    vh264_factory.pool = 1;
    return PJ_SUCCESS;
}

// Unregister OpenH264 codecs factory from pjmedia endpoint.
PJ_DEF(pj_status_t) pjmedia_codec_vh264_vid_deinit(void)
{
    pj_status_t status = PJ_SUCCESS;

    if(!vh264_factory.pool) {
		// Already deinitialized
		return PJ_SUCCESS;
    }
    // Unregister OpenH264 codecs factory
    status = pjmedia_vid_codec_mgr_unregister_factory(vh264_factory.mgr,
						      &vh264_factory.base);
    vh264_factory.pool = 0;
    return status;
}

static pj_status_t vh264_test_alloc(pjmedia_vid_codec_factory *factory,
                                    const pjmedia_vid_codec_info *info)
{
    PJ_ASSERT_RETURN(factory == &vh264_factory.base, PJ_EINVAL);
    if(info->fmt_id == PJMEDIA_FORMAT_H264 && info->pt)
		return PJ_SUCCESS;
    return PJMEDIA_CODEC_EUNSUP;
}

static pj_status_t vh264_default_attr(pjmedia_vid_codec_factory *factory,
                                      const pjmedia_vid_codec_info *info,
                                      pjmedia_vid_codec_param *param)
{
    PJ_ASSERT_RETURN(factory == &vh264_factory.base, PJ_EINVAL);
    PJ_ASSERT_RETURN(info && param, PJ_EINVAL);
    pj_bzero(param, sizeof(pjmedia_vid_codec_param));
    param->dir			= PJMEDIA_DIR_ENCODING_DECODING;
    param->packing		= PJMEDIA_VID_PACKING_PACKETS;
    // Encoded format
    pjmedia_format_init_video(&param->enc_fmt, PJMEDIA_FORMAT_H264,
                              DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_FPS, 1);
    // Decoded format
    pjmedia_format_init_video(&param->dec_fmt, PJMEDIA_FORMAT_I420,
                              DEFAULT_WIDTH, DEFAULT_HEIGHT, DEFAULT_FPS, 1);
    // Decoding fmtp
    param->dec_fmtp.cnt = 2;
    param->dec_fmtp.param[0].name	= pj_str((char*)"profile-level-id");
    param->dec_fmtp.param[0].val	= pj_str((char*)"42e01e");
    param->dec_fmtp.param[1].name	= pj_str((char*)" packetization-mode");
    param->dec_fmtp.param[1].val	= pj_str((char*)"1");
    // Bitrate
    param->enc_fmt.det.vid.avg_bps	= DEFAULT_AVG_BITRATE;
    param->enc_fmt.det.vid.max_bps	= DEFAULT_MAX_BITRATE;
    // Encoding MTU */
    param->enc_mtu		= PJMEDIA_MAX_VID_PAYLOAD_SIZE;
    return PJ_SUCCESS;
}

static pj_status_t vh264_enum_info(pjmedia_vid_codec_factory *factory,
                                   unsigned *count,
                                   pjmedia_vid_codec_info info[])
{
    PJ_ASSERT_RETURN(info && *count > 0, PJ_EINVAL);
    PJ_ASSERT_RETURN(factory == &vh264_factory.base, PJ_EINVAL);

    *count = 1;
    info->fmt_id			= PJMEDIA_FORMAT_H264;
    info->pt				= PJMEDIA_RTP_PT_H264;
    info->encoding_name		= pj_str((char *)"H264");
    info->encoding_desc		= pj_str((char *)"H264 codec");
    info->clock_rate		= 90000;
    info->dir				= PJMEDIA_DIR_ENCODING_DECODING;
    info->dec_fmt_id_cnt	= 1;
    info->dec_fmt_id[0]		= PJMEDIA_FORMAT_I420;
    info->packings			= PJMEDIA_VID_PACKING_PACKETS | PJMEDIA_VID_PACKING_WHOLE;
    info->fps_cnt			= 3;
    info->fps[0].num =		15;
    info->fps[0].denum =	1;
    info->fps[1].num =		25;
    info->fps[1].denum =	1;
    info->fps[2].num =		30;
    info->fps[2].denum =	1;
    return PJ_SUCCESS;
}

static pjmedia_vid_codec	_vid_code;
static vh264_codec_data		_codec_data;	

static pj_status_t vh264_alloc_codec(pjmedia_vid_codec_factory *factory,
                                     const pjmedia_vid_codec_info *info,
                                     pjmedia_vid_codec **p_codec)
{
    pjmedia_vid_codec *codec;
    int rc;
    int log_level = PJMEDIA_CODEC_OPENH264_LOG_LEVEL;

    PJ_ASSERT_RETURN(factory == &vh264_factory.base && info && p_codec,
                     PJ_EINVAL);
    *p_codec = NULL;
    codec = &_vid_codec;
    codec->factory = factory;
    codec->op = &vh264_codec_op;
    codec->codec_data = NULL;
	*p_codec = codec;
    return PJ_SUCCESS;
}

static pj_status_t vh264_dealloc_codec(pjmedia_vid_codec_factory *factory,
                                       pjmedia_vid_codec *codec)
{
    vh264_codec_data	*vh264_data;

    PJ_ASSERT_RETURN(codec, PJ_EINVAL);
    PJ_UNUSED_ARG(factory);
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_init(pjmedia_vid_codec *codec, pj_pool_t *pool)
{
    PJ_ASSERT_RETURN(codec, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_open(pjmedia_vid_codec *codec,
                                    pjmedia_vid_codec_param *codec_param)
{
    pjmedia_vid_codec_param		*param;
    pjmedia_h264_packetizer_cfg  pktz_cfg;
    pjmedia_vid_codec_h264_fmtp  h264_fmtp;
    pj_status_t	status;
    int		rc;

    PJ_ASSERT_RETURN(codec && codec_param, PJ_EINVAL);
    PJ_LOG(5,(THIS_FILE, "Opening codec.."));
    param = codec_param;
    // Parse remote fmtp */
    pj_bzero(&h264_fmtp, sizeof(h264_fmtp));
    status = pjmedia_vid_codec_h264_parse_fmtp(&param->enc_fmtp, &h264_fmtp);
    if(status != PJ_SUCCESS) return status;
    // Apply SDP fmtp to format in codec param
    if(!param->ignore_fmtp) {
		status = pjmedia_vid_codec_h264_apply_fmtp(param);
		if(status != PJ_SUCCESS) return status;
    }
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_close(pjmedia_vid_codec *codec)
{
    PJ_ASSERT_RETURN(codec, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_modify(pjmedia_vid_codec *codec,
                                      const pjmedia_vid_codec_param *param)
{
    PJ_ASSERT_RETURN(codec && param, PJ_EINVAL);
    PJ_UNUSED_ARG(codec);
    PJ_UNUSED_ARG(param);
    return PJ_EINVALIDOP;
}

static pj_status_t vh264_codec_get_param(pjmedia_vid_codec *codec,
                                         pjmedia_vid_codec_param *param)
{
    struct vh264_codec_data *vh264_data;

    PJ_ASSERT_RETURN(codec && param, PJ_EINVAL);
    vh264_data = (vh264_codec_data *)codec->codec_data;
    pj_memcpy(param, vh264_data->prm, sizeof(*param));
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_encode_begin(pjmedia_vid_codec *codec,
                                            const pjmedia_vid_encode_opt *opt,
                                            const pjmedia_frame *input,
                                            unsigned out_size,
                                            pjmedia_frame *output,
                                            pj_bool_t *has_more)
{
	output->type = PJMEDIA_FRAME_TYPE_VIDEO;
	output->size = payload_size;
	output->timestamp = input->timestamp;
	pj_memcpy(output->buf, payload, payload_size);
	return PJ_SUCCESS;
    return vh264_codec_encode_more(codec, out_size, output, has_more);
}


static pj_status_t vh264_codec_encode_more(pjmedia_vid_codec *codec,
                                           unsigned out_size,
                                           pjmedia_frame *output,
                                           pj_bool_t *has_more)
{
    return PJ_SUCCESS;
}

static pj_status_t vh264_codec_decode(pjmedia_vid_codec *codec,
                                      pj_size_t count,
                                      pjmedia_frame packets[],
                                      unsigned out_size,
                                      pjmedia_frame *output)
{
    return PJ_SUCCESS;
}

#endif

