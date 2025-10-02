#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "des.h"

#define KEYLENGTH		64
#define BLOCKLENGTH		64
#define PC1LENGTH		56
#define PC2LENGTH		48
#define ELENGTH			48
#define PLENGTH			32
#define INVERSEIPLENGTH	64
#define NUMSUBKEYS		16
#define INTWIDTH		32


// Permutation tables
const int PC1[56] = {
	57, 49, 41, 33, 25, 17,  9,
	 1, 58, 50, 42, 34, 26, 18,
	10,  2, 59, 51, 43, 35, 27,
	19, 11,  3, 60, 52, 44, 36,
	63, 55, 47, 39, 31, 23, 15,
	 7, 62, 54, 46, 38, 30, 22,
    14,  6, 61, 53, 45, 37, 29,
	21, 13,  5, 28, 20, 12,  4
};

const int PC2[48] = {
	14, 17, 11, 24,  1,  5,
	 3, 28, 15,  6, 21, 10,
	23, 19, 12,  4, 26,  8,
	16,  7, 27, 20, 13,  2,
	41, 52, 31, 37, 47, 55,
	30, 40, 51, 45, 33, 48,
	44, 49, 39, 56, 34, 53,
	46, 42, 50, 36, 29, 32
};

const int IP[64] = {
	58, 50, 42, 34, 26, 18, 10,  2,
	60, 52, 44, 36, 28, 20, 12,  4,
	62, 54, 46, 38, 30, 22, 14,  6,
	64, 56, 48, 40, 32, 24, 16,  8,
	57, 49, 41, 33, 25, 17,  9,  1,
	59, 51, 43, 35, 27, 19, 11,  3,
	61, 53, 45, 37, 29, 21, 13,  5,
	63, 55, 47, 39, 31, 23, 15,  7
};

const int ETABLE[48] = {
	32,  1,  2,  3,  4,  5,
	 4,  5,  6,  7,  8,  9,
	 8,  9, 10, 11, 12, 13,
	12, 13, 14, 15, 16, 17,
	16, 17, 18, 19, 20, 21,
	20, 21, 22, 23, 24, 25,
	24, 25, 26, 27, 28, 29,
	28, 29, 30, 31, 32,  1
};

const int PTABLE[32] = {
	16,  7, 20, 21,
	29, 12, 28, 17,
	 1, 15, 23, 26,
	 5, 18, 31, 10,
	 2,  8, 24, 14,
	32, 27,  3,  9,
	19, 13, 30,  6,
	22, 11,  4, 25
};

const int INVERSEIP[64] = {
	40,  8, 48, 16, 56, 24, 64, 32,
	39,  7, 47, 15, 55, 23, 63, 31,
	38,  6, 46, 14, 54, 22, 62, 30,
	37,  5, 45, 13, 53, 21, 61, 29,
	36,  4, 44, 12, 52, 20, 60, 28,
	35,  3, 43, 11, 51, 19, 59, 27,
	34,  2, 42, 10, 50, 18, 58, 26,
	33,  1, 41,  9, 49, 17, 57, 25
};

// The famous DES S-Boxes. 
const unsigned char SBOX1[4][16] = {
	{14,  4, 13,  1,  2, 15, 11,  8,  3, 10,  6, 12,  5,  9,  0,  7},
	{ 0, 15,  7,  4, 14,  2, 13,  1, 10,  6, 12, 11,  9,  5,  3,  8},
	{ 4,  1, 14,  8, 13,  6,  2, 11, 15, 12,  9,  7,  3, 10,  5,  0},
	{15, 12,  8,  2,  4,  9,  1,  7,  5, 11,  3, 14, 10,  0,  6, 13}
};

const unsigned char SBOX2[4][16] = {
	{15,  1,  8, 14,  6, 11,  3,  4,  9,  7,  2, 13, 12,  0,  5, 10},
	{ 3, 13,  4,  7, 15,  2,  8, 14, 12,  0,  1, 10,  6,  9, 11,  5},
	{ 0, 14,  7, 11, 10,  4, 13,  1,  5,  8, 12,  6,  9,  3,  2, 15},
	{13,  8, 10,  1,  3, 15,  4,  2, 11,  6,  7, 12,  0,  5, 14,  9}
};

