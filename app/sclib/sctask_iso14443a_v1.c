#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "sc_iso14443a.h"
#include "sc_iso14443_4.h"
#include "sc_mifare.h"
#include "cardcrypto.h"
#include "sccfg.h"
#include "sctask_com.h"

static unsigned char  scScore, scRetry, sak, uid[12];


static void _scaToFatal(void)
{
printf("Fatal error...........\n");    
	scState = SC_FATAL;
}

static void _scaToOff(void)
{
	scState = SC_OFF;
}

static void _scaToIdleNoWait(void)
{
	scState = SC_IDLE; scTimer = SC_TIMER;
}

static void _scaToIdle(void)
{
	scState = SC_IDLE; scTimer = SC_TIMER; usleep(10000);
}

 static void _scaError(void)
{
	iso14443AToIdle();		// set PICC to idle state	iso14443AHalt()
	_scaToIdle(); uid[0] = 0;
}

static void _scaHalt(void)
{
	if(scState >= SC_ACTIVE) iso14443_4Deselect(0);
	else if(scState == SC_READY && mifareCiplerActive()) mifareHalt();
	else    iso14443AHalt();    // set PICC to halt state
	if(scDataIsFull()) _scaToOff();
	else {
		_scaToIdle(); uid[0] = 0;    
	}
}

unsigned char hynix_aid[9] = { 0xd0,0x00,0x53,0x4b,0x20,0x43,0x41,0x50,0x53 };
unsigned char hynix_key[16] = {
//0xc2,0x16,0x90,0xf5,0xc4,0x48,0xc4,0x60,0x82,0x74,0x87,0x47,0xe2,0x97,0x54,0x4c
0xca,0xf9,0x63,0xdf,0xc9,0x6c,0xb9,0x23,0x5b,0x78,0x50,0x04,0x61,0x6d,0x58,0x7c
};

