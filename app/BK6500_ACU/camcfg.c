#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "camcfg.h"


void camSetDefault(CAM_CFG *cam_cfg)
{
	int		i;

	cam_cfg->Mode		= 0;	// 0:Disable 1:Automatic 2:Manual
	cam_cfg->RunState	= 0;	// 0:Off 1:On
	cam_cfg->Channel	= 0;
	cam_cfg->Option		= 0x00;
	for(i = 0;i < 4;i++) cam_cfg->AccessPoints[i] = 0xff;
}

int camEncode(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cam_cfg->Mode;
	*p++ = cam_cfg->RunState;
	*p++ = cam_cfg->Channel;
	*p++ = cam_cfg->Option;
	for(i = 0;i < 4;i++) *p++ = cam_cfg->AccessPoints[i];
 	return p - (unsigned char *)buf;
}

int camDecode(CAM_CFG *cam_cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	cam_cfg->Mode		= *p++;
	cam_cfg->RunState	= *p++;
	cam_cfg->Channel	= *p++;
	cam_cfg->Option		= *p++;
	for(i = 0;i < 4;i++) cam_cfg->AccessPoints[i] = *p++;
 	return p - (unsigned char *)buf;
}

int camValidate(CAM_CFG *cam_cfg)
{
	if(cam_cfg->Mode > 2 || cam_cfg->Channel > 3) return 0; 	
	return 1;
}

