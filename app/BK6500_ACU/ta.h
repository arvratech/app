#ifndef _TA_H_
#define _TA_H_


int  taCalculateState(void);
int	 taGetItemSize(void);
int	 taGetTextSize(void);
void taSetItemsDefault(void);
int  taEncodeItems(SYS_CFG *sys_cfg, unsigned char *data);
int  taDecodeItems(SYS_CFG *sys_cfg, unsigned char *data);
int  taEncodeTexts(SYS_CFG *sys_cfg, unsigned char *data);
int  taDecodeTexts(SYS_CFG *sys_cfg, unsigned char *data);
int  taCompareTexts(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg);
int  taGetOption(void);
void taSetOption(int Option);
void taSetOptionEx(int KeyOption, int DisplayOption);
int  taGetKeyOption(void);
void taSetKeyOption(int Option);
int  taGetDisplayOption(void);
void taSetDisplayOption(int Option);
int  taGetItem(int ItemNo);
void taSetItem(int ItemNo, int Value);
void taSetItemEx(int ItemNo, int Enable, int Index);
int  taItemIsEnable(int ItemNo);
void taSetItemEnable(int ItemNo, int Enable);
int  taGetItemIndex(int ItemNo);
void taSetItemIndex(int ItemNo, int Index);
char *taGetTextAt(int Index);
int  taAdjustItem(int ItemNo);
int  taNextItem(int ItemNo);
int  taPreviousItem(int ItemNo);


#endif	/* _TA_H_ */