unsigned char aid_transport[8]	= { 0xa0, 0x00, 0x00, 0x04, 0x52, 0x00, 0x01, 0x00 };	// 전국호환교통카드
unsigned char hynix_blkkey[6]	= { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
unsigned char norm_blkkey[6]	= { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };  
int  _Iso14443_4Task(void);
int  _SkSamTask(void);

void iso14443AActiveTask(void)
{
	unsigned char	*p, buf[320], temp[16];
    int     rval, len;

	switch(scState) {
	case SC_OFF:
		if(!scDataIsFull()) {
			_scaToIdleNoWait();
			if(scOpenMode != SC_ISO14443A) {
				iso14443AOpen(); scOpenMode = SC_ISO14443A;
				usleep(150);	// stable time( >=1ms OK)
			}
		}
		scCount = scScore = 0; uid[0] = 0;
		break;
	case SC_IDLE:
		iso14443ASendRequest(ISO14443A_CMD_REQA);
		scState = SC_REQUEST;
		break;
	case SC_REQUEST:
		rval = iso14443AReceiveRequest(buf);
		if(rval == ERR_CHIP) {
			_scaToFatal();
		} else if(rval == ERR_TIMEOUT) {
//printf("ReceiveRequest %d-%d: Timeout\n", (int)scCount, (int)scScore);
			if(scCount) {
				scScore += 2;
				if(scScore < 4) _scaToIdle();
				else	_scaToOff();
			} else	_scaToOff();
		} else if(rval >= 0) {
//printf("ReceiveRequest %d-%d: %d\n", (int)scCount, (int)scScore, rval);
			scScore = 0; scState = SC_ANTICOLLSION;
		}
		break;
   case SC_ANTICOLLSION:
		if(uid[0] && uid[1] != 0x08) {
			rval = iso14443ASelect(uid, &sak);
			if(rval) _scaError();
			else {
#ifdef _SC_DEBUG 
printf("%d SEL UID=[%02x", (int)scRetry, (int)uid[1]); for(rval = 1;rval < uid[0];rval++) printf("-%02x", uid[rval+1]); printf("]\n");
#endif
				if(scRetry < 3) scState = SC_READY;
				else    _scaHalt();
			}
		} else {
			scCount++;
			rval = iso14443AActivate(uid, &sak);
			if(rval) _scaError();
			else {
#ifdef _SC_DEBUG 
printf("ACT UID=[%02x", (int)uid[1]); for(rval = 1;rval < uid[0];rval++) printf("-%02x", uid[rval+1]); printf("]\n");
#endif
				if(scReaderFormat == 0) {
					scAddData(uid+1, (int)uid[0]);
					_scaHalt();
				} else {
					scRetry = 0; scState = SC_READY;
				}
			}
		}
		break;
	case SC_READY:
		if(scReaderFormat == 1 && (scCardApp == 1 && !(sak & COMPLIANT_WITH_ISO14443_4) || scCardApp != 1)) {
			if(scCardApp == 1) {
				CryptoDiversifyKey(uid+1, temp); p = temp;
				len = 16;
			} else	{
				p = CrMifareKey();
				len = CrBlockDataLength();
				if(len < 1) len = 16;
			}
			mifareOpen();
//printf("mifareRead: %d key=%02x-%02x-%02x-%02x-%02x-%02x\n", CrBlockNumber(), (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
			rval = mifareRead(CrBlockNumber(), buf, len, uid, p);
			if(rval) {
				scRetry++; mifareClose(); _scaToIdle();
			} else {
				if(scCardApp == 1) {
					rval = CryptoDecode(buf, uid+1, temp);
					if(rval) len = 0; else len = 8;
					p = temp;
#ifdef _SC_DEBUG 
if(rval) printf("CryptoDecode error\n");
else { printf("Hynix [%02x", (int)p[0]); for(rval = 1;rval < len;rval++) printf("-%02x", p[rval]); printf("]\n"); }
#endif
				} else {
					p = buf;
#ifdef _SC_DEBUG 
printf("block=%d len=%d [%02x", CrBlockNumber(), len, (int)p[0]); for(rval = 1;rval < len;rval++) printf("-%02x", p[rval]); printf("]\n");
#endif
				}
				if(len) scAddData(p, len);
				_scaHalt();
			}
		} else {
			if(sak & COMPLIANT_WITH_ISO14443_4) {
				rval = iso14443_4AnswerToSelect(0);
				if(!rval) rval = iso14443_4PPS(0);
				if(rval) _scaError();
				else {
					scSubState = 0; scState = SC_ACTIVE;
				}
			} else {
				_scaHalt();
			}
		}
		break;
	case SC_ACTIVE:
//		if(scCardApp == 2) rval = _SkSamTask();
		rval = _Iso14443_4Task();
printf("### iso14443_4Task=%d\n", rval);
		if(scReaderFormat == 1 && scCardApp == 1) _scaHalt();
		else if(!rval) _scaHalt();
		else if(rval > 0) _scaToIdle();
		else    scSubState++;
		break;     
	}
}

void iso14443AExitTask(void)
{
	unsigned char	buf[12];
	int		rval;

	switch(scState) {
	case SC_OFF:
//printf("ExitTask\n");
		_scaToIdleNoWait();
		if(scOpenMode != SC_ISO14443A) {
			iso14443AOpen(); scOpenMode = SC_ISO14443A;
			usleep(150);	// stable time( >=1ms OK)
		}
		scScore = 0;
		break;
	case SC_IDLE:
		iso14443ASendRequest(ISO14443A_CMD_WUPA);
		scState = SC_REQUEST;
		break;
	case SC_REQUEST:
		rval = iso14443AReceiveRequest(buf);
		if(rval == ERR_CHIP) {
			_scaToFatal();
		} else if(rval == ERR_TIMEOUT) {
printf("Exit ReceiveRequest: Timeout\n");
			scScore += 2;
			if(scScore < 4) _scaToIdle();
			else	_scaToOff();
		} else if(rval >= 0) {			
			iso14443AToIdle();		// set PICC to idle state	iso14443AHalt()
			_scaToIdle(); scScore = 0;
		}
		break;
	}
}

void prtkey(char *msg, unsigned char *buf)
{
	int		i;

	printf("%s [%02x", msg, (int)buf[0]);
	for(i = 1;i < 16;i++) printf("-%02x", (int)buf[i]);
	printf("]\n");
}

void prtsc(char *msg, int rval, unsigned char *buf, int len)
{
	int		i;

	if(rval) printf("%d: %d\n", msg, rval);
	else {
		printf("%s %d [%02x", msg, len, (int)buf[0]);
		for(i = 1;i < len;i++) printf("-%02x", (int)buf[i]);
		printf("]\n");
	}
}

#include "aes.h"
struct AES_ctx		*ctx, _ctx;

int _Iso14443_4Task(void)
{
	unsigned char	*p, buf[300], temp[20], r1[16], r2[16], k[16], dk[16], sk[16];
    int     rval, n, len, doff, dlen, sfi;

	if(scReaderFormat == 1 && scCardApp == 1) {	// Hynix card
		ctx = &_ctx;
   		rval = iso14443_4SelectDF(hynix_aid, 9, buf, &len);
 prtsc("SelectDF", rval, buf, len);
        if(rval || len < 12 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
		rval = iso14443_4GetData(8, buf, &len);
 prtsc("GetData", rval, buf, len);
        if(rval || len < 10 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
		memcpy(dk, buf, 8);
		for(n = 0;n < 8;n++) dk[8+n] = ~buf[n];
//dk[8]=0x29; dk[9]=0xaa; dk[10]=0xec; dk[11]=0x1e; dk[12]=0x92; dk[13]=0x2e; dk[14]=0x46; dk[15]=0x96;
prtkey("csn", dk);
		AES_init_ctx(ctx, hynix_key);
		AES_ECB_encrypt(ctx, dk);
prtkey("DK", dk);
		rval = iso14443_4GetChallenge(16, buf, &len);
 prtsc("GetChanllenge", rval, buf, len);
        if(rval || len < 18 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
		memcpy(r2, buf, 16);
	 	memrand(r1, 16);
prtkey("R1", r1);
		len = 16;
		rval = iso14443_4InternalAuthenticate(r1, 16, buf, &len);
 prtsc("InternalAuth", rval, buf, len);
        if(rval || len < 18 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
		AES_init_ctx(ctx, dk);
		memcpy(k, r1, 16);
		AES_ECB_encrypt(ctx, k);
		AES_init_ctx(ctx, k);
		memcpy(sk, r2, 16);
		AES_ECB_encrypt(ctx, sk);
prtkey("SK", sk);
		AES_init_ctx(ctx, sk);
		memcpy(k, dk, 16);
		AES_ECB_encrypt(ctx, k);
prtkey("K", k);
		AES_init_ctx(ctx, k);
		AES_ECB_encrypt(ctx, r1);
		if(memcmp(r1, buf, 16)) {
printf("Authentication error\n");
			return -1;
		}
printf("Authentication OK...\n");
		AES_ECB_encrypt(ctx, r2);
		rval = iso14443_4ExternalAuthenticate(r2, 16, buf, &len);
prtsc("ExternalAuth", rval, buf, len);
        if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
		rval = iso14443_4ReadRecord(1, 10, 16, buf, &len);
        if(rval || len < 18 || buf[len-2] != 0x90 || buf[len-1] != 0x00) return -1;
prtsc("ReadRecord", rval, buf, len);
		AES_init_ctx(ctx, sk);
		AES_ECB_decrypt(ctx, buf);
prtkey("Final", buf);
		scAddData(buf, 8);
	} else if(scCardApp == 1) {  // transport card
        rval = iso14443_4SelectDF(aid_transport, 7, buf, &len);
   		p = temp;
        if(!rval && len > 15 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
			dlen = scExtractTransportCardNumber(buf, len-2, p);
        } else if(!rval) {
   			rval = iso14443_4SelectDF(CrAID(), CrAIDLength(), buf, &len);
       		if(!rval && len > 15 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
           		dlen = scExtractSpecialTransportCardNumber(buf, len-2, p);
            } else if(!rval) dlen = 0;
        }
     } else {
        switch(scSubState) {
        case 0:
            if(CrIso7816EnableSelectMF()) { 
                rval = iso14443_4SelectMF(buf, &len);
                if(!rval && len > 1 && buf[len-2] == 0x90 && buf[len-1] == 0x00) rval = -1;
                else if(!rval) dlen = 0;
            } else {
                rval = -1;
            }
            break;
        case 1:
            if(CrIso7816EnableSelectMF()) {           
                sfi = CrIso7816SFI();
                doff = CrIso7816DataOffset(); dlen = CrIso7816DataLength();
                rval = iso14443_4SelectDF(CrAID(), CrAIDLength(), buf, &len);
                if(!rval && len > 1 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
                    if(sfi) rval = -1;
                    else if(len < doff+dlen+2) dlen = 0;
                    else    p = buf + doff;
                } else if(!rval) dlen = 0;
           } else {
                rval = -1;
           }
           break;
        case 2:
            sfi = CrIso7816SFI();
            doff = CrIso7816DataOffset(); dlen = CrIso7816DataLength();
            n = CrIso7816RecordNumber();
            if(n) rval = iso14443_4ReadRecord(sfi, n, CrIso7816ReadLength(), buf, &len);
            else  rval = iso14443_4ReadBinary(sfi, doff, dlen, buf, &len);
            if(!rval && len > 1 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
                if(n) {
                    if(len < doff+dlen+2) dlen = 0;
                    else    p = buf + doff;
                } else {
                    if(len < dlen+2) dlen = 0;
                    else    p = buf;
                }
            } else if(!rval) dlen = 0;
           break;
        }
    }
    if(!rval && dlen) scAddData(p, dlen);
    return rval;
}

void isoprt(unsigned char *buf, int size)
{
	int		i;	
	printf("Rx %d [%02x", size, (int)buf[0]); for(i = 1;i < size;i++) printf("-%02x", (int)buf[i]); printf("]\n");	
}

/*
#include "sam.h"

extern unsigned short	fileId;
extern unsigned char	fileOffset, fileLen;

unsigned char aid_sksam[12] = { 0xa0, 0x00, 0x00, 0x05, 0x61, 0x00, 0x00, 0x01, 0x03, 0xfe, 0x01  };

int _SkSamTask(void)
{
	unsigned char	*p, buf[300], head[8], temp[32], csn[12];
    int     rval, len, idLen;
	unsigned long timer;
timer = MS_TIMER;

	rval = iso14443_4SelectDF(aid_sksam, 11, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4GetData(8, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	memcpy(csn, buf, 8);
	rval = iso14443_4SelectFile(fileId, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4GetChallenge(8, buf, &len);
	if(rval || len < 10 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	head[0] = 0x90; head[1] = 0xf0; head[2] = 0x00; head[3] = 0x00; head[4] = 0x12;
	memcpy(temp, buf, 8); memcpy(temp+8, csn, 8); temp[16] = fileId >> 8; temp[17] = fileId;
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x61 || buf[len-1] != 0x20) {
		return rval;
	}
	head[0] = 0x00; head[1] = 0xc0; head[2] = 0x00; head[3] = 0x00; head[4] = 0x20;
	rval = samReceiveT0(head, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 34 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	memcpy(temp, buf, 32);
	rval = iso14443_4Authenticate(temp, 32, buf, &len);
	if(rval || len < 34 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	head[0] = 0x90; head[1] = 0xf2; head[2] = 0x00; head[3] = 0x00; head[4] = 0x20;
	memcpy(temp, buf, 32);
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	rval = iso14443_4SecureReadBinary((int)fileOffset, (int)fileLen, buf, &len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	idLen = len - 2;
printf("EF_ID len=%d\n", idLen);
	head[0] = 0x90; head[1] = 0xf4; head[2] = 0x00; head[3] = 0x00; head[4] = idLen;
	memcpy(temp, buf, idLen);
	rval = samSendT0(head, temp, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x61) {
		return rval;
	}
	head[0] = 0x00; head[1] = 0xc0; head[2] = 0x00; head[3] = 0x00; head[4] = fileLen;
	rval = samReceiveT0(head, buf, &len, 90000);
if(!rval) samPrint(buf, len);
	if(rval || len < 2 || buf[len-2] != 0x90 || buf[len-1] != 0x00) {
		return rval;
	}
	scAddData(buf, (int)fileLen);
printf("duration=%d ms\n", MS_TIMER-timer);
	return rval;
}
*/
