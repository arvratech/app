void ReinitializeDevice(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		return;
	}
cprintf("ReinitializeDevice\n");
	gResetFlag = 1;
	rspnet->DataLength = 0;
	netCodeResponse(rspnet);
	tsm->SvcPending = 1;	
}

#ifndef STAND_ALONE

void CrDiarm(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	int		result;

cprintf("CrDisarm: crId=%d\n", nID);
	result = 0;
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(!sys_cfg->CredentialReaders[nID].IntrusionMode || !crGetIntrusionPresentValue(nID)) result = R_ACTION_NOT_ALLOWED;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		rspnet->DataLength = 1;
		rspnet->Data[0] = 0;
		netCodeResponse(rspnet);
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;		
		CliWriteProperty(cr, OP_INTRUSION_PRESENT_VALUE);
	}
}

void CrReadProperty(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet, int PropertyID)
{
	CREDENTIAL_READER	*cr;
	int		result;

cprintf("CrReadProperty: id=%d op=%02x\n", nID, PropertyID);
	result = 0;
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		if(reqnet->DataLength > 5) {
			rspnet->DataLength = reqnet->DataLength - 5;
			memcpy(rspnet->Data, reqnet->Data+5, rspnet->DataLength);
		} else	reqnet->DataLength = 0;
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliReadProperty(cr, PropertyID);
	}	
}

void CrWriteProperty(int nID, NET_TSM *tsm, NET *reqnet, NET *rspnet, int PropertyID)
{
	CREDENTIAL_READER	*cr;
	int		result;

cprintf("CrWriteProperty: crId=%d op=%02x\n", nID, PropertyID);
	result = 0;
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		if(reqnet->DataLength > 5) {
			rspnet->DataLength = reqnet->DataLength - 5;
			memcpy(rspnet->Data, reqnet->Data+5, rspnet->DataLength);
		} else	reqnet->DataLength = 0;
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;		
		CliWriteProperty(cr, PropertyID);
	}
}

void CrReinitialize(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrReinitialize: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliReinitialize(cr);
	}
}

void CrRunScript(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrRunScript: CrId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliRunScript(cr, data+1, reqnet->DataLength-1);
	}
}

void CrDownloadFirmware(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrDownloadFirmware: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliDownloadFirmware(cr);
	}
}

void CrDownloadWallPaper(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrDownloadWallPaper: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliDownloadWallPaper(cr);
	}
}

void CrDownloadKernel(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrDownloadKernel: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliDownloadKernel(cr);
	}
}

void CrDownloadVideo(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
cprintf("CrDownloadVideo: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliDownloadVideo(cr);
	}
}

void CrDownloadPoll(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	CREDENTIAL_READER	*cr;
	unsigned char	*data;
	int		result, nID;

	if(reqnet->DataLength < 1) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;	
		return;
	}
	data = reqnet->Data;
	result = 0;
	nID = *data;
//cprintf("CrDownloadStatus: crId=%d\n", nID);
	if(unitGetType(nID) != 2 || !crIsEnable(nID)) result = R_OBJECT_NOT_FOUND;
	else if(CliIsBusy(nID)) result = R_ACTION_NOT_ALLOWED;
	if(result) {
		netCodeError(rspnet, 1, result);
		tsm->SvcPending = 1;
	} else {
		cr = crsGetAt(nID);
		cr->nettsm = tsm;
		cr->Client = CLI_BYPASS_REQUESTED;
		CliDownloadPoll(cr);
	}
}

#endif
