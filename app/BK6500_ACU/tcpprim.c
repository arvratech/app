void TcpConnect(void)
{
	switch(gCommand) {
	case 0:
		if((DS_TIMER-gTcpTimer) > gTcpTimeout) gCommand = 1;
		break;
	case 1:
			rval = sinStartClient(SOCK_SERVER, sys_cfg->ServerIPAddress, sys_cfg->ServerPort);
			if(rval) gTcpTimeout = 50;
			else {
				gTcpTimeout = 350; gCommand = 1;
			}
			gTcpTimer = DS_TIMER;
		}
		break;
	case 1:
		rval = sinPollClient(SOCK_SERVER);	// -1:Error 0:Continue 1:Closed 2:Connected
//if(rval < 0) cprintf("%ld Connect error...\n", DS_TIMER);
//else if(rval == 1) cprintf("%ld Connect closed...\n", DS_TIMER);
		if(rval == 0 && (DS_TIMER-gTcpTimer) > gTcpTimeout) { rval = 1; cprintf("%ld Connect Timeout..\n", DS_TIMER); }
		if(rval > 1) {
//cprintf("%ld Connected...\n", DS_TIMER);
			TcpDeviceID = sys_cfg->DeviceID;
			size = CodeLogin(buf, TcpDeviceID);
			rval = sinAsyncWrite(SINint s, void *buf, int size)
			rval = sinAsyncWrite(SOCK_SERVER, buf, size);
				if(rval <= 0) CloseTCP();
				else {
					gTcpTimer = DS_TIMER; gTcpTimeout = 90; gCommand = 2;
				}
			} else if(rval) {
				sinClose(SOCK_SERVER);
				gTcpTimer = DS_TIMER; gTcpTimeout = 70; gCommand = 0;
			}
			break;
		case 2:
			rval = sinWriteCompleted(SOCK_SERVER);
			if(rval == 0 && (DS_TIMER-gTcpTimer) > gTcpTimeout) rval = -1;
			if(rval > 0) {
				gCommand = 3;
			} else if(rval) {
				sinClose(SOCK_SERVER);
				gTcpTimer = DS_TIMER; gTcpTimeout = 70; gCommand = 0;
			}
			break;
		case 3:
			rval = sinReadGetResponseData(&Offset, &Length);
//if(rval > 0) svrLog(svrBuf, size, 0);
			if(rval == 0 && (DS_TIMER-gTcpTimer) > gTcpTimeout) rval = -1;
			if(rval > 0) {
				
				Result = CheckPacket(svrBuf, rval);
				if(Result == 0) Result = GetResult(svrBuf);
			}
			if(rval > 0 && Result == 0) {
				memset(OldStatus, 0, 8);	
				gTcpTimer = DS_TIMER; gCommand = 0;
				SetServerNetState(3);
			} else if(rval) {
				gTcpTimer = DS_TIMER; gTcpTimeout = 70; gCommand = 0;				
			}
			break;
		}
		break;
	case 3:
