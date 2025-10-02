#ifndef _UNIT_H_
#define _UNIT_H_


#define REL_COMMUNICATION_FAILURE	9

#define S_UNIT_INACTIVE		0
#define S_UNIT_ACTIVE		1


typedef struct _UNIT {
	unsigned char	ID;
	unsigned char	Reliability;
	unsigned char	ReliabilityTime[6];		
	unsigned char	IouReq[3];
	unsigned char	IouRsp[4];	
	unsigned char	CruReq[6];
	unsigned char	CruRsp[4];
	unsigned char	reserve[3];
	unsigned long	Timer;
} UNIT;

typedef struct _IOUNIT {
	unsigned char	ID;
	unsigned char	Reliability;	// 0:NoFauliDetected 9:CommunicationFailure	
	unsigned char	ReliabilityTime[6];
	unsigned char	Req[2];
	unsigned char	Rsp[6];
} IOUNIT;


void unitsResetSystem(void);
void unitsInit(UNIT *Units, int size);
int  unitsEncodeStatusAll(void *buf);
UNIT *unitsGetAt(int ID);
int  unitGetReliability(int ID);
void unitSetReliability(int ID, int Reliability);
void iounitsInit(IOUNIT *Units, int size);
IOUNIT *iounitsGetAt(int ID);
int  iounitGetReliability(int ID);
void iounitSetReliability(int ID, int Reliability);
void unitEncodeIouConfig(int UnitID);
void iounitEncodeConfig(int UnitID);
void unitInitRequest(int ID);
void unitcrInitRequest(int ID);
void unitcrConnected(int ID, int reset);
void iounitInitRequest(int UnitID);
void unitSetCruVersion(int UnitID);
void unitSetDoorPV(int UnitID, int PresentValue);
void unitSetOutputPV(int boID, int PresentValue);
void unitcrSetDoorPV(int UnitID, int PresentValue);
void unitcrSetOutputPV(int boID, int PresentValue);
void unitcrSetState(int ID);
void unitSetCruState(int UnitID);
void unitSetCruDoor(int UnitID);
void unitSetCruCaptureStart(int ID, int Mode);
void unitSetCruCaptureEnd(int ID);
void unitcrSetCaptureStart(int ID, int Mode);
void unitcrSetCaptureEnd(int ID);
void unitAuthResult(int UnitID, int Result);
void unitAdd(int UnitID, int Control);
void unitcrAdd(int UnitID, int Control);
void iounitAdd(int UnitID, int Control);
void iounitSetPresentValue(int UnitID, int boID, int PresentValue);
void unitIouDataRequest(int UnitID);
void unitCruDataRequest(int UnitID);
void unitcrDataRequest(int UnitID);
void iounitDataRequest(int UnitID);
void unitcrProcessReq(int ID, unsigned char *buf);
UNIT *unitsGetUnits(void);
int  acunitEncodeStatus(int ID, unsigned char *Buffer);
void acunitNotifyStatus(int ID);
void acunitNotifyObjectsStatus(int ID);
int  acunitsEncodeStatus(unsigned char *Buffer);
int  iounitEncodeStatus(int ID, unsigned char *Buffer);
void iounitNotifyStatus(int ID);
void iounitNotifyObjectsStatus(int ID);
int  iounitsEncodeStatus(unsigned char *Buffer);

int  adGetReliability(int adID);
int  adEvaluateAlarmState(int adID);
int  adGetDoorPV(int adID);
int  biGetPresentValue(int biID);
int  biGetAlarmState(int biID);
int  biGetReliability(int biID);
int  crGetTamperPresentValue(int crID);
int  crGetTamperAlarmState(int crID);
int  crGetIntrusionPresentValue(int crID);
int  crGetIntrusionAlarmState(int crID);


#endif	/* _UNIT_H_ */