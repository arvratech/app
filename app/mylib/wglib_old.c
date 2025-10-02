#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern const unsigned char _OneCountTbl[];


/*  e=even pararity  o=odd paraty
 *   data[0]  data[1]  data[2]  data[3]
 * |exxxxxxx|xxxxxxxx|xxxxxxxx|xo      |
 * 
 * |xxxxxxxx|xxxxxxxx|xxxxxxxx|
 */
int wgCheck26Data(unsigned char *data)
{
	unsigned char	e_p, o_p;

	if(data[0] & 0x80) e_p = 1; else e_p = 0;
	if(data[3] & 0x40) o_p = 1; else o_p = 0;
	data[0] <<= 1; if(data[1] & 0x80) data[0] |= 0x01;
	data[1] <<= 1; if(data[2] & 0x80) data[1] |= 0x01;
	data[2] <<= 1; if(data[3] & 0x80) data[2] |= 0x01;
	e_p += _OneCountTbl[(data[0] >> 4) & 0x0f];
	e_p += _OneCountTbl[data[0] & 0x0f];
	e_p += _OneCountTbl[(data[1] >> 4) & 0x0f];
	o_p += _OneCountTbl[data[1] & 0x0f];
	o_p += _OneCountTbl[(data[2] >>  4) & 0x0f];
	o_p += _OneCountTbl[data[2] & 0x0f];
	if(!(e_p & 0x01) && (o_p & 0x01)) e_p = 26;
	else	e_p = 0;
	return (int)e_p;
}

void wgReverse26Data(unsigned char *data)
{
	unsigned char	e_p, o_p;

	e_p  = _OneCountTbl[(data[0] >> 4) & 0x0f];
	e_p += _OneCountTbl[data[0] & 0x0f];
	e_p += _OneCountTbl[(data[1] >> 4) & 0x0f];
	o_p  = _OneCountTbl[data[1] & 0x0f];
	o_p += _OneCountTbl[(data[2] >>  4) & 0x0f];
	o_p += _OneCountTbl[data[2] & 0x0f];
	if(e_p & 0x01) e_p = 1; else e_p = 0;
	if(o_p & 0x01) o_p = 0; else o_p = 1;

	if(o_p) data[3] = 0x01; else data[3] = 0x00; data[3] |= data[2] << 1; 
	if(data[2] & 0x80) data[2] = 0x01; else data[2] = 0x00; data[2] |= data[1] << 1; 
	if(data[1] & 0x80) data[1] = 0x01; else data[1] = 0x00; data[1] |= data[0] << 1; 
	if(data[0] & 0x80) data[0] = 0x01; else data[0] = 0x00;
	if(e_p) data[0] |= 0x02;
}

/*  e=even pararity  o=odd paraty
 *   data[0]  data[1]  data[2]  data[3]  data[4]
 * |exxxxxxx|xxxxxxxx|xxxxxxxx|xxxxxxxx|xo      |
 * 
 * |xxxxxxxx|xxxxxxxx|xxxxxxxx|xxxxxxxx|
 */
int wgCheck34Data(unsigned char *data)
{
	unsigned char	e_p, o_p;

	if(data[0] & 0x80) e_p = 1; else e_p = 0;
	if(data[4] & 0x40) o_p = 1; else o_p = 0;
	data[0] <<= 1; if(data[1] & 0x80) data[0] |= 0x01;
	data[1] <<= 1; if(data[2] & 0x80) data[1] |= 0x01;
	data[2] <<= 1; if(data[3] & 0x80) data[2] |= 0x01;
	data[3] <<= 1; if(data[4] & 0x80) data[3] |= 0x01;
	e_p += _OneCountTbl[(data[0] >> 4) & 0x0f];
	e_p += _OneCountTbl[data[0] & 0x0f];
	e_p += _OneCountTbl[(data[1] >> 4) & 0x0f];
	e_p += _OneCountTbl[data[1] & 0x0f];
	o_p += _OneCountTbl[(data[2] >> 4) & 0x0f];
	o_p += _OneCountTbl[data[2] & 0x0f];
	o_p += _OneCountTbl[(data[3] >>  4) & 0x0f];
	o_p += _OneCountTbl[data[3] & 0x0f];
	if(!(e_p & 0x01) && (o_p & 0x01)) e_p = 34;
	else	e_p = 0;
	return (int)e_p;
}

