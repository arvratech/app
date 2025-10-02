#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defs.h"
#include "prim.h"
#include "rtc.h"
#include "lcdc.h"
#include "lang.h"
#include "msg.h"
#include "sysdep.h"
#include "syscfg.h"
#include "gactivity.h"
#include "gapp.h"
#include "viewprim.h"
#include "dlg.h"
#include "topprim.h"
#include "admprim.h"
#include "actprim.h"
#include "av.h"
#include "pjsualib.h"
#include "pjsua_app.h"
#include "appact.h"

//extern AV_VIDEO_SINK	*videoSink;

void PjsCallOnCreate(GACTIVITY *act);
void PjsCallOnDestroy(GACTIVITY *act);
void PjsCallOnTimer(GACTIVITY *act, int timerID);
void PjsCallOnClick(GACTIVITY *act, void *view);
void PjsCallOnAppMessage(GACTIVITY *act, unsigned char *msg);

static int	pjsCallState, timerCount;


void PjsCallStart(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(PjsCallOnCreate);
	appStartActivityForResult(act, requestCode, intent, 0);
	appMuteSounds();
}

void PjsCallRestart(int requestCode, char *intent)
{
	GACTIVITY	*act;

	act = CreateActivity(PjsCallOnCreate);
	appStartActivityForResult(act, requestCode, intent, 1);
}

void GetCallStateMsg(int callState, char *msg)
{
	char	*p;
	int		val;

	p = msg;
	if(callState ==	PJSIP_INV_STATE_CONFIRMED) {
		sprintf(p, "%02d:%02d", timerCount/60, timerCount%60);
	} else if(callState == PJSIP_INV_STATE_DISCONNECTED) {
		val = pjsuaCallDuration();
		sprintf(p, "%s %02d:%02d", xmenu_call_state[2], val/60, val%60);
	} else {
		if(callState ==	PJSIP_INV_STATE_EARLY) strcpy(p, xmenu_call_state[1]);
		else	strcpy(p, xmenu_call_state[0]);
		p += strlen(p);
		val = (timerCount % 3) + 1;
		while(val--) *p++ = '.';
		*p = 0;

	}
}

void PjsCallOnCreate(GACTIVITY *act)
{
	void	*wnd, *v;
	CGRect	rt;
	char	*p, temp[128], name[64], url[128];
	int		x, y, w, h;

	activitySetOnDestroy(act, PjsCallOnDestroy);
	activitySetOnTimer(act, PjsCallOnTimer);
	activitySetOnClick(act, PjsCallOnClick);
	activitySetOnAppMessage(act, PjsCallOnAppMessage);
	activitySetViewBuffer(act, ViewAllocBuffer());
	wnd = activityWindow(act);
	UIsetOpaque(wnd, TRUE);
	UIsetBackgroundColor(wnd, 0xff2f313e);
	UIframe(wnd, &rt);
	p = activityIntent(act);
	p = act->intent;
	p = get_keyvalue(p, temp, name);
	p = get_keyvalue(p, temp, url);

	x = 32; y = 40;
	w = 250;
	h = 34;
	if(name[0]) {
		v = ViewAddLabel(wnd, TAG_PHONE_NO_NAME, name, x, y, w, h, UITextAlignmentLeft);
		UIsetFont(v, 24);
		UIsetTextColor(v, whiteColor);
		y += h;
		h = 26;
	}
	v = ViewAddLabel(wnd, TAG_PHONE_NO_URL, url, x, y, w, h, UITextAlignmentLeft);
	if(h == 32) UIsetFont(v, 26);
	else	UIsetFont(v, 20);
	UIsetTextColor(v, whiteColor);
	y += h;
	pjsCallState = 0; timerCount = 0;
	h = 26; w = 150;
	GetCallStateMsg(pjsCallState, temp);
	v = ViewAddLabel(wnd, TAG_PHONE_CALL_STATE, temp, x, y, w, h, UITextAlignmentLeft);
	UIsetFont(v, 20);
	UIsetTextColor(v, whiteColor);

	h = 48;
	v = ViewAddButtonAlphaMask(wnd, TAG_PHONE_HANGUP_BTN, "phone_hangup.bmp", 0, rt.height-h-16, rt.width, h+16, h);
	UIsetOpaque(v, TRUE);
	UIsetBackgroundColor(v, 0xfff44a4d);
	UIsetTintColor(v, whiteColor);
	w = h = 64; x = (rt.width - w) >> 1;
	y = (rt.height - h) >> 1;
	v = ViewAddImageViewAlphaMask(wnd, TAG_PHONE_DIAL, "phone_dial.bmp", x, y, w, h);
	appMuteSounds();
	avAudioMixerSetVolume(syscfgCallSoundVolume(NULL));
	sprintf(temp, "sip:%s", url);
	pjsuaMakeCall(temp);
	UItimerStart(1, 500, 1);
}

