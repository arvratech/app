#include "ams_types.h"
#include "sc_mifare_parity.h"


void calculateParity(unsigned short *data, int length)
{
	int		i;

	for(i = 0;i < length;i++) {
        if(ODD_PARITY(data[i])) data[i] |= (u16)0x0100;
		else	data[i] &= ~((u16)0x0100);
	}
}

int checkParity(unsigned short *data, int length)
{
	int		i, par, par2;

	for(i = 0;i < length;i++) {
		par = ODD_PARITY(data[i]);
		if(data[i] & 0x100) par2 = 1; else par2 = 0;
		if(par != par2) break;
	}
	if(i < length) i = -1;
	else	i = 0;
	return i;
}

