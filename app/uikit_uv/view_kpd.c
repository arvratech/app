static char  QT1_Keys[28] = {
	'q', 'w', 'e', 'r', 't', 'y', 'u', 'i', 'o', 'p',
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l',
	'z', 'x', 'c', 'v', 'b', 'n', 'm', ' ', '.'
}; 

static char  QT2_Keys[29] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'!', '@', '#', '$', '%', '^', '&', '*', '(', ')',
	'-', '\047', '\042', ':', ';', ',', '?', ' ', '.' 
}; 

static char  QT3_Keys[29] = {
	'1', '2', '3', '4', '5', '6', '7', '8', '9', '0',
	'`', '_', '\134', '|', '<', '>', '{', '}', '[', ']',
	'+', '=', '~', '/', ';', ',', '?', ' ',  '.'
}; 

int Get_KeyCode(int mode, int index)
{
	int		c;

	if(mode == 0) c = QT1_Keys[index];
	else if(mode == 1 || mode == 2) c = QT1_Keys[index]  - 'a' + 'A';
	else if(mode == 3) c = QT2_Keys[index];
	else	c = QT3_Keys[index];
	return c;
}

// type  0:Normal  1:LargeDigit  2:ImageDigit
void ViewAddKeypad(KEYPAD_CONTEXT *kpd, void *view, int type)
{
	void 	*v;
	CGRect	rt;
	int		y, h;

	UIframe(view, &rt);
	if(type == 2) h = 300;
	else if(type == 1) h = 252;	// h = 284;
	else	h = 220;
	y = rt.height - h;
	v = ViewAddChild(view, TAG_VIEW_KEYPAD, 0, y, rt.width, h);
	if(type == 2) UIsetOpaque(v, FALSE); 
	else {
		UIsetOpaque(v, TRUE); 
		UIsetBackgroundColor(v, TABLE_GREY_400);
	}
	kpd->view = v;
	kpd->memBuffer = NULL;
}

void ViewAddImageKeypad(KEYPAD_CONTEXT *kpd, void *view)
{
	void 	*v;
	CGRect	rt;
	int		h;

	UIframe(view, &rt);
    h = 454;    // 320*432=336*454
    v = ViewAddImageViewBitmap(view, TAG_VIEW_KEYPAD, "keypad_bg2_24.bmp", 0, rt.height-h, rt.width, h);
	UIsetUserInteractionEnabled(v, TRUE);
	UIsetClipToBounds(v, FALSE);
	kpd->view = v;
	UIframe(v, &rt);
//	kpd->label = ViewAddLabel(view, TAG_KEYPAD_TEXT, "123", 48, 148, rt.width-96, 36, UITextAlignmentCenter);
    kpd->label = ViewAddLabel(v, TAG_KEYPAD_TEXT, NULL, 48, 42, rt.width-96, 36, UITextAlignmentCenter);
    UIsetFont(kpd->label, 32);
	kpd->memBuffer = NULL;
}

void ViewRemoveKeypad(KEYPAD_CONTEXT *kpd)
{
	UIremoveFromChild(kpd->view);
}

static void *_KeypadAddTitle(KEYPAD_CONTEXT *kpd, int tag, char *title, int x, int y, int width, int height)
{
	void	*btn, *v;

	btn = ViewAddTextButton(kpd->view, tag, title, x, y, width, height);
	UIsetOpaque(btn, TRUE); 
	UIsetBackgroundColor(btn, TABLE_GREY_50);
	UIsetTitleColor(btn, TABLE_GREY_900);
	v = UItitleLabel(btn);
	UIsetFont(v, kpd->size);
	return btn;
}

static void *_KeypadAddTitle2(KEYPAD_CONTEXT *kpd, int tag, char *title, int x, int y, int width, int height)
{
	void	*btn, *v;

	btn = ViewAddTextButton(kpd->view, tag, title, x, y, width, height);
	UIsetOpaque(btn, TRUE); 
	UIsetBackgroundColor(btn, TABLE_GREY_300);
	UIsetTitleColor(btn, TABLE_GREY_700);
	v = UItitleLabel(btn);
	UIsetFont(v, kpd->size);
	return btn;
}

static void *_KeypadAddImage(KEYPAD_CONTEXT *kpd, int tag, char *fileName, int x, int y, int width, int height, UIColor tintColor)
{
	void	*btn, *v;
	CGRect	rt;

	btn = ViewAddButtonAlphaMask(kpd->view, tag, fileName, x, y, width, height, kpd->size);
	if(UIisOpaque(kpd->view)) {
		UIsetOpaque(btn, TRUE); 
	} else {
		UIsetOpaque(btn, FALSE); 
	}
	UIsetBackgroundColor(btn, TABLE_GREY_300);
	v = UIimageView(btn);
	UIsetTintColor(v, tintColor);
	//txColor = 0xff616161;
}

