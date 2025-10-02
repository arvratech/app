#ifndef _CLI_H_
#define _CLI_H_


#define CLIENT_TIMEOUT		35

#define LOCAL_DATETIME_CHANGED		0x0080
#define IO_CONFIG_CHANGED			0x0040
#define CR_CONFIG_CHANGED			0x0020
#define SCKEY_CHANGED				0x0010
#define TA_CONFIG_CHANGED			0x0008
#define ADMIN_PIN_CHANGED			0x0004
#define XFER_FILE_CHANGED			0x0002
#define CLI_BYPASS_REQUESTED		0x0001

void CliInit(void);
unsigned char *acunitGetXferBuffer(void);
int  acunitGetXferBufferSize(void);
void ClisTimeoutXferCompleted(void);
void CliDownload(int ID);
void CliProcessRxCommon(int ID, unsigned char *buf);
void CliProcessRequest(int ID);
void CliProcessResponse(int ID);
void CliProcessSegResponse(int ID);
void CliProcessSegRequest(int ID);
void CliProcessRequest2(int ID);
int  CliIsBusy(int ID);

void CliWriteLocalDateTime(CREDENTIAL_READER *cr);
void CliWriteIOConfig(CREDENTIAL_READER *cr);
void CliWriteCRConfig(CREDENTIAL_READER *cr);
void CliWriteSCKey(CREDENTIAL_READER *cr);
void CliWriteTAConfig(CREDENTIAL_READER *cr);
void CliWriteAdminPIN(CREDENTIAL_READER *cr);
void CliReadProperty(CREDENTIAL_READER *cr, int PropertyID);
void CliWriteProperty(CREDENTIAL_READER *cr, int PropertyID);
void CliReinitialize(CREDENTIAL_READER *cr);
void CliRunScript(CREDENTIAL_READER *cr, unsigned char *script, int len);
void CliDownloadFirmware(CREDENTIAL_READER *cr);
void CliDownloadWallPaper(CREDENTIAL_READER *cr);
void CliDownloadKernel(CREDENTIAL_READER *cr);
void CliDownloadVideo(CREDENTIAL_READER *cr);
void CliDownloadPoll(CREDENTIAL_READER *cr);
void ClisTAStateChanged(void);
void ClisLocalDateTimeChanged(void);
void ClisSCKeyChanged(void);
void ClisTAConfigChanged(void);
void ClisAdminPINChanged(void);
void ClisXferFileChanged(int IDMask);
void ClisXferFileCanceled(void);
void CliLocalDateTimeChanged(int ID);
void CliIOConfigChanged(int ID);
void CliCRConfigChanged(int ID);
void CliAdminPINChanged(int ID);

int  CliGetXferState(void);
void CliSetXferState(int State);
unsigned char *CliGetXferBuffer(void);
int  CliGetXferBufferSize(void);
int  CliGetXferSize(void);
void CliSetXferSize(int nSize);
int  CliGetXferCount(void);
int  CliGetXferID(void);
void CliGetXferLastTime(unsigned char *ctm);
void CliXferWRQ(CREDENTIAL_READER *cr);
void CliXferData(CREDENTIAL_READER *cr);
void CliXferError(CREDENTIAL_READER *cr, int Result);


#endif
