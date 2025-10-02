int bmpPutBiMask(BMPC *bmpD, int x, int y, int width, int height, unsigned int color, BMPC *bmpMask)
{
	unsigned char	*s, *s0;
	unsigned int	*d, *d0;
	int		i, j, w, h, c, m, m0, sw, dw, dbl;

	dw = width; d = bmpD->buffer + y * dw + x;
	sw = (bmpMask->width + 7) >> 3;
	s0 = (unsigned char *)bmpMask->buffer;
	s0 += y * sw + (x >> 3);
	m0 = 0x80 >> (x & 0x7);
	color |= 0xff000000;
	dw -= w;
	for(i = 0;i < h;i++) {
		s = s0;
		c = *s; m = m0;
		for(j = 0;j < w;j++) {
			if(!m) {
				m = 0x80; c = *(++s);
			}
			if(c & m) *d = color;
			else	*d = 0;
			d++; m >>= 1;
		}
		s0 += sw; d += dw;
	}
}

int bmpDrawText(BMPC *bmpD, char *string, unsigned int color)
{
	BMPC	*bmpS, _bmpS;
	char	*s;
	unsigned char	*font;
	int		x, w, h, tw, dw;

	if(!string || !string[0]) tw = 0;
	else {
		bmpS = &_bmpS;
		h = GetFontHeight();
		tw = dw = 0;
		s = string;
		x = 0;
		while(*s) {
			s += textCharFont(s, &font, &w);
			bmpInit(bmpS, font, GetFontHeight(), h);
			bmpDrawBiMask(bmpD, x, y, w, h, color, bmpS);
			x += w; tw += w;
			dw += width;
		}
	}
	return tw;
}

int CGdrawText(int x, int y, char *string, unsigned int color)
{
	BMPC	*bmp, _bmp;
	int		w;

	bmp = &_bmp;
	bmpcInit(bmp, bmpBuffer, lcdGetWidth(), lcdGetHeight());
	w = bmpDrawText(bmp, string, color);
	h = GetFontHeight();
	_lcdBlit(x, y, w, h, bmp);
	return w;
}
