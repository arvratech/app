#ifndef _TA_CFG_H_
#define _TA_CFG_H_


// TA(Time & Attendance) items size
#define TA_ITEM_SIZE		9		// 9, 30, 99
#define TA_TEXT_SIZE		9		// 9, 30, 99
#define TA_ICON_SIZE		6


typedef struct _TA_CFG {
	unsigned char	TAOption;
	unsigned char	funcKeyTimer;
	unsigned short	TAItems[TA_ITEM_SIZE];
	char			TATexts[TA_TEXT_SIZE][24];
#ifdef _TA_EXT
	unsigned char	TAResetTime[8];
#endif
} TA_CFG;


extern TA_CFG	*ta_cfg;


int  tacfgInitialize(void);
void tacfgSetDefault(TA_CFG *cfg);
int  tacfgEncode(TA_CFG *cfg, void *buf);
int  tacfgDecode(TA_CFG *cfg, void *buf, int size);
int  tacfgCompare(TA_CFG *cfg, TA_CFG *tcfg);
void tacfgCopy(TA_CFG *cfg, TA_CFG *tcfg);
int  tacfgRead(TA_CFG *cfg);
int  tacfgWrite(TA_CFG *cfg);

int	 taGetItemSize(void);
int	 taGetTextSize(void);
int  taGetIconSize(void);
void taSetItemsDefault(void);
int  taEncodeItems(TA_CFG *cfg, unsigned char *data);
int  taDecodeItems(TA_CFG *cfg, unsigned char *data, int nCount);
int  taEncodeTexts(TA_CFG *cfg, unsigned char *data);
int  taDecodeTexts(TA_CFG *cfg, unsigned char *data, int nCount);
int  taGetOption(void);
void taSetOption(int Option);
void taSetOptionEx(int KeyOption, int DisplayOption);
int  taGetKeyOption(void);
void taSetKeyOption(int Option);
int  taGetDisplayOption(void);
void taSetDisplayOption(int Option);
int  taGetItem(int ItemNo);
void taSetItem(int ItemNo, int Value);
void taSetItemEx(int ItemNo, int Enable, int TextIndex, int IconIndex);
int  taItemIsEnable(int ItemNo);
void taSetItemEnable(int ItemNo, int Enable);
int  taGetItemTextIndex(int ItemNo);
void taSetItemTextIndex(int ItemNo, int Index);
int  taGetItemIconIndex(int ItemNo);
void taSetItemIconIndex(int ItemNo, int Index);
char *taGetTextAt(int Index);
int  taAdjustItem(int ItemNo);
int  taNextItem(int ItemNo);
int  taPreviousItem(int ItemNo);


#endif