const unsigned char SBOX3[4][16] = {
	{10,  0,  9, 14,  6,  3, 15,  5,  1, 13, 12,  7, 11,  4,  2,  8},
	{13,  7,  0,  9,  3,  4,  6, 10,  2,  8,  5, 14, 12, 11, 15,  1},
	{13,  6,  4,  9,  8, 15,  3,  0, 11,  1,  2, 12,  5, 10, 14,  7},
	{ 1, 10, 13,  0,  6,  9,  8,  7,  4, 15, 14,  3, 11,  5,  2, 12}
};

const unsigned char SBOX4[4][16] = {
	{ 7, 13, 14,  3,  0,  6,  9, 10,  1,  2,  8,  5, 11, 12,  4, 15},
	{13,  8, 11,  5,  6, 15,  0,  3,  4,  7,  2, 12,  1, 10, 14,  9},
	{10,  6,  9,  0, 12, 11,  7, 13, 15,  1,  3, 14,  5,  2,  8,  4},
	{ 3, 15,  0,  6, 10,  1, 13,  8,  9,  4,  5, 11, 12,  7,  2, 14}
};

const unsigned char SBOX5[4][16] = {
	{ 2, 12,  4,  1,  7, 10, 11,  6,  8,  5,  3, 15, 13,  0, 14,  9},
	{14, 11,  2, 12,  4,  7, 13,  1,  5,  0, 15, 10,  3,  9,  8,  6},
	{ 4,  2,  1, 11, 10, 13,  7,  8, 15,  9, 12,  5,  6,  3,  0, 14},
	{11,  8, 12,  7,  1, 14,  2, 13,  6, 15,  0,  9, 10,  4,  5,  3}
};

const unsigned char SBOX6[4][16] = {
	{12,  1, 10, 15,  9,  2,  6,  8,  0, 13,  3,  4, 14,  7,  5, 11},
	{10, 15,  4,  2,  7, 12,  9,  5,  6,  1, 13, 14,  0, 11,  3,  8},
	{ 9, 14, 15,  5,  2,  8, 12,  3,  7,  0,  4, 10,  1, 13, 11,  6},
	{ 4,  3,  2, 12,  9,  5, 15, 10, 11, 14,  1,  7,  6,  0,  8, 13}
};

const unsigned char SBOX7[4][16] = {
	{ 4, 11,  2, 14, 15,  0,  8, 13,  3, 12,  9,  7,  5, 10,  6,  1},
	{13,  0, 11,  7,  4,  9,  1, 10, 14,  3,  5, 12,  2, 15,  8,  6},
	{ 1,  4, 11, 13, 12,  3,  7, 14, 10, 15,  6,  8,  0,  5,  9,  2},
	{ 6, 11, 13,  8,  1,  4, 10,  7,  9,  5,  0, 15, 14,  2,  3, 12}
};

const unsigned char SBOX8[4][16] = {
	{13,  2,  8,  4,  6, 15, 11,  1, 10,  9,  3, 14,  5,  0, 12,  7},
	{ 1, 15, 13,  8, 10,  3,  7,  4, 12,  5,  6, 11,  0, 14,  9,  2},
	{ 7, 11,  4,  1,  9, 12, 14,  2,  0,  6, 10, 13, 15,  3,  5,  8},
	{ 2,  1, 14,  7,  4, 10,  8, 13, 15, 12,  9,  0,  3,  5,  6, 11}
};

// Rotate a 28-bit half-key one bit to the left (for key schedule generation).
uint32_t rotate_half_key(uint32_t half_key)
{
    uint32_t top_bit = (half_key & 0x8000000) >> 27;
    return ((half_key << 1) | top_bit) & 0x0fffffff;
}

// Permute a key according to a given table, source length and destination length.
des_block_t permute(des_block_t key, const int *table, uint32_t src_len, uint32_t dest_len)
{
	des_block_t permuted;
	int		i;

	permuted.c = 0;
	permuted.d = 0;
	for(i = 0;i < dest_len;i++) {
		if(i < dest_len/2) {
			if(table[i] <= src_len/2)
				permuted.c |= ((key.c >> (src_len/2-table[i])) & 0x01) << ((dest_len/2-1)-i);
			else
				permuted.c |= ((key.d >> (src_len/2-(table[i] - src_len/2))) & 0x01)
									<< ((dest_len/2-1)-i);
		} else {
			if(table[i] <= src_len/2)
				permuted.d |= ((key.c >> (src_len/2-table[i])) & 0x01)
						<< ((dest_len/2 - 1) - (i - dest_len/2));
			else
				permuted.d |= ((key.d >> (src_len/2-(table[i] - src_len/2))) & 0x01)
						<< ((dest_len/2 - 1) - (i - dest_len/2));
		}
	}
	return permuted;
}

