#ifndef _TA_H_
#define _TA_H_


int	 taGetItemSize(void);
int	 taGetTextSize(void);
int  taGetIconSize(void);
void taSetItemsDefault(void);
int  taEncodeItems(SYS_CFG *sys_cfg, unsigned char *data);
int  taDecodeItems(SYS_CFG *sys_cfg, unsigned char *data, int nCount);
int  taEncodeTexts(SYS_CFG *sys_cfg, unsigned char *data);
int  taDecodeTexts(SYS_CFG *sys_cfg, unsigned char *data, int nCount);
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
