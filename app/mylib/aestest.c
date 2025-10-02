#include <stdio.h>
#include <string.h>
#include "aes.h"


unsigned char hynix_key[16] = {
0xc2,0x16,0x90,0xf5,0xc4,0x48,0xc4,0x60,0x82,0x74,0x87,0x47,0xe2,0x97,0x54,0x4c
};
unsigned char iv[16] = {
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,0x02,0x02,0x02,0x02
};


int main(void)
{
	struct AES_ctx	*ctx, _ctx;
	unsigned char	src[32], dst[32], tmp[32];
	int		i;

	for(i = 0;i < 32;i++) src[i] = i;
	AES_ECB_encrypt2(hynix_key, src, dst);
	AES_ECB_decrypt2(hynix_key, dst, tmp);
	for(i = 0;i < 16;i++) printf("%02x-", (int)tmp[i]); printf("\n");

	AES_CBC_encrypt2(hynix_key, iv, src, dst, 32);
	for(i = 0;i < 32;i++) printf("%02x-", (int)dst[i]); printf("\n");
	AES_CBC_decrypt2(hynix_key, iv, dst, tmp, 32);
	for(i = 0;i < 32;i++) printf("%02x-", (int)tmp[i]); printf("\n");

	ctx = &_ctx;
	AES_init_ctx_iv(ctx, hynix_key, iv);
	memcpy(tmp, src, 32);
	AES_CBC_encrypt_buffer(ctx, tmp, 32);
	for(i = 0;i < 32;i++) printf("%02x-", (int)tmp[i]); printf("\n");
	AES_CBC_decrypt_buffer(ctx, tmp, 32);
	for(i = 0;i < 32;i++) printf("%02x-", (int)tmp[i]); printf("\n");
	return 1;
}

