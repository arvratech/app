void ReadObjectList(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*d;
	int		rval, Result, nSize;

	d = rspnet->Data;
	if(reqnet->Type == T_REQ) {
		if(reqnet->DataLength < 1) {
			netCodeReject(rspnet, R_PARAMETER_ERROR);
			tsm->SvcPending = 1;
			return;
		}
		tsm->ObjectType = reqnet->Data[0];
		tsm->Position	= 0;
		*d++ = tsm->ObjectType;
	}
cprintf("ReadObjectList: OT=%02x Position=%d\n", (int)tsm->ObjectType, tsm->Position);
	Result = 0; nSize = 2048;
	switch(tsm->ObjectType) {
	case OT_CALENDAR:
		calfsSeek(tsm->Position);
		rval = calfsReadBulk(d, nSize);
		if(calfsEof()) tsm->Position = 0L;
		else	tsm->Position = calfsTell();		
		break;
	case OT_SCHEDULE:
		schefsSeek(tsm->Position);
		rval = schefsReadBulk(d, nSize);
		if(schefsEof()) tsm->Position = 0L;
		else	tsm->Position = schefsTell();
		break;
	case OT_USER:
		userfsSeek(tsm->Position);
		rval = userfsReadBulk(d, nSize);
//cprintf("userfsReadBulk=%d %02x-%02x-%02x-%02x\n", rval, (int)d[0], (int)d[1], (int)d[2], (int)d[3]);
		if(userfsEof()) tsm->Position = 0L;
		else	tsm->Position = userfsTell();
		break;
	case OT_ACCESS_RIGHTS:
		arfsSeek(tsm->Position);
		rval = arfsReadBulk(d, nSize);
		if(arfsEof()) tsm->Position = 0L;
		else	tsm->Position = arfsTell();
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(!Result && rval < 0) Result = R_R_SYSTEM_ERROR;	
	if(reqnet->Type == T_REQ) {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeError(rspnet, 1, Result);
		else {
			d += rval; rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	} else {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeAbortSrv(rspnet, Result);
		else {
			d += rval; rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	}
	tsm->SvcPending = 1;
}

void ReadPropertyList(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned long	lbuf[460];
	unsigned char	*d;
	int		rval, Result, nSize;

	d = rspnet->Data;
	if(reqnet->Type == T_REQ) {
		if(reqnet->DataLength < 2) {
			netCodeReject(rspnet, R_PARAMETER_ERROR);
			tsm->SvcPending = 1;
			return;
		}
		tsm->ObjectType = reqnet->Data[0]; tsm->PropertyID = reqnet->Data[1];
		tsm->Position = 0;
		*d++ = tsm->ObjectType; *d++ = tsm->PropertyID;
	}
cprintf("ReadPropertyList: OT=%02x OP=%02x Position=%d\n", (int)tsm->ObjectType, (int)tsm->PropertyID, tsm->Position);
	Result = 0; nSize = 2048;
	switch(tsm->ObjectType) {
	case OT_CREDENTIAL_READER:
		switch(tsm->PropertyID) {
		case OP_FP_IMAGE:
			//nSize = 260 * 300;
			//memcpy(p, sfpGetFPImage(), nSize);
			//p += nSize;
			rval = 0;
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	case OT_USER:
		switch(tsm->PropertyID) {
		case OP_FP_AUTHENTICATION_FACTOR:
			rval = userfsReadBulkFPTemplate((unsigned char *)lbuf, nSize, (long *)&tsm->Position);
			memcpy(d, lbuf, rval);
			break;
		case OP_USER_EX:
			userfsSeekEx(tsm->Position);
			rval = userfsReadBulkEx(d, nSize);
			if(userfsEofEx()) tsm->Position = 0L;
			else	tsm->Position = userfsTellEx();
			break;
		case OP_USER_PHOTO:
			userfsSeekPhoto(tsm->Position);
			rval = userfsReadBulkPhoto(d, nSize);
			if(userfsEofPhoto()) tsm->Position = 0L;
			else	tsm->Position = userfsTellPhoto();
			break;
		case OP_USER_ACCESS_RIGHTS:
			userfsSeekAccessRights(tsm->Position);
			rval = userfsReadBulkAccessRights(d, nSize);
			if(userfsEofAccessRights()) tsm->Position = 0L;
			else	tsm->Position = userfsTellAccessRights();
			break;
		default:	Result = R_PARAMETER_ERROR;
		}
		break;
	default:	Result = R_PARAMETER_ERROR;
	}
	if(!Result && rval < 0) Result = R_R_SYSTEM_ERROR;	
	if(reqnet->Type == T_REQ) {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeError(rspnet, 1, Result);
		else {
			d += rval; rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	} else {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeAbortSrv(rspnet, Result);
		else {
			d += rval; rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	}
	tsm->SvcPending = 1;
}

void ReadStatusList(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	unsigned char	*d;
	int		i, Result, nSize;

	d = rspnet->Data;
	if(reqnet->Type == T_REQ) {
		if(reqnet->DataLength < 1) {
			netCodeReject(rspnet, R_PARAMETER_ERROR);
			tsm->SvcPending = 1;
			return;
		}
		tsm->ObjectType = reqnet->Data[0];
		tsm->Position = 0;
		*d++ = tsm->ObjectType;;
	}
cprintf("ReadStatusList: OT=%02x Position=%d\n", (int)tsm->ObjectType, tsm->Position);
	Result = 0;
	nSize = 2048;
	switch(tsm->ObjectType) {
#ifndef STAND_ALONE
	case OT_BINARY_INPUT:
		for(i = 0;i < MAX_BI_SZ+MAX_XBI_SZ;i++) {
			if(biIsEnable(i)) {
				*d++ = OT_BINARY_INPUT;		// ???????
				*d++ = i;
				d += biEncodeStatus(i, d);
			}
		}
		tsm->Position = 0L;
		break;
#endif
	default:	Result = R_PARAMETER_ERROR;
	}
	if(reqnet->Type == T_REQ) {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeError(rspnet, 1, Result);
		else {
			rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	} else {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeAbortSrv(rspnet, Result);
		else {
			rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	}
	tsm->SvcPending = 1;
}

void ReadFile(NET_TSM *tsm, NET *reqnet, NET *rspnet)
{
	FS_FILE	*f;
	BMPC	*bmp, _bmp;
	unsigned char	*s, *d;
	int		rval, Result, nSize, File;

	if(reqnet->Type == T_REQ) {
		if(reqnet->DataLength < 1) {
			netCodeReject(rspnet, R_PARAMETER_ERROR);
			tsm->SvcPending = 1;
			return;
		}
		tsm->ObjectType = reqnet->Data[0];
		tsm->Position = 0;
	}
	File = tsm->ObjectType;
cprintf("ReadFile: File=%d Position=%d\n", File, tsm->Position);
	d = rspnet->Data;
	Result = 0;
	nSize = 2048;
	if(File == 1) {	// screen dump
		s = GetXferBuffer(0);
		if(!tsm->Position) {
			bmp = &_bmp;
			bmpcInit(bmp, s, lcdGetWidth(), lcdGetHeight());
			lcdGet(0, 0, bmp);
			SetXferSize(0, lcdGetWidth() * lcdGetHeight() * 2);
			*d++ = (unsigned char)File;
		}
		rval = GetXferSize(0) - tsm->Position;
		if(rval < 2048) nSize = rval;
		memcpy(d, s+tsm->Position, nSize); d += nSize;
		if(rval > 2048) tsm->Position += nSize;
		else	tsm->Position = 0L;
	} else if(File < 64 || File > 255) {
		Result = R_PARAMETER_ERROR;
	} else {
		f = fsOpen(File, FS_READ);
		if(!f) {
			if(fsErrno == E_FS_FILE_NO_EXIST) Result = R_OBJECT_NOT_FOUND;
			else	Result = R_R_SYSTEM_ERROR;
		} else {
			if(!tsm->Position) *d++ = (unsigned char)File;
			fsSeek(f, tsm->Position, SEEK_SET);
			rval = fsRead(f, d, nSize);
			if(rval < 0) Result = R_R_SYSTEM_ERROR;
			else	d += rval;
			if(fsEof(f)) tsm->Position = 0L;
			else	tsm->Position = fsTell(f);
			fsClose(f); 
		}
	}
	if(reqnet->Type == T_REQ) {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeError(rspnet, 1, Result);
		else {
			rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	} else {
		if(Result == R_PARAMETER_ERROR) netCodeReject(rspnet, Result);
		else if(Result) netCodeAbortSrv(rspnet, Result);
		else {
			rspnet->DataLength = d - rspnet->Data;
			if(tsm->Position) netCodeResponseMor(rspnet);
			else	netCodeResponse(rspnet);
		}
	}
	tsm->SvcPending = 1;
}
