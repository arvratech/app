#include "defs.h"
#include "gfont.h"
#include "msg.h"
#include "syscfg.h"
#include "ta.h"
#include "lang.h"

void syscfgSetByLanguage(SYS_CFG *sys_cfg);


void InitializeLanguage(void)
{
	int		rval, lang;
	
	lang = sys_cfg->Device.Language;
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
	InitFont();
	SetLanguage(lang);
	msgInitialize();
	if(lang != sys_cfg->Device.Language) {
		sys_cfg->Device.Language = lang;
		syscfgSetByLanguage(sys_cfg);
		rval = syscfgWrite(sys_cfg);
	}
}

void ChangeLanguage(int lang)
{
	int		rval;

	SetLanguage(lang);
	msgInitialize();
	sys_cfg->Device.Language = lang;
	syscfgSetByLanguage(sys_cfg);
	rval = syscfgWrite(sys_cfg);
}