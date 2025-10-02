#ifndef _SC_MIFARE_CRYPTO1_H_
#define _SC_MIFARE_CRYPTO1_H_

#include <stddef.h>
#include "stdint.h"
#include "sc_mifare_parity.h"
#include "sc_mifare_uint64emu.h"

//typedef unsigned int size_t;
//struct _crypto1_state;

typedef struct _crypto1_context {
	uint64emu_storage_t lfsr;   // The 48 bit LFSR for the main cipher state and keystream generation
	uint16_t    prng;           // The 16 bit LFSR for the card PRNG state, also used during authentication.
	uint8_t     is_card;        // Boolean whether this instance should perform authentication in card mode.
	const struct _crypto1_ops *ops;
} crypto1_context;

// ToDo: integrate reauthentication step 1 into the crypto1 framework
void crypto1_clean_mutual_1_2(crypto1_context *ctx, const uint32_t uid, const uint32_t card_challange);

struct _crypto1_ops {
  void(*init)(crypto1_context *ctx, const uint64emu_t key);
  void(*mutual_1)(crypto1_context *ctx, const uint32_t uid, const uint32_t card_challenge);
  void(*mutual_2_reader)(crypto1_context *ctx, u16 *reader_response);
  int (*mutual_3_reader)(crypto1_context *ctx, const u16 *card_response);
  int (*mutual_2_card)(crypto1_context *ctx, const u16 *reader_response);
  void(*mutual_3_card)(crypto1_context *ctx, u16 *card_response);
  void(*transcrypt_bits)(crypto1_context *ctx, u16 *data, size_t bytes, size_t bits);
};

enum crypto1_cipher_implementation {
	CRYPTO1_IMPLEMENTATION_CLEAN,
	CRYPTO1_IMPLEMENTATION_OPTIMIZED
};

enum crypto1_cipher_type {
	CRYPTO1_CIPHER_READER = 0,
	CRYPTO1_CIPHER_CARD = 1
};

int  crypto1_new(crypto1_context *ctx, enum crypto1_cipher_type, enum crypto1_cipher_implementation implementation);
void crypto1_init(crypto1_context *ctx, uint64emu_t key);
void crypto1_mutual_1(crypto1_context *ctx, uint32_t uid, uint32_t card_challenge);
int  crypto1_mutual_2(crypto1_context *ctx, u16 *reader_response);
int  crypto1_mutual_3(crypto1_context *ctx, u16 *card_response);
void crypto1_transcrypt(crypto1_context *ctx, u16 *data, int length);
void crypto1_transcrypt_bits(crypto1_context *ctx, u16 *data, int bytes, int bits);

int  _crypto1_new_opt(crypto1_context *ctx);
int  _crypto1_new_clean(crypto1_context *ctx);

/* Reverse the bit order in the 8 bit value x */
#define rev8(x)			((((x)>>7)&1)^((((x)>>6)&1)<<1)^((((x)>>5)&1)<<2)^((((x)>>4)&1)<<3)^((((x)>>3)&1)<<4)^((((x)>>2)&1)<<5)^((((x)>>1)&1)<<6)^(((x)&1)<<7))
/* Reverse the bit order in the 16 bit value x */
#define rev16(x)		(rev8 (x)^(rev8 (x>> 8)<< 8))
/* Reverse the bit order in the 32 bit value x */
#define rev32(x)		(rev16(x)^(rev16(x>>16)<<16))
/* Return the nth bit from x */
#define bit(x,n)		(((x)>>(n))&1)

/**
 * Convert 4 array entries (a[0], a[1], a[2] and a[3]) into a 32 bit integer, where 
 * a[0] is the MSByte and a[3] is the LSByte
 */
#define ARRAY_TO_UINT32(a) ( ((uint32_t)((a)[0]&0xff) << 24) | ((uint32_t)((a)[1]&0xff) << 16) | \
	((uint32_t)((a)[2]&0xff) << 8) | ((uint32_t)((a)[3]&0xff) << 0) )

/* Copy an uint32_t i into 4 array entries (a[0], a[1], a[2] and a[3]), where a[0] is the MSByte */
#define UINT32_TO_ARRAY(i, a) do{ \
	(a)[0] = ((i)>>24) & 0xff; \
	(a)[1] = ((i)>>16) & 0xff; \
	(a)[2] = ((i)>> 8) & 0xff; \
	(a)[3] = ((i)>> 0) & 0xff; \
}while(0);


#endif
