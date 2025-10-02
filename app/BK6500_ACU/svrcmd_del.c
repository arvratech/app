void DeleteObject(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*p, *data;
	long	nID;
	int		rval, Result;

	if(reqnet->DataLength < 4) {
		netCodeReject(rspnet, R_PARAMETER_ERROR);
		tsm->SvcPending = 1;
		return;
	}
	data = reqnet->Data;
	if(tsmsWritingObject(RspTSMs, MAX_RSP_TSM_SZ, tsm, (int)data[0])) {
		Result = R_ACTION_NOT_ALLOWED;
		netCodeError(rspnet, 1, Result);
		tsm->SvcPending = 1;
		return;
	}
	Result = 0;
	PACK3toID(data+1, &nID);
	p = data + 4;
cprintf("DeleteObject: ID=%d %02x-%02x-%02x-%02x Data=%d\n", nID, (int)data[0], (int)data[1], (int)data[2], (int)data[3], reqnet->DataLength-4);
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	switch(data[0]) {
#ifndef STAND_ALONE
	case OT_ACCESS_ZONE:
		if(nID < 0) rval = azRemoveAll();
		else if(azValidateID(nID) && nID) rval = azRemove(nID);  
		else	Result = R_PARAMETER_ERROR;
		break;
#endif
	case OT_OUTPUT_COMMAND:
		if(nID < 0) rval = ocRemoveAll();
		else if(ocValidateID(nID) && nID > 1) rval = ocRemove(nID);  
		else	Result = R_PARAMETER_ERROR;
		break;
	case OT_CALENDAR:
		if(nID < 0) rval = calfsRemoveAll();
		else if(calValidateID(nID)) rval = calfsRemove(nID);  
		else	Result = R_PARAMETER_ERROR;
		break;
	case OT_SCHEDULE:
		if(nID < 0) rval = schefsRemoveAll();
		else if(scheValidateID(nID)) rval = schefsRemove(nID);
		else	Result = R_PARAMETER_ERROR;
		break;
	case OT_USER:
		wdtResetLong();
		if(nID < 0)	rval = userfsRemoveAll();
		else if(userValidateID(nID)) rval = userfsRemove(nID);
		else	Result = R_PARAMETER_ERROR;
		wdtReset();
		break;
	case OT_ACCESS_RIGHTS:
		if(nID < 0) rval = arfsRemoveAll();
		else if(arValidateID(nID)) rval = arfsRemove(nID);  
		else	Result = R_PARAMETER_ERROR;
		break;
	case OT_DEVICE_FILE:
		if(nID < 64 || nID > 255) Result = R_PARAMETER_ERROR;
		else {
			flashWriteEnable();
			rval = fsDelete(nID);
			flashWriteDisable();
			if(rval < 0) {
				if(fsErrno == E_FS_FILE_NO_EXIST) Result = R_OBJECT_NOT_FOUND;
				else	Result = R_MSG_SYSTEM_ERROR;
			}
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(!Result) {
		if(rval < 0) Result = R_R_SYSTEM_ERROR;
		else if(!rval) Result = R_OBJECT_NOT_FOUND;
	}
	if(Result) {
		netCodeError(rspnet, 1, Result);
	} else {
		rspnet->DataLength = 0;
		netCodeResponse(rspnet);
	}
	tsm->SvcPending = 1;
}
