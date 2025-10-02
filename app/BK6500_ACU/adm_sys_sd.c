void MenuImportFirmware(GMENU *ParentMenu, char *Title);
void MenuImportResources(GMENU *ParentMenu, char *Title);
void MenuImportSetupInitializatiion(GMENU *ParentMenu, char *Title);
void MenuImportUserFiles(GMENU *ParentMenu, char *Title);
void MenuImportSetupFiles(GMENU *ParentMenu, char *Title);
#ifndef STAND_ALONE
void MenuDownloadFirmware(GMENU *ParentMenu, char *Title);
#endif

void MenuImportFromSDMemory(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
#ifndef STAND_ALONE
	rval = 6;
#else
	rval = 5;
#endif
	MenuInitialize(menu, Title, xmenu_imexport, rval);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:	MenuImportFirmware(menu, ItemText); break;
		case 2:	MenuImportResources(menu, ItemText); break;
		case 3:	MenuImportSetupInitializatiion(menu, ItemText); break;
		case 4:	MenuImportUserFiles(menu, ItemText); break;
		case 5:	MenuImportSetupFiles(menu, ItemText); break;
#ifndef STAND_ALONE
		case 6:	MenuDownloadFirmware(menu, ItemText); break;
#endif
		}
	}
	MenuTerminate(menu);
}

void MenuExportUserFiles(GMENU *ParentMenu, char *Title);
void MenuExportSetupFiles(GMENU *ParentMenu, char *Title);

void MenuExportToSDMemory(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	AdminConsoleUnsupported();
	menu = &_menu;
	MenuInitialize(menu, Title, &xmenu_imexport[3], 2);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:
			MenuExportUserFiles(menu, ItemText);
			break;
		case 2:
			MenuExportSetupFiles(menu, ItemText);
			break;
		}
	}
	MenuTerminate(menu);
}

static int _CheckSDIState(GMENU *ParentMenu, char *Title)
{
	int		state;
	
	state = sdiGetState();
	if(state < 2) ResultMsg((GWND *)ParentMenu, Title, R_NO_SD_DISK);
	else if(state == 2) ResultMsg((GWND *)ParentMenu, Title, R_NOT_READY_SD_DISK);
	if(state < 3) state = -1;
	else	state = 0;
	return state;
}

static void _DirOpenError(GMENU *ParentMenu, char *Title, char *DirPath, FRESULT res)
{
	if(res == FR_NO_DISK) ResultMsg((GWND *)ParentMenu, Title, R_NO_SD_DISK);
	else if(res == FR_NOT_READY) ResultMsg((GWND *)ParentMenu, Title, R_NOT_READY_SD_DISK);
	else if(res == FR_NO_PATH) ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONWARNING, "[%s]%s", DirPath, GetResultMsg(R_DIRECTORY_NOT_FOUND));
	else	ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONERROR, "[%s]%s", DirPath, GetResultMsg(R_DIR_OPEN_ERROR));
}

static void _FileOpenError(GMENU *ParentMenu, char *Title, char *FilePath, FRESULT res)
{
	if(res == FR_NO_DISK) ResultMsg((GWND *)ParentMenu, Title, R_NO_SD_DISK);
	else if(res == FR_NOT_READY) ResultMsg((GWND *)ParentMenu, Title, R_NOT_READY_SD_DISK);
	else if(res == FR_NO_FILE) ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONWARNING, "[%s]%s", FilePath, GetResultMsg(R_FILE_NOT_FOUND));
	else	ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONERROR, "[%s]%s", FilePath, GetResultMsg(R_FILE_OPEN_ERROR));
}

static void _FileReadError(GMENU *ParentMenu, char *Title, char *FilePath, FRESULT res)
{
	ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONERROR, "[%s]%s", FilePath, GetResultMsg(R_FILE_READ_ERROR));
}

static void _FileWriteError(GMENU *ParentMenu, char *Title, char *FilePath, FRESULT res)
{
	ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONERROR, "[%s]%s", FilePath, GetResultMsg(R_FILE_READ_ERROR));
}

