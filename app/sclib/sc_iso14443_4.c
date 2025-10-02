#include <stdio.h>
#include <string.h>
#include "as3911_def.h"
#include "as3911.h"
#include "emv_standard.h"
#include "sc_iso14443a.h"
#include "sc_iso14443_4.h"

#define ISO14443_CMD_DESELECT  0xca     // command DESELECT


int iso14443_4Deselect(int cid)
{
	unsigned char	buf[16];
	int		rval, len;

	buf[0] = ISO14443_CMD_DESELECT;
	buf[1] = (unsigned char)cid;
	rval = as3911TransceiveData(buf, 2, buf, 16, &len, EMV_FWT_ACTIVATION_PCD);	// len=1
//printf("DSELECT=%d\n", rval);
	return rval;
}

unsigned char	bitRate, fsci, sfgi, fwi;
 
int iso14443_4AnswerToSelect(int cid)
{
	unsigned char	*tx_byte, buf[64];	
	int		rval, rxlen, i, t0;

	buf[0] = 0xe0;
	buf[1] = (EMV_FSDI_MIN_PCD << 4) | cid;	// [7..4]FSDI=8(FSD=256) [3..0]CID=0 
	rval = as3911TransceiveData(buf, 2, buf, 32, &rxlen, EMV_FWT_ACTIVATION_PCD);
if(!rval) {
	//printf("AnswerToSelect=OK Len=%d\n", rxlen);
	//printf("[%02x", buf[0]); for(i = 1;i < rxlen;i++) printf("-%02x", buf[i]); printf("]\n");
} else {
	printf("AnswerToSelect=0x%x\n", rval);	
}
	if(!rval) {
		if(rxlen < 1) return ERR_PROTOCOL;
		// Check TL (length byte) for correctness
		if(buf[0] != rxlen) return ERR_PROTOCOL;
		// Check for the presence of the T0 byte. If no T0 byte is present use a default T0 byte which specifies no TA,
		// TB, and TC bytes and the default FSCI value to be used.
		if(buf[0] == 1) t0 = EMV_FSCI_DEFAULT;
		else	t0 = buf[1];
		// Set PICC struct default values
		bitRate = 0;	// dPiccPcd = EMV_D_PICC_PCD_DEFAULT; dPcdPicc = EMV_D_PCD_PICC_DEFAULT;
		sfgi = EMV_SFGI_DEFAULT;
		fwi = EMV_FWI_DEFAULT;
		// Parse and check FSCI
		fsci = t0 & 0x0f;
		if(fsci < EMV_FSCI_MIN_PCD) return ERR_PROTOCOL;
		else if(fsci > 8) fsci = 8;
		tx_byte = buf + 2;
		// Parse TA(1) (datarate flags)
		if(t0 & 0x10) bitRate = *tx_byte++;
		//  Parse TB(1) (FWI, SFGI)/
		if(t0 & 0x20) {
			sfgi = *tx_byte & 0x0F;
			fwi = *tx_byte >> 4;
			if(sfgi > EMV_SFGI_MAX_PCD) sfgi = EMV_SFGI_DEFAULT;
			if(fwi > EMV_FWI_MAX_PCD) fwi = EMV_FWI_DEFAULT;
			tx_byte++;
		}
printf("FSCI=%d SFGI=%d FWI=%d\n", fsci, sfgi, fwi);
		emvInitLayer4();
	}
	return rval;
}

int iso14443_4PPS(int cid)
{
	unsigned char	buf[32];	
	int		rval, rxlen, brate;

	buf[0] = 0xd0 | cid; buf[1] = 0x11; 
	rval = bitRate & (bitRate >> 4);
	if(rval & 0x04) brate = 0x03;
	else if(rval & 0x02) brate = 0x02;
	else if(rval & 0x01) brate = 0x01;
	else	brate = 0x00;
//printf("bitRate=%02x\n", brate);
brate = 0;
	if(brate) {
		brate |= brate << 2;
		buf[2] = brate;
//printf("bitRate=%02x %02x\n", (int)bitRate, brate);
		rval = as3911TransceiveData(buf, 3, buf, 32, &rxlen, EMV_FWT_ACTIVATION_PCD);
//if(!rval) printf("PPS: OK Len=%d\n", rxlen);
//else	printf("PPS: error=%d\n", rval);	
		if(!rval) iso14443ASetBitRate(brate);
	} else	rval = 0;
	return rval;
}