int wgCheck35Data(unsigned char *data)
{
	unsigned char	e_p, o_p, o2_p, se, so;
	int		i;
	
	if(data[0] & 0x40) se = e_p = 1; else se = e_p = 0;
	if(data[4] & 0x20) so = o_p = 1; else so = o_p = 0;
	if(data[0] & 0x80) o2_p = 1; else o2_p = 0;
	data[0] = (data[0] << 2) | (data[1] >> 6);
	data[1] = (data[1] << 2) | (data[2] >> 6);
	data[2] = (data[2] << 2) | (data[3] >> 6);
	data[3] = (data[3] << 2) | (data[4] >> 6);
	e_p += _OneCountTbl[(data[0] >> 4) & 0x0d];
	e_p += _OneCountTbl[data[0] & 0x0b];
	e_p += _OneCountTbl[(data[1] >> 4) & 0x06];
	e_p += _OneCountTbl[data[1] & 0x0d];
	e_p += _OneCountTbl[(data[2] >> 4) & 0x0b];
	e_p += _OneCountTbl[data[2] & 0x06];
	e_p += _OneCountTbl[(data[3] >> 4) & 0x0d];
	e_p += _OneCountTbl[data[3] & 0x0b];

	o_p += _OneCountTbl[(data[0] >> 4) & 0x0b];
	o_p += _OneCountTbl[data[0] & 0x06];
	o_p += _OneCountTbl[(data[1] >> 4) & 0x0d];
	o_p += _OneCountTbl[data[1] & 0x0b];
	o_p += _OneCountTbl[(data[2] >> 4) & 0x06];
	o_p += _OneCountTbl[data[2] & 0x0d];
	o_p += _OneCountTbl[(data[3] >> 4) & 0x0b];
	o_p += _OneCountTbl[data[3] & 0x06];
	o_p += se;

	for(i = 0;i < 4;i++) {
		o2_p += _OneCountTbl[(data[i] >> 4) & 0x0f];
		o2_p += _OneCountTbl[data[i] & 0x0f];
	}
	o2_p += se + so;
	
	if(!(e_p & 0x01) && (o_p & 0x01) && (o2_p & 0x01)) i = 35;
	else	i = 0;
	return i;
}

/*  e=even pararity  o=odd paraty
 *   data[0]     data[15]  data[16]
 * |exxxxxxx|...|xxxxxxxx|xo      |
 * 
 * |xxxxxxxx|...|xxxxxxxx|
 */
int wgCheck130Data(unsigned char *data)
{
	unsigned char	e_p, o_p;
	int		i;

	if(data[0] & 0x80) e_p = 1; else e_p = 0;
	if(data[16] & 0x40) o_p = 1; else o_p = 0;
	for(i = 0;i < 16;i++) {
		data[i] <<= 1; if(data[i+1] & 0x80) data[i] |= 0x01;
	}
	for(i = 0;i < 8;i++) {
		e_p += _OneCountTbl[(data[i] >> 4) & 0x0f];
		e_p += _OneCountTbl[data[i] & 0x0f];
	}
	for( ;i < 16;i++) {
		o_p += _OneCountTbl[(data[i] >> 4) & 0x0f];
		o_p += _OneCountTbl[data[i] & 0x0f];
	}
	if(!(e_p & 0x01) && (o_p & 0x01)) i = 130;
	else	i = 0;
	return i;
}

int wgCheck66Data(unsigned char *data)
{
	unsigned char	e_p, o_p;
	int		i;

	if(data[0] & 0x80) e_p = 1; else e_p = 0;
	if(data[8] & 0x40) o_p = 1; else o_p = 0;
	for(i = 0;i < 8;i++) {
		data[i] <<= 1; if(data[i+1] & 0x80) data[i] |= 0x01;
	}
	for(i = 0;i < 4;i++) {
		e_p += _OneCountTbl[(data[i] >> 4) & 0x0f];
		e_p += _OneCountTbl[data[i] & 0x0f];
	}
	for( ;i < 8;i++) {
		o_p += _OneCountTbl[(data[i] >> 4) & 0x0f];
		o_p += _OneCountTbl[data[i] & 0x0f];
	}
	if(!(e_p & 0x01) && (o_p & 0x01)) i = 66;
	else	i = 0;
	return i;
}
