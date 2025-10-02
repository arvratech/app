#include <stdio.h>
#include <string.h>
#include "blowfish.h"
#ifndef WIN32
#include "prim.h"
#endif
#include "cardcrypto.h"

static const unsigned char key1[16] = { 0xfd,0xa9,0x29,0x9e,0xa6,0xea,0xe3,0x4d,0xbe,0xc4,0xe1,0xb6,0xfd,0xf8,0xc5,0x29 };
static const unsigned char key2[16] = { 0x85,0xd6,0x05,0x65,0x78,0x57,0x78,0x15,0x62,0xab,0x4f,0xa2,0x52,0x97,0xea,0x12 };
static unsigned char k2[8];
static BLOWFISH_CTX	 *ctx1, _ctx1, *ctx2, _ctx2;


void CryptoInit(void)
{
	unsigned char	in[12], out[12];
	int		i, msb;

	ctx1 = &_ctx1;
	BlowfishInitKey(ctx1, key1, 16);
	memset(in, 0x00, 8);
	BF_Encrypt(ctx1, in, out);
	if(out[0] & 0x80) msb = 1; else msb = 0;
	out[8] = 0x00;
	for(i = 0;i < 8;i++) {
		in[i] = out[i] << 1;
		if(out[i+1] & 0x80) in[i] |= 0x01;
	}	
	if(msb) in[7] ^= 0x87;
	if(in[0] & 0x80) msb = 1; else msb = 0;
	in[8] = 0x00;
	for(i = 0;i < 8;i++) {
		k2[i] = in[i] << 1;
		if(in[i+1] & 0x80) k2[i] |= 0x01;
	}	
	if(msb) k2[7] ^= 0x87;
	ctx2 = &_ctx2;	
	BlowfishInitKey(ctx2, key2, 16);
}

void CryptoDiversifyKey(unsigned char *uid, unsigned char *key)
{
	unsigned char	*s, *d, in[16], out[8];
	int		i;

	s = in; 
	s[0] = 0x65; s[1] = uid[0]; s[2] = 0x16; s[3] = uid[1];
	s[4] = 0xb7; s[5] = uid[2]; s[6] = 0x9d; s[7] = uid[3]; 
	s[8] = 0x82; s[9] = 0x54;
	memset(s+10, 0x00, 6);
	d = out;
	BF_Encrypt(ctx1, s, d); s += 8;
	for(i = 0;i < BF_BLOCK;i++) s[i] ^= d[i] ^ k2[i];
	BF_Encrypt(ctx1, s, d);
	memcpy(key, d+1, 6);
}

void CryptoEncode(unsigned char *data, unsigned char *uid, unsigned char *out)
{
	unsigned char	*s, in[16];
//int		rval;
	
	memcpy(in, data, 8);
	s = in + 8;
	s[0] = 0x67; s[1] = uid[0]; s[2] = 0x29; s[3] = uid[1];
	s[4] = 0x5c; s[5] = uid[2]; s[6] = 0x15; s[7] = uid[3]; 
	BlowfishEncryptCBC(ctx2, NULL, in, out, 16);
//printf("CryptoEncode: UID=[%02x", (int)uid[0]); for(rval = 1;rval < 4;rval++) printf("-%02x", (int)uid[rval]); printf("]\n");
//printf("In =[%02x", (int)in[0]); for(rval = 1;rval < 16;rval++) printf("-%02x", (int)in[rval]); printf("]\n");
//printf("Out=[%02x", (int)out[0]); for(rval = 1;rval < 16;rval++) printf("-%02x", (int)out[rval]); printf("]\n");
}

int CryptoDecode(unsigned char *in, unsigned char *uid, unsigned char *data)
{
	unsigned char	*s, out[16], tmp[8];
	int		rval;

	BlowfishDecryptCBC(ctx2, NULL, in, out, 16);
//printf("CryptoDecode: UID=[%02x", (int)uid[0]); for(rval = 1;rval < 4;rval++) printf("-%02x", (int)uid[rval]); printf("]\n");
//printf("In =[%02x", (int)in[0]); for(rval = 1;rval < 16;rval++) printf("-%02x", (int)in[rval]); printf("]\n");
//printf("Out=[%02x", (int)out[0]); for(rval = 1;rval < 16;rval++) printf("-%02x", (int)out[rval]); printf("]\n");
	s = tmp;
	s[0] = 0x67; s[1] = uid[0]; s[2] = 0x29; s[3] = uid[1];
	s[4] = 0x5c; s[5] = uid[2]; s[6] = 0x15; s[7] = uid[3]; 
#ifdef WIN32
	if(memcmp(out+8, tmp, 8)) rval = -1;
#else
	if(memcmp(out+8, tmp, 8)) rval = -1;
#endif
	else {
		memcpy(data, out, 8);
		rval = 0;
	}
	return rval;
}
