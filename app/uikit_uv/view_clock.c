
#define NOON_WIDTH		44
#define NOON_HEIGHT		30
#define COLON_WIDTH		15
#define DIGIT_WIDTH		60
#define DIGIT_HEIGHT	60
#define DATE_HEIGHT		24

static int	clock_hh, clock_mm;
static unsigned char	colon_cnt;

void *ViewAddClock(void *view, int viewTag, int baseTag, int y)
{
	void	*clockv, *v;
	int		w, h;
/*
char	temp[21];
ftSetSize(48);
printf("48 size: %d %d\n", ftTextWidth("58"), ftHeight());
printf("48 size: %d %d\n", ftTextWidth(":"), ftHeight());
ftSetSize(24);
dtGetNoon(LANG_KOR, 18, temp);
printf("24 size: %d %d\n", ftTextWidth(temp), ftHeight());
ftSetSize(20);
printf("20 size: %d %d\n", ftTextWidth("Wed, December 20"), ftHeight());
*/
	w = DIGIT_WIDTH + COLON_WIDTH + DIGIT_WIDTH + NOON_WIDTH; h = DIGIT_HEIGHT + DATE_HEIGHT;
	w += 32;
	clockv = ViewAddChild(view, viewTag, (lcdWidth()-w)>>1, y, w, h);
	v = ViewAddLabel(clockv, baseTag+0, NULL, 0, 0, DIGIT_WIDTH, DIGIT_HEIGHT, UITextAlignmentCenter);
	UIsetFont(v, 48);
	v = ViewAddLabel(clockv, baseTag+1, NULL, 0, 0, COLON_WIDTH, DIGIT_HEIGHT, UITextAlignmentCenter);
	UIsetFont(v, 48);
	UIsetText(v, ":");
	v = ViewAddLabel(clockv, baseTag+2, NULL, 0, 0, DIGIT_WIDTH, DIGIT_HEIGHT, UITextAlignmentCenter);
	UIsetFont(v, 48);
	v = ViewAddLabel(clockv, baseTag+3, NULL, 0, 20, NOON_WIDTH, NOON_HEIGHT, UITextAlignmentCenter);
	UIsetFont(v, 24);
	v = ViewAddLabel(clockv, baseTag+4, NULL, 0, DIGIT_HEIGHT, w, DATE_HEIGHT, UITextAlignmentCenter);
	UIsetFont(v, 20);
	return clockv;
}

void _ClockColonUpdate(void *view, int baseTag, int flag)
{
	void	*v;
	char	temp[8];
	
	v = UIviewWithTag(view, baseTag+1);
	if(flag) temp[0] = ':'; else temp[0] = ' ';
	temp[1] = 0;
	UIsetText(v, temp);
}

void _ClockHourUpdate(void *view, int baseTag)
{
	void	*v;
	char	temp[32];
	int		lang, hour24;
	
	lang = GetLanguage();
	//if(sys_cfg->dateNotation & TIME_NOTATION) hour24 = 1;
	if(syscfgTimeNotation(NULL)) hour24 = 1;
	else	hour24 = 0;
	dtGetNoon(lang, clock_hh, temp);
	v = UIviewWithTag(view, baseTag+3);
	UIsetText(v, temp);
	dtGetHour(hour24, clock_hh, temp);
	v = UIviewWithTag(view, baseTag+0);
	UIsetText(v, temp);
}

void _ClockMinuteUpdate(void *view, int baseTag)
{
	void	*v;
	char	temp[32];
	
	sprintf(temp, "%02d", clock_mm);
	v = UIviewWithTag(view, baseTag+2);
	UIsetText(v, temp);
}

void _ClockDateUpdate(void *view, int baseTag, unsigned char *ctm)
{
	void	*v;
	char	*p, temp[32];
	int		lang, weekDay;
	
	lang = GetLanguage();
	weekDay = get_week_day((int)ctm[0], (int)ctm[1], (int)ctm[2]);
	p = temp;
	if(lang == LANG_KOR || lang == LANG_JPN) {
		sprintf(p, "%04d%s", ctm[0] + 2000, GetCJKYear());
		p += strlen(p); *p++ = ' ';
		dtGetLongMonth(lang, (int)ctm[1], p);
		p += strlen(p); *p++ = ' ';
		sprintf(p, "%d%s", (int)ctm[2], GetCJKDay());
		p += strlen(p); *p++ = ' ';
		dtGetLongDayOfWeek(lang, weekDay, p);
	} else {
		sprintf(p, "%04d", ctm[0] + 2000);
		p += strlen(p); *p++ = ' ';
		dtGetShortDayOfWeek(lang, weekDay, p);
		p += strlen(p); *p++ = ','; *p++ = ' ';
		dtGetLongMonth(lang, (int)ctm[1], p); p += strlen(p);
		p += strlen(p); *p++ = ' ';
		sprintf(p, "%d", (int)ctm[2]);
	}
	v = UIviewWithTag(view, baseTag+4);
	UIsetText(v, temp);
}

void ClockViewUpdate(void *view, int baseTag)
{
	unsigned char	ctm[8];
	int		val;

	rtcGetDateTime(ctm);
	val = ctm[4];
	if(clock_mm != val) {
		clock_mm = val;
		_ClockMinuteUpdate(view, baseTag);
	}
	val = ctm[3];
	if(clock_hh != val) {
		clock_hh = val;
		_ClockHourUpdate(view, baseTag);
		_ClockDateUpdate(view, baseTag, ctm);
	}
	colon_cnt++;
	if(colon_cnt & 1) val = 1; else val = 0;
	_ClockColonUpdate(view, baseTag, val);
}

void ClockViewResize(void *view, int baseTag)
{
	void	*v, *nv;
	CGRect	rt;
	int		x, w, lang, hour24;

	colon_cnt = 0;
	lang = GetLanguage();
	nv = UIviewWithTag(view, baseTag+3);
	if(syscfgTimeNotation(NULL)) {
		hour24 = 1;
		w = DIGIT_WIDTH + COLON_WIDTH + DIGIT_WIDTH;
		UIsetHidden(nv, TRUE);
	} else {
		hour24 = 0;
		w = DIGIT_WIDTH + COLON_WIDTH + DIGIT_WIDTH + NOON_WIDTH;
		UIsetHidden(nv, FALSE); 
	}
	UIframe(view, &rt);
	x = (rt.width - w) >> 1;
	if(hour24 == 0 && (lang == LANG_KOR || lang == LANG_JPN)) {
		UIframe(nv, &rt); rt.x = x; UIsetFrame(nv, &rt);
		x += NOON_WIDTH;
	}
	v = UIviewWithTag(view, baseTag+0);
	UIframe(v, &rt); rt.x = x; UIsetFrame(v, &rt);
	x += DIGIT_WIDTH;
	v = UIviewWithTag(view, baseTag+1);
	UIframe(v, &rt); rt.x = x; UIsetFrame(v, &rt);
	x += COLON_WIDTH;
	v = UIviewWithTag(view, baseTag+2);
	UIframe(v, &rt); rt.x = x; UIsetFrame(v, &rt);
	x += DIGIT_WIDTH;
	if(hour24 == 0 && lang != LANG_KOR && lang != LANG_JPN) {
		UIframe(nv, &rt); rt.x = x; UIsetFrame(nv, &rt);
	}
	clock_hh = clock_mm = -1;
	UIsetNeedsDisplay(view);
}

