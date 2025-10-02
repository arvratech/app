#include <stdio.h>
#include <string.h>
#include "rtc.h"
#include "log.h"
#include "as3911.h"
#include "sc_iso15693_2.h"
#include "sc_iso15693_3.h"
#include "sccfg.h"
#include "sctask_com.h"

static unsigned char	scScore;
static iso15693Card_t	scCards[MAX_CARD_SIZE];
static int				scCardCount, newCardCount;


static void _sccToFatal(void)
{
printf("Fatal error...........\n");    
	scState = SC_FATAL;
}

static void _sccToOff(void)
{
	scState = SC_OFF;
}

static void _sccToIdleNoWait(void)
{
	scState = SC_IDLE;
}

static void _sccToIdle(void)
{
	scState = SC_IDLE; ms_sleep(10);		// 5ms
}

void iso15693ActiveTask(void)
{
	iso15693PhyConfig_t _iso15693cfg;
	iso15693PiccSystemInformation_t	_sysInfo;
	iso15693PiccMultipleBlock_t		_block;
	int     rval, i, blockNo, blockSize, blockCount, dataLen;

	switch(scState) {
	case SC_OFF:
		if(scOpenMode) iso14443AClose();
		_iso15693cfg.coding = ISO15693_VCD_CODING_1_4;
		_iso15693cfg.mi = ISO15693_MODULATION_INDEX_10;	
		_iso15693cfg.mi = ISO15693_MODULATION_INDEX_OOK;
		iso15693Open(&_iso15693cfg); scOpenMode = SC_ISO15693; 
		ms_sleep(10);
		_sccToIdleNoWait();
		scCount = 0; scScore = 0; scCardCount = 0;
		break;
	case SC_IDLE:
		rval = iso15693InventoryOneSlot(&scCards[scCardCount], &newCardCount, MAX_CARD_SIZE - scCardCount);
		if(rval == ERR_CHIP) {
			_sccToFatal();
		} else if(newCardCount) {
printf("RVAL=%d count=%d %d scReaderFormat=%d\n", rval, scCardCount, newCardCount, (int)scReaderFormat);
			scCardCount += newCardCount;
			if(scReaderFormat) {
				scState = SC_READY;
			} else {
				for(i = scCardCount-newCardCount;i < scCardCount;i++) scAddData(0, scCards[i].uid, 8);
				if(scCardCount<  MAX_CARD_SIZE) {
					for(i = scCardCount-newCardCount;i < scCardCount;i++) iso15693StayQuiet(scCards[i].uid);
					_sccToIdle(); scScore = 0;
				} else {
					for(i = 0;i < scCardCount-newCardCount;i++) iso15693ResetToReady(scCards[i].uid);
					_sccToOff();
				}
			}
			scCount = 1;
		} else if(rval == ERR_TIMEOUT) {
			if(scCount) {
				scScore += 2;
				if(scScore < 4) _sccToIdle();
				else {
					for(i = 0;i < scCardCount;i++) iso15693ResetToReady(scCards[i].uid);
					_sccToOff();
				}
			} else	 _sccToOff();
		} else {
			_sccToIdle(); scScore = 0; scCount = 1;
		}
		break;
	case SC_READY: 
		for(i = scCardCount-newCardCount;i < scCardCount;i++) {
			rval = iso15693GetPiccSystemInformation(&scCards[i], &_sysInfo);
			if(rval) {
				iso15693StayQuiet(scCards[i].uid);
				continue;
			}
printf("Block size=%d num=%d\n", _sysInfo.memBlockSize, _sysInfo.memNumBlocks);
			blockSize = _sysInfo.memBlockSize + 1;
			blockNo = CrBlockNumber();
			//dataLen = CrBlockDataLength(); if(dataLen < 1) dataLen = blockSize;
			dataLen = blockSize;	// modify at 2022.6.15
			blockCount = (dataLen + blockSize - 1) / blockSize;
			if(blockNo + blockCount > _sysInfo.memNumBlocks+1) {
				iso15693StayQuiet(scCards[i].uid);
				continue;
			} else {
				rval = iso15693ReadMultipleBlocks(&scCards[i], blockNo, blockSize, blockCount, &_block);			
				iso15693StayQuiet(scCards[i].uid);
				if(!rval && !_block.errorCode) scAddData(1, _block.data, dataLen); 
			}
		}	
        break;
	}
}

void iso15693ExitTask(void)
{
	iso15693PhyConfig_t _iso15693cfg;
	iso15693Card_t	_card;	
	int		rval;

	switch(scState) {
	case SC_OFF:
		if(scOpenMode != SC_ISO15693) {
			if(scOpenMode) iso14443AClose();
			_iso15693cfg.coding = ISO15693_VCD_CODING_1_4;
			_iso15693cfg.mi = ISO15693_MODULATION_INDEX_10;		// _iso15693cfg.mi = ISO15693_MODULATION_INDEX_OOK;
			iso15693Open(&_iso15693cfg); scOpenMode = SC_ISO15693; 
			usleep(150);	// stable time( >=1ms OK)
		}
		_sccToIdleNoWait();
		scScore = 0;
		break;
	case SC_IDLE:
		rval = iso15693InventoryPresence(&_card);
		if(rval == ERR_CHIP) {
	        _sccToFatal();
		} else if(rval == ERR_TIMEOUT) {
			scScore += 2;
printf("iso15693: Timeout: score=%d\n", (int)scScore);
			if(scScore < 4) _sccToIdle();
			else	_sccToOff();
		} else { 
if(rval) printf("RVAL=%d\n", rval);
	        _sccToIdle(); scScore = 0;
		}
		break;
	}
}
