#include <stdio.h>
#include <stdlib.h>
#include <string.h>
////////////////////////////////
#include "defs.h"
#include "defs_obj.h"
#include "defs_pkt.h"
#include "prim.h"
#include "cotask.h"
#include "rtc.h"
#include "iodev.h"
#include "key.h"
#include "syscfg.h"
#include "unit.h"
#include "bio.h"
#include "acap.h"
#include "acad.h"
#include "cr.h"
#include "user.h"
#include "fsuser.h"
#include "sclib.h"
#include "msg.h"
#include "sche.h"
#include "ta.h"
#include "evt.h"
#include "fpapi_new.h"
#include "sysdep.h"
#include "gwnd.h"
#include "gedit.h"
#include "gmenu.h"
#include "sinport.h"
#include "slavenet.h"
#include "menuprim.h"
#include "wnds.h"
#include "admprim.h"
#include "topprim.h"
#include "desktop.h"
#include "slave.h"

extern ACCESS_POINT	_gAccessPoint;
extern unsigned char gDataReceived, gLocalDateTimeVersion;

#include "slave_read.c"
#include "slave_write.c"


void SlaveProcessRequest(int UnitID)
{
	unsigned char	*s, *d;
	int		Command, Result, size;

	s = SlaveNetGetRxReqData(UnitID); size = SlaveNetGetRxReqDataSize(UnitID);
	Command = *s++; size--;	
cprintf("CRU-%d ProcessCommand: Cmd=%02x DataSize=%d\n", UnitID, Command, size);
	d = SlaveNetGetTxRspData(UnitID);
	switch(Command) {
	case P_GET_PROPERTY:
		Result = SlaveGetProperty(UnitID, d+1, s, &size);
		break;
	case P_FIND_OBJECT:
		Result = SlaveFindObject(UnitID, d+1, s, &size);
		break;
	case P_FIND_PROPERTY:
		Result = SlaveFindProperty(UnitID, d+1, s, &size);
		break;
	case P_READ_OBJECT:
		Result = SlaveReadObject(UnitID, d+1, s, &size);
		break;
	case P_READ_PROPERTY:
		Result = SlaveReadProperty(UnitID, d+1, s, &size);
		break;
	case P_WRITE_OBJECT:
		Result = SlaveWriteObject(UnitID, d+1, s, &size);
		break;
	case P_WRITE_PROPERTY:
		Result = SlaveWriteProperty(UnitID, d+1, s, &size);
		break;
	case P_DELETE_OBJECT:
		Result = SlaveDeleteObject(UnitID, d+1, s, &size);
		break;
	case P_EVENT_NOTIFICATION:
		Result = SlaveEventNotification(UnitID, d+1, s, &size);
		break;
	default:
		break;
	}
	*d = Result;
	if(Result) size = 1;
	else	size++;
//cprintf("CRU-%d ProcessCommand: Result=%d DataSize=%d\n", UnitID, Result, size);
	SlaveNetSetTxRspDataSize(UnitID, size, 0);
}