#define IMG_TINT_COLOR	 0xff03a9f4

// 0:Lower 1:Upper temporal  2:Upper always  3:Symbol I  4:Symbol II
void KeypadChangeQwerty(KEYPAD_CONTEXT *kpd, int mode)
{
	void	*btn, *imgv;
	CGRect	rt;
	char	temp[4], fileName[32];
	int		i, j, idx, n, x, y, w, h, x0;

	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	if(kpd->memBuffer) ViewFreeBuffer(kpd->memBuffer);
	else	kpd->memBuffer = ViewAllocBuffer();
	UIframe(kpd->view, &rt);
	w = (rt.width - 50) / 10;
	x0 = (rt.width - w*10 - 36) >> 1;
	h = (rt.height - 32) / 4;
	y = (rt.height - h*4 - 18) >> 1;
	kpd->size = (w * 85) / 100;		// 0.85
	idx = 0; temp[1] = 0;
	x = x0;
	for(i = 0;i < 10;i++) {
		temp[0] = Get_KeyCode(mode, idx);
		_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
		x += w + 4; idx++;
	}
	y += h + 6;
	if(mode > 2) {
		x = x0; n = 10;
	} else {
		x = x0 + ((w+4)>>1); n = 9;
	}
	for(i = 0;i < n;i++) {
		temp[0] = Get_KeyCode(mode, idx);
		_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
		x += w + 4; idx++;
	}
	y += h + 6;
	x = x0;
	if(mode == 4) strcpy(fileName, "kpd_sym2.bmp");
	else if(mode == 3) strcpy(fileName, "kpd_sym1.bmp");
	else	strcpy(fileName, "kpd_shift.bmp");
	n = ((w + 4) >> 1) + w;
	btn = _KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN+1, fileName, x, y, n, h, IMG_TINT_COLOR);
	if(mode == 1) {
		imgv = UIimageView(btn);
		UIsetTintColor(imgv, IMG_TINT_COLOR);
	} else if(mode == 2) {
		UIsetBackgroundColor(btn, TABLE_CYAN_400);
		imgv = UIimageView(btn);
		UIsetTintColor(imgv, TABLE_GREY_50);
	}
	x += n + 4;
	for(i = 0;i < 7;i++) {
		temp[0] = Get_KeyCode(mode, idx);
		_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
		x += w + 4; idx++;
	}
	btn = _KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN, "kpd_backspace.bmp", x, y, n, h, IMG_TINT_COLOR);
	y += h + 6;
	x = x0;
	if(mode == 3 || mode == 4) strcpy(fileName, "kpd_abc.bmp");
	else	strcpy(fileName, "kpd_123.bmp");
	btn = _KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN+2, fileName, x, y, n, h, IMG_TINT_COLOR);
	x += n + 4;
	_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, "space", x, y, 6*w+20, h);
	x += 6*w+20 + 4; idx++;
	temp[0] = Get_KeyCode(mode, idx);
	_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
	x += w + 4;
	btn = _KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN+3, "kpd_return.bmp", x, y, n, h, IMG_TINT_COLOR);
}

static char	Num_Keys[12] = { '1', '2', '3', '4', '5', '6', '7', '8', '9', '*', '0', '#' }; 

void KeypadChangeNumeric(KEYPAD_CONTEXT *kpd)
{
	CGRect	rt;
	char	fileName[32], temp[4];
	int		i, j, idx, x, y, w, h, x0;
	
	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	if(kpd->memBuffer) ViewFreeBuffer(kpd->memBuffer);
	else	kpd->memBuffer = ViewAllocBuffer();
	UIframe(kpd->view, &rt);
	w = (rt.width - 32) / 4;
	x0 = (rt.width - w*4 - 18) >> 1;
	h = (rt.height - 32) / 4;
	y = (rt.height - h*4 - 18) >> 1;
	kpd->size = (h << 1) / 3;	// 2/3
	idx = 0; temp[1] = 0;
	for(i = 0;i < 4;i++) {
		x = x0;
		for(j = 0;j < 3;j++) {
			temp[0] = Num_Keys[idx];
			if(i == 3 && (j == 0 || j == 2)) _KeypadAddTitle2(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
			else	_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
			x += w + 6; idx++;
		}
		if(i == 0) strcpy(fileName, "kpd_backspace.bmp");
		else if(i == 1) strcpy(fileName, "arrow_right.bmp");
		else if(i == 2) strcpy(fileName, "arrow_left.bmp");
		else	strcpy(fileName, "kpd_return.bmp");
		_KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN+i, fileName, x, y, w, h, IMG_TINT_COLOR);
		y += h + 6; 
	}
}

