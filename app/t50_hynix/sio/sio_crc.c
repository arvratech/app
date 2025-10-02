void InitCRC8(unsigned char *crc8tab)
{
	// terms of polynomial defining this crc(except x^8) 
//	const unsigned char poly = 0x07;	// ATM-8 HEC 0x83=x^8 + x^2 + x^1 + 1
//	const unsigned char poly = 0xd5;	// CRC-8     0xea=x^8 + x^7 + x^6 + x^4 + x^2 + 1
//	const unsigned char poly = 0x2f;	// C2        0x97=x^8 + x^5 + x^3 + x^2 + x^1 + 1
	const unsigned char poly = 0x4d;	//           0xa6=x^8 + x^6 + x^3 + x^2 + 1
	unsigned char	temp;
	int		i, j;

	for(i = 0;i < 256;i++) {
		temp = i;
		for(j = 0; j < 8;j++) {
			if(temp & 0x80) temp = (temp << 1) ^ poly;
			else	temp <<= 1;
		}
 		crc8tab[i] = temp;
    }
}

void InitCRC16_CCITT(unsigned char *crc16tab_h, unsigned char *crc16tab_l)
{
	const unsigned short poly = 0x1021;		// 
    unsigned short	crc, c;
    int		i, j;

	for(i = 0;i < 256;i++) {
		crc = 0;
		c   = (unsigned short)(i << 8);
		for(j = 0;j < 8;j++) {
			if((crc ^ c) & 0x8000) crc = (crc << 1) ^ poly;
            else	crc <<= 1;
			c <<= 1;
		}
		//for(j = 0; j < 8;j++) {
		//	if(crc &  0x8000) crc = (crc << 1) ^ poly);
		//	else	crc <<= 1;
		//}
		crc16tab_h[i] = (unsigned char)(crc >> 8);
		crc16tab_l[i] = (unsigned char)crc;
    }
}