static void _FileSyncError(GMENU *ParentMenu, char *Title, char *FilePath, FRESULT res)
{
	ResultMsgEx((GWND *)ParentMenu, Title, MSG_TIMEOUT, MB_ICONERROR, "[%s]%s", FilePath, GetResultMsg(R_FILE_SYNC_ERROR));
}

void MenuImportFirmware(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	DIR		dir;
	FIL		fil;
	FRESULT	res;
	char	*p, path[20], fpath[64];
    unsigned char	*buf;
    unsigned int	br;
	int		rval, size;

	AdminConsoleUnsupported();
	sprintf(fpath, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, fpath);
	if(rval != IDYES) return;
	wnd = &_wnd;
	wndInitialize(wnd, Title, 35, 63, 250, 114); 
	wndModifyStyle(wnd, 0, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	//lbl = &_lbl;
	//labelInitialize(lbl, wnd,  6,  8, 150, 24); 
	//labelSetWindowText(lbl, GetPromptMsg(M_ENTER_USER_ID));
	wndDrawWindow(wnd);
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(path, "/Firmware");
	res = f_opendir(&dir, path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	p = fpath;
	strcpy(p, path); p += strlen(p); *p++ = '/'; strcpy(p, sys_cfg->Device.Model); p += strlen(p); strcpy(p, ".bin");
	res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
	if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		return;
	}
	buf = GetMainBuffer();
	res = f_read(&fil, buf, GetMainBufferSize(), &br);
	if(res) {	// error
		_FileReadError(ParentMenu, Title, fpath, res);
		f_close(&fil);
		return;		
	}
	f_close(&fil);
	size = br;
	rval = WriteAppProgram(buf, size >> 2);
	if(rval < 1) {
		taskDelay(5);
		rval = WriteAppProgram(buf, size >> 2);
	}
	if(rval < 1) ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
	else {
		ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
		ResultMsg((GWND *)ParentMenu, Title, R_SYSTEM_RESTART);
		ResetSystem();
	}
}

int _ImportResource(char *path, GMENU *ParentMenu, char *Title, GLABEL *lbl);

void MenuImportResources(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	GLABEL	*lbl, _lbl;
	char	path[64];
	int		rval;

	AdminConsoleUnsupported();
	sprintf(path, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, path);
	if(rval != IDYES) return;
	wnd = &_wnd;
	wndInitialize(wnd, Title, 35, 63, 250, 114); 
	wndModifyStyle(wnd, 0, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	lbl = &_lbl;
	labelInitialize(lbl, wnd,  6,  8, 180, 24); 
	wndDrawWindow(wnd);
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(path, "/Icon");
	rval = _ImportResource(path, ParentMenu, Title, lbl);
	if(!rval) ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

int _ImportResource(char *path, GMENU *ParentMenu, char *Title, GLABEL *lbl)
{
	DIR		dir;
	FILINFO	fno;
	FIL		fil;
  	FS_FILE	*f;
	FRESULT	res;
	char	*p, *fn, fpath[64], temp[64];
    unsigned char	*buf;
    unsigned int	br;
	int		rval, size, file, sum, icon, buf_size;

	res = f_opendir(&dir, path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return -1;
	}
	icon = 0;
	buf = GetMainBuffer(); buf_size = GetMainBufferSize();
	flashWriteEnable();
	while(1) {
		res = f_readdir(&dir, &fno);
		if(res != FR_OK || fno.fname[0] == 0) {
			temp[0] = 0;
			break;
		}
		if(fno.fname[0] == '.') continue;
		fn = *fno.lfname ? fno.lfname : fno.fname;	// fno.lfname = OEM coded long filename
		p = fpath;
		strcpy(p, path);
		if(!strcmp(p, "/")) strcpy(p+1, fn);
		else {
			p += strlen(p); *p++ = '/'; strcpy(p, fn);
		}
		if(fno.fattrib & AM_DIR) continue;	// cprintf("DIR [%s]\n", fpath);
		extract_filename(fpath, temp);
		if(isdigits(temp) != 3) continue;
		file = n_atoi(temp);
		if(file < 100 && file > 255) continue;
		res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
		if(res) {
			_DirOpenError(ParentMenu, Title, fpath, res);
			break;
		}
//		if(!flashWriteEnabled()) flashWriteEnable();
		rval = fsDelete(file);
		f = fsCreate(file);
		if(!f) {
			ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			res = 255;
			break;
		}
		sum = 0;
		while(1) {
			res = f_read(&fil, buf, buf_size, &br);
			if(res) {	// error
				_FileReadError(ParentMenu, Title, fpath, res);
				break;
			}
			if(br == 0) break;	// eof
			size = br;
			rval = fsWrite(f, buf, size);
			if(rval != size) {
				ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
				res = 255;
				break;
			}
			sum += size;	
		}
		f_close(&fil);
		fsClose(f);
		if(res) fsDelete(file);
		if(res) break;
		labelDrawBackGround(lbl);
    	sprintf(temp, "%s => %d", fn, file);
   		labelSetWindowText(lbl, temp);
		labelDrawWindow(lbl);
		taskYield();
	}
	flashWriteDisable();
	if(res) rval = -1;
    else	rval = 0;
	return rval;
}

char dir_backup[12] = "/Backup";

int _ImportFile(GMENU *ParentMenu, char *Title, char *Path, int File)
{
	FIL		fil;
	FRESULT	res;
	FS_FILE	*f;
	char	*p, fpath[64];
	unsigned char	*buf;
	unsigned int	br;
	int		rval, size, buf_size;

	p = fpath; strcpy(p, Path); p += strlen(p); *p++ = '/'; sprintf(p, "%d", File);	
	res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
    if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		return (int)res;
	}
	buf = GetMainBuffer(); buf_size = GetMainBufferSize();
	buf_size = buf_size / 2048 * 2048;
	f = fsOpenAlreadyOpened(File);
	rval = fsSeek(f, 0L, SEEK_SET);	
	flashWriteEnable();
	while(1) {
		res = f_read(&fil, buf, buf_size, &br);
//if(res) cprintf("Import Tmpl res=%d\n", res);
//else	cprintf("Import Tmpl=%d\n", br);
		if(res) {	// error
			_FileReadError(ParentMenu, Title, fpath, res);
			break;
		}
     	if(br == 0) break;	// eof
		size = br;	
		rval = fsWrite(f, buf, size);
		if(rval < 0) {
			ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			res = 255;
			break;
		}
	}
	f_close(&fil);
	flashWriteDisable();
	return (int)res;
}

void MenuImportUserFiles(GMENU *ParentMenu, char *Title)
{
	DIR		dir;
	FRESULT	res;
	char	 path[64];
 	int		rval, file;

	AdminConsoleUnsupported();
	sprintf(path, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, path);
	if(rval != IDYES) return;
	if(_CheckSDIState(ParentMenu, Title)) return;
	ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING);
	strcpy(path, dir_backup);
	res = f_opendir(&dir, path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	rval = userfsRemoveAll();
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		return;	
	}
	rval = fsRemoveAllEvent();
	rval = pnofsRemoveAll();
	rval = fsRemoveAllCamImage();
	rval = calfsRemoveAll();
	rval = schefsRemoveAll();
	rval = arfsRemoveAll();
	for(file = 1;file < 13;file++) {
		rval = _ImportFile(ParentMenu, Title, path, file);
		if(rval) break;
	}
	if(rval) return;
	wdtResetLong();
	sfpExitTemplate();
	sfpInitTemplate(1);
	wdtReset();
	ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

int _ExportFile(GMENU *ParentMenu, char *Title, char *Path, int File)
{
	FIL		fil;
	FRESULT	res;
	FS_FILE	*f;
	char	*p, fpath[64];
	unsigned char	*buf;
	unsigned int	br;
	int		rval, size, buf_size;

	p = fpath; strcpy(p, Path); p += strlen(p); *p++ = '/'; sprintf(p, "%d", File);	
	res = f_open(&fil, fpath, FA_CREATE_ALWAYS | FA_WRITE);
    if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		return (int)res;
	}
	buf = GetMainBuffer(); buf_size = GetMainBufferSize();
	buf_size = buf_size / 2048 * 2048;
	f = fsOpenAlreadyOpened(File);	
	rval = fsSeek(f, 0L, SEEK_SET);
	while(1) {
		size = fsRead(f, buf, buf_size);
//cprintf("Export Tmpl=%d\n", size);
		if(size < 0) {
			ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			res = 255;
			break;
		} else if(size == 0) {
			res = 0;
			break;
		}
		res = f_write(&fil, buf, size, &br);
//cprintf("WR res=%d size=%d br=%d\n", res, size, br);
		if(res) {
			_FileWriteError(ParentMenu, Title, fpath, res);
			break;
		}
	}
	if(!res) {
		res = f_sync(&fil);
		if(res) _FileSyncError(ParentMenu, Title, fpath, res);
	}
	f_close(&fil);
	if(res) f_unlink(fpath);
	return (int)res;
}

void MenuExportUserFiles(GMENU *ParentMenu, char *Title)
{
	DIR		dir;
	FRESULT	res;
	char	path[64];
	int		rval, file;

	sprintf(path, "%s\n%s", xmenu_system[3], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, path);	
	if(rval != IDYES) return;
	if(_CheckSDIState(ParentMenu, Title)) return;
	ResultMsgNoWait((GWND *)ParentMenu, Title, R_PROCESSING);
	strcpy(path, dir_backup);
	res = f_opendir(&dir, path);
	if(res == FR_NO_PATH) res = f_mkdir(path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	for(file = 1;file < 13;file++) {
		rval = _ExportFile(ParentMenu, Title, path, file);
		if(rval) break;
	}
	if(!rval) ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

void MenuImportSetupFiles(GMENU *ParentMenu, char *Title)
{
	DIR		dir;
	FIL		fil;
	FRESULT	res;
	char	*p, path[16], fpath[64];
    unsigned char	*buf;
    unsigned int	br;
	int		rval, size, file;

	AdminConsoleUnsupported();
	sprintf(fpath, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, fpath);	
	if(rval != IDYES) return;
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(path, dir_backup);
	res = f_opendir(&dir, path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	buf = GetMainBuffer();
	file = 0;
	p = fpath; strcpy(p, path); p += strlen(p); *p++ = '/'; sprintf(p, "%d", file);
	res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
   	if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		return;
	}
	res = f_read(&fil, buf, 1024, &br);
	if(res) {
		_FileReadError(ParentMenu, Title, fpath, res);
		f_close(&fil);
		return;
	}
	f_close(&fil);
	size = br;
	rval = syscfgRecovery(buf, size);
	if(rval < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		res = 255;
	}
	if(!res) {
		ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
		ResultMsg((GWND *)ParentMenu, Title, R_SYSTEM_RESTART);
		ResetSystem();
	}
}

void MenuExportSetupFiles(GMENU *ParentMenu, char *Title)
{
	DIR		dir;
	FIL		fil;
	FRESULT	res;
	char	*p, path[16], fpath[64];
 	unsigned char	*buf;
    unsigned int	br;
	int		rval, size, file;

	sprintf(fpath, "%s\n%s", xmenu_system[3], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, fpath);	
	if(rval != IDYES) return;
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(path, dir_backup);
	res = f_opendir(&dir, path);
	if(res == FR_NO_PATH) res = f_mkdir(path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	buf = GetMainBuffer(); 
	file = 0;
	p = fpath; strcpy(p, path); p += strlen(p); *p++ = '/'; sprintf(p, "%d", file);
	res = f_open(&fil, fpath, FA_CREATE_ALWAYS | FA_WRITE);
   	if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		return;
	}
	size = syscfgBackup(buf);
	if(size < 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
		f_close(&fil); f_unlink(fpath);
		return;
	}
	res = f_write(&fil, buf, size, &br);
	if(res) {
		_FileOpenError(ParentMenu, Title, fpath, res);
		f_close(&fil); f_unlink(fpath);
		return;
	}
	res = f_sync(&fil);
	if(res) {
		_FileSyncError(ParentMenu, Title, fpath, res);
		f_close(&fil); f_unlink(fpath);
		return;
	}
	f_close(&fil);
	ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

extern SYS_CFG	*tmp_cfg, _tmp_cfg;
void _SetScKey(unsigned char *OldKey, char *NewKeyString);

void MenuImportSetupInitializatiion(GMENU *ParentMenu, char *Title)
{
	DIR		dir;
	FIL		fil;
	FRESULT	res;
	char	*p, name[128], value[128];
    unsigned char	*buf;
    unsigned int	br;
	int		rval, val, size, sckey, tatext;

	AdminConsoleUnsupported();
	sprintf(name, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, name);	
	if(rval != IDYES) return;
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(name, dir_backup);
	res = f_opendir(&dir, name);
	if(res) {
		_DirOpenError(ParentMenu, Title, name, res);
		return;
	}
	buf = GetMainBuffer();
	p = value; strcpy(p, name); p += strlen(p); *p++ = '/'; strcpy(p, "setup.cfg");
	res = f_open(&fil, value, FA_OPEN_EXISTING | FA_READ);
    if(res) {
 		_FileOpenError(ParentMenu, Title, value, res);
		return;
	}
	res = f_read(&fil, buf, 1024, &br);
	if(res) {
 		_FileReadError(ParentMenu, Title, value, res);
		f_close(&fil);
		return;
	}
	f_close(&fil);
	size = br;
	buf[size] = 0;
	tmp_cfg = &_tmp_cfg;
	memcpy(tmp_cfg, sys_cfg, sizeof(SYS_CFG));
	size = taGetTextSize();
	p = (char *)buf;
	while(p = get_profile(p, name, value)) {
		if(!name[0] || value[0] != '=') continue;
		stolower(name);
		if(!strcmp(name, "company")) strcpy(tmp_cfg->Device.Maker, value+1);
		else if(!strcmp(name, "primarykey")) _SetScKey(tmp_cfg->SCPrimaryKey, value+1);
		else if(!strcmp(name, "secondarykey")) _SetScKey(tmp_cfg->SCSecondaryKey, value+1);
		else if(!n_memcmp(name, "tatext", 6)) {
			val = n_atoi(name+6);
			if(val > 0 && val < size) strcpy(tmp_cfg->TATexts[val], value+1);
		}
	}
	sckey = sckeyCompare(sys_cfg->SCPrimaryKey, tmp_cfg->SCPrimaryKey);
	tatext = taCompareTexts(sys_cfg, tmp_cfg);
	if(syscfgCompare(sys_cfg, tmp_cfg)) {
#ifndef STAND_ALONE
		if(sckey) {
			tmp_cfg->SCKeyVersion++; if(!tmp_cfg->SCKeyVersion) tmp_cfg->SCKeyVersion = 1;
		}
		if(tatext) {
			tmp_cfg->TAVersion++; if(!tmp_cfg->TAVersion) tmp_cfg->TAVersion = 1;
		}
#endif
		rval = syscfgWrite(tmp_cfg);
		if(rval < 0) {
			ResultMsg((GWND *)ParentMenu, Title, R_MSG_SYSTEM_ERROR);
			return;
		}
		syscfgCopy(sys_cfg, tmp_cfg);
#ifndef STAND_ALONE
		if(sckey) ClisSCKeyChanged();
		else if(tatext) ClisTAConfigChanged();
#endif
	}
	ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

void _SetScKey(unsigned char *OldKey, char *NewKeyString)
{
	unsigned char	NewKey[8];
	int		size;

	size = strlen(NewKeyString);
	if(size == 12) {
		size = hexstr2bin(NewKeyString, NewKey);
		if(size == 6 && sckeyValidate(NewKey)) memcpy(OldKey, NewKey, 6);
	}
}

#include "event.h"

void MenuSafelyRemoveSDMemory(GMENU *ParentMenu, char *Title)
{
	int		state;	

	AdminConsoleUnsupported();
	state = sdiGetState();
	if(state < 2) {
		ResultMsg((GWND *)ParentMenu, Title, R_NO_SD_DISK);
		return;
	}
	SDCardCloseEvtFile();
//	SDCardExitCamFile();
	sdiRemoveCardSafely();
	f_mount((unsigned char)0, NULL);
	ResultMsg((GWND *)ParentMenu, Title, R_SAFELY_REMOVE_SD_DISK);
}

#ifndef STAND_ALONE

#include "unit.h"

void MenuImportDownloadFirmware(GMENU *ParentMenu, char *Title);
void MenuStartDownload(GMENU *ParentMenu, char *Title);
void MenuDownloadIncompleted(GMENU *ParentMenu, char *Title);
void MenuCancelDownload(GMENU *ParentMenu, char *Title);
void MenuViewDownloadStatus(GMENU *ParentMenu, char *Title);

void MenuDownloadFirmware(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	char	*ItemText;
	int		rval;

	menu = &_menu;
	MenuInitialize(menu, Title, xmenu_downfirmware, 5);
	while(1) {
		rval = MenuPopup(menu);
		if(rval <= 0) break;
		ItemText = MenuGetItemText(menu, rval - 1);
		switch(rval) {
		case 1:	MenuImportDownloadFirmware(menu, ItemText); break;
		case 2:	MenuStartDownload(menu, ItemText); break;
		case 3:	MenuDownloadIncompleted(menu, ItemText); break;
		case 4:	MenuCancelDownload(menu, ItemText); break;
		case 5:	MenuViewDownloadStatus(menu, ItemText); break;
		}
	}
	MenuTerminate(menu);
}

void MenuImportDownloadFirmware(GMENU *ParentMenu, char *Title)
{
	GWND	*wnd, _wnd;
	DIR		dir;
	FIL		fil;
	FRESULT	res;
	char	*p, path[20], fpath[64];
    unsigned char	*buf;
    unsigned int	br;
	int		rval, size;

	AdminConsoleUnsupported();
	if(CliGetXferState()) {
		ResultMsg((GWND *)ParentMenu, Title, R_ALREADY_PROCESSING);
		return;	
	}
	sprintf(fpath, "%s\n%s", xmenu_system[2], GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, fpath);
	if(rval != IDYES) return;
	wnd = &_wnd;
	wndInitialize(wnd, Title, 35, 63, 250, 114); 
	wndModifyStyle(wnd, 0, WS_CAPTION | WS_BORDER | WS_VISIBLE);
	//lbl = &_lbl;
	//labelInitialize(lbl, wnd,  6,  8, 150, 24); 
	//labelSetWindowText(lbl, GetPromptMsg(M_ENTER_USER_ID));
	wndDrawWindow(wnd);
	if(_CheckSDIState(ParentMenu, Title)) return;
	strcpy(path, "/Firmware");
	res = f_opendir(&dir, path);
	if(res) {
		_DirOpenError(ParentMenu, Title, path, res);
		return;
	}
	p = fpath;
	strcpy(p, path);
	p += strlen(p); *p++ = '/'; 
	strcpy(p, "BK6200.bin");
	res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
	if(res) {
		strcpy(p, "t35s");
		res = f_open(&fil, fpath, FA_OPEN_EXISTING | FA_READ);
		if(res) {
			_FileOpenError(ParentMenu, Title, fpath, res);	
			return;
		}
	}
	buf = CliGetXferBuffer();
	res = f_read(&fil, buf, CliGetXferBufferSize(), &br);
	if(res) {	// error
		_FileReadError(ParentMenu, Title, fpath, res);
		f_close(&fil);
		return;		
	}
	f_close(&fil);
	size = br;
	CliSetXferSize(size);
cprintf("size=%d...\n", CliGetXferSize());
	ResultMsg((GWND *)ParentMenu, Title, R_COMPLETED);
}

void MenuStartDownload(GMENU *ParentMenu, char *Title)
{
	char	*xmenu[MAX_UNIT_SZ+1];
	char	menuStr[MAX_UNIT_SZ][MAX_MSG_LEN];
	unsigned long	lVal;
	int		rval, i, Item, IDMask, IDs[MAX_UNIT_SZ];;
	
	if(CliGetXferState()) {
		ResultMsg((GWND *)ParentMenu, Title, R_ALREADY_PROCESSING);
		return;	
	}
	if(CliGetXferSize() <= 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_FILE_NOT_FOUND);
		return;	
	}
	xmenu[0] = xmenu_other[9];
	Item = 0;
	for(i = 0;i < MAX_UNIT_SZ;i++)
		if(unitGetType(i) == 2 && crIsEnable(i)) {
			GetUnitName(i, menuStr[i]);
			xmenu[Item+1] = menuStr[i];
			IDs[Item] = i; Item++;
		}
	lVal = 0x01;
	rval = MenuCheck2((GWND *)ParentMenu, Title, &lVal, xmenu, Item+1, 0);
	if(rval > 0) {
		if(lVal & 0x01) IDMask = 0xffffff;
		else {
			IDMask = 0;
			for(i = 0;i < Item;i++) {
				lVal >>= 1;
				if(lVal & 0x01) IDMask |= 0x01 << IDs[i];
			}
		}
		if(IDMask) {
			ClisXferFileChanged(IDMask);
			ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
		}
	}
}

void MenuDownloadIncompleted(GMENU *ParentMenu, char *Title)
{
	CREDENTIAL_READER	*cr;
	int		rval, i, IDMask, mask;
	
	if(CliGetXferState()) {
		ResultMsg((GWND *)ParentMenu, Title, R_ALREADY_PROCESSING);
		return;	
	}
	if(CliGetXferSize() <= 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_FILE_NOT_FOUND);
		return;	
	}
	IDMask = 0; mask = 0x1;
	cr = crsGetAt(0);
	for(i = 0;i < MAX_UNIT_SZ;i++, cr++) {
		if(unitGetType(i) == 2 && cr->XferState && cr->XferState != 2) IDMask |= mask;
		mask <<= 1;
	}
	if(IDMask) {
		ClisXferFileChanged(IDMask);
		ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
	}
}

void MenuCancelDownload(GMENU *ParentMenu, char *Title)
{
	char	buf[64];
	int		rval;

	if(CliGetXferState() <= 0) {
		ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
		return;	
	}
	sprintf(buf, "%s\n%s", Title, GetPromptMsg(M_ARE_YOU_REALLY));
	rval = WndMessageBoxYesNo((GWND *)ParentMenu, Title, buf);
	if(rval != IDYES) return;
	ClisXferFileCanceled();
	ResultMsg((GWND *)ParentMenu, Title, R_PROCESSED);
}

void MenuViewDownloadStatus(GMENU *ParentMenu, char *Title)
{
	GMENU	*menu, _menu;
	CREDENTIAL_READER	*cr;
	unsigned char	ctm[8];
	char	*p, temp[80], temp1[20], temp2[20];
	int		i, rval;

	menu = &_menu;
	MenuInitialize(menu, Title, NULL, 0);	// menu, Title, MenuStr, Size
	menuSetType(menu, GMENU_TYPE_VIEW_ONLY);
	menuSetPrefixNumber(menu, 0);
	CliGetXferLastTime(ctm);
	if(CliGetXferState()) sprintf(temp, "    %s", xmenu_other[21]);
	else	sprintf(temp, " %s %04d/%02d/%02d %02d:%02d", xmenu_other[22], (int)ctm[0]+2000, (int)ctm[1], (int)ctm[2], (int)ctm[3], (int)ctm[4]);
	MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);		 
	if(CliGetXferState() || ctm[0]) {
		cr = crsGetAt(0);
		for(i = 0;i < MAX_UNIT_SZ;i++, cr++)
			if(unitGetType(i) == 2 && cr->XferState) {
				GetUnitName(i, temp1);
				if(i == CliGetXferID()) {
					p = GetResultMsg(R_PROCESSING);
					rval = 100 * CliGetXferCount() / CliGetXferSize();
					sprintf(temp2, "%d %%", rval); 
				} else {
					if(cr->XferState == 2) p = xmenu_other[18];		// OK
					else if(cr->XferState == 3) p = xmenu_other[19];	// Fail
					else	p = xmenu_other[17];						// Wait
					if(!crGetReliability(i)) temp2[0] = 0;
					else	strcpy(temp2, xmenu_other[20]);
				}
				sprintf(temp, "%-8s %-13s %-12s", temp1, p, temp2);
				MenuAddItem(menu, temp, 0, GMENU_ITEM_ENABLE);		 
			}
	}
	rval = MenuPopup(menu);
	MenuTerminate(menu);	
}

#endif