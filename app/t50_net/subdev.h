#ifndef _SUBDEV_H_
#define _SUBDEV_H_

#include "cr.h"
#include "netsvc.h"

// Device Model definitions
#define DEV_MODEL_WIEGAND				0
#define DEV_MODEL_STANDALONE_WITHOUT_IO	1
#define DEV_MODEL_STANDALONE_WITH_IO	2

typedef struct _SUBDEVICE_CFG {
	unsigned char	model;
	char			venderName[17];
	char			modelName[17];
	unsigned char	firmwareVersion[3];
	unsigned char	language;
	unsigned char	deviceOption;
	unsigned char	reserve[4];
} SUBDEVICE_CFG;

typedef struct _SUBDEVICE {
	unsigned char	cls;
	unsigned char	id;
	unsigned char	reliability;
	unsigned char	reserve[1];
	unsigned char	changeOfStateTime[6];
	unsigned char	alarmStateTime[6];
	SUBDEVICE_CFG	*cfg;
	NET_SVC			confSvc;
	unsigned char	confBuf[2048];
} SUBDEVICE;


void subdevsInit(void);
void *subdevsGet(int id);
int  subdevsMaxSize(void);

void _SubdevSetDefault(SUBDEVICE_CFG *cfg);
int  _SubdevEncode(SUBDEVICE_CFG *cfg, void *buf);
int  _SubdevDecode(SUBDEVICE_CFG *cfg, void *buf);
int  _SubdevValidate(SUBDEVICE_CFG *cfg);

int  subdevId(void *self);
BOOL subdevIsNonNull(void *self);
int  subdevModel(void *self);
BOOL subdevHaveIo(void *self);
void subdevCreate(void *self, int model);
void subdevDelete(void *self);
int  subdevReliability(void *self);
int  subdevGetAvailableModels(void *self, int *models);
NET_SVC *subdevConfSvc(void *self);


#endif

