#ifndef _PIN_KPD_H_
#define _PIN_KPD_H_


typedef struct _PIN_KEYPAD_CONTEXT {
	void	*view;
	void	*lbl;
	char	text[16];
	void	(*burstCallback)(int c);
	unsigned char	state;
	unsigned char	burst;
	unsigned char	entryStopSentinel;
	unsigned char	entryInterval;
	unsigned char	maximumLength;
	unsigned char	reserve[3];
} PIN_KEYPAD_CONTEXT;


void pinKeypadOpen(PIN_KEYPAD_CONTEXT *kpd);
void pinKeypadClose(PIN_KEYPAD_CONTEXT *kpd);
int  pinKeypadOpened(PIN_KEYPAD_CONTEXT *kpd);
int  pinKeypadPressed(PIN_KEYPAD_CONTEXT *kpd);


#endif

