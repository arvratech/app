#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "asciidefs.h"
#include "pktdefs.h"
#include "NSEnum.h"
#include "prim.h"
#include "rtc.h"
#include "syscfg.h"
#include "schecfg.h"
#include "tacfg.h"
#include "sclib.h"
#include "ad.h"
#include "cr.h"
#include "cf.h"
#include "aa.h"
#include "user.h"
#include "fpuser.h"
#include "sche.h"
#include "evt.h"
#include "slvmlib.h"
#include "slvm.h"
#include "svr.h"
#include "msg.h"
#include "lang.h"
#include "msgsvr.h"
#include "xfer.h"
#include "sysdep.h"
#include "topprim.h"
#include "svrcmd.h"

extern int 	gRequest, gCommand;
extern unsigned char RemoteEnroll, EnrollAccessMode;
extern char 	EnrollUserID[];

SYS_CFG		*tmp_cfg, _tmp_cfg;
SCHE_CFG	*tmpsche_cfg, _tmpsche_cfg;
TA_CFG		*tmpta_cfg, _tmpta_cfg;
FP_USER		*sUser, _sUser;


int svrProcessCommand(int sock, unsigned char *buf, int svrSize)
{
	unsigned char	*data;
	int		Result, Command, DataSize, resetflg;
	
	Command = GetCommand(buf);
	DataSize = svrSize - 9;
	data = buf + 7;
printf("Command=0x%02x 0x%02x size=%d\n", Command, (int)data[2], svrSize);
//if(Command == 0xfd) {
//	DumpFlash(sock, buf);
//	return 0;	
//}
	resetflg = 0;
	switch(Command) {
	case P_PING:
		Result = R_SUCCESS; DataSize = 0;
		break;
	case P_STORE:
		switch(data[2]) {
		case 0x11:		Result = StoreTerminal(sock, data, &DataSize); break;
		case 0x12:		Result = StoreAccessDoor(sock, data, &DataSize); break;
		case 0x13:		Result = StoreAccessPoint(sock, data, &DataSize); break;
		case 0x14:		Result = StoreOperation(sock, data, &DataSize); break;
		case 0x21:		Result = StoreHoliday(sock, data, &DataSize); break;
		case 0x22:		Result = StoreTimeZone(sock, data, &DataSize); break;
		case 0x23:		Result = StoreSchedule(sock, data, &DataSize); break;
		case 0x31:		Result = StoreUser(sock, data, &DataSize); break;
		case 0x33:		Result = StoreUserEx(sock, data, &DataSize); break;
		default:		Result = R_PARAMETER_ERROR;
		}
		break;
	case P_GET:
		switch(data[2]) {
		case 0x11:		Result = GetTerminal(sock, data, &DataSize); break;
		case 0x12:		Result = GetAccessDoor(sock, data, &DataSize); break;
		case 0x13:		Result = GetAccessPoint(sock, data, &DataSize); break;
		case 0x14:		Result = GetOperation(sock, data, &DataSize); break;
		case 0x21:		Result = GetHoliday(sock, data, &DataSize); break;
		case 0x22:		Result = GetTimeZone(sock, data, &DataSize); break;
		case 0x23:		Result = GetSchedule(sock, data, &DataSize); break;
		case 0x31:		Result = GetUser(sock, data, &DataSize); break;
		case 0x33:		Result = GetUserEx(sock, data, &DataSize); break;
		default:		Result = R_PARAMETER_ERROR;
		}
		break;
	case P_GET_EX:
		if(DataSize != 4) Result = R_PARAMETER_ERROR;
		else {
			switch(data[2]) {
			case 0x11:		Result = GetExTerminal(sock, data, &DataSize); break;
			default:		Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_SET:
		if(DataSize < 5) Result = R_PARAMETER_ERROR;
		else {
			switch(data[2]) {
			case 0x11:		Result = SetTerminal(sock, data, &DataSize); break;
			default:		Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_DELETE:
#ifdef _USER_ID_EX
		if(DataSize != 15) Result = R_PARAMETER_ERROR;
#else
		if(DataSize != 11) Result = R_PARAMETER_ERROR;
#endif
		else {
			switch(data[2]) {
			case 0x21:		Result = DeleteHoliday(sock, data, &DataSize); break;
			case 0x22:		Result = DeleteTimeZone(sock, data, &DataSize); break;
			case 0x23:		Result = DeleteSchedule(sock, data, &DataSize); break;
			case 0x31:		Result = DeleteUser(sock, data, &DataSize); break;		
			case 0x33:		Result = DeleteUserEx(sock, data, &DataSize); break;		
			default:		Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_EXPORT:
#ifdef _USER_ID_EX
		if(DataSize != 35) Result = R_PARAMETER_ERROR;
#else
		if(data[2] != 0x55 && DataSize < 31 || data[2] == 0x55 && DataSize < 43) Result = R_PARAMETER_ERROR;	// 31 = 43 
#endif
		else {
			switch(data[2]) {
			case 0x00:	Result = ExportFile(sock, data, &DataSize); break;
			case 0x21:	Result = ExportHolidays(sock, data, &DataSize); break;
			case 0x22:	Result = ExportTimeZones(sock, data, &DataSize); break;
			case 0x23:	Result = ExportSchedules(sock, data, &DataSize); break;
			case 0x31:	Result = ExportUsers(sock, data, &DataSize); break;
			case 0x32:	Result = ExportFPTemplates(sock, data, &DataSize); break;
			case 0x33:	Result = ExportUserExs(sock, data, &DataSize); break;
			case 0x34:	Result = ExportUserPhotos(sock, data, &DataSize); break;
			//case 0x51:	Result = ExportFPImage(sock, data, &DataSize); break;
			//case 0x55:	Result = ExportBackupEvents(sock, data, &DataSize); break;
			default:	Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_IMPORT:
		if(DataSize != 19) Result = R_PARAMETER_ERROR;
		else {
			switch(data[2]) {
			case 0x00:	Result = ImportFile(sock, data, &DataSize); break;
			case 0x21:	Result = ImportHolidays(sock, data, &DataSize); break;
			case 0x22:	Result = ImportTimeZones(sock, data, &DataSize); break;
			case 0x23:	Result = ImportSchedules(sock, data, &DataSize); break;
			case 0x31:	Result = ImportUsers(sock, data, &DataSize); break;
			case 0x32:	Result = ImportFPTemplates(sock, data, &DataSize); break;
			case 0x33:	Result = ImportUserExs(sock, data, &DataSize); break;
			case 0x34:	Result = ImportUserPhotos(sock, data, &DataSize); break;
			case 0x51:	Result = ImportFPImage(sock, data, &DataSize); break;
			case 0x52:	Result = ImportFirmware(sock, data, &DataSize); if(!Result) resetflg = 1; break;
			default:	Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_GET_STATUS:
		if(DataSize != 3) Result = R_PARAMETER_ERROR;
		else {
			switch(data[2]) {
			case 0x11:	Result = GetTerminalStatus(sock, data, &DataSize); break;		
			case 0x12:	Result = GetAccessDoorStatus(sock, data, &DataSize); break;		
			default:	Result = R_PARAMETER_ERROR;
			}
		}
		break;
	case P_ACCESS_CONTROL:
		if(DataSize != 2) Result = R_PARAMETER_ERROR;
		else	Result = AccessControl(sock, data, &DataSize);	
		break;
	case P_OUTPUT_CONTROL:
		if(DataSize != 2) Result = R_PARAMETER_ERROR;
		else	Result = OutputControl(sock, data, &DataSize);	
		break;
	case P_XFER_RRQ:
		Result = XferServerRRQ(sock, (char *)buf, svrSize);
		break;
	case P_XFER_WRQ:
		Result = XferServerWRQ(sock, (char *)buf, svrSize);
		break;
	case P_AUTH_RESULT:
		Result = AuthenticationResult(sock, data, &DataSize);
		break;
	default:
		Result = R_UNKNOWN_COMMAND;
	}
	if(Command != P_XFER_RRQ && Command != P_XFER_WRQ) {
		if(Result) DataSize = 0;
		gCommand = 0;
		svrSendResult(sock, buf, Result, DataSize);
	}
	if(resetflg) {
		//fsbevtSync();
		//
	} 
	return Result;
}

#include "svrcmd1.c"
#include "svrcmd2.c"
