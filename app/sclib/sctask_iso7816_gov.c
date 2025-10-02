#include "seed.h"

unsigned char gov1_aid[8]  = { 0xd4,0x10,0x76,0x00,0x00,0x00,0x90,0x00 };
unsigned char gov2_aid[8]  = { 0xd4,0x10,0x76,0x00,0x00,0x00,0xa0,0x00 };
unsigned char gov1_key[16] = { 0x49,0x38,0x28,0xb2,0xae,0x34,0x0c,0x4a,
							   0xb1,0x49,0x19,0x08,0x9d,0xf5,0xe1,0xe1 };
unsigned char gov2_key[16] = { 0xfe,0x20,0xfa,0x3e,0x01,0xea,0xa2,0x39,
							   0x15,0xae,0xe1,0x1b,0x62,0x9d,0x91,0x87 };

int _GovEmvCard(int type, unsigned char *cardbuf)
{
	SEED_CTX	*ctx, _ctx;
	unsigned char	*p, buf[260], temp[32], crn[16], drn[16], key[16], dk[16], sk[16];
    int		rval, n, len;
 
	if(type) p = gov1_aid;
	else	 p = gov2_aid;
	rval = iso14443_4SelectDF(p, 7, buf, -16, &len);
prtsc("SelectDF", rval, buf, len);
	if(rval == ERR_EMV_STATUS && buf[0] == 0x6a && buf[1] == 0x82) {
		rval = ERR_FILE_NOT_FOUND;
	}
	if(rval) return rval;
	ctx = &_ctx;
	memcpy(temp, buf+len-10, 8); memcpy(temp+8, buf+len-10, 8);
	if(type) p = gov1_key;
	else	 p = gov2_key;
	SeedInit(ctx, p);
	SeedEncryptCBC(ctx, NULL, temp, key, 16);	// RK = ENC(gov_key, Code||Code)
	rval = iso14443_4GetData(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	for(n = 0;n < 8;n++) buf[8+n] = ~buf[n];
	SeedInit(ctx, key);
	SeedEncryptCBC(ctx, NULL, buf, dk, 16);		// DK = ENC(RK, CSN||CSN-1)
	rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(crn, buf, 16);
 	memrand(drn, 16);
	SeedInit(ctx, dk);
	SeedEncryptCBC(ctx, NULL, crn, key, 16);	// tempKey = ENC(dk, CRN)
	SeedInit(ctx, key);
	SeedEncryptCBC(ctx, NULL, drn, sk, 16);		// SK = ENC(tempKey, DRN);
	SeedInit(ctx, sk);
	SeedEncryptCBC(ctx, NULL, crn, temp+16, 16);	// Auth = ENC(SK, CRN) upper 8 bytes
	memcpy(temp, drn, 16);
	rval = iso14443_4CreateSession(0, temp, 24, buf, 2, &len);
	if(rval) return rval;
//prtsc("CreateSeswsion", rval, buf, len);
    rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(crn, buf, 16);
	SeedInit(ctx, sk);
	SeedEncryptCBC(ctx, NULL, dk, key, 16);	// TempKey = ENC(SK, DK)
	SeedInit(ctx, key);
	SeedEncryptCBC(ctx, NULL, crn, temp, 16);	// Auth = ENC(TempKey, CRN) upper 8 bytes
	rval = iso14443_4ExternalAuthenticate(temp, 8, buf, 2, &len);
//prtsc("ExternalAuth", rval, buf, len);
	if(rval) return rval;
 	memrand(drn, 16);
	rval = iso14443_4InternalAuthenticate(drn, 16, buf, 10, &len);
//prtsc("InternalAuth", rval, buf, len);
	if(rval) return rval;
	SeedInit(ctx, sk);
	SeedEncryptCBC(ctx, NULL, dk, key, 16);		// TempKey = ENC(SK< DK)
	SeedInit(ctx, key);
	SeedEncryptCBC(ctx, NULL, drn, temp, 16);	// Auth = ENC(CRN) upper 8 bytes
	if(memcmp(temp, buf, 8)) {
printf("gov authentication error\n");
		return ERR_AUTHENTICATE;
	}
	if(type) {
		// SFI=1:개인정보파일  3:주민번호(13) + 4:개인별발급차수(3)
		rval = iso14443_4ReadRecord2(1, 3, buf, -22, &len);		// 22 => -22
//prtsc("ReadRecord2", rval, buf, len);
		if(rval) return rval;
		len -= 6;
		SeedInit(ctx, sk);
		SeedEncryptCBC(ctx, NULL, buf, temp, len);
		if(memcmp(buf+len, temp, 4)) {
			printf("gov: ctypt error\n");
			return ERR_READ_RECORD;
		}
		SeedDecryptCBC(ctx, NULL, buf, temp, len);
//printf("1 [%02x", temp[0]); for(n = 1;n < len;n++) printf("-%02x", temp[n]); printf("]\n");
	    memcpy(cardbuf, temp+2, 13);
		rval = iso14443_4ReadRecord2(1, 4, buf, -22, &len);
//prtsc("ReadRecord2", rval, buf, len);
		if(rval) return rval;
		len -= 6;
		SeedEncryptCBC(ctx, NULL, buf, temp, len);
		if(memcmp(buf+len, temp, 4)) {
			return ERR_READ_RECORD;
		}
		SeedDecryptCBC(ctx, NULL, buf, temp, len);
//printf("2 [%02x", temp[0]); for(n = 1;n < len;n++) printf("-%02x", temp[n]); printf("]\n");
		memcpy(cardbuf+13, temp+2, 3);
//printf("Final [%02x", cardbuf[0]); for(n = 1;n < 16;n++) printf("-%02x", cardbuf[n]); printf("]\n");
	} else {
		// SFI=2:발급정보파일  9:발급번호(16)
		rval = iso14443_4ReadRecord2(2, 9, buf, -22, &len);
//prtsc("ReadRecord2", rval, buf, len);
		if(rval) return rval;
//printf("1 buf [%02x", buf[0]); for(n = 1;n < len;n++) printf("-%02x", buf[n]); printf("]\n");
		len -= 6;
		SeedInit(ctx, sk);
		SeedEncryptCBC(ctx, NULL, buf, temp, len);
//printf("1 temp [%02x", temp[0]); for(n = 1;n < len;n++) printf("-%02x", temp[n]); printf("]\n");
		if(memcmp(buf+len, temp, 4)) {
			printf("gov: crypt error\n");
			return ERR_READ_RECORD;
		}
		SeedDecryptCBC(ctx, NULL, buf, temp, 32);
//printf("Data [%02x", temp[0]); for(n = 1;n < len;n++) printf("-%02x", temp[n]); printf("]\n");
		cardbuf[0] = '0'; memcpy(cardbuf+1, temp+2, 7); memcpy(cardbuf+8, temp+10, 5);
		rval = iso14443_4ReadRecord2(1, 4, buf, -22, &len);
//prtsc("ReadRecord2", rval, buf, len);
		if(rval) return rval;
//printf("2 buf [%02x", buf[0]); for(n = 1;n < len;n++) printf("-%02x", buf[n]); printf("]\n");
		len -= 6;
		SeedEncryptCBC(ctx, NULL, buf, temp, len);
//printf("2 temp [%02x", temp[0]); for(n = 1;n < len;n++) printf("-%02x", temp[n]); printf("]\n");
		if(memcmp(buf+len, temp, 4)) {
			return ERR_READ_RECORD;
		}
		SeedDecryptCBC(ctx, NULL, buf, temp, len);
		memcpy(cardbuf+13, temp+2, 3);
//printf("Final [%02x", cardbuf[0]); for(n = 1;n < 16;n++) printf("-%02x", cardbuf[n]); printf("]\n");
	}
	return 0;
}

