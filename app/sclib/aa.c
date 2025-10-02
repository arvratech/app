#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "sc_iso14443a.h"
#include "sc_iso14443_4.h"
#include "sc_mifare.h"
#include "cardcrypto.h"
#include "des.h"
#include "sccfg.h"
#include "sctask_com.h"

#define _SC_DEBUG		1

static unsigned char  scScore, scRetry, sak, scHynixSector, uid[12];


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
	scState = SC_IDLE;
}

static void _scaToIdle(void)
{
	scState = SC_IDLE; ms_sleep(10);
}

 static void _scaError(void)
{
	iso14443AToIdle();		// set PICC to idle state	iso14443AHalt()
	_scaToIdle();
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

unsigned char hynix1_aid[10] = { 0xd0,0x00,0x53,0x4b,0x20,0x43,0x41,0x50,0x53 };
unsigned char hynix1_key[16] = {
	0xc2,0x16,0x90,0xf5,0xc4,0x48,0xc4,0x60,0x82,0x74,0x87,0x47,0xe2,0x97,0x54,0x4c
};
unsigned char hynix2_aid[12] = { 0xf0,0x73,0x6b,0x68,0x79,0x6e,0x69,0x78,0x00,0x01,0x01 };
unsigned char hynix2_key[16] = {
	0x7c,0xc0,0xe6,0x97,0x45,0xa0,0x78,0x3c,0x30,0x2b,0xfb,0x1d,0x6f,0x44,0x4e,0x77
};

unsigned char aid_transport[8]	= { 0xa0, 0x00, 0x00, 0x04, 0x52, 0x00, 0x01, 0x00 };	// 전국호환교통카드
unsigned char hynix_blkkey[6]	= { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
unsigned char norm_blkkey[6]	= { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };  
void _HynixSectorCard(void);
int  _Iso14443_4Task(void);
int  _SkSamTask(void);

void iso14443AActiveTask(void)
{
	unsigned char	*p, buf[280], temp[20];
    int     rval, i, len;

	switch(scState) {
	case SC_OFF:
		if(scOpenMode) { iso14443AClose(); usleep(2000); }
		iso14443AOpen(); scOpenMode = SC_ISO14443A;
		//us_sleep(1200);		// >= 580us
		ms_sleep(10);
		_scaToIdleNoWait();
		scCount = scScore = 0; uid[0] = 0;
		break;
	case SC_IDLE:
		if(scHynixSector == 1) iso14443ASendRequest(ISO14443A_CMD_WUPA);
		else	iso14443ASendRequest(ISO14443A_CMD_REQA);
		scState = SC_REQUEST;
		break;
	case SC_REQUEST:
		rval = iso14443AReceiveRequest(buf);
//printf("%lu ReceiveRequest: %d %d-%d\n", MS_TIMER, rval, (int)scCount, (int)scScore);
		if(rval == ERR_CHIP) {
printf("### sc fatal error ###\n");
			_scaToFatal();
		} else if(rval == 0 || rval == ERR_COLLISION) {
//if(scCount) printf("ReceiveRequest: %d OK\n", rval);
//printf("ATQA: %02x-%02x\n", (int)buf[0], (int)buf[1]);
			scScore = 0; scState = SC_ANTICOLLSION;
		} else if(rval > 0) {	//// ERR_TIMEOUT, 11, 5, 3
//if(scCount) printf("ReceiveRequest: %d %d-%d\n", rval, (int)scCount, (int)scScore);
			if(scCount) {
				if(rval == ERR_TIMEOUT) scScore += 2;
				else	scScore++;
				if(scScore < 4) _scaToIdle();
				else	_scaToOff();
			} else	_scaToOff();
		}
		break;
   case SC_ANTICOLLSION:
		//if(uid[0] && uid[1] != 0x08) {
		if(uid[0]) {	// Floating UID(08-xx-xx-xx) are same until halt 
			rval = iso14443ASelect(uid, &sak);
			if(rval) _scaError();
			else {
#ifdef _SC_DEBUG 
printf("%d UID=[%02x", (int)scRetry, (int)uid[1]); for(rval = 1;rval < uid[0];rval++) printf("-%02x", uid[rval+1]); printf("]\n");
#endif
				if(scRetry < 3) scState = SC_READY;
				else    _scaHalt();
			}
		} else {
			rval = iso14443AActivate(buf, &sak);
			if(rval) _scaError();
			else {
				uid[0] = buf[0]; for(i = 0;i < uid[0];i++) uid[1+i] = buf[1+i]; 
#ifdef _SC_DEBUG 
printf("ACT UID=[%02x", (int)uid[1]); for(rval = 1;rval < uid[0];rval++) printf("-%02x", uid[rval+1]); printf("]\n");
#endif
				scCount++;
				if(scReaderFormat == 0) {
					scAddData(0, uid+1, (int)uid[0]);
					_scaHalt();
				} else {
					scRetry = 0; scState = SC_READY;
				}
			}
		}
		break;
	case SC_READY:
		if(scReaderFormat == 1 && scCardApp == 1) {
//printf("sak=%02x %x\n", sak,  sak & COMPLIANT_WITH_ISO14443_4);
			//if(uid[1] == 0x08 || (sak & COMPLIANT_WITH_ISO14443_4) && sak != 0x28) {
			if(!scHynixSector && (uid[1] == 0x08 || (sak & COMPLIANT_WITH_ISO14443_4))) {
				rval = iso14443_4AnswerToSelect(0);
				if(!rval) rval = iso14443_4PPS(0);
				if(rval) _scaError();
				else {
					scSubState = 0; scState = SC_ACTIVE;
				}
			} else {
				_HynixSectorCard();
				scHynixSector = 2;
			}
		} else {
			if(scReaderFormat == 1) {				
				p = CrMifareKey();
				//len = CrBlockDataLength();
				//if(len < 1) len = 16;
				len = 16;	// modify at 2022.6.15
				mifareOpen();
//printf("mifareRead: %d key=%02x-%02x-%02x-%02x-%02x-%02x\n", CrBlockNumber(), (int)p[0], (int)p[1], (int)p[2], (int)p[3], (int)p[4], (int)p[5]);
				rval = mifareRead(CrBlockNumber(), buf, len, uid, p);
				if(rval) {
					scRetry++; mifareClose(); _scaToIdle();
				} else {
					scAddData(1, buf, len);
					_scaHalt();
#ifdef _SC_DEBUG 
printf("block=%d len=%d [%02x", CrBlockNumber(), len, (int)p[0]); for(rval = 1;rval < len;rval++) printf("-%02x", p[rval]); printf("]\n");
#endif
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
		}
		break;
	case SC_ACTIVE:
//		if(scCardApp == 2) rval = _SkSamTask();
		rval = _Iso14443_4Task();
		if(rval >= 0) {
			_scaHalt();
			if(rval == ERR_FILE_NOT_FOUND) scHynixSector = 1;
			else	scHynixSector = 0;
		}
		break;     
	}
}

void _HynixSectorCard(void)
{
	unsigned char	buf[32], tmp[32];;
	int		rval;

	CryptoDiversifyKey(uid+1, tmp);
	mifareOpen();
	rval = mifareRead(48, buf, 16, uid, tmp);
	if(rval) {
		scRetry++; mifareClose(); _scaToIdle();
	} else {
		rval = CryptoDecode(buf, uid+1, tmp);
		if(rval) {
#ifdef _SC_DEBUG 
printf("CryptoDecode error\n");
#endif
		} else {
#ifdef _SC_DEBUG 
printf("Hynix [%02x", (int)tmp[0]); for(rval = 1;rval < 8;rval++) printf("-%02x", tmp[rval]); printf("]\n");
#endif
			scAddData(1, tmp, 8);
		}
		_scaHalt();
	}
}

void iso14443AExitTask(void)
{
	unsigned char	buf[20];
	int		rval;

	switch(scState) {
	case SC_OFF:
		// LG-V20 phone: ISO14443A_CMD_WUPA fail to read Halt card
		//if(scOpenMode != SC_ISO14443A) { 
			if(scOpenMode) { iso14443AClose(); usleep(2000); }
			iso14443AOpen(); scOpenMode = SC_ISO14443A;
        //}
		_scaToIdleNoWait();
		ms_sleep(10);
		scScore = 0;
		scHynixSector = 0;
		break;
	case SC_IDLE:
		iso14443ASendRequest(ISO14443A_CMD_WUPA);
		scState = SC_REQUEST;
		break;
	case SC_REQUEST:
		rval = iso14443AReceiveRequest(buf);
//printf("%lu Exit ReceiveRequest: %d score=%d\n", MS_TIMER, rval, (int)scScore);
		if(rval == ERR_CHIP) {
			_scaToFatal();
		} else if(rval == ERR_TIMEOUT || rval == ERR_SOFT_FRAMING || rval == ERR_HARD_FRAMING || rval == ERR_PARITY) {
			if(rval == ERR_TIMEOUT) scScore += 2;
			else	scScore++;
//printf("Exit ReceiveRequest: %d score=%d\n", rval, (int)scScore);
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

	if(rval) printf("%s: %d\n", msg, rval);
	else {
		printf("%s %d [%02x", msg, len, (int)buf[0]);
		for(i = 1;i < len;i++) printf("-%02x", (int)buf[i]);
		printf("]\n");
	}
}

#include "aes.h"
#define emv_error(rval, buf, len, slen) do { if(!rval && (len < slen || buf[len-2] != 0x90 || buf[len-1] != 0x00)) rval = ERR_PARAM; if(rval) return rval; } while(0)

int  _HynixEmv1Card(unsigned char *buf);
int  _HynixEmv2Card(unsigned char *buf);

int _Iso14443_4Task(void)
{
	unsigned char	*p, buf[260], temp[20], r1[16], r2[16], k[16], dk[16], sk[16];
    int     rval, n, len, doff, dlen, sfi;
 
	if(scReaderFormat == 1 && scCardApp == 1) { // Hynix card
		rval = _HynixEmv2Card(buf);
		if(rval) return rval;
		p = buf; dlen = 8;
//prtkey("Final", buf);
	} else if(scCardApp == 1) {  // transport card
        rval = iso14443_4SelectDF(aid_transport, 7, buf, -16, &len);
   		p = temp;
        if(!rval && len > 15 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
			dlen = scExtractTransportCardNumber(buf, len-2, p);
        } else if(!rval || rval == ERR_EMV_STATUS) {
   			rval = iso14443_4SelectDF(CrAID(), CrAIDLength(), buf, -16, &len);
       		if(!rval && len > 15 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
           		dlen = scExtractSpecialTransportCardNumber(buf, len-2, p);
            } else if(!rval) dlen = 0;
        }
     } else {
        switch(scSubState) {
        case 0:
            if(CrIso7816EnableSelectMF()) { 
                rval = iso14443_4SelectMF(buf, -16, &len);
                if(!rval && len > 1 && buf[len-2] == 0x90 && buf[len-1] == 0x00) rval = -1;
                else if(!rval) dlen = 0;
            } else {
				scSubState++;
                rval = -1;
            }
            break;
        case 1:
            if(CrIso7816EnableSelectMF()) {           
                sfi = CrIso7816SFI();
                doff = CrIso7816DataOffset(); dlen = CrIso7816DataLength();
                rval = iso14443_4SelectDF(CrAID(), CrAIDLength(), buf, -16, &len);
                if(!rval && len > 1 && buf[len-2] == 0x90 && buf[len-1] == 0x00) {
                    if(sfi) rval = -1;
                    else if(len < doff+dlen+2) dlen = 0;
                    else    p = buf + doff;
                } else if(!rval) dlen = 0;
           } else {
				scSubState++;
                rval = -1;
           }
           break;
        case 2:
            sfi = CrIso7816SFI();
            doff = CrIso7816DataOffset(); dlen = CrIso7816DataLength();
            n = CrIso7816RecordNumber();
            if(n) rval = iso14443_4ReadRecord(sfi, CrIso7816ReadLength(), buf, dlen+2, &len);
            else  rval = iso14443_4ReadBinary(sfi, doff, buf, dlen, &len);
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
    if(!rval && dlen) scAddData(1, p, dlen);
    return rval;
}

int _HynixEmv1Card(unsigned char *cardbuf)
{
	unsigned char	buf[260], temp[20], r1[16], r2[16], k[16], dk[16], sk[16];
    int     rval, n, len, doff, dlen, sfi;
unsigned long timer;
 
  	rval = iso14443_4SelectDF(hynix1_aid, 9, buf, -16, &len);
//prtsc("SelectDF", rval, buf, len);
	if(rval) return rval;
	rval = iso14443_4GetData(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	for(n = 0;n < 8;n++) buf[8+n] = ~buf[n];
	AES_ECB_encrypt2(hynix1_key, buf, dk);
	rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(r2, buf, 16);
 	memrand(r1, 16);
	rval = iso14443_4InternalAuthenticate(r1, 16, buf, 18, &len);
//prtsc("InternalAuth", rval, buf, len);
	if(rval) return rval;
	AES_ECB_encrypt2(dk, r1, k);
	AES_ECB_encrypt2(k, r2, sk);
	AES_ECB_encrypt2(sk, dk, k);
	AES_ECB_encrypt2(k, r1, r1);
	if(memcmp(r1, buf, 16)) {
//pintf("Authentication error\n");
		return ERR_AUTHENTICATE;
	}
	AES_ECB_encrypt2(k, r2, r2);
	rval = iso14443_4ExternalAuthenticate(r2, 16, buf, 2, &len);
//prtsc("ExternalAuth", rval, buf, len);
	if(rval) return rval;
	rval = iso14443_4ReadRecord(1, 10, buf, 18, &len);
//prtsc("ReadRecord", rval, buf, len);
	if(rval) return rval;
	AES_ECB_decrypt2(sk, buf, cardbuf);
//prtkey("Final", buf);
	return 0;
}

int _HynixEmv2Card(unsigned char *cardbuf)
{
	DES3_CTX	*ctx, _ctx;
	unsigned char	buf[260], iv[8], tmp[16], crn[16], trn[16], dk[16], sk[16];
    int     rval, i, len, doff, dlen, sfi;
unsigned long timer;
 
  	 rval = iso14443_4SelectDF(hynix2_aid, 11, buf, 15, &len);
	if(rval == ERR_EMV_STATUS && buf[0] == 0x6a && buf[1] == 0x82) {
		rval = ERR_FILE_NOT_FOUND;
	}
if(rval) printf("SelectDF=0x%x\n", rval);
	if(rval) return rval;
	rval = iso14443_4GetData2(buf, 10, &len);
//prtsc("GetData", rval, buf, len);
	if(rval) return rval;
	ctx = &_ctx;
	memset(iv, 0, 8);
	for(i = 0;i < 8;i++) buf[8+i] = ~buf[i];
//prtkey("csn", buf);
	des3InitKey(ctx, hynix2_key, 16);
	des3EncryptCBC(ctx, iv, buf, dk, 2);
//prtkey(" dk", dk);
	rval = iso14443_4GetChallenge(buf, 18, &len);
//prtsc("GetChanllenge", rval, buf, len);
	if(rval) return rval;
	memcpy(crn, buf, 16);
//prtkey("crn", crn);
 	memrand(trn, 16);
//prtkey("trn", trn);
	des3InitKey(ctx, dk, 16);
	des3EncryptCBC(ctx, iv, crn, tmp, 2);
	des3InitKey(ctx, tmp, 16);
	des3EncryptCBC(ctx, iv, trn, sk, 2);
//prtkey("sk ", sk);
	des3InitKey(ctx, sk, 16);
	des3EncryptCBC(ctx, iv, crn, tmp, 2);
	memcpy(buf, trn, 16);
	memcpy(buf+16, tmp+8, 4);
	rval = iso14443_4CreateSession(3, buf, 20, buf, 2, &len);
	if(rval) return rval;
	rval = iso14443_4ReadBinary2(5, 0, buf, 22, &len);
	if(rval) return rval;
	des3EncryptCBC(ctx, iv, buf, tmp, 2);
	if(memcmp(tmp+8, buf+16, 4)) {
		printf("MAC invalid\n");
		return -1;
	}
	des3DecryptCBC(ctx, iv, buf, tmp, 2);
	memcpy(cardbuf, tmp, 8);
	return 0;
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
	scAddData(2, buf, (int)fileLen);
printf("duration=%d ms\n", MS_TIMER-timer);
	return rval;
}
*/
