#ifndef _CAM_CFG_H_
#define _CAM_CFG_H_


#if !defined(_EXTERN_C)
#ifdef __cplusplus
#define _EXTERN_C	extern "C"
#else
#define _EXTERN_C
#endif
#endif


#define EVENT_WITH_CAM_IMAGE		0x01


typedef struct _CAM_CFG {
	unsigned char	Mode;
	unsigned char	RunState;
	unsigned char	Channel;
	unsigned char	Option;
	unsigned char	AccessPoints[4];
} CAM_CFG;


_EXTERN_C void camSetDefault(CAM_CFG *cam_cfg);
_EXTERN_C int  camEncode(CAM_CFG *cam_cfg, void *buf);
_EXTERN_C int  camDecode(CAM_CFG *cam_cfg, void *buf);
_EXTERN_C int  camValidate(CAM_CFG *cam_cfg);


#endif	/* _CAM_CFG_H_ */