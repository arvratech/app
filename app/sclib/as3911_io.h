#ifndef _AS3911_IO_H
#define _AS3911_IO_H


#define SC_TIMER            MS_TIMER


int  as3911OpenCom(void);
void as3911CloseCom(void);
int  as3911GetReg(int reg, unsigned char *data);
int  as3911GetRegs(int reg, unsigned char *buf, int length);
int  as3911SetReg(int reg, int data);
int  as3911SetRegs(int reg, unsigned char *buf, int length);
int  as3911ModReg(int reg, int clr_mask, int set_mask);
int  as3911ReadFifo(unsigned char *buf, int length);
int  as3911WriteFifo(unsigned char *buf, int length);
int  as3911ExecuteCommand(int command);
int  as3911ExecuteCommands(unsigned char *commands, int length);
int  as3911WaitScEvent(int timeout);


#endif
