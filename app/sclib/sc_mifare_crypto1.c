#include "ams_types.h"
#include "sc_mifare_crypto1.h"

/**
 * Create a new cipher instance of either card or reader side.
 *
 * @param state Pointer to the new cipher instance.
 * @param ciphter_type Type of the cipher.
 * @param implementation Implementation type of the cipher.
 * @return 1 if the specified cipher could be created, otherwise 0 is returned.
 */
int crypto1_new(crypto1_context *ctx, enum crypto1_cipher_type cipher_type, enum crypto1_cipher_implementation implementation)
{
	int		rval;

	//	memset(state, 0, sizeof(*state));
	if(cipher_type == CRYPTO1_CIPHER_CARD)ctx->is_card = 1;
	else	ctx->is_card = 0;
	switch(implementation) {
	case CRYPTO1_IMPLEMENTATION_CLEAN:
		rval = _crypto1_new_clean(ctx);
		break;
	case CRYPTO1_IMPLEMENTATION_OPTIMIZED:
		/* return _crypto1_new_opt(state);
         * No optimized version implemented */
		rval = 0;
		break;
	default:
		rval = 0;
	}	
	return rval;
}


/**
 * Initialize a cipher instance with secret key.
 *
 * @param state Cipher instance.
 * @param key MiFare key.
 */
void crypto1_init(crypto1_context *ctx, uint64emu_t key)
{
	ctx->ops->init(ctx, key);
}

/**
 * First stage of mutual authentication given a card's UID.
 * card_challenge is the card nonce as an integer
 */
void crypto1_mutual_1(crypto1_context *ctx, uint32_t uid, uint32_t card_challenge)
{
	ctx->ops->mutual_1(ctx, uid, card_challenge);
}

/**
 * Second stage of mutual authentication.
 * If this is the reader side, then the first 4 bytes of reader_response must
 * be preloaded with the reader nonce (and parity) and all 8 bytes will be
 * computed to be the correct reader response to the card challenge.
 * If this is the card side, then the response to the card challenge will be
 * checked.
 */
int crypto1_mutual_2(crypto1_context *ctx, u16 *reader_response)
{
	int		rval;

	if(ctx->is_card) rval = ctx->ops->mutual_2_card(ctx, reader_response);
	else {
		ctx->ops->mutual_2_reader(ctx, reader_response);
		rval = 1;
	}
	return rval;
}

/**
 * Third stage of mutual authentication.
 * If this is the reader side, then the card response to the reader
 * challenge will be checked.
 * If this is the card side, then the card response to the reader
 * challenge will be computed.
 */
int crypto1_mutual_3(crypto1_context *ctx, u16 *card_response)
{
	int		rval;
	
	if(ctx->is_card) {
		ctx->ops->mutual_3_card(ctx, card_response);
		rval = 1;
	} else {
		rval = ctx->ops->mutual_3_reader(ctx, card_response);
	}
	return rval;
}

/**
 * Perform the Crypto-1 encryption or decryption operation on 'length' bytes
 * of data with associated parity bits.
 */
void crypto1_transcrypt(crypto1_context *ctx, u16 *data, int length)
{
	crypto1_transcrypt_bits(ctx, data, length, 0);
}

/**
 * Perform the Crypto-1 encryption or decryption operation on 'bytes' bytes
 * of data with associated parity bits. 
 * The additional parameter 'bits' allows processing incomplete bytes after the
 * last byte. That is, if bits > 0 then data should contain (bytes+1) bytes where
 * the last byte is incomplete. 
 */
void crypto1_transcrypt_bits(crypto1_context *ctx, u16 *data, int bytes, int bits)
{
	ctx->ops->transcrypt_bits(ctx, data, bytes, bits);
}
