#ifndef _LANG_H_
#define _LANG_H_


// Language definitions
#define LANG_ENG		1
#define LANG_CHN		2
#define LANG_JPN		3
#define LANG_KOR		4
#define LANG_ITA		5
#define LANG_TUR		6
#define LANG_THA		7
#define LANG_ARA		8
#define LANG_TWN		9

void langOpen(void);
void langClose(void);
void ChangeLanguage(int lang);
void SetLanguage(int lang);
int  GetLanguage(void);
int  GetBidi(void);


#endif

