#ifndef _SVR_CMD_H_
#define _SVR_CMD_H_


void SvrProcessConfRequest(NET_TSM *tsm, NET *reqnet);
void SvrProcessConfRequestMor(NET_TSM *tsm, NET *reqnet);
void SvrProcessAck(NET_TSM *tsm, NET *reqnet);

void ReadObject(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadProperty(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadStatus(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void WriteObject(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void WriteProperty(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void DeleteObject(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadObjectList(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadPropertyList(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadStatusList(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void WriteObjectList(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void WritePropertyList(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void ReadFile(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void WriteFile(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrReadProperty(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet, int PropertyID);
void ReinitializeDevice(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDiarm(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrWriteProperty(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet, int PropertyID);
void CrReinitialize(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrRunScript(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDownloadFirmware(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDownloadWallPaper(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDownloadKernel(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDownloadVideo(NET_TSM *tsm, NET *reqnet, NET *rspnet);
void CrDownloadPoll(NET_TSM *tsm, NET *reqnet, NET *rspnet);


#endif
