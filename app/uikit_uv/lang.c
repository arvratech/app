#include "msg.h"
#include "syscfg.h"
#include "ctfont.h"
#include "lang.h"

static int	_Language;


void langOpen(void)
{
	int		rval, lang;
	
	lang = sys_cfg->language;
printf("lang = %d %d\n", lang, LANG_KOR);
#ifdef _ITA
	if(lang != LANG_ITA) lang = LANG_ENG;
#else
#ifdef _JPN
	if(lang == LANG_KOR) lang = LANG_JPN;
	else if(lang == LANG_ITA) lang = LANG_ENG;
#else
	if(lang == LANG_JPN) lang = LANG_KOR;
	else if(lang == LANG_ITA) lang = LANG_ENG;
#endif
#endif
#ifdef _A20
#ifdef _FREETYPE_
	ftInit("font/NanumBarunGothic.ttf");
#else
	ftInit("font");
#endif
#else
#ifdef _FREETYPE_
	ftInit("/mnt/nand1-2/font/NanumBarunGothic.ttf");
#else
	ftInit("/mnt/nand1-2/font");
#endif
#endif
printf("############## lang = %d\n", lang);
	SetLanguage(lang);
	msgInitialize();
	if(lang != sys_cfg->language) {
		sys_cfg->language = lang;
		//syscfgSetByLanguage(sys_cfg);
		rval = syscfgWrite(sys_cfg);
	}
}

void langClose(void)
{
	ftExit();
}

void SetLanguage(int lang)
{
	_Language = lang;
}

int GetLanguage(void)
{
	return _Language;
}

int GetBidi(void)
{
	if(_Language == LANG_ARA) return 1;
	else	return 0;
}

void ChangeLanguage(int lang)
{
	int		rval;

	SetLanguage(lang);
	msgInitialize();
	sys_cfg->language = lang;
	//syscfgSetByLanguage(sys_cfg);
	rval = syscfgWrite(sys_cfg);
}

