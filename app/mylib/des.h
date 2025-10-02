#ifndef _INC_DES
#define _INC_DES

#ifdef  __cplusplus
extern "C" {
#endif

/* Define __cdecl for non-Microsoft compilers */
#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


struct des_block {
	unsigned int	c;
	unsigned int	d;
} typedef des_block_t;

typedef struct _DES_CTX {
	des_block_t		key_schedule[16];
} DES_CTX;

typedef struct _DES2_CTX {
	DES_CTX			_des_ctx[2];
} DES2_CTX;

typedef struct _DES3_CTX {
	DES_CTX			_des_ctx[3];
} DES3_CTX;


void __cdecl desInitKey(DES_CTX *ctx, unsigned char *key);
void __cdecl desEncrypt(DES_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl desDecrypt(DES_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl desEncryptCBC(DES_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl desdecryptCBC(DES_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen);

int  __cdecl des3InitKey(DES3_CTX *ctx, unsigned char *key, int keyLen);
void __cdecl des3Encrypt(DES3_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl des3Decrypt(DES3_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl des3EncryptCBC(DES3_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen);
void __cdecl des3DecryptCBC(DES3_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen);

#ifdef  __cplusplus
}
#endif


#endif

