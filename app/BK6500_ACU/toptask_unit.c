unsigned char taOldState;


int taGetState(void)
{
	return (int)taState;
}

extern volatile	int  sio0_state; 
extern volatile unsigned long slvLoop;
unsigned long	taskLoop, oldTickTimer, oldSlvLoop, slvTimer;

void ctUnitsInit(void)
{
	int		SioPort;
	int		i, Control;

	taState = taOldState = 0;
	InitLocalUnit();
	CliInit();
	taskLoop = 0L;
	oldTickTimer = TICK_TIMER;
	oldSlvLoop = slvLoop;
	slvTimer = DS_TIMER;	
	SioPort = 0;
//	sioClose(SioPort);
	sioOpen(SioPort, 38400L, 1);
	SlaveSioInit(38400L);
	SlaveSinInit();
	for(i = 0;i < MAX_UNIT_SZ;i++) {
		if(unitGetType(i) == 2) {
			unitcrInitRequest(i);
			if(unitGetEnable(i)) Control = 0x01; else Control = 0x00;	
			if(Control) {
				unitcrDataRequest(i);
				if(crGetType(i)) Control |= 0x02;
				unitcrAdd(i, Control);
			}
		} else {
			if(!(i & 1)) {
				unitInitRequest(i); unitInitRequest(i+1);
			}
			if(i > 1 && unitGetEnable(i)) Control = 0x02; else Control = 0x00; 
			if(crGetType(i)) Control |= 0x01;
			if(Control) {
				unitIouDataRequest(i);
				unitCruDataRequest(i);
				unitAdd(i, Control);
			}
		}
	}
	for(i = 0;i < MAX_IOUNIT_SZ;i++) {
		if(iounitGetEnable(i)) {
			iounitInitRequest(i);
			//iounitDataRequest(i);
			iounitAdd(i, 0x01);
		}
	}
}