static char	HexaNum_Keys[16] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' }; 

void KeypadChangeHexaNumeric(KEYPAD_CONTEXT *kpd)
{
	CGRect	rt;
	char	fileName[32], temp[4];
	int		i, j, idx, x, y, w, h, x0;
	
	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	if(kpd->memBuffer) ViewFreeBuffer(kpd->memBuffer);
	else	kpd->memBuffer = ViewAllocBuffer();
	UIframe(kpd->view, &rt);
	w = (rt.width - 32) / 5;
	x0 = (rt.width - w*5 - 18) >> 1;
	h = (rt.height - 36) / 4;
	y = (rt.height - h*4 - 24) >> 1;
	kpd->size = (h << 1) / 3;	// 2/3
	idx = 0; temp[1] = 0;
	for(i = 0;i < 4;i++) {
		x = x0;
		for(j = 0;j < 4;j++) {
			temp[0] = HexaNum_Keys[idx];
			_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
			x += w + 6; idx++;
		}
		if(i == 0) strcpy(fileName, "kpd_backspace.bmp");
		else if(i == 1) strcpy(fileName, "arrow_right.bmp");
		else if(i == 2) strcpy(fileName, "arrow_left.bmp");
		else	strcpy(fileName, "kpd_return.bmp");
		_KeypadAddImage(kpd, TAG_KEYPAD_FUNC_BTN+i, fileName, x, y, w, h, IMG_TINT_COLOR);
		y += h + 6;
	}
}

void KeypadChangeDigit(KEYPAD_CONTEXT *kpd)
{
	CGRect	rt;
	char	fileName[32], temp[4];
	int		i, j, idx, x, y, w, h, x0;
	
	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	if(kpd->memBuffer) ViewFreeBuffer(kpd->memBuffer);
	else	kpd->memBuffer = ViewAllocBuffer();
	UIframe(kpd->view, &rt);
	w = (rt.width - 26) / 3;
	x0 = (rt.width - w*3 - 12) >> 1;
	h = (rt.height - 32) / 4;
	y = (rt.height - h*4 - 18) >> 1;
	kpd->size = (h << 1) / 3;	// 2/3
	idx = 0; temp[1] = 0;
	for(i = 0;i < 3;i++) {
		x = x0;
		for(j = 0;j < 3;j++) {
			temp[0] = Num_Keys[idx];
			_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+idx, temp, x, y, w, h);
			x += w + 6; idx++;
		}
		y += h + 6; 
	}
	x = x0;
	strcpy(fileName, "kpd_backspace.bmp");
	_KeypadAddImage(kpd, TAG_KEYPAD_BASE_BTN+9, fileName, x, y, w, h, IMG_TINT_COLOR); x += w + 6;
	temp[0] = Num_Keys[10];
	_KeypadAddTitle(kpd, TAG_KEYPAD_BASE_BTN+10, temp, x, y, w, h); x += w + 6;
	strcpy(fileName, "kpd_return.bmp");
	_KeypadAddImage(kpd, TAG_KEYPAD_BASE_BTN+11, fileName, x, y, w, h, IMG_TINT_COLOR);
}

int KeypadQwertyCode(void *view, int mode)
{
	int		i, c, max;

	if(mode == 3 || mode == 4) max = 29;
	else	max = 28; 
	i = UItag(view) - TAG_KEYPAD_BASE_BTN; 
	if(i >= 0 && i < max) {
		if(mode == 0) c = QT1_Keys[i];
		else if(mode == 1 || mode == 2) c = QT1_Keys[i]  - 'a' + 'A';
		else if(mode == 3) c = QT2_Keys[i];
		else	c = QT3_Keys[i];
	} else	c = -1;
	return c;
}

int KeypadNumericCode(void *view)
{
	int		i, c;

	i = UItag(view) - TAG_KEYPAD_BASE_BTN; 
	if(i >= 0 && i < 12) c = Num_Keys[i];
	else	c = -1;
	return c;
}

int KeypadHexaNumericCode(void *view)
{
	int		i, c;

	i = UItag(view) - TAG_KEYPAD_BASE_BTN; 
	if(i >= 0 && i < 16) c = HexaNum_Keys[i];
	else	c = -1;
	return c;
}

void KeypadSetNumericCode(void *view, int c)
{
	int		i;

	for(i = 0;i < 12;i++) 
		if(c == Num_Keys[i]) break;
	if(i < 12) UIsetTag(view, i + TAG_KEYPAD_BASE_BTN); 
	else	UIsetTag(view, -1);
}

// 0:Lower 1:Upper temproal  2:Upper always  3:Symbol I  4:Symbol II