// The initial 64-bit key is permuted according to the PC1 table,
// resulting in a 56-bit key (every eighth bit is discarded).
des_block_t permute_pc1(des_block_t key)
{
	return permute(key, PC1, KEYLENGTH, PC1LENGTH);
}

// The final key schedule is created by permuting the shifted subkeys
// according to the PC2 table, resulting in 16 48-bit keys.
void permute_pc2(des_block_t *shifted_subkeys, des_block_t *key_schedule)
{
	des_block_t permuted;
	int		i;

	for(i = 0;i < NUMSUBKEYS;i++) {
		permuted = permute(shifted_subkeys[i], PC2, PC1LENGTH, PC2LENGTH);
		key_schedule[i].c = permuted.c;
		key_schedule[i].d = permuted.d;
	}
}

// Permute block according to IP table.
des_block_t initial_permutation(des_block_t block)
{
	return permute(block, IP, BLOCKLENGTH, BLOCKLENGTH);
}

// Permute a 32-bit half block using the E table.
des_block_t permute_e(des_block_t block)
{
    return permute(block, ETABLE, BLOCKLENGTH/2, ELENGTH);
}

// Permute 32-bit half block using the P table.
des_block_t permute_p(des_block_t block)
{
	return permute(block, PTABLE, PLENGTH, PLENGTH);
}

// Permute 64-bit block using IP^(-1) table.
des_block_t final_permutation(des_block_t block)
{
	return permute(block, INVERSEIP, BLOCKLENGTH, BLOCKLENGTH);
}

// Create 16 subkeys by successive bit rotation according to schedule.
void shift_subkeys(des_block_t permuted, des_block_t *shifted_subkeys)
{
    uint32_t	shift_schedule[NUMSUBKEYS] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
	int		i, j;

	shifted_subkeys[0].c = rotate_half_key(permuted.c);
	shifted_subkeys[0].d = rotate_half_key(permuted.d);
	for(i = 1;i < NUMSUBKEYS;i++) {
		shifted_subkeys[i].c = shifted_subkeys[i-1].c;
		shifted_subkeys[i].d = shifted_subkeys[i-1].d;
		for(j = 0;j < shift_schedule[i];j++) {
			shifted_subkeys[i].c = rotate_half_key(shifted_subkeys[i].c);
			shifted_subkeys[i].d = rotate_half_key(shifted_subkeys[i].d);
		}
	}
}

// Generate 16 48-bit keys.
void generate_key_schedule(des_block_t key_block, des_block_t *key_schedule)
{
	des_block_t		block, shifted_subkeys[16];

	block = permute_pc1(key_block);
	shift_subkeys(block, shifted_subkeys);
	permute_pc2(shifted_subkeys, key_schedule);
}

/*
 * Use the first and last bits as row index, and middle four bits as col index.
 */
uint32_t lookup_sbox(uint32_t group, uint32_t sbox)
{
	uint32_t	val;
	int		row, col;

	row = group & 0x01;
	row |= ((group >> 5) & 0x01) << 1;
	col = (group >> 1) & 0xF;
	switch(sbox) {
	case 0:	val = SBOX1[row][col]; break;
	case 1:	val = SBOX2[row][col]; break;
	case 2:	val = SBOX3[row][col]; break;
	case 3:	val = SBOX4[row][col]; break;
	case 4:	val = SBOX5[row][col]; break;
	case 5: val = SBOX6[row][col]; break;
	case 6: val = SBOX7[row][col]; break;
	default: val = SBOX8[row][col];
	}
    return val;
}

