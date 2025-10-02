#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h> 
#include "dirent.h"
#include "defs.h"
#include "prim.h"
#include "msg.h"
#include "rtc.h"
#include "lang.h"
#include "sysdep.h"
#include "syscfg.h"
#include "tacfg.h"

TA_CFG	*ta_cfg, _ta_cfg;

static char tacfgFileName[16] = "tacfg.dat";


int tacfgInitialize(void)
{	

	ta_cfg = &_ta_cfg;
	tacfgSetDefault(ta_cfg);
	tacfgRead(ta_cfg);
	return 0;
}

void syscfgSetDefaultByLanguage(SYS_CFG *sys_cfg);

void tacfgSetDefault(TA_CFG *cfg)
{
	unsigned char	temp[4];
	int		i, j;
	
	taSetOptionEx(2, 1);	// taSetOption(Keyoption=2(Fixed 1), DisplayOption=1(Text))
	cfg->funcKeyTimer = 0;
	taSetItemsDefault();
#ifdef _TA_EXT
	memset(cfg->TAResetTime, 0, 6);
#endif
	syscfgSetDefaultByLanguage(sys_cfg);
}

FILE *_OpenSIB(char *fileName);
void _DeleteSIB(char *fileName);

int tacfgEncode(TA_CFG *cfg, void *buf)
{
	unsigned char	*p;
	int		i;

	p = (unsigned char *)buf;
	*p++ = cfg->TAOption;
	*p++ = cfg->funcKeyTimer;
	p += taEncodeItems(cfg, p);	
	p += taEncodeTexts(cfg, p);
#ifdef _TA_EXT
	memcpy(p, cfg->TAResetTime, 6); p += 6;
#endif
 	return p - (unsigned char *)buf;
}

int tacfgDecode(TA_CFG *cfg, void *buf, int size)
{
	unsigned char	*p;
	int		c, i;

	p = (unsigned char *)buf;
	c = (size - 2) / 22;
	cfg->TAOption = *p++;
	cfg->funcKeyTimer = *p++;
	p += taDecodeItems(cfg, p, c);
	p += taDecodeTexts(cfg, p, c);
#ifdef _TA_EXT
	if(c == taGetItemSize()) {
		memcpy(cfg->TAResetTime, p, 6); p += 6;
	}
#endif
	if(c != taGetItemSize()) {
		_DeleteSIB(tacfgFileName);
		tacfgWrite(cfg);
	}
//	taSetItemEnable(0, 1);	// taSetItemEnable(ItemNo, Enable)
 	return p - (unsigned char *)buf;
}

int tacfgCompare(TA_CFG *cfg, TA_CFG *tcfg)
{
	unsigned char	buf[2048], *s1, *s2;
	int		size;
	
	s1 = buf; s2 = buf + 1024;
	tacfgEncode(cfg, s1);
	size = tacfgEncode(tcfg, s2);
	if(!memcmp(s1, s2, size)) return 0;
	else	return -1;
}

void tacfgCopy(TA_CFG *cfg, TA_CFG *tcfg)
{
	memcpy(cfg, tcfg, sizeof(TA_CFG));
}

int tacfgRead(TA_CFG *cfg)
{
	FILE		*fp;
	unsigned char	buf[2200];
	int		i, rval, size[6];

	fp = _OpenSIB(tacfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fread(buf, 1, 2200, fp);
//printf("fsRead(%d)=%d fsErrno=%d\n", (int)f->file, rval, fsErrno);
	if(rval > 200) {
		rval = tacfgDecode(cfg, buf, rval);
printf("tacfgDecode=%d\n", rval);
	}
	fclose(fp);
	return rval;
}

int tacfgWrite(TA_CFG *cfg)
{
	FILE		*fp;
	unsigned char	buf[1200];
	int		i, rval, size;

	size = tacfgEncode(cfg, buf);
printf("tacfgEncode=%d\n", size);
	fp = _OpenSIB(tacfgFileName);
	fseek(fp, 0L, SEEK_SET);
	rval = fwrite(buf, 1, size, fp);
//printf("fsWrite(%d)=%d\n",  size, rval);
	if(rval != size) {
#ifdef BK_DEBUG
		printf("fsWrite error\n");
#endif
		rval = -1;
	} else	rval = 0;
	fclose(fp);
	sync();
	return rval;
}

