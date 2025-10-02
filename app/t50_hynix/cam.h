#ifndef _CAM_H_
#define _CAM_H_


#define EVENT_WITH_CAM_IMAGE		0x01


typedef struct _CAM_CFG {
	unsigned char	mode;
	unsigned char	runState;
	unsigned char	channel;
	unsigned char	option;
	unsigned char	frameRate;
	unsigned char	reserve[3];
} CAM_CFG;


void camSetDefault(CAM_CFG *cam_cfg);
int  camEncode(CAM_CFG *cam_cfg, void *buf);
int  camDecode(CAM_CFG *cam_cfg, void *buf);
int  camValidate(CAM_CFG *cam_cfg);
int  camEncode2(CAM_CFG *cam_cfg, void *buf);
int  camDecode2(CAM_CFG *cam_cfg, void *buf);


int  camMode(void *self);
void camSetMode(void *self, int mode);
int  camFrameRate(void *self);
void camSetFrameRate(void *self, int frameRate);
BOOL camEventWithCamImage(void *self);
void camSetEventWithCamImage(void *self, BOOL enable);

int  camRun(void *loop);
void camStop(void);
int  camCodecState(void);
int  camPreviewState(void);
int  camRunPreview(int width, int height);
int  camStopPreview(void);
void camRunFrameRate(int frameRate);

void camjpgOpenServer(void);
void camjpgCloseServer(void);
unsigned char *camjpgBuffer(int *bufLength);


#endif

