#include <stdio.h>
#include <string.h>
#include "log.h"
#include "as3911_def.h"
#include "as3911.h"
#include "as3911_io.h"
#include "as3911_interrupt.h"
#include "sc_iso14443a.h"
#include "sc_mifare_crypto1.h"
#include "sc_mifare_crc.h"
#include "sc_mifare_parity.h"
#include "sc_mifare.h"

/*! Reader nonce used by the MiFare single step authentication code. */
#define MIFARE_DEFAULT_READER_NONCE				0xaa55aa55

/*! Timeout for mifare authentication step 1 in milliseconds. */
#define MIFARE_AUTHENTICATION_STEP1_TIMEOUT		5

/*! Timeout for mifare authentication step 2 in milliseconds. */
#define MIFARE_AUTHENTICATION_STEP2_TIMEOUT		5

/*! Global crypto1 cipher state object. */
static crypto1_context mifareCipherContext;

/*!
 *****************************************************************************
 * If \a mifareCipherActive is \a TRUE, then a block authentication has been
 * successfully performed and all data transmission will be encrypted.
 *****************************************************************************
 */
static unsigned char	mifareCipherActive;

void _MifareSetKey(const unsigned char *key)
{
	uint64emu_storage_t	corrected_key;
	int		i;

	crypto1_new(&mifareCipherContext, CRYPTO1_CIPHER_READER, CRYPTO1_IMPLEMENTATION_CLEAN);
	for(i = 0;i < 6;i++) uint64emu_setbyte(&corrected_key, 5 - i, key[i]);
	crypto1_init(&mifareCipherContext, &corrected_key);
}

void _MifareResetCipher(void)
{
    memset(&mifareCipherContext, 0, sizeof(mifareCipherContext));
    mifareCipherActive = FALSE;
}

void mifareOpen(void)
{
	_MifareResetCipher();
}

void mifareClose(void)
{
	_MifareResetCipher();
}

int mifareCiplerActive(void)
{
    if(mifareCipherActive) return 1;
    else    return 0;
}

int _MifareTranceiveRaw(unsigned short *req, int reqLen, unsigned short *rsp, int maxRspLen, int *rspLen, int timeout, bool_t fourBitResponse);

int _MifareAuthenticateStep1(int auth_mode, unsigned char *key, unsigned char *uid, int addr)
{
	unsigned char	authbuf[8], cipherActive;
    unsigned short	crc, buf[32];
    unsigned long	uid_as_u32, tag_nonce;
	int		i, rval, len;

	authbuf[0] = auth_mode;
    authbuf[1] = addr;
	crc = crcCalculateCcitt(0x6363, authbuf, 2);
	authbuf[2] = crc & 0xff;
    authbuf[3] = (crc >> 8) & 0xff;
 	for(i = 0;i < 4;i++) buf[i] = authbuf[i];
	calculateParity(buf, 4);
	if(uid[0] < 4) return ERR_PARAM;
	cipherActive = mifareCipherActive;
	if(cipherActive) crypto1_transcrypt(&mifareCipherContext, buf, 4);
	rval = _MifareTranceiveRaw(buf, 4, buf, 5, &len, MIFARE_AUTHENTICATION_STEP1_TIMEOUT, FALSE);
	if(rval == 0) {
    	if(len != 4) {
//if(len == 0) printf("AuthStep1 Reveiced ACK/NAK(4-bit) [%02x]\n", buf[0]);
//else	printf("AuthStep1 Reveiced %d bytes != 4\n", len);
			_MifareResetCipher();
            return ERR_NOTFOUND;
        }
		for(i = 0;i < 4;i++) authbuf[i] = buf[i];
    	uid_as_u32 = ARRAY_TO_UINT32(uid+1);
    	tag_nonce = ARRAY_TO_UINT32(authbuf); 
    	_MifareResetCipher();
    	_MifareSetKey(key);
		if(cipherActive) {
			crypto1_clean_mutual_1_2(&mifareCipherContext, uid_as_u32, tag_nonce);
		} else {
			crypto1_mutual_1(&mifareCipherContext, uid_as_u32, tag_nonce);
		}
#ifdef _SC_DEBUG
printf("AuthStep1 OK\n");
#endif
    } else {
printf("AuthStep1 error=%d\n", rval);
    }
    return rval;
}

static unsigned long	rdr;

