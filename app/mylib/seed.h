#ifndef _SEED_H
#define _SEED_H

#ifdef  __cplusplus
extern "C" {
#endif


#define SEED_BLOCK		16


#ifndef _SEED_CTX_
#define _SEED_CTX_
typedef struct _SEED_CTX 
{
	unsigned long	key_data[32];				
} SEED_CTX;
#endif


void SeedInit(SEED_CTX *ctx, unsigned char *Key);
void SeedEncryptECB(SEED_CTX *ctx, unsigned char *in, unsigned char *out, int len);
void SeedDecryptECB(SEED_CTX *ctx, unsigned char *in, unsigned char *out, int len);
void SeedEncryptCBC(SEED_CTX *ctx, unsigned char *IV, unsigned char *in, unsigned char *out, int len);
void SeedDecryptCBC(SEED_CTX *ctx, unsigned char *IV, unsigned char *in, unsigned char *out, int len);
int  SeedPadding(unsigned char *data, int len, int pad_method, int pad_byte);


#ifdef  __cplusplus
}
#endif

#endif

