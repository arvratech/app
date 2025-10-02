#ifndef _SC_MIFARE_PARITY_H_
#define _SC_MIFARE_PARITY_H_


/*! Calculate the odd parity bit for one byte */
#define ODD_PARITY(i) (( (i) ^ (i)>>1 ^ (i)>>2 ^ (i)>>3 ^ (i)>>4 ^ (i)>>5 ^ (i)>>6 ^ (i)>>7 ^ 1) & 0x01)

/*!
 * Like UINT32_TO_ARRAY, but put the correct parity in the 8th bit of each array
 * entry (thus the array entries must at least be 16 bit wide)
 */
#define UINT32_TO_ARRAY_WITH_PARITY(i, a) do{ \
	(a)[0] = ((i)>>24) & 0xff; (a)[0] |= ODD_PARITY((a)[0])<<8; \
	(a)[1] = ((i)>>16) & 0xff; (a)[1] |= ODD_PARITY((a)[1])<<8; \
	(a)[2] = ((i)>> 8) & 0xff; (a)[2] |= ODD_PARITY((a)[2])<<8; \
	(a)[3] = ((i)>> 0) & 0xff; (a)[3] |= ODD_PARITY((a)[3])<<8; \
} while(0);

void calculateParity(unsigned short *data, int length);
int  checkParity(unsigned short *data, int length);


#endif
