#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "NSEnum.h"
#include "prim.h"
#include "syscfg.h"
#include "cr.h"
#include "cf.h"


int cfCardDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData)
{
	CF_CARD	*cf = self;
	CARD_FIELD	*fld;
	unsigned char	*s, *d, temp[16];
	int		i, len, cred;

	if(dataLength < cfCardEncodedLength(cf)) return 0; 
	cred = cf->cfg->credential; 
	fld = cf->cfg->cardFields;
	s = data;
	len = 0;
	switch(cred) {
	case CRED_INT32:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		len = 9;
		break;
	case CRED_INT64:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		len = 9;
		break;
	case CRED_FC32_CN32:
		s += fld->offset;
		len = fld->length;
		memcpy(temp, s, len); s += len;
		fld++;
		s += fld->offset;
		memcpy(temp+len, s, (int)fld->length);
		len += fld->length;
		bin2card(s, len, credData);
		len = 9;
		break;
	case CRED_BCD_DIGITS:
		if(cf->cfg->coding == CF_CODING_ASCII) {
			s += fld->offset;
			digitslen2card(s, (int)fld->length, credData);
		} else {
			s += fld->offset >> 1;
			d = temp;
			if(fld->offset & 1) {
				for(i = 0;i < fld->length;i++) {
					if(i & 1) {
						*d |= (*s) >> 4; d++;
					} else {
						*d = (*s) << 4; s++;
					}
				}
				bcd2card(temp, (int)fld->length, credData);
			} else {
				bcd2card(s, (int)fld->length, credData);
			}
		}
		len = 9;
		break;
	case CRED_CHARACTER_STRING:
		s += fld->offset;
		strlen2card(s, (int)fld->length, credData);
		len = 9;
		break;
	}
	return len;
}

int cfWiegandDecodeCard(void *self, unsigned char *data, int dataLength, unsigned char *credData)
{
	CF_WIEGAND	*cf = self;
	CARD_FIELD	*fld;
	unsigned char	*s, *d, temp[16];
	int		i, len, cred;

	if(dataLength < cfWiegandEncodedLength(cf)) return 0; 
	cred = cf->cfg->credential; 
	fld = cf->cfg->cardFields;
	s = data;
	switch(cred) {
	case CRED_INT32:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		break;
	case CRED_INT64:
		s += fld->offset;
		bin2card(s, (int)fld->length, credData);
		break;
	case CRED_FC32_CN32:
		s += fld->offset;
		len = fld->length;
		memcpy(temp, s, len); s += len;
		fld++;
		s += fld->offset;
		memcpy(temp+len, s, (int)fld->length);
		len += fld->length;
		bin2card(s, len, credData);
		break;
	case CRED_BCD_DIGITS:
		*d++ = fld->length;
		if(cf->cfg->coding == CF_CODING_ASCII) {
			s += fld->offset;
			digitslen2card(s, (int)fld->length, credData);
		} else {
			s += fld->offset >> 1;
			d = temp;
			if(fld->offset & 1) {
				for(i = 0;i < fld->length;i++) {
					if(i & 1) {
						*d |= (*s) >> 4; d++;
					} else {
						*d = (*s) << 4; s++;
					}
				}
				bcd2card(temp, (int)fld->length, credData);
			} else {
				bcd2card(s, (int)fld->length, credData);
			}
		}
		break;
	case CRED_CHARACTER_STRING:
		s += fld->offset;
		strlen2card(s, (int)fld->length, credData);
		break;
	}
	return 9;
}

