#ifndef _ADM_H_
#define _ADM_H_


#define AdminConsoleUnsupported() { if(MenuGetAdminConsole()) { ResultMsg(NULL, NULL, -1); return; } }

int  MenuAdminAuth(void);
void MenuAdm(void);

void MenuSetViewEnable(int Enable);
void MenuSetViewPage(int Page);
int  MenuGetViewPage(void);
void UpdateMainState(int ID);
void UpdateIOUnit(int ID);
void UpdateDoorStatus(int ID);
void UpdateDoorPV(int ID);
void UpdateReaderState(int ID);
void UpdateReaderEvent(int ID);
void UpdateInputState(int biID);
void UpdateOutputState(int boID);
void MenuViewUnits(char *Title);
void MenuViewUnitsInOut(char *Title);
void MenuViewIOUnits(char *Title);
void MenuViewMain(void);
void UpdateViewMain(void);

void _SetOptionYesNo(char *buf, char *MsgString, unsigned char Data, int OptionFlag);
void _SetValueYesNo(char *buf, char *MsgString, unsigned char Data);
void _SetValueAsterisk(char *buf, char *MsgString, unsigned char Data);



#endif	/* _ADM_H_ */