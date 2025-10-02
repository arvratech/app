#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "iodev.h"
#include "unit.h"
#include "acad.h"
#include "acap.h"
#include "bio.h"
#include "cr.h"
#include "unit.h"
#include "sclib.h"
#include "ta.h"
#include "event.h"
#include "user.h"
#include "fsuser.h"
#include "sinport.h"
#include "slavenet.h"
#include "auth.h"
#include "slave.h"
#include "net.h"
#include "cli.h"

extern CREDENTIAL_READER _gCredentialReaders[];
extern FP_USER *gUser;
extern unsigned char  gLocalDateTimeVersion;

#define XFER_BUFFER_SIZE		 1400000		// 1.4
#define XFER_BUFFER_LONG_SIZE	  350000

#define XFER_DATA_SIZE		800

static unsigned long	XferBuf[XFER_BUFFER_LONG_SIZE];
static unsigned char	XferLastTime[8];
static unsigned long	XferTimer;
static int	CliReq, XferSize, XferCount, XferState;
int			CliID;


void CliInit(void)
{
	CREDENTIAL_READER	*cr;
	int		i;

 	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++) cr->XferRetry = cr->XferState = cr->CliRetry = 0;
	CliID = -1; CliReq = 0; XferSize = 0;
	XferState = 0;
	memset(XferLastTime, 0, 6);
}

void _ClisCheckXferCompleted(void)
{
	CREDENTIAL_READER	*cr;
	int		i;
	
 	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++)
		if(unitGetType(i) == 2 && cr->XferState == 1) break;
	if(i >= MAX_UNIT_SZ) {
		CliSetXferState(0);	// Completed
cprintf("%lu Check ClisXfer Completed...\n", DS_TIMER);
	}
}

void ClisTimeoutXferCompleted(void)
{
	if(CliGetXferState() && CliID < 0 && (SEC_TIMER - XferTimer) > 60) {
		CliSetXferState(0);	// Completed
printf("%lu Timeout ClisXfer Completed...\n", DS_TIMER);
	}
}

void CliDownload(int ID)
{
	CREDENTIAL_READER	*cr;
	unsigned short	val;

	cr = crsGetAt(ID);
 	if(cr->Client || CliID == ID) {
		if(crGetReliability(ID) || (DS_TIMER-cr->Timer) > CLIENT_TIMEOUT) {
			if(CliReq == XFER_FILE_CHANGED) {
cprintf("%lu slvnets(%d): Rsp-timeout: rel=%d xferFile retry=%d\n", DS_TIMER, ID, crGetReliability(ID), (int)cr->XferRetry);
				if(cr->XferRetry >= 5) {
					cr->XferState = 3;		// Fail Completed
					rtcGetDateTime(XferLastTime);
					_ClisCheckXferCompleted();
				}
				XferTimer = SEC_TIMER;
			} else {
cprintf("%lu slvnets(%d): Rsp-timeout: rel=%d cliReq=%02x retry=%d\n", DS_TIMER, ID, crGetReliability(ID), (int)cr->Client, (int)cr->CliRetry);
				if(cr->Client == CLI_BYPASS_REQUESTED || cr->CliRetry >= 5) {
		 			cr->Update &= ~cr->Client;
	 				cr->CliRetry = 0;
				}
			}
			cr->Client = 0;
			CliID = -1; CliReq = 0;
		}
 	} else if(cr->Update) {
 		cr->Client = 0;
 		val = cr->Update;
		if(val & LOCAL_DATETIME_CHANGED) {
 			cr->Client = LOCAL_DATETIME_CHANGED;
 			CliWriteLocalDateTime(cr);
 		} else if(val & IO_CONFIG_CHANGED) {
 			cr->Client = IO_CONFIG_CHANGED;
 			CliWriteIOConfig(cr);
 		} else if(val & CR_CONFIG_CHANGED) {
 			cr->Client = CR_CONFIG_CHANGED;
 			CliWriteCRConfig(cr);
 		} else if(val & SCKEY_CHANGED) {
 			cr->Client = SCKEY_CHANGED;
 			CliWriteSCKey(cr);
 		} else if(val & TA_CONFIG_CHANGED) {
 			cr->Client = TA_CONFIG_CHANGED;
 			CliWriteTAConfig(cr);
 		} else if(val & ADMIN_PIN_CHANGED) {
 			cr->Client = ADMIN_PIN_CHANGED;
 			CliWriteAdminPIN(cr);
  		}
		if(cr->Client) cr->CliRetry++;
   	} else if(CliGetXferState() && cr->XferState == 1 && CliID < 0) {
 		CliID = ID; CliReq = XFER_FILE_CHANGED;
 		cr->XferRetry++;
 		CliXferWRQ(cr);
	}
}

void CliProcessRequest(int ID)
{
	ACCESS_POINT	*ap;
	CREDENTIAL_READER *cr;
	unsigned char	*p;
	int		rval, val, size;

	ap = apsGetAt(ID);
	cr = &_gCredentialReaders[ID];
	p = SlaveNetGetRxReqData(ID); size = SlaveNetGetRxReqDataSize(ID);
	val = *p;
	switch(val) {		
	case 0x30:
		cr->FuncKey = p[1];
		rval = crRead(cr, p+2, size-2);
cprintf("%lu slvnets(%d): Identify: type=%d data=%d\n", DS_TIMER, ID, (int)cr->CredentialType, cr->DataLength);
//cprintf("["); for(val = 0;val < cr->DataLength;val++) cprintf("%02x", cr->Data[val]); cprintf("]\n");
		if(rval > 1) ApIdentifyCredential(ap, cr);
		break;
	case 0x31:
		cr->SessionNo = p[1];
		rval = crRead(cr, p+2, size-2);
cprintf("%lu slvnets(%d): Verify: Session=%d type=%d data=%d\n", DS_TIMER, ID, (int)cr->SessionNo, (int)cr->CredentialType, cr->DataLength);
		if(rval > 1) ApVerifyCredential(ap, cr);
		break;
	case 0x32:
		cr->Result = p[1];
		if(!cr->Result) rval = crRead(cr, p+2, size-2);
		else	cr->DataLength = 0;
cprintf("%lu slvnets(%d): Capture: type=%d data=%d\n", DS_TIMER, ID, (int)cr->CredentialType, cr->DataLength);
		ApCaptureCredential(cr);
		break;
	default:
		SlaveProcessRequest((int)cr->ID);
		break;
 	} 	
}

