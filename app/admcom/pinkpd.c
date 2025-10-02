#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include "dirent.h"
#include "defs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "sche.h"
#include "msg.h"
#include "hw.h"
#include "cf.h"
#include "authwg.h"
#include "viewprim.h"
#include "pinkpd.h"


static void _PinKeypadIgnoreChar(PIN_KEYPAD_CONTEXT *kpd)
{
	UItimerStop(0);
	if(kpd->burst) UIsetText(kpd->lbl, NULL);
	else	LabelSetHiddenText(kpd->lbl, kpd->text);
}

static void _PinKeypadBurstChar(PIN_KEYPAD_CONTEXT *kpd, int c)
{
	char	temp[4];

	temp[0] = c; temp[1] = 0;
	UIsetText(kpd->lbl, temp);
	if(kpd->burstCallback) (*kpd->burstCallback)(c);
}

void pinKeypadOpen(PIN_KEYPAD_CONTEXT *kpd)
{
	void	*pf;
	int		val;

	pf = cfPinGet();
	val = cfPinOutputWiegand(pf);
	if(val < 2) kpd->burst = 1; else kpd->burst = 0;
	kpd->burstCallback = PinCharWiegand;
	kpd->entryStopSentinel = cfPinEntryStopSentinel(pf);
	kpd->entryInterval = cfPinEntryInterval(pf);
	kpd->maximumLength = cfPinMaximumLength(pf);
	kpd->text[0] = 0;
	kpd->state = 1;	
}

void  pinKeypadClose(PIN_KEYPAD_CONTEXT *kpd)
{
	kpd->state = 0;	
}

int pinKeypadOpened(PIN_KEYPAD_CONTEXT *kpd)
{
	return (int)kpd->state;
}

int pinKeypadPressed(PIN_KEYPAD_CONTEXT *kpd)
{
	int		c, len, rval;

	c = KeypadNumericCode(kpd->view);
	len = strlen(kpd->text);
	rval = 0;
	switch(c) {
	case '*':
		if(kpd->burst) {
			kpd->text[0] = 0;
			_PinKeypadBurstChar(kpd, c);
			UItimerStart(0, 1000, 0);
			UItimerStart(1, kpd->entryInterval*1000, 0);
		} else if(len > 0) {
			len--; kpd->text[len] = 0;
			LabelSetHiddenText(kpd->lbl, kpd->text);
			UItimerStop(0);
			UItimerStart(1, kpd->entryInterval*1000, 0);
		} else {
			_PinKeypadIgnoreChar(kpd);
		}
		break;
	case '#':
		if(kpd->entryStopSentinel) {
			if(kpd->burst) {
				_PinKeypadBurstChar(kpd, c);
				UItimerStart(0, 1000, 0);
			} else {
				LabelSetHiddenText(kpd->lbl, kpd->text);
				UItimerStop(0);
			}
			UItimerStop(1);
			rval = 1;
		} else {
			_PinKeypadIgnoreChar(kpd);
		}
		break;
	default:
		if(len < kpd->maximumLength && c >= '0' && c <= '9') {
			kpd->text[len++] = c; kpd->text[len] = 0;
			if(kpd->burst) _PinKeypadBurstChar(kpd, c);
			else	LabelSetHiddenTextExceptLast(kpd->lbl, kpd->text);
			UItimerStart(0, 1000, 0);
			if(!kpd->entryStopSentinel && len >= kpd->maximumLength) {
				UItimerStop(1);
				rval = 1;
			} else {
				UItimerStart(1, kpd->entryInterval*1000, 0);
			}
		} else {
			_PinKeypadIgnoreChar(kpd);
		}
	}
	return rval;
}