int _MifareAuthenticateStep2()
{
	unsigned short	authcmd[8], buf[32];
	unsigned long readerNonce;
	int		i, rval, len;

	readerNonce = MIFARE_DEFAULT_READER_NONCE;
	rdr++;
	readerNonce ^= rdr;
	UINT32_TO_ARRAY_WITH_PARITY(readerNonce, authcmd);
	crypto1_mutual_2(&mifareCipherContext, authcmd);
	rval = _MifareTranceiveRaw(authcmd, 8, buf, 5, &len, MIFARE_AUTHENTICATION_STEP2_TIMEOUT, FALSE);
    // if key mismatch, return timeout=1
	if(rval == 0) {
    	if(len != 4) {
//if(len == 0) printf("AuthStep2 Reveiced ACK/NAK(4-bit) [%02x]\n", buf[0]);
//else	printf("AuthStep2 Reveiced %d bytes != 4\n", len);
        	_MifareResetCipher();
        	return ERR_NOTFOUND;
        }
       for(i = 0;i < 5;i++) buf[i] &= 0x00ff;
    	if(!crypto1_mutual_3(&mifareCipherContext, buf)) {
printf("AuthStep2 Invalid card response. Abort\n");
        	_MifareResetCipher();
            return ERR_NOMSG;
    	}
    	mifareCipherActive = TRUE;
#ifdef _SC_DEBUG 
printf("AuthStep2 OK...\n");
#endif
    } else {
printf("AuthStep2 error=%d\n", rval);        
    }
    return rval;
}

// uid : uid[0]=len uid[1..]=uid
int mifareAuthenticate(int auth_mode, unsigned char *key, unsigned char *uid, int addr)
{
 	int		rval;

	rval = _MifareAuthenticateStep1(auth_mode, key, uid, addr);
    if(!rval) rval = _MifareAuthenticateStep2();
//printf("authenticate=%d addr=%d\n", rval, addr);
	return rval;
}

int _MifareTranceive(unsigned char *req, int reqLen, unsigned char *rsp, int maxRspLen, int *rspLen, int timeout, bool_t fourBitResponse)
{
	unsigned short	crc, buf[MAX_MIFARE_TX_BUF_SZ];
	int		i, err, len;

	crc = crcCalculateCcitt(0x6363, req, reqLen);
	for(i = 0;i < reqLen;i++) buf[i] = req[i];
	buf[reqLen] = (u8)(crc & 0xff);
    buf[reqLen+1] = (u8)((crc>>8) & 0xff);
	calculateParity(buf, reqLen+2);
	if(mifareCipherActive) crypto1_transcrypt(&mifareCipherContext, buf, reqLen+2);
    err = _MifareTranceiveRaw(buf, reqLen+2, buf, maxRspLen, rspLen, timeout, fourBitResponse);
 	if(err) return err;
	len = *rspLen;
	// Decrypt message in transceive buffer if cipher is in use.
    if(mifareCipherActive) {
		// If a response with a length of 0, it is asumed that this actually was an ACK,NACK and 4 bits are fed into the cipher
        if(len == 0) {
			crypto1_transcrypt_bits(&mifareCipherContext, buf, 0, 4);
 		} else {
			crypto1_transcrypt_bits(&mifareCipherContext, buf, len, 0);
		}
	}
    if(len == 0) rsp[0] = buf[0];
 	else if(checkParity(buf, len)) {
printf("_MifareTranceive PARYTY error......\n");
		err = ERR_PARITY;
	} else {
		for(i = 0;i < len;i++) rsp[i] = (unsigned char)buf[i];
	}
	return err;
}

//  Timeout of mifare read command in milliseconds.
#define MIFARE_READ_TIMEOUT             5
// Timeout of mifare write command request transmission part in milliseconds.
#define MIFARE_WRITE_COMMAND_TIMEOUT    5
// Timeout of mifare write command data transmission part in milliseconds.
#define MIFARE_WRITE_DATA_TIMEOUT       10

int mifareReadBlock(int addr, unsigned char *buf)
{
	unsigned char	req[8], rsp[20];
	unsigned short	crc, rx_crc;
	int		rval, len;

	req[0] = MIFARE_READ_BLOCK;
	req[1] = addr;
	rval = _MifareTranceive(req, 2, rsp, 18, &len, MIFARE_READ_TIMEOUT, FALSE);
	if(!rval) {
		if(len == 16) {
			memcpy(buf, rsp, 16);
		} else if(len == 18) {
			crc = crcCalculateCcitt(0x6363, rsp, 16);
			rx_crc = rsp[16];
			rx_crc |= rsp[17] << 8;
			if(crc == rx_crc) memcpy(buf, rsp, 16);
			else {
printf("mifareReadBlock: CRC error......\n");
				rval = ERR_CRC;
			}
		} else {
			rval = ERR_PARAM;
		}
    }
	return rval;
}

int mifareWriteBlock(int addr, unsigned char *buf)
{
	unsigned char	req[8], rsp[20];
 	int		rval, len;

	req[0] = MIFARE_WRITE_BLOCK;
	req[1] = addr;
	rval = _MifareTranceive(req, 2, rsp, 1, &len, MIFARE_WRITE_COMMAND_TIMEOUT, TRUE);
	if(!rval) {
        if(len) rval = ERR_PARAM;
        else if(rsp[0] != 0xa) rval = ERR_PARAM; 
    }
    if(!rval) {
		// 6ms
    	rval = _MifareTranceive(buf, 16, rsp, 18, &len, MIFARE_WRITE_DATA_TIMEOUT, FALSE);
    	if(!rval) {
            if(len) rval = ERR_PARAM;
            else if(rsp[0] != 0xa) rval = ERR_PARAM; 
        }
	}
    return rval;
}

void mifareHalt(void)
{
	unsigned char	cmd[4];
    unsigned short	crc, buf[8];
	int		rval, i, len;

	cmd[0] = ISO14443A_CMD_HLTA; cmd[1] = 0;
	crc = crcCalculateCcitt(0x6363, cmd, 2);
	cmd[2] = crc & 0xff;
    cmd[3] = (crc >> 8) & 0xff;
 	for(i = 0;i < 4;i++) buf[i] = cmd[i];
	calculateParity(buf, 4);
	crypto1_transcrypt(&mifareCipherContext, buf, 4);
	rval = _MifareTranceiveRaw(buf, 4, buf, 5, &len, MIFARE_AUTHENTICATION_STEP1_TIMEOUT, FALSE);
//	else	rval = ERR_NOMSG;
	mifareCipherActive = FALSE;
#ifdef _SC_DEBUG 
printf("mifareHalt=%d\n", rval);
#endif
}

static void _ParData2RawData(u16 *src, u8 *dst, int length)
{
	u16		c;
	u8		mask;
	int		i, bytep, bitp;

  	bytep = bitp = 0; mask = 0xff;
	dst[0] = 0;
	for(i = 0;i < length;i++)  {
		c = src[i];
		dst[bytep] |= (c & mask) << bitp;
		bytep++;
		dst[bytep] = (c >> (8-bitp));
		bitp++; mask >>= 1;
		if(bitp > 7) {
             bytep++; bitp = 0; mask = 0xff;
			 dst[bytep] = 0;
        }
    }
}

static void _RawData2ParData(u8 *src, u16 *dst, int length)
{
	u16		c;
 	int		i, bytep, bitp;

	bytep = bitp = 0;
	for(i = 0;i < length;i++) {
		c = (src[bytep] >> bitp);
		bytep++;
		c |= (src[bytep] << (8-bitp));
		dst[i] = c;
		bitp++;
		if(bitp > 7) {
			bitp = 0; bytep++;
		}
	}
}
#include "rtc.h"
// Return:	0
//				rspLen = 0  : Received 4 bits(ACK,NAK)
//				rspLen > 0	: Received rspLen * 9bits(8bits + 1 Parity bit)
int _MifareTranceiveRaw(unsigned short *req, int reqLen, unsigned short *rsp, int maxRspLen, int *rspLen, int timeout, bool_t fourBitResponse)
{
	unsigned char	buf[MAX_MIFARE_TX_RAWBUF_SZ];
	int		err, len, tmo;

    as3911ExecuteCommand(AS3911_CMD_MASK_RECEIVE_DATA);	// Receive after this command is ignored
	_ParData2RawData(req, buf, reqLen);
	tmo = timeout * 13564;
	err = as3911TransceiveRawBitData(buf, reqLen*9, buf, MAX_MIFARE_TX_BUF_SZ*9, &len, tmo);
if(err)	printf("_MifareTranceiveRaw: as3911TranceiveRawBitData=%d\n", err);
 	if(err) return err;
	if(len == 4) {
		rsp[0] = buf[0]; *rspLen = 0;
printf("_MifareTranceiveRaw: BitLen=%d ACK/NAK\n", len);
	} else if(len > 8 && !(len % 9)) {
		len /= 9;
		if(len > maxRspLen) err = ERR_PROTOCOL;
		else {
			_RawData2ParData(buf, rsp, len); *rspLen = len;
printf("_MifareTranceiveRaw: ByteLen=%d, %d\n", len, len);
		}
	} else {
printf("_MifareTranceiveRaw: BitLen=%d isn't multiples of 9\n", len);
		err = ERR_PROTOCOL;
	}
	return err;
}

int mifareRead(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *key)
{
	unsigned char	*p;
	int		rval, blk, blk_max, cnt;
   
//printf("mifareRead: block=%d len=%d\n", block, size);
	blk = block;
	p = buf; cnt = size;
	while(cnt > 0) {
		rval = mifareAuthenticate(MIFARE_AUTH_KEY_A, key, uid, blk);
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
			rval = mifareReadBlock(blk, p);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		blk++;
	}
	return rval;
}

int mifareWrite(int block, unsigned char *buf, int size, unsigned char *uid, unsigned char *key)
{
	unsigned char	*p;
	int		rval, blk, blk_max, cnt;
   
 //printf("mifareWrite: block=%d len=%d\n", block, size);
	blk = block;
	p = buf; cnt = size;
	while(cnt > 0) {
		rval = mifareAuthenticate(MIFARE_AUTH_KEY_A, key, uid, blk);
		if(rval) break;
		blk_max = (blk & 0xfc) + 0x03;
		for( ;blk < blk_max;blk++) {
            if(cnt < 16) memset(p+cnt, 0xff, 16-cnt);
			rval = mifareWriteBlock(blk, p);
			if(rval) break;
			p += 16; cnt -= 16;
			if(cnt <= 0) break;
		}
		if(rval) break;
		blk++;
	}
	return rval;
}
