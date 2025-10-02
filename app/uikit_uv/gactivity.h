#ifndef _GACTIVITY_H
#define _GACTIVITY_H


#include "cgrect.h"


// resultCode definitions
#define RESULT_CANCELLED	0
#define RESULT_OK			1
#define RESULT_FIRST_USER	2


typedef struct _GACTIVITY {
	unsigned char	classType;
	unsigned char	state;
	unsigned char	startForResult;
	unsigned char	isFinishing;
	void			*window;
	void			*touchView;
	int				requestCode;
	int				resultCode;
	char			intent[512];
	char			param1[64];
	char			param2[64];
	void			*context;
	void			*psContext;
	void			*viewBuffer;
	int				timeout;
	void (*onCreate)(struct _GACTIVITY *self);
	void (*onStart)(struct _GACTIVITY *self);
	void (*onStop)(struct _GACTIVITY *self);
	void (*onDestroy)(struct _GACTIVITY *self);
	void (*onActivityResult)(struct _GACTIVITY *self, int requestCode, int resultCode, char *intent);
	void (*onTimer)(struct _GACTIVITY *self, int timerID);
	void (*onClick)(struct _GACTIVITY *self, void *view);
	void (*onAppMessage)(struct _GACTIVITY *self, unsigned char *msg);
	void (*onValueChanged)(struct _GACTIVITY *self, void  *view);
	void (*onDown)(struct _GACTIVITY *self, void  *view, CGPoint *pt);
	void (*onTabUp)(struct _GACTIVITY *self, void  *view);
	void (*onLongPress)(struct _GACTIVITY *self, void  *view);
	void (*onScroll)(struct _GACTIVITY *self, void  *view, CGPoint *pt);
	void (*onFlying)(struct _GACTIVITY *self, void  *view, CGPoint *pt);
	void (*onTextFieldFocused)(struct _GACTIVITY *self, void *view, int textPosition);
	void (*onDismiss)(struct _GACTIVITY *self);
} GACTIVITY;


void activityInit(GACTIVITY *self);
void *activityWindow(GACTIVITY *self);
void activityAddView(GACTIVITY *self, void  *view);
int  activityRequestCode(GACTIVITY *self);
void activitySetResult(GACTIVITY *self, int resultCode, char *intent);
int  activityResultCode(GACTIVITY *self);
char *activityIntent(GACTIVITY *act);
char *activityParam1(GACTIVITY *self);
void activitySetParam1(GACTIVITY *self, char *param);
char *activityParam2(GACTIVITY *self);
void activitySetParam2(GACTIVITY *self, char *param);
void *activityContext(GACTIVITY *self);
void activitySetContext(GACTIVITY *self, void *context);
void *activityPsContext(GACTIVITY *self);
void activitySetPsContext(GACTIVITY *self, void *psContext);
void *activityViewBuffer(GACTIVITY *self);
void activitySetViewBuffer(GACTIVITY *self, void *viewBuffer);
int  activityTimeout(GACTIVITY *self);
void *activityParent(GACTIVITY *act);
void activitySetTimeout(GACTIVITY *self, int timeout);
int  activityIsFinishing(GACTIVITY *self);
void activitySetOnCreate(GACTIVITY *self, void (*onCreate)(GACTIVITY *));
void activitySetOnStart(GACTIVITY *self, void (*onStart)(GACTIVITY *));
void activitySetOnStop(GACTIVITY *self, void (*onStop)(GACTIVITY *));
void activitySetOnDestroy(GACTIVITY *self, void (*onDestroy)(GACTIVITY *));
void activitySetOnActivityResult(GACTIVITY *self, void (*onActivityResult)(GACTIVITY *, int, int, char *));
void activitySetOnTimer(GACTIVITY *self, void (*onTimer)(GACTIVITY *, int));
void activitySetOnClick(GACTIVITY *self, void (*onClick)(GACTIVITY *, void *));
void activitySetOnValueChanged(GACTIVITY *self, void (*onValueChanged)(GACTIVITY *, void *));
void activitySetOnAppMessage(GACTIVITY *self, void (*onAppMessage)(GACTIVITY *, unsigned char *));
void activitySetOnDown(GACTIVITY *self, void (*onDown)(GACTIVITY *, void *, CGPoint *));
void activitySetOnTabUp(GACTIVITY *self, void (*onTabUp)(GACTIVITY *, void *));
void activitySetOnLongPress(GACTIVITY *self, void (*onLongPress)(GACTIVITY *, void *));
void activitySetOnScroll(GACTIVITY *self, void (*onScroll)(GACTIVITY *, void *, CGPoint *));
void activitySetOnFlying(GACTIVITY *self, void (*onFlying)(GACTIVITY *, void *, CGPoint *));
void activitySetOnTextFieldFocused(GACTIVITY *self, void (*onTextFieldFocused)(GACTIVITY *, void *, int));
void activitySetOnDismiss(GACTIVITY *self, void (*onDismiss)(GACTIVITY *));
void activityClearTouchView(GACTIVITY *self);


#endif


