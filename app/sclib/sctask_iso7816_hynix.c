#include "aes.h"

unsigned char hynix1_aid[10] = { 0xd0,0x00,0x53,0x4b,0x20,0x43,0x41,0x50,0x53 };
unsigned char hynix1_key[16] = {
	0xc2,0x16,0x90,0xf5,0xc4,0x48,0xc4,0x60,0x82,0x74,0x87,0x47,0xe2,0x97,0x54,0x4c
};
unsigned char hynix2_aid[12] = { 0xf0,0x73,0x6b,0x68,0x79,0x6e,0x69,0x78,0x00,0x01,0x01 };
unsigned char hynix2_key[16] = {
	0x7c,0xc0,0xe6,0x97,0x45,0xa0,0x78,0x3c,0x30,0x2b,0xfb,0x1d,0x6f,0x44,0x4e,0x77
};

unsigned char iv_null[16] =  {
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

unsigned char handy_aid[12] =  { 0x53,0x4b,0x68,0x79,0x6e,0x69,0x78,0x49,0x44,0x01 };
unsigned char handy_key[16] =  {
	0xb8,0x9e,0xae,0xf8,0x64,0x53,0x36,0x33,0x06,0x81,0x2b,0x95,0x95,0x39,0x55,0x75
};

int _HynixEmv1Card(unsigned char *cardbuf)
{
	unsigned char	buf[260], r1[16], r2[16], k[16], dk[16], sk[16];
    int     rval, i, len;
 
  	rval = iso14443_4SelectDF(hynix1_aid, 9, buf, -16, &len);
	if(rval == ERR_EMV_STATUS && ((buf[0] == 0x6a && buf[1] == 0x82) ||
					 (buf[0] == 0x69 && buf[1] == 0x83) || (buf[0] == 0x69 && buf[1] == 0x88))) {
		rval = ERR_FILE_NOT_FOUND;
	}
//prtsc("SelectDF", rval, buf, len);
	if(rval) return rval;
	rval = iso14443_4GetData(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	for(i = 0;i < 8;i++) buf[8+i] = ~buf[i];
	AES_ECB_encrypt2(hynix1_key, buf, dk);
	rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(r2, buf, 16);
 	memrand(r1, 16);
	rval = iso14443_4InternalAuthenticate(r1, 16, buf, 18, &len);
//prtsc("InternalAuth", rval, buf, len);
	if(rval) return rval;
	AES_ECB_encrypt2(dk, r1, k);
	AES_ECB_encrypt2(k, r2, sk);
	AES_ECB_encrypt2(sk, dk, k);
	AES_ECB_encrypt2(k, r1, r1);
	if(memcmp(r1, buf, 16)) {
//pintf("Authentication error\n");
		return ERR_AUTHENTICATE;
	}
	AES_ECB_encrypt2(k, r2, r2);
	rval = iso14443_4ExternalAuthenticate(r2, 16, buf, 2, &len);
//prtsc("ExternalAuth", rval, buf, len);
	if(rval) return rval;
	rval = iso14443_4ReadRecord(1, 10, buf, 18, &len);
//prtsc("ReadRecord", rval, buf, len);
	if(rval) return rval;
	AES_ECB_decrypt2(sk, buf, cardbuf);
//prtkey("Final", buf);
	return 0;
}

int _HynixEmv2Card(unsigned char *cardbuf)
{
	DES3_CTX	*ctx, _ctx;
	unsigned char	buf[260], tmp[16], crn[16], trn[16], dk[16], sk[16];
    int     rval, i, len;
 
	rval = iso14443_4SelectDF(hynix2_aid, 11, buf, 15, &len);
	if(rval == ERR_EMV_STATUS && ((buf[0] == 0x6a && buf[1] == 0x82) ||
					 (buf[0] == 0x69 && buf[1] == 0x83) || (buf[0] == 0x69 && buf[1] == 0x88))) {
		rval = ERR_FILE_NOT_FOUND;
	}
//if(rval) printf("SelectDF=%d\n", rval);
	if(rval) return rval;
	rval = iso14443_4GetData2(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	ctx = &_ctx;
	for(i = 0;i < 8;i++) buf[8+i] = ~buf[i];
//prtkey("csn", buf);
	des3InitKey(ctx, hynix2_key, 16);
	des3EncryptCBC(ctx, iv_null, buf, dk, 2);
//prtkey(" dk", dk);
	rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(crn, buf, 16);
//prtkey("crn", crn);
 	memrand(trn, 16);
//prtkey("trn", trn);
	des3InitKey(ctx, dk, 16);
	des3EncryptCBC(ctx, iv_null, crn, tmp, 2);
	des3InitKey(ctx, tmp, 16);
	des3EncryptCBC(ctx, iv_null, trn, sk, 2);
//prtkey("sk ", sk);
	des3InitKey(ctx, sk, 16);
	des3EncryptCBC(ctx, iv_null, crn, tmp, 2);
	memcpy(buf, trn, 16);
	memcpy(buf+16, tmp+8, 4);
	rval = iso14443_4CreateSession2(3, buf, 20, buf, 2, &len);
	if(rval) return rval;
	rval = iso14443_4ReadBinary2(5, 0, buf, 22, &len);
	if(rval) return rval;
	des3EncryptCBC(ctx, iv_null, buf, tmp, 2);
	if(memcmp(tmp+8, buf+16, 4)) {
		printf("MAC invalid\n");
		return -1;
	}
	des3DecryptCBC(ctx, iv_null, buf, tmp, 2);
	memcpy(cardbuf, tmp, 8);
	return 0;
}

int _HynixEmv3Card(unsigned char *cardbuf)
{
	unsigned char	buf[32], temp[32], crn[16], hrn[16], dk[16], sk[16];
    int     rval, i, len;
 
  	rval = iso14443_4SelectDF(handy_aid, 10, buf, -20, &len);
	if(rval == ERR_EMV_STATUS && ((buf[0] == 0x6a && buf[1] == 0x82) ||
					 (buf[0] == 0x69 && buf[1] == 0x83) || (buf[0] == 0x69 && buf[1] == 0x88))) {
		rval = ERR_FILE_NOT_FOUND;
	}
//prtsc("SelectDF", rval, buf, len);
	if(rval) return rval;
	rval = iso14443_4GetData(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	for(i = 0;i < 8;i++) buf[8+i] = ~buf[i];
	AES_CBC_encrypt2(handy_key, iv_null, buf, dk, 16);

	rval = iso14443_4GetChallenge(buf, 10, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(crn, buf, 8);
	memrand(hrn, 8);
	memcpy(buf+8, hrn, 8); 
	AES_CBC_encrypt2(dk, iv_null, buf, sk, 16);

	memcpy(temp, hrn, 8);
	crn[8] = 0x80; memset(crn+9, 0, 7);		// padding
	AES_CBC_encrypt2(sk, iv_null, crn, temp+8, 16);	// sign1
	rval = iso14443_4ExternalAuthenticate(temp, 12, buf, 6, &len);
prtsc("ExternalAuth", rval, buf, len);
	if(rval) return rval;
	
	hrn[8] = 0x80; memset(hrn+9, 0, 7);		// padding
	AES_CBC_encrypt2(sk, iv_null, hrn, temp, 16);	// sign2
	if(memcmp(buf, temp, 4)) {
printf("Authentication error\n");
		return ERR_AUTHENTICATE;
	}
	rval = iso14443_4ReadRecord(4, 0, buf, 22, &len);
//prtsc("ReadRecord", rval, buf, len);
	if(rval) return rval;
	AES_CBC_decrypt2(sk, iv_null, buf, temp, 16);
	memcpy(cardbuf, temp, 8);
//prtkey("Final", temp);
	return 0;
}