// Apply sbox transformations to 48-bit message block.
des_block_t sbox_transform(des_block_t block)
{
	uint32_t groups[8] = {0};
	int		i, j;
	
	// Mask off groups of six bits.
	i = 0;
	for(j = 18;j >= 0;j-=6) {
		groups[i] = (block.c >> j) & 0x3F;
		groups[i+4] = (block.d >> j) & 0x3F;
		i++;
	}
	// Convert six bit groups to four.
    for(j = 0;j < 8;j++)
		groups[j] = lookup_sbox(groups[j], j);
	// Put the groups back together in a block.
	block.c = 0;
	block.d = 0;
	i = 0;
	for(j = 12;j >= 0;j-=4) {
		block.c |= (groups[i] << j);
		block.d |= (groups[i+4] << j);
		i++;
	}
	return block;
}

// Apply Feistel function to right half of message block.
uint32_t feistel(uint32_t right_block, des_block_t key)
{
	des_block_t		block;

	// Convert 32-bit integer to block for E permutation.
	block.c = right_block >> 16;
	block.d = right_block & 0xFFFF;
	// E permutation.
	block = permute_e(block);
	// XOR with subkey.
	block.c ^= key.c;
	block.d ^= key.d;
	// S-box transformations.
	block = sbox_transform(block);
	// P permutation.
	block = permute_p(block);
	// Convert back to a single 32-bit integer.
	return block.d | (block.c << 16);
}

// Do one of the 16 rounds of encryption.
des_block_t encode_round(des_block_t block, des_block_t key)
{
	uint32_t old_c = block.c;

	block.c = block.d;
	block.d = old_c ^ feistel(block.d, key);
	return block;
}

void desInitKey(DES_CTX *ctx, unsigned char *key)
{
	des_block_t		block;
	int		i, shift;

	block.c = block.d = 0;
	shift = 24;
	for(i = 0;i < 4;i++) {
		block.c |= key[i]   << shift;
        block.d |= key[i+4] << shift;
		shift -= 8;
	}
	generate_key_schedule(block, ctx->key_schedule);
}

static void _DesEncryptBlock(DES_CTX *ctx, unsigned char *in, unsigned char *out)
{
	des_block_t	block;
    uint32_t	tmp;
	int		i, shift;

	block.c = block.d = 0;
	shift = 24;
	for(i = 0;i < 4;i++) {
		block.c |= in[i]   << shift;
        block.d |= in[i+4] << shift;
		shift -= 8;
	}
	block = initial_permutation(block);
	for(i = 0;i < NUMSUBKEYS;i++)
		block = encode_round(block, ctx->key_schedule[i]);
	tmp = block.c;
	block.c = block.d;
	block.d = tmp;
	block = final_permutation(block);
	shift = 24;
	for(i = 0;i < 4;i++) {
		out[i]   = block.c >> shift;
		out[i+4] = block.d >> shift;
		shift -= 8;
    }
}

static void _DesDecryptBlock(DES_CTX *ctx, unsigned char *in, unsigned char *out)
{
	des_block_t	block;
    uint32_t	tmp;
	int		i, shift;

	block.c = block.d = 0;
	shift = 24;
	for(i = 0;i < 4;i++) {
		block.c |= in[i]   << shift;
        block.d |= in[i+4] << shift;
		shift -= 8;
	}
	block = initial_permutation(block);
	for(i = NUMSUBKEYS-1;i >= 0;i--)
		block = encode_round(block, ctx->key_schedule[i]);
	tmp = block.c;
	block.c = block.d;
	block.d = tmp;
	block = final_permutation(block);
	shift = 24;
	for(i = 0;i < 4;i++) {
		out[i]   = block.c >> shift;
		out[i+4] = block.d >> shift;
		shift -= 8;
    }
}

void desEncrypt(DES_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d;
	int		i;

	s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesEncryptBlock(ctx, s, d);
		s += 8; d += 8;
	}
}

void desDecrypt(DES_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d;
	int		i;

	s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesDecryptBlock(ctx, s, d);
		s += 8; d += 8;
	}
}

void desEncryptCBC(DES_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, *p, tmp[8];
	int		i, j;

	p = iv; s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		for(j = 0;j < 8;j++) tmp[i] = s[i] ^ p[i];
		_DesEncryptBlock(ctx, tmp, d);
		p = d; s += 8; d += 8;
	}
}

