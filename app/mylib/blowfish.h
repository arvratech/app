#ifndef _INC_BLOWFISH
#define _INC_BLOWFISH

#ifdef  __cplusplus
extern "C" {
#endif

/* Define __cdecl for non-Microsoft compilers */
#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif

#define BF_ROUNDS		16
#define BF_BLOCK		8
#define MAXKEYBYTES		56		// 448 bits


typedef struct _BLOWFISH_CTX {
	unsigned long	P[BF_ROUNDS+2];
	unsigned long	S[4 * 256];
} BLOWFISH_CTX;


void __cdecl BF_Encrypt(BLOWFISH_CTX *ctx, unsigned char *in, unsigned char *out);
void __cdecl BF_Decrypt(BLOWFISH_CTX *ctx, unsigned char *in, unsigned char *out);
void __cdecl BlowfishInitKey(BLOWFISH_CTX *ctx, const unsigned char *key, int key_len);
void __cdecl BlowfishEncryptECB(BLOWFISH_CTX *ctx, unsigned char *in, unsigned char *out, int len);
void __cdecl BlowfishDecryptECB(BLOWFISH_CTX *ctx, unsigned char *in, unsigned char *out, int len);
void __cdecl BlowfishEncryptCBC(BLOWFISH_CTX *ctx, unsigned char *IV, unsigned char *in, unsigned char *out, int len);
void __cdecl BlowfishDecryptCBC(BLOWFISH_CTX *ctx, unsigned char *IV, unsigned char *in, unsigned char *out, int len);


#ifdef  __cplusplus
}
#endif


#endif