void ctUnits(void *arg)
{
	unsigned char	buf[8], ctm[8];
	int		i, rval, type, ID;

	fsClean();
	ProcessBackLightTimer();
	if(unitGetType(0) != 2) ProcessLocalUnit();
	ProcessDoors();
	ProcessAlarmAction();
	if(taState != taOldState) {
		taOldState = taState;
		ClisTAStateChanged();
	} 
	if(oldTickTimer == TICK_TIMER) {
		taskLoop++;
		if(taskLoop > 12000L) {		// below 1000, variable with RS-485 unit number 
cprintf("RTC error, Reset...: SEC_TIMER=%lu DS_TIMER=%lu\n", SEC_TIMER, DS_TIMER);
			ResetSystem();
		}
	} else {
		oldTickTimer = TICK_TIMER;
//cprintf("%ld ", taskLoop);
		taskLoop = 0L;
	}
//cprintf("%ld %ld %ld %ld\n", DS_TIMER, slvTimer, oldSlvLoop, slvLoop);
	if(oldSlvLoop == slvLoop) {
		 if((DS_TIMER-slvTimer) > 20) {
cprintf("%lu SIO Fail, Reset...: S=%d 485=%d SUBMSK=%08x MSK=%08x\n", DS_TIMER, sio0_state, rINTSUBMSK, rINTMSK);
			//ServerNetState = 0;
			//DesktopDrawServerNetState(); DesktopDrawDoor();
			//sioResetSlave();
			ResetSystem();
		}
	} else {
		oldSlvLoop = slvLoop;
		slvTimer = DS_TIMER;	
	}	
	rval = sioReadNoWait(0, buf, 8);
	if(rval != 8) taskAbort();
//cprintf("Rx [%02x", (int)buf[0]); for(i = 1;i < rval;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	type = buf[1] >> 5; ID = buf[1] & 0x1f;
	if(buf[0] == CONNECT_RSP) {
		if(type == 3) {
cprintf("%lu ACUnit-%d: Connected: Reset=%d\n", DS_TIMER, ID, buf[2]);
			unitcrConnected(ID, (int)buf[2]);
			crSetReliability(ID, 0);
			if(ServerNetState > 2) {
				crNotifyStatus(ID); crNotifyObjectsStatus(ID);
			}
			rtcGetDateTime(ctm);
			EventAdd(OT_CREDENTIAL_READER, ID, E_UNIT_CONNECTED, ctm, NULL);
			UpdateReaderState(ID); if(!(ID & 1)) UpdateMainState(ID);
		}
	} else if(buf[0] == DISCONNECT_RSP) {
		if(type == 3) {
cprintf("%lu ACUnit-%d: Disconneted\n", DS_TIMER, ID);
			crSetReliability(ID, REL_COMMUNICATION_FAILURE);
			if(ServerNetState > 2) crNotifyStatus(ID);
			rtcGetDateTime(ctm);
			EventAdd(OT_CREDENTIAL_READER, ID, E_UNIT_DISCONNECTED, ctm, NULL);
			UpdateReaderState(ID); if(!(ID & 1)) UpdateMainState(ID);
		} else if(type == 2) {
cprintf("%lu IOUnit-%d: Disconneted\n", DS_TIMER, ID);
			iounitSetReliability(ID, REL_COMMUNICATION_FAILURE);
			if(ServerNetState > 2) iounitNotifyStatus(ID);
			rtcGetDateTime(ctm);
			EventAdd(OT_IO_UNIT, ID, E_UNIT_DISCONNECTED, ctm, NULL);
			UpdateIOUnit(ID);
		} else if(type == 1) {
cprintf("%lu CRU-%d: Disconneted\n", DS_TIMER, ID);
			if(crGetType(ID)) {
				crSetReliability(ID, REL_COMMUNICATION_FAILURE);
				if(ServerNetState > 2) crNotifyStatus(ID);
				rtcGetDateTime(ctm);
				EventAdd(OT_CREDENTIAL_READER, ID, E_UNIT_DISCONNECTED, ctm, NULL);
				UpdateReaderState(ID);
			}
		} else {
cprintf("%lu IOU-%d: Disconneted\n", DS_TIMER, ID);
			if(!unitGetReliability(ID)) {
				unitSetReliability(ID, REL_COMMUNICATION_FAILURE);
				if(ServerNetState > 2) acunitNotifyStatus(ID);
				rtcGetDateTime(ctm);
				EventAdd(OT_AC_UNIT, ID, E_UNIT_DISCONNECTED, ctm, NULL);
			}
			UpdateMainState(ID);
		}
	} else if(buf[0] == DATA_IND || buf[0] == REQ_IND || buf[0] == REQ2_IND) {
		if(type == 3) {
			if(buf[0] == DATA_IND) {
cprintf("%lu ACUnit-%d: %02x-%02x\n", DS_TIMER, ID, (int)buf[2], (int)buf[3]);
				CliProcessRxCommon(ID, buf);
			} else if(buf[0] == REQ_IND) {
cprintf("%lu ACUnit-%d: %02x-%02x Req=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxReqDataSize(ID));
				CliProcessRxCommon(ID, buf);
				CliProcessRequest(ID);
			} else {
cprintf("%lu ACUnit-%d: %02x-%02x Req2=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxReq2DataSize(ID));
				CliProcessRequest2(ID);
			}
		} else if(type == 2) {
cprintf("%lu IOUnit-%d: %02x-%02x-%02x-%02x-%02x-%02x\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], (int)buf[4], (int)buf[5], (int)buf[6], (int)buf[7]);
			if(iounitGetReliability(ID)) {
				iounitSetReliability(ID, 0);
				if(ServerNetState > 2) {
					iounitNotifyStatus(ID); iounitNotifyObjectsStatus(ID);
				}
				rtcGetDateTime(ctm);
				EventAdd(OT_IO_UNIT, ID, E_UNIT_CONNECTED, ctm, NULL);
				UpdateIOUnit(ID);
			}
			if(iounitGetType(ID) == 0) ProcessRemoteIU(ID, buf);
		} else if(type == 1) {
if(buf[0] == DATA_IND) cprintf("%lu CRU-%d: %02x-%02x\n", DS_TIMER, ID, (int)buf[2], (int)buf[3]);
else	cprintf("%lu CRU-%d: %02x-%02x Data=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxReqDataSize(ID));
			if(crGetType(ID) && crGetReliability(ID)) {
				crSetReliability(ID, 0);
				if(ServerNetState > 2) crNotifyStatus(ID);
				rtcGetDateTime(ctm);
				EventAdd(OT_CREDENTIAL_READER, ID, E_UNIT_CONNECTED, ctm, NULL);
				UpdateReaderState(ID);
			}
			ProcessRemoteCRU(ID, buf);
		} else {
if(buf[0] == DATA_IND) cprintf("%lu IOU-%d: %02x-%02x\n", DS_TIMER, ID, (int)buf[2], (int)buf[3]);
else	cprintf("%lu IOU-%d: %02x-%02x Data=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxReqDataSize(ID));
			if(unitGetReliability(ID)) {
				unitSetReliability(ID, 0);
				if(ID & 1) i = ID - 1; else i = ID;
				rtcGetDateTime(ctm);
				EventAdd(OT_AC_UNIT, ID, E_UNIT_CONNECTED, ctm, NULL);
				if(adIsEnable(i) && adGetPresentValue(i) == 1) adSetPresentValue(i, 0);
				i++; 
				if(adIsEnable(i) && adGetPresentValue(i) == 1) adSetPresentValue(i, 0);
				if(ServerNetState > 2) {
					acunitNotifyStatus(ID); acunitNotifyObjectsStatus(ID);
				}
				UpdateMainState(ID);
			}
			ProcessRemoteIOU(ID, buf);
		}
	} else if(type == 3) {
		if(buf[0] == RSP_CNF) {
cprintf("%lu ACUnit-%d: %02x-%02x Rsp=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxRspDataSize(ID));
			CliProcessRxCommon(ID, buf);
			CliProcessResponse(ID);
		} else if(buf[0] == SEG_RSP_CNF) {
//cprintf("%lu ACUnit-%d: %02x-%02x SegRsp=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxRspDataSize(ID));
			CliProcessRxCommon(ID, buf);
			CliProcessSegResponse(ID);
		} else if(buf[0] == SEG_REQ_IND) {
//cprintf("%lu ACUnit-%d: %02x-%02x SegReq=%d\n", DS_TIMER, ID, (int)buf[2], (int)buf[3], SlaveNetGetRxReqDataSize(ID));
			CliProcessRxCommon(ID, buf);
			CliProcessSegRequest(ID);
		} else {
cprintf("%lu ACUnit-%d: %02x-%02x unknown", DS_TIMER, ID, (int)buf[2], (int)buf[3]);
		}
	} else {
cprintf("%lu Unknown ID=%d [%02x", DS_TIMER, ID, (int)buf[0]); for(i = 1;i < rval;i++) cprintf("-%02x", (int)buf[i]); cprintf("]\n");
	}
}
