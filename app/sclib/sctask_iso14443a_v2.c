#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "prim.h"
#include "as3911.h"
#include "sc_iso14443a.h"
#include "sc_iso14443_4.h"
#include "sc_mifare.h"
#include "sccfg.h"
#include "sctask_com.h"

//#define _SC_DEBUG		1

static unsigned char  scScore, scRetry, sak, scEmv, uid[12];


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

#ifdef _GOV_CARD
unsigned char gov_blkkey[8] = { 0x25, 0x80, 0x13, 0x25, 0x80, 0x13 };
void _GovSectorCard(void);
#else
#include "cardcrypto.h"
#include "des.h"
unsigned char aid_transport[8]	= { 0xa0, 0x00, 0x00, 0x04, 0x52, 0x00, 0x01, 0x00 };	// 전국호환교통카드
unsigned char hynix_blkkey[6]	= { 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5 };
unsigned char norm_blkkey[6]	= { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };  
void _HynixSectorCard(void);
int  _SkSamTask(void);
#endif
int  _Iso14443_4Task(void);

//#define _TEST
//unsigned long	cardTimer;
//int		cardExist;

void iso14443AActiveTask(void)
{
	unsigned char	*p, buf[280];
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
#ifdef _TEST
		if(cardExist) {
			rval = ISO14443A_CMD_WUPA;
			cardExist = 0;
		} else	rval = ISO14443A_CMD_REQA;
#else
		if(scEmv == 1) rval = ISO14443A_CMD_WUPA;
		else	rval = ISO14443A_CMD_REQA;
#endif
		iso14443ASendRequest(rval);
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
#ifdef _GOV_CARD
				scRetry = 0; scState = SC_READY;
#else
				if(scReaderFormat == 0) {
					scAddData(0, uid+1, (int)uid[0]);
					_scaHalt();
				} else {
					scRetry = 0; scState = SC_READY;
				}
#endif
			}
		}
		break;
	case SC_READY:
#ifdef _GOV_CARD
		if(uid[1] == 0x08 || (sak & COMPLIANT_WITH_ISO14443_4)) {
			rval = iso14443_4AnswerToSelect(0);
			if(!rval) rval = iso14443_4PPS(0);
			if(rval) _scaError();
			else {
				scSubState = 0; scState = SC_ACTIVE;
			}
		} else {
			_GovSectorCard();
		}
#else
		if(scReaderFormat == 1 && scCardApp == 1) {
//printf("sak=%02x %x\n", sak,  sak & COMPLIANT_WITH_ISO14443_4);
			//if(uid[1] == 0x08 || (sak & COMPLIANT_WITH_ISO14443_4) && sak != 0x28) {
			if(!scEmv && (uid[1] == 0x08 || (sak & COMPLIANT_WITH_ISO14443_4))) {
				rval = iso14443_4AnswerToSelect(0);
				if(!rval) rval = iso14443_4PPS(0);
				if(rval) _scaError();
				else {
					scSubState = 0; scState = SC_ACTIVE;
				}
			} else {
				_HynixSectorCard();
				scEmv = 2;
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
#endif
		break;
	case SC_ACTIVE:
//		if(scCardApp == 2) rval = _SkSamTask();
		rval = _Iso14443_4Task();
		if(rval >= 0) {
			_scaHalt();
#ifdef _GOV_CARD
			if(rval == ERR_FILE_NOT_FOUND) {
				if(scEmv) scEmv = 0;
				else    scEmv = 1;
			} else  scEmv = 0;
#else
			if(rval == ERR_FILE_NOT_FOUND) scEmv = 1;
			else	scEmv = 0;
#endif
		}
		break;
	}
}

#ifdef _GOV_CARD

void _GovSectorCard(void)
{
	unsigned char	buf[32];
	int		rval;

	mifareOpen();
	rval = mifareRead(60, buf, 16, uid, gov_blkkey);
	if(rval) {
		scRetry++; mifareClose(); _scaToIdle();
	} else {
printf("gov [%02x", (int)buf[0]); for(rval = 1;rval < 16;rval++) printf("-%02x", buf[rval]); printf("]\n");
		buf[16] = 0;
		if(isdigits((char *)buf) == 16) scAddData(1, buf, 16);
		_scaHalt();
	}
}

#else

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

#endif

void iso14443AExitTask(void)
{
	unsigned char	buf[20];
	int		rval;

	switch(scState) {
	case SC_OFF:
#ifdef _TEST
		cardTimer = MS_TIMER;
#endif
		// LG-V20 phone: ISO14443A_CMD_WUPA fail to read Halt card
		//if(scOpenMode != SC_ISO14443A) { 
			if(scOpenMode) { iso14443AClose(); usleep(2000); }
			iso14443AOpen(); scOpenMode = SC_ISO14443A;
        //}
		_scaToIdleNoWait();
		ms_sleep(10);
		scScore = 0;
		scEmv = 0;
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
#ifdef _TEST
			if((int)(MS_TIMER - cardTimer) > 3000) {
				_scaToOff();
			} else {
				iso14443AToIdle();		// set PICC to idle state	iso14443AHalt()
				_scaToIdle(); scScore = 0;
			}
#else
			iso14443AToIdle();		// set PICC to idle state	iso14443AHalt()
			_scaToIdle(); scScore = 0;
#endif
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

#define emv_error(rval, buf, len, slen) do { if(!rval && (len < slen || buf[len-2] != 0x90 || buf[len-1] != 0x00)) rval = ERR_PARAM; if(rval) return rval; } while(0)
#ifdef _GOV_CARD
#include "sctask_iso7816_gov.c"
#else
#include "sctask_iso7816_hynix.c"
//#include "sctask_iso7816_sam.c"
#endif

int _Iso14443_4Task(void)
{
	unsigned char	*p, buf[260], temp[20];
    int     rval, dlen, n, len, doff, sfi;
 
	dlen = 0; p = buf;
#ifdef _GOV_CARD
	rval = _GovEmvCard(buf);
	if(rval) return rval;
	p = buf; dlen = 16;
#else
	if(scReaderFormat == 1 && scCardApp == 1) { // Hynix card
		rval = _HynixEmv2Card(buf);
		if(rval) return rval;
		p = buf; dlen = 16;
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
#endif
    if(!rval && dlen > 0) scAddData(1, p, dlen);
    return rval;
}

void isoprt(unsigned char *buf, int size)
{
	int		i;	
	printf("Rx %d [%02x", size, (int)buf[0]); for(i = 1;i < size;i++) printf("-%02x", (int)buf[i]); printf("]\n");	
}

