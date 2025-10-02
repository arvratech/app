#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "sche.h"
#include "rtc.h"
#include "syscfg.h"
#include "tacfg.h"
#include "ta.h"


int taCalculateState(void)
{
	unsigned char	ctm[8];
	int		i, state;

	rtcGetTime(ctm);
	for(i = 0;i < 2;i++)
		if(taItemIsEnable(i+1) && tmsValidateEx(sys_cfg->InOutTimeSection[i]) && tmsBoundEx(sys_cfg->InOutTimeSection[i], ctm)) {
			state = i + 1;		// 1:Ãâ±Ù 2:Åð±Ù
			break;
		}
	if(i >= 2) state = 0;
	return state;
}

int	taGetItemSize(void)
{
	return TA_ITEM_SIZE;
}

int	taGetTextSize(void)
{
	return TA_TEXT_SIZE;
}

void taSetItemsDefault(void)
{
	int		i, size, enable, index, text_size;
	
	// TAItems[i] = TA_ITEM_ENABLE + TA_ITEM_INDEX
	// No,F1,F2....F8
	size = taGetItemSize();
	text_size = taGetTextSize();
	for(i = index = 0;i < size;i++) {
		if(i < 3) enable = 1; else enable = 0;
		taSetItemEx(i, enable, index);	// taSetItemEx(ItemNo, Enable, index)
		index++; if(index >= text_size) index = 1;
	}
}

int taEncodeItems(SYS_CFG *sys_cfg, unsigned char *data)
{
	int		size;

	size = taGetItemSize();
	memcpy(data, sys_cfg->TAItems, size);
	return size;
}

int taDecodeItems(SYS_CFG *sys_cfg, unsigned char *data)
{
	int		size;

	size = taGetItemSize();
	memcpy(sys_cfg->TAItems, data, size);
	return size;
}

int taEncodeTexts(SYS_CFG *sys_cfg, unsigned char *data)
{
	unsigned char	*p;
	int		i, size;

	size = taGetTextSize();
	p = data;
	for(i = 0;i < size;i++) {
		memcpy_pad(p, sys_cfg->TATexts[i], 20); p += 20;
	}
	return p - data;
}

int taDecodeTexts(SYS_CFG *sys_cfg, unsigned char *data)
{
	unsigned char	*p;
	int		i, size;

	size = taGetTextSize();
	p = data;	
	for(i = 0;i < size;i++) {
		memcpy_chop(sys_cfg->TATexts[i], p, 20); p += 20;
	}
	return p - data;
}

int taCompareTexts(SYS_CFG *sys_cfg, SYS_CFG *tmp_cfg)
{
	int		i, size;

	size = taGetTextSize();
	for(i = 0;i < size;i++) 
		if(strcmp(sys_cfg->TATexts[i], tmp_cfg->TATexts[i])) break;
	if(i < size) i = 1;
	else	i = 0;
	return i;
}

#define TA_KEY			0x03
#define TA_DISPLAY		0x0c
#define TA_ITEM_INDEX	0x7f
#define TA_ITEM_ENABLE	0x80

// 0000DDKK: DD=Display Option KK=Key Option
int taGetOption(void)
{
	return (int)sys_cfg->TAOption;
}

void taSetOption(int Option)
{
	sys_cfg->TAOption = (unsigned char)Option;
}

void taSetOptionEx(int KeyOption, int DisplayOption)
{
	sys_cfg->TAOption  = (unsigned char)((DisplayOption << 2) + KeyOption);
}

// 1:Up/Down 2:Fixed1 3:Fixed2
int taGetKeyOption(void)
{
	return (int)(sys_cfg->TAOption & TA_KEY);
}

void taSetKeyOption(int Option)
{
	sys_cfg->TAOption = (unsigned char)((sys_cfg->TAOption & ~TA_KEY) | Option);
}

// 1:Disable 2:Text 3:Icon
int taGetDisplayOption(void)
{
	return (int)((sys_cfg->TAOption & TA_DISPLAY) >> 2);
}

void taSetDisplayOption(int Option)
{
	sys_cfg->TAOption = (unsigned char)((sys_cfg->TAOption & ~TA_DISPLAY) | (Option << 2));
}

int taGetItem(int ItemNo)
{
	return (int)sys_cfg->TAItems[ItemNo];
}

void taSetItem(int ItemNo, int Value)
{
	sys_cfg->TAItems[ItemNo] = (unsigned char)Value;
}

void taSetItemEx(int ItemNo, int Enable, int Index)
{
	int		c;

	c = Index;
	if(Enable) c |= TA_ITEM_ENABLE;
	sys_cfg->TAItems[ItemNo] = (unsigned char)c;
}

int taItemIsEnable(int ItemNo)
{
	if(sys_cfg->TAItems[ItemNo] & TA_ITEM_ENABLE) return 1;
	else	return 0;
}

void taSetItemEnable(int ItemNo, int Enable)
{
	if(Enable) sys_cfg->TAItems[ItemNo] |= TA_ITEM_ENABLE;
	else  sys_cfg->TAItems[ItemNo] &= ~TA_ITEM_ENABLE;
}

int taGetItemIndex(int ItemNo)
{
	return sys_cfg->TAItems[ItemNo] & TA_ITEM_INDEX;
}

void taSetItemIndex(int ItemNo, int Index)
{
	int		c;

	c = sys_cfg->TAItems[ItemNo] & ~TA_ITEM_INDEX;
	sys_cfg->TAItems[ItemNo] = (unsigned char)(c | Index);
}

char *taGetTextAt(int Index)
{
	char	*p;
	int		size;

	size = taGetTextSize();
	if(Index >= 0 && Index < size) p = sys_cfg->TATexts[Index];
	else	p = NULL;
	return p;
}

int taAdjustItem(int ItemNo)
{
	int		i, size;

	size = taGetItemSize();
	if(ItemNo < 0) ItemNo = 0;
	i = ItemNo;
	do {
		if(taItemIsEnable(i)) return i;
		i++;
		if(i >= size) i = 0;
	} while(i != ItemNo) ;
	return -1;
}

int taNextItem(int ItemNo)
{
	int		i, size;

	size = taGetItemSize();
	if(ItemNo <= 0) ItemNo = size-1;	// 2010/05/28
	i = ItemNo;
	do {
		i++;
		if(i >= size) i = 1;			// 2010/05/28
		if(taItemIsEnable(i)) return i;
	} while(i != ItemNo) ;
	return -1;
}

int taPreviousItem(int ItemNo)
{
	int		i, size;

	size = taGetItemSize();
	if(ItemNo <= 0) ItemNo = 1;			// 2010/05/28
	i = ItemNo;
	do {
		i--;
		if(i < 1) i = size - 1;			// 2010/05/28
		if(taItemIsEnable(i)) return i;
	} while(i != ItemNo) ;
	return -1;
}
