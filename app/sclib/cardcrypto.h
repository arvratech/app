#ifndef _INC_CARD_CRYPTO
#define _INC_CARD_CRYPTO

#ifdef  __cplusplus
extern "C" {
#endif

/* Define __cdecl for non-Microsoft compilers */
#if     ( !defined(_MSC_VER) && !defined(__cdecl) )
#define __cdecl
#endif


void __cdecl CryptoInit(void);
void __cdecl CryptoDiversifyKey(unsigned char *uid, unsigned char *key);
void __cdecl CryptoEncode(unsigned char *data, unsigned char *uid, unsigned char *out);
int  __cdecl CryptoDecode(unsigned char *in, unsigned char *uid, unsigned char *data);


#ifdef  __cplusplus
}
#endif


#endif  /* _INC_CARD_CRYPTO */
