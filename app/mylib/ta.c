#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "prim.h"
#include "sche.h"
#include "syscfg.h"
#include "tacfg.h"
#include "ta.h"


int	taGetItemSize(void)
{
	return TA_ITEM_SIZE;
}

int	taGetTextSize(void)
{
	return TA_TEXT_SIZE;
}

int taGetIconSize(void)
{
	return TA_ICON_SIZE;
}

void taSetItemsDefault(void)
{
	int		i, size, text, icon, text_size, icon_size;
	
	// TAItems[i] = TA_ITEM_ENABLE + TA_ITEM_ICON + TA_ITEM_TEXT
	// No,F1,F2....F8
	size = taGetItemSize();
	text_size = taGetTextSize();
	icon_size = taGetIconSize();
	text = icon = 0;
	for(i = 0;i < size;i++) {	
		taSetItemEx(i, 0, text, icon);	// taSetItemEx(ItemNo, Enable, TextIndex, IconIndex)
		text++; if(text >= text_size) text = 1;
		icon++; if(icon >= icon_size) icon = 1;
	}
	taSetItemEnable(0, 1);	// taSetItemEnable(ItemNo, Enable)	2010/05/28
	taSetItemEnable(1, 1);	// taSetItemEnable(ItemNo, Enable)
	taSetItemEnable(2, 1);	// taSetItemEnable(ItemNo, Enable)
#ifdef BK4300
	for(i = 0;i < 4;i++) taSetItemEnable(i+3, 1);	// taSetItemEnable(i, Enable)
#endif
}

int taEncodeItems(SYS_CFG *sys_cfg, unsigned char *data)
{
	int		n;

	n = taGetItemSize();
	n <<= 1;
	memcpy(data, sys_cfg->TAItems, n);
	return n;
}

int taDecodeItems(SYS_CFG *sys_cfg, unsigned char *data, int nCount)
{
	int		count;

	nCount <<= 1;
	count = taGetItemSize();
	count <<= 1;
	if(count > nCount) count = nCount; 
	memcpy(sys_cfg->TAItems, data, count);
	return nCount;
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

int taDecodeTexts(SYS_CFG *sys_cfg, unsigned char *data, int nCount)
{
	unsigned char	*p;
	int		i, count;

	count = taGetTextSize();
	p = data;	
	for(i = 0;i < nCount;i++) {
		if(i < count) memcpy_chop(sys_cfg->TATexts[i], p, 20);
		p += 20;
	}
	return p - data;
}

#define TA_KEY			0x03
#define TA_DISPLAY		0x0c
#define TA_ITEM_TEXT	0x007f
#define TA_ITEM_ICON	0x3f80
#define TA_ITEM_ENABLE	0x4000

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
	sys_cfg->TAItems[ItemNo] = (unsigned short)Value;
}

void taSetItemEx(int ItemNo, int Enable, int TextIndex, int IconIndex)
{
	int		c;
	
	c = (IconIndex << 7)	+ TextIndex;
	if(Enable) c |= TA_ITEM_ENABLE;
	sys_cfg->TAItems[ItemNo] = (unsigned short)c;
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

int taGetItemTextIndex(int ItemNo)
{
	return sys_cfg->TAItems[ItemNo] & TA_ITEM_TEXT;
}

void taSetItemTextIndex(int ItemNo, int Index)
{
	int		c;

	c = sys_cfg->TAItems[ItemNo];
	sys_cfg->TAItems[ItemNo] = (unsigned short)((c & ~TA_ITEM_TEXT) | Index);
}

int taGetItemIconIndex(int ItemNo)
{
	return (sys_cfg->TAItems[ItemNo] & TA_ITEM_ICON) >> 7;
}

void taSetItemIconIndex(int ItemNo, int Index)
{
	int		c;

	c = sys_cfg->TAItems[ItemNo];	
	sys_cfg->TAItems[ItemNo] = (unsigned short)((c & ~TA_ITEM_ICON) | (Index << 7));
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