void desDecryptCBC(DES_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, *p;
	int		i, j;

	p = iv; s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesDecryptBlock(ctx, s, d);
		for(j = 0;j < 8;j++) d[i] ^= p[i];
		p = s; s += 8; d += 8;
	}
}

void des2InitKey(DES2_CTX *ctx, unsigned char *key)
{
	desInitKey(&ctx->_des_ctx[0], key);
	desInitKey(&ctx->_des_ctx[1], key+8);
}

void des2EncryptCBC(DES2_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, *p, tmp[8];
	int		i, j;

	p = iv; s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		for(j = 0;j < 8;j++) tmp[j] = s[j] ^ p[j];
		_DesEncryptBlock(&ctx->_des_ctx[0], tmp, d);
		_DesEncryptBlock(&ctx->_des_ctx[1], d, d);
		p = d; s += 8; d += 8;
	}
}

int des3InitKey(DES3_CTX *ctx, unsigned char *key, int keyLen)
{
	int		rval;

	rval = 0;
	if(keyLen == 8) {
		// This option provides backward compatibility with DES, because the
		// first and second DES operations cancel out.
		desInitKey(&ctx->_des_ctx[0], key);
		desInitKey(&ctx->_des_ctx[1], key);
		desInitKey(&ctx->_des_ctx[2], key);
	} else if(keyLen == 16) {
		// If the key length is 128 bits including parity, the first 8 bytes of the
		// encoding represent the key used for the two outer DES operations, and
		// the second 8 bytes represent the key used for the inner DES operation.
		desInitKey(&ctx->_des_ctx[0], key);
		desInitKey(&ctx->_des_ctx[1], key+8);
		desInitKey(&ctx->_des_ctx[2], key);
//printf("128-bit key...\n");
	} else if(keyLen == 24) {
		// If the key length is 192 bits including parity, then 3 independent DES
		// keys are represented, in the order in which they are used for encryption.
		desInitKey(&ctx->_des_ctx[0], key);
		desInitKey(&ctx->_des_ctx[1], key+8);
		desInitKey(&ctx->_des_ctx[2], key+16);
//printf("192-bit key...\n");
	} else {
		// The length of the key is not valid
		rval = -1;
    }
	return rval;
}
  
void des3Encrypt(DES3_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, tmp[8];
	int		i;

	s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesEncryptBlock(&ctx->_des_ctx[0], s, d);
		_DesDecryptBlock(&ctx->_des_ctx[1], d, tmp);
		_DesEncryptBlock(&ctx->_des_ctx[2], tmp, d);
		s += 8; d += 8;
	}
//printf("in [%02x", (int)in[0]); for(i = 1;i < 8;i++) printf("-%02x", (int)in[i]); printf("]\n");
}
  
void des3Decrypt(DES3_CTX *ctx, unsigned char *in, unsigned char *out, int blkLen)
 {
	unsigned char	*s, *d, tmp[8];
	int		i;

	s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesDecryptBlock(&ctx->_des_ctx[2], s, d);
		_DesEncryptBlock(&ctx->_des_ctx[1], d, tmp);
		_DesDecryptBlock(&ctx->_des_ctx[0], tmp, d);
		s += 8; d += 8;
	}
}

void des3EncryptCBC(DES3_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, *p, tmp[8];
	int		i, j;

	p = iv; s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		for(j = 0;j < 8;j++) tmp[j] = s[j] ^ p[j];
		_DesEncryptBlock(&ctx->_des_ctx[0], tmp, d);
		_DesDecryptBlock(&ctx->_des_ctx[1], d, tmp);
		_DesEncryptBlock(&ctx->_des_ctx[2], tmp, d);
		p = d; s += 8; d += 8;
	}
}

void des3DecryptCBC(DES3_CTX *ctx, unsigned char *iv, unsigned char *in, unsigned char *out, int blkLen)
{
	unsigned char	*s, *d, *p, tmp[8];
	int		i, j;

	p = iv; s = in; d = out;
	for(i = 0;i < blkLen;i++) {
		_DesDecryptBlock(&ctx->_des_ctx[2], s, d);
		_DesEncryptBlock(&ctx->_des_ctx[1], d, tmp);
		_DesDecryptBlock(&ctx->_des_ctx[0], tmp, d);
		for(j = 0;j < 8;j++) d[j] ^= p[j];
		p = s; s += 8; d += 8;
	}
}