void PjsCallOnDestroy(GACTIVITY *act)
{
	appRecoverSounds();
	AdmActExit(act);
}

void MainShutdown(void);

void PjsCallOnTimer(GACTIVITY *act, int timerID)
{
	void	*wnd, *v;
	unsigned char	msg[4];
	char	temp[64];

	wnd = activityWindow(act);
	if(timerID == 1) {
		timerCount++;
		switch(pjsCallState) {
		case PJSIP_INV_STATE_EARLY:
			v = UIviewWithTag(wnd, TAG_PHONE_DIAL);
			UIsetHidden(v, !UIisHidden(v));
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
			break;
		case PJSIP_INV_STATE_CONFIRMED:
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
			break;
		case PJSIP_INV_STATE_DISCONNECTED:
printf("%lu ### disconnected 1st timeout\n", MS_TIMER, pjsCallState);
			UItimerStop(1);
			audioFlush();
			msg[0] = 128; msg[1] = sys_cfg->userSoundVolume; msg[2] = 0;
			audioPost(msg);
			UItimerStart(1, 4000, 0);
			pjsCallState = PJSIP_INV_STATE_DISCONNECTED + 1;
			break;
		case PJSIP_INV_STATE_DISCONNECTED+1:
printf("%lu ### disconnected 2nd timeout\n", MS_TIMER, pjsCallState);
			DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
			break;
		default:
			v = UIviewWithTag(wnd, TAG_PHONE_DIAL);
			UIsetHidden(v, !UIisHidden(v));
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
		}
	}
}

void PjsCallOnClick(GACTIVITY *act, void *view)
{
	int		tag;

	tag = UItag(view);
	switch(tag) {
	case TAG_PHONE_HANGUP_BTN:
		if(pjsCallState > PJSIP_INV_STATE_NULL && pjsCallState < PJSIP_INV_STATE_DISCONNECTED) {
			pjsuaHangupCall();
		}		
		break;
	case TAG_BACK_BTN:
		UItimerStop(1);
		DestroyActivityForResult(act, RESULT_CANCELLED, NULL);
		break;
	}
}

void PjsCallOnAppMessage(GACTIVITY *act, unsigned char *msg)
{
	void	*wnd, *v;
	char	temp[64];

	wnd = activityWindow(act);
	switch(msg[0]) {
	case GM_CALL_STATE_CHANGED:
		pjsCallState = msg[1];
printf("%lu ### call_state: %d.....\n", MS_TIMER, pjsCallState);
		switch(pjsCallState) {
		case PJSIP_INV_STATE_EARLY:
			UItimerStop(1);
			timerCount = 0;
			v = UIviewWithTag(wnd, TAG_PHONE_DIAL);
			UIsetHidden(v, TRUE);
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
			UItimerStart(1, 500, 1);
			break;
		case PJSIP_INV_STATE_CONFIRMED:
			UItimerStop(1);
			v = UIviewWithTag(wnd, TAG_PHONE_DIAL);
			UIsetHidden(v, FALSE);
			timerCount = 0;
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
			UItimerStart(1, 1000, 1);
			break;
		case PJSIP_INV_STATE_DISCONNECTED:
			UItimerStop(1);
			GetCallStateMsg(pjsCallState, temp);
			v = UIviewWithTag(wnd, TAG_PHONE_CALL_STATE);
			UIsetText(v, temp);
			v = ViewChangeImageViewAlphaMask(wnd, TAG_PHONE_DIAL, "phone_hangup.bmp");
			UIsetHidden(v, FALSE);
			v = UIviewWithTag(wnd, TAG_PHONE_HANGUP_BTN);
			UIsetHidden(v, TRUE);
			UItimerStart(1, 1108, 0);
			break;
		}
		break;
	}
}

