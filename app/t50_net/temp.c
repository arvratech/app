include <ft2build.h>
#include FT_FREETYPE_H

FT_Library	ftLibrary;
FT_Face		ftFace;
\

void fontInit(char* fontPath)
{
	int		rval;
	
	rval = FT_Init_FreeType(&ftLibrary);
	if(rval) {
printf("FT_Init_FreeType error\n");
		return -1;
	}
	rval = FT_New_Face(ftLibrary, fontPath, 0, &ftFace);
	if(rval) {	// FT_Err_Unknown_File_Format)
printf("FT_Init_FreeType error\n");
		return -1;
	}
	return 0;
}

void fontExit(void)
{
	FT_Done_Face(ftFace);
	FT_Done_FreeType(ftLibrary);
}


FT_Load_Char(face,TEXT('��'),FT_LOAD_RENDER | FT_LOAD_NO_BITMAP);
rror=FT_Set_Char_Size(face, 256 * 64, 0,

              GetDeviceCaps(hdc,LOGPIXELSX), GetDeviceCaps(hdc,LOGPIXELSY));


FT_Load_Glyph(face,index,FT_LOAD_DEFAULT | FT_LOAD_NO_BITMAP);

	FT_GlyphSlot		slot = m_Face->glyph;
	FT_Glyph_Metrics	*pMetrics = 0;
	FT_Bitmap			*pBitmap = &(slot->bitmap);
	FT_Pos				By=0,Bx=0,Py=0,Px=0;

	FT_Set_Pixel_Sizes(ftFace, nSize, nSize);
	FT_Load_Char(ftFace, c, FT_LOAD_RENDER);
	pBitmap = &slot->bitmap;
	pMetrics = &slot->metrics;

	// ���� ũ�⸦ /12 �Ͽ� �����Ѵ�.
	Height = pMetrics->height >> 6;

	// ������ ������� ������ ���̸� ���Ѵ�.
	By = ((pMetrics->vertAdvance - pMetrics->horiBearingY) - (pMetrics->vertAdvance / 6)) >> 6;

	// ���ݱ��� ������ ���ں��� �� ���̰� ũ��
	if(nHeight < By + Height)
		{
					// ���Խ�Ų��.
					nHeight = By + Height;
				}

				// ���ڿ� ����ũ�� ������Ų��.
				nWidth += pMetrics->horiAdvance >> 6;
			}



			// �����̽��� �������� �����ϱ� ����.
			pSurface->LockRect(&FontTextureRect, NULL, D3DLOCK_DISCARD);

			for (int a = 0;a < nLength;a++)
			{
				FT_Load_Char(m_Face, buf[a], FT_LOAD_RENDER);

				pBitmap = &(slot->bitmap);
				pMetrics = &(slot->metrics);

				pDestData = (BYTE *)FontTextureRect.pBits;

				nTWidth = pMetrics->width >> 6;
				nTHeight = pMetrics->height >> 6;

				Bx = pMetrics->horiBearingX >> 6;
				By = ((pMetrics->vertAdvance - pMetrics->horiBearingY) - (pMetrics->vertAdvance / 6)) >> 6;


				for(int i = 0; i < nTHeight; i++)
				{
					TexData = (DWORD*)(&pDestData[(Py + By + i) * FontTextureRect.Pitch]);

					for(int j = 0; j < nTWidth; j++)
					{
					ZZ	if(pBitmap->buffer[i * pBitmap->width + j])
						{	  
							TexData[Px + Bx + j] = ((pBitmap->buffer[i * pBitmap->width + j]) << 24) + RGB(255,255,255);
						}
					}
				}
				Px += pMetrics->horiAdvance >> 6;
			}
		}
	}
}

