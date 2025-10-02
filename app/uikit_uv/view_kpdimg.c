void *_KeypadAddImageDigit(KEYPAD_CONTEXT *kpd, int tag, int x, int y, int width, int height);

void KeypadChangeImageDigit(KEYPAD_CONTEXT *kpd)
{
	CGRect	rect;
	char	fileName[32], temp[4];
	int		i, j, idx, x, y, w, h;
	
	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	if(kpd->memBuffer) ViewFreeBuffer(kpd->memBuffer);
	else	kpd->memBuffer = ViewAllocBuffer();
	idx = 0;
	y = 2; w = 76; h = 60;
	for(i = idx = 0;i < 4;i++) {
		x = 38;
		for(j = 0;j < 3;j++) {
			_KeypadAddImageDigit(kpd, TAG_KEYPAD_BASE_BTN+idx, x, y, w, h);
			x += w + 8; idx++;
		}
		y += h + 7;
	}
}

void KeypadChangeImageDigit2(KEYPAD_CONTEXT *kpd)
{
	CGRect	rect;
	char	fileName[32], temp[4];
	int		i, j, idx, x, y, w, h;
	
//	UIremoveChildAll(kpd->view);
	UIsetNeedsDisplay(kpd->view);
	idx = 0;
	y = 142; w = 80; h = 63;
	for(i = idx = 0;i < 4;i++) {
		x = 40;
		for(j = 0;j < 3;j++) {
			_KeypadAddImageDigit(kpd, TAG_KEYPAD_BASE_BTN+idx, x, y, w, h);
			x += w + 8; idx++;
		}
		y += h + 7;
	}
}

void *_KeypadAddImageDigit(KEYPAD_CONTEXT *kpd, int tag, int x, int y, int width, int height)
{
	char	fileName[64], highlightedFileName[64];
	void	*btn, *lbl;

	sprintf(fileName, "keypad_up%02d.bmp", tag-TAG_KEYPAD_BASE_BTN+1);
	sprintf(highlightedFileName, "keypad_dn%02d.bmp", tag-TAG_KEYPAD_BASE_BTN+1);
	btn = ViewAddButtonBitmap(kpd->view, tag, x, y, width, height, fileName, highlightedFileName);
	return btn;
}