#define MAX_EMV_RETRY		8	// 2

int _EmvTranceiveRetryApdu(unsigned char *txBuf, int txLen, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	int		rval, retry, i, len;

 	for(retry = 0;retry < MAX_EMV_RETRY; ) {
		rval = emvTransceiveApdu(txBuf, txLen, rxBuf, 256, &len);
		if(!rval) {
			if((len == 2 || len == 4) && rxBuf[0] >= 0x61 && rxBuf[0] <= 0x6f) rval = ERR_EMV_STATUS;
			else if(maxLen < 0) {
				i = -maxLen;
				if(len < i) rval = ERR_EMV_PROTOCOL;
				else if(rxBuf[len-2] != 0x90 || rxBuf[len-1] != 0x00) rval = ERR_EMV_PROTOCOL;
			} else if(maxLen < 3) {
				if((len != 2 && len != 4) || rxBuf[len-2] != 0x90 || rxBuf[len-1] != 0x00) rval = ERR_EMV_PROTOCOL;
			} else {				
				if(len == maxLen+2 && rxBuf[len-4] == 0x90 && rxBuf[len-3] == 0x00) len -= 2;
				if(len != maxLen || rxBuf[len-2] != 0x90 || rxBuf[len-1] != 0x00) rval = ERR_EMV_PROTOCOL;
			}
			if(rval) {
				if(rval == ERR_EMV_PROTOCOL) 
					printf("emvTransc(0x%02x) protocol error: %d [%02x", (int)txBuf[1], len, (int)rxBuf[0]);
				else
					printf("emvTransc(0x%02x) status byte: %d [%02x", (int)txBuf[1], len, (int)rxBuf[0]);
				for(i = 1;i < len;i++) printf("-%02x", (int)rxBuf[i]);
				printf("]\n");
			}
		} else {
			printf("emvTransc(0x%02x): rval=%d\n", (int)txBuf[1], rval);
		}
		if(!rval) {
			*rxLen = len;
			break;
		} else if(rval == ERR_CHIP || rval == ERR_EMV_STATUS) break;
		else if(rval == ERR_TIMEOUT) retry += 4;
		else	retry++;
	}
	return rval;
}