int SlaveEventNotification(int UnitID, unsigned char *TxData, unsigned char *RxData, int *DataSize);

void CliProcessRequest2(int ID)
{
	unsigned char	*s, *d;
	int		command, result, size;

	s = SlaveNetGetRxReq2Data(ID); size = SlaveNetGetRxReq2DataSize(ID);
	command = *s++; size--;
	d = SlaveNetGetTxRsp2Data(ID);
	switch(command) {		
	case P_EVENT_NOTIFICATION:
cprintf("%lu slvnets(%d): eventNotification: %d\n", DS_TIMER, ID, size);
		result = SlaveEventNotification(ID, d+1, s, &size);
		break;
	default:
		result = R_UNKNOWN_COMMAND;
	}
	*d = result;
	if(result) size = 1;
	else	size++;
	SlaveNetSetTxRsp2DataSize(ID, size);
}

void CliProcessRxCommon(int ID, unsigned char *buf)
{
	unitcrProcessReq(ID, buf+2);
}

void CliProcessResponse(int ID)
{
	CREDENTIAL_READER *cr;
	NET_TSM	*tsm;
	NET		*rspnet, *reqnet;
	unsigned char	*p;
	int		result, size;

	cr = crsGetAt(ID);
	p = SlaveNetGetRxRspData(ID);
	size = SlaveNetGetRxRspDataSize(ID);
	result = *p++; size--;
	if(CliID == ID) {
cprintf("%lu slvnets(%d): Rx rsp: CliReq=%02x size=%d\n", DS_TIMER, ID, CliReq, size);	
		if(CliReq == XFER_FILE_CHANGED) {
cprintf("%lu slvnets(%d): xfer completed: result=%d size=%d\n", DS_TIMER, ID, result, CliGetXferCount());
			if(result && cr->XferRetry >= 5) cr->XferState = 3;	// Fail Completed
			else if(!result) cr->XferState = 2;		// OK Completed
			if(cr->XferState > 1) {
				rtcGetDateTime(XferLastTime);
				_ClisCheckXferCompleted();
			}
			CliID = -1; CliReq = 0; XferTimer = SEC_TIMER;
		}
	} else {
cprintf("%lu slvnets(%d): Rx rsp-%d: client=%02x size=%d\n", DS_TIMER, ID, result, (int)cr->Client, size);	
		if(cr->Client == CLI_BYPASS_REQUESTED) {
			tsm = cr->nettsm;			
			rspnet = (NET *)tsm->SvcNet;
			if(result) netCodeError(rspnet, 1, result);
			else {
				if(size > 0) { 
					reqnet = (NET *)tsm->PduNet;
					memcpy(rspnet->Data, reqnet->Data, 5);
					memcpy(rspnet->Data+5, p, size);
					rspnet->DataLength = 5 + size;
				} else	rspnet->DataLength = 0;
				netCodeResponse(rspnet);
			}
			tsm->SvcPending = 1;
		}
		if(cr->Client == CLI_BYPASS_REQUESTED || !result || cr->CliRetry >= 5) {
	 		cr->Update &= ~cr->Client;
	 		cr->CliRetry = 0;
		}
	}
	cr->Client = 0;
}

void CliProcessSegResponse(int ID)
{
	CREDENTIAL_READER *cr;
	unsigned char	*p;

	cr = crsGetAt(ID);
	p = SlaveNetGetRxRspData(ID);
	if(CliID == ID) {
		if(CliReq == XFER_FILE_CHANGED) {
			if(p[0] == P_XFER_ACK) {
//cprintf("%lu ID=%d Rx SegRsp-XferAck\n", DS_TIMER, ID);
				if(CliGetXferState()) CliXferData(cr);
				else {
					CliXferError(cr, R_XFER_PROTOCOL_ERROR);	// Cancelled
					CliID = -1; CliReq = 0;
				}
			} else {
cprintf("%lu slvnets(%d): Rx segRsp: not xferAck\n", DS_TIMER, ID);
				CliXferError(cr, R_XFER_PROTOCOL_ERROR);
				CliID = -1; CliReq = 0;
			}
		} else {
cprintf("%lu slvnets(%d): Rx unexpected SegRsp\n", DS_TIMER, ID);
			CliID = -1; CliReq = 0;
		}
	} else {
cprintf("%lu slvnets(%d): Rx unexpected segRsp\n", DS_TIMER, ID);		
	}
}

void CliProcessSegRequest(int ID)
{
}

int CliIsBusy(int ID)
{
	CREDENTIAL_READER *cr;
	int		rval;

	cr = crsGetAt(ID);
cprintf("crId=%d Client=%02x Update=%02x CliID=%d\n", ID, (int)cr->Client, (int)cr->Update, (int)CliID);
 	if(cr->Client || CliID == ID || cr->Update) rval = 1;
 	else	rval = 0;
 	return rval;
 }

#include "cli_req.c"
#include "cli_xfer.c"