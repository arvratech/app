#ifndef _APP_ACT_H_
#define _APP_ACT_H_


#define GM_CRED_CAPTURED		GM_USER+0
#define GM_IDENTIFY_REQUEST		GM_USER+1
#define GM_IDENTIFY_RESULT		GM_USER+2
#define GM_VERIFY_REQUEST		GM_USER+3
#define GM_VERIFY_RESULT		GM_USER+4
#define GM_NI_CHANGED			GM_USER+5
#define GM_SLVNET_CHANGED		GM_USER+6
#define GM_UNCONF_IND			GM_USER+7
#define GM_CONF_IND				GM_USER+8
#define GM_CONF_CNF				GM_USER+9
#define GM_SEG_IND				GM_USER+10
#define GM_SEG_CNF				GM_USER+11
#define GM_CONTROL_REQ			GM_USER+12
#define GM_SVRNET_CHANGED		GM_USER+13
#define GM_SVRNET_RESULT_USER	GM_USER+14
#define GM_AD_LOCK_CHANGED		GM_USER+15
#define GM_AD_DOOR_CHANGED		GM_USER+16
#define GM_WPA_STATE_CHANGED	GM_USER+17
#define GM_WPA_SCAN_RESULTS		GM_USER+18
#define GM_SLV_SYNC				GM_USER+19
#define GM_TAMPER_NORMAL		GM_USER+20
#define GM_POWER_LEVEL			GM_USER+21
#define GM_SS_CHANGED			GM_USER+22
#define GM_FILE_START			GM_USER+23
#define GM_FILE_CONTROL			GM_USER+24
#define GM_BEACON_RECEIVED		GM_USER+25
#define GM_DATETIME_CHANGED		GM_USER+26
#define GM_FPM_TOUCHED			GM_USER+27
#define GM_FPM_REQUEST			GM_USER+28
#define GM_FPM_RESPONSE			GM_USER+29
#define GM_ALERT_CHANGED		GM_USER+30
#define GM_CAM_CHANGED			GM_USER+31
#define GM_BLE_CHANGED			GM_USER+32
#define GM_CONF2_CNF			GM_USER+33
#define GM_BEACONS_UPDATED		GM_USER+34
#define GM_WALLPAPER_CHANGED	GM_USER+35
#define GM_PING_RESULT			GM_USER+36
#define GM_CALL_STATE_CHANGED	GM_USER+37
#define GM_FILE_CHANGED			GM_USER+38

#define ACT_STATBAR				1
#define ACT_TOPMSG				2
#define ACT_PJSCALL				3
#define ACT_ADMPASSWORD			4
#define ACT_AUTHREQUEST			5
#define ACT_AUTHRESULT			6
#define ACT_AUTHDONE			7
#define ACT_AUTHCAPTURE			8
#define ACT_MP					9
#define ACT_USERID				10
#define ACT_AUTHPIN				11
#define ACT_TEXT_INPUT			12	
#define ACT_ADM					13	
#define ACT_PHONECALL			14	

#define ACT_ADM_ADD				50
#define ACT_ADM_EDIT			51
#define ACT_ADM_DELETE			52
#define ACT_ADM_INPUT			60

#define RESULT_TIMEOUT			RESULT_FIRST_USER+0
#define RESULT_ERROR			RESULT_FIRST_USER+1
#define RESULT_CRED_CAPTURED	RESULT_FIRST_USER+2


typedef struct _AUTH_USER {
	long	id;
	unsigned char	accessTime[6];
	unsigned char	accessEvent;
	unsigned char	status;
	unsigned char	credType;
	unsigned char	credData[9];
	unsigned char	msgIndex;
	unsigned char	authIndex;
	unsigned char	ments[4];
	char	employNo[12];
	char	userName[26];
	char	userExternalId[14];
	char	company[50];
	char	department[50];
	char	position[36];
	int		jpegSize;
	unsigned long	jpegBuffer[20480];	// 81,920 bytes
} AUTH_USER;


void MainOnCreate(GACTIVITY *act);
void AuthRequestStart(void);
void AuthRequestRestart(void);
void AuthCaptureStart(void);
void AuthResultStart(void);
void AuthDoneStart(void);
void TopMsgStart(char *intent);
void PjsCallStart(int requestCode, char *intent);
void StatBarStart(int y);
void SleepStart(void);
void BgScreenStart(void);
void AcuRequestStart(char *intent, void *ps, void (*AcuRequestOnResult)(GACTIVITY *, int, unsigned char *, int));
void AcuRequestRestart(char *intent, void (*AcuRequestOnResult)(GACTIVITY *, int, unsigned char *, int));
void AcuRequestSetOnSuccess(void (*AcuRequestOnSuccess)(GACTIVITY *));
void AcuRequestDestroy(GACTIVITY *act);

void AdmRoot(int requestCode);
void AdmStart(void *pss);
void AdmRootExit(void);
void AdmPassword(int requestCode);
void PhoneCall(int requestCode);
void PhoneCallStart(void *pss);

void AdmMasterPassword(int requestCode, void *ps);
void AdmUsr(int requestCode, void *ps);
void AdmClearTopUsr(int requestCode, void *ps);
void AdmUsrRdStart(int requestCode, void *puser);
void AdmUsrWrStart(int requestCode, void *puser);
void AdmUsrCredStart(int requestCode, void *ps);
void AdmAcct(int requestCode, void *ps);
void AdmClearTopAcct(int requestCode, void *ps);
void AdmAcctRdStart(int requestCode, void *puser);
void AdmAcctWrStart(int requestCode, void *puser);

void AdmHwStart(void *pss);
void AdmHwDevStart(void *pss);
void AdmHwUnitStart(void *pss);
void AdmHwDevChStart(void *pss);
void AdmHwSubdevChStart(void *pss);
void AdmHwUnitChStart(void *pss);
void AdmHwAdStart(void *pss);
void AdmHwCrStart(void *pss);
void AdmHwSubdevStart(void *pss);
void AdmHwSubdevFromCrStart(void *pss);
void AdmHwBiStart(void *pss);
void AdmHwBoStart(void *pss);

void AdmAcStart(void *pss);

void AdmSet(void *pss);
void AdmSetAdmin(void *pss);
void AdmSetDateTime(void *pss);
void AdmSetDisplay(void *pss);
void AdmSetSound(void *pss);
void AdmSetSleepMode(void *pss);
void AdmSetPowerMgmt(void *pss);
void AdmVf(int requestCode, void *pss);
void AdmSetCam(void *pss);
void AdmSetBle(void *pss);
void AdmSetBleBeacon(int requestCode, void *pss);
void AdmSetIpPhone(void *pss);

void AdmNet(void *pss);
void AdmNetLan(void *pss);
void AdmNetWifi(int requestCode, void *ps);
void AdmNetPing(void *pss);
void AdmNetPingRun(int requestCode, void *ps);

void AdmSys(void *pss);
void AdmSysTest(void *pss);
void AdmSysTestRun(int requestCode, void *ps);


#endif