int iso14443_4SelectMF(unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[16];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xa4;					// INS	SELECT
	*p++ = 0x00;					// P1	Reference control parameter P1(04=Select by name) 
	*p++ = 0x00;					// P2	Reference control parameter P2(00=First or only occurence)
	*p++ = 0x02;					// Lc	Length of FID
	*p++ = 0x3f;
	*p++ = 0x00;
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4SelectDF(unsigned char *aid, int len, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];	
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xa4;					// INS	SELECT
	*p++ = 0x04;					// P1	00000100 Select by name
	*p++ = 0x00;					// P2	00000000 First or only occurence
	*p++ = (unsigned char)len;		// Lc	Length of AID
	memcpy(p, aid, len); p += len;	// Data	AID of Application to be selected 
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4SelectFile(unsigned short fileId, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[16];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xa4;					// INS	SELECT
	*p++ = 0x02;					// P1	Reference control parameter P1(04=Select by name) 
	*p++ = 0x0c;					// P2	Reference control parameter P2(00=First or only occurence)
	*p++ = 0x02;					// Lc	Length of FID
	*p++ = fileId >> 8;
	*p++ = fileId; 
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4GetData(unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xca;					// INS	GET DATA
	*p++ = 0x00;					// P1	00 
	*p++ = 0x01;					// P2	01:CSN 02:Applet Information
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4GetData2(unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xca;					// INS	GET DATA
	*p++ = 0x01;					// P1	00 
	*p++ = 0x01;					// P2	01:CSN 02:Applet Information
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4GetChallenge(unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0x84;					// INS	GET CHALLENGE
	*p++ = 0x00;					// P1	00 
	*p++ = 0x00;					// P2	00
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4CreateSession(int keyNum, unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	old=0x80
	*p++ = 0x8a;					// INS	CREATE SESSION
	*p++ = 0x00;					// P1	00 
	*p++ = keyNum;					// P2	Reference key number(00..02)
	*p++ = (unsigned char)len;		// Lc	Length of Random Number
	memcpy(p, rnd, len); p += len;	// Data	Random Number
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4CreateSession2(int keyNum, unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen)
{
    unsigned char   *p, buf[64];
    int     rval;

    p = buf;
    *p++ = 0x80;                    // CLA  ISO/IEC 7816-4 command  old=0x80
    *p++ = 0x8a;                    // INS  CREATE SESSION
    *p++ = 0x00;                    // P1   00
    *p++ = keyNum;                  // P2   Reference key number(00..02)
    *p++ = (unsigned char)len;      // Lc   Length of Random Number
    memcpy(p, rnd, len); p += len;  // Data Random Number
    *p++ = maxLen-2;                // Le
    rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
    return rval;
}

int iso14443_4InternalAuthenticate(unsigned char *rnd, int len, unsigned char *rxBuf, int maxLen, int *rxLen)
{
    unsigned char   *p, buf[64];
    int     rval;

    p = buf;
    *p++ = 0x00;                    // CLA  ISO/IEC 7816-4 command
    *p++ = 0x88;                    // INS  INTERNAL AUTHENTICATION
    *p++ = 0x00;                    // P1   00
	*p++ = 0x00;					// P2	Reference key number(00..02)
    *p++ = len;     				// Lc   Length of Random Number
    memcpy(p, rnd, len); p += len;	// Data Random Number 
    *p++ = maxLen-2;				// Le
 	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
   return rval;
}

int iso14443_4ExternalAuthenticate(unsigned char *auth, int len, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[64];	
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0x82;					// INS	EXTERNAL AUTHENTICATION
	*p++ = 0x00;					// P1	00
	*p++ = 0x00;					// P2	Reference key number(00..02)
	*p++ = len;						// Lc	lengthh of auth
	memcpy(p, auth, len); p += len;	// Data	auth 
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4ReadBinary(int SFI, int offset, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[260];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xb0;					// INS	READ BINARY
	*p++ = 0x80 + SFI;				// P1	100xxxxx SFI(short EF identifier) 
	*p++ = offset;					// P2	xxxxxxxx offset
	*p++ = maxLen-2;				// Le	
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4ReadBinary2(int SFI, int offset, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[260];
	int		rval;

 	p = buf;
	*p++ = 0xa4;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xb0;					// INS	READ BINARY
	*p++ = 0x80 + SFI;				// P1	100xxxxx SFI(short EF identifier) 
	*p++ = offset;					// P2	xxxxxxxx offset
	*p++ = maxLen-2;				// Le	
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4SecureReadBinary(int offset, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[260];
	int		rval;

 	p = buf;
	*p++ = 0x90;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xb0;					// INS	READ BINARY
	*p++ = 0x00;					// P1	100xxxxx SFI(short EF identifier) 
	*p++ = offset;					// P2	xxxxxxxx offset
	*p++ = maxLen-2;				// Le	
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4ReadRecord(int sfi, int recordNumber, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[260];
	int		rval;

 	p = buf;
	*p++ = 0x00;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xb2;					// INS	READ RECORD
	*p++ = recordNumber;			// P1	Record number
	*p++ = (sfi << 3) + 0x04;		// P2	xxxxx100  SFI(short EF identifier) 
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

int iso14443_4ReadRecord2(int sfi, int recordNumber, unsigned char *rxBuf, int maxLen, int *rxLen)
{
	unsigned char	*p, buf[260];
	int		rval;

 	p = buf;
	*p++ = 0xa4;					// CLA	ISO/IEC 7816-4 command	
	*p++ = 0xb2;					// INS	READ RECORD
	*p++ = recordNumber;			// P1	Record number
	*p++ = (sfi << 3) + 0x04;		// P2	xxxxx100  SFI(short EF identifier) 
	*p++ = maxLen-2;				// Le
	rval = _EmvTranceiveRetryApdu(buf, p-buf, rxBuf, maxLen, rxLen);
	return rval;
}

