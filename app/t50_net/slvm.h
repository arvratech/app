#ifndef _SLVM_H_
#define _SLVM_H_


// Status Register definitions
#define STAT_COMMON_STATUS		0
#define STAT_IO_STATUS			2
#define STAT_KEY_CODE			5

// Sync flag degfinitions
#define SYNC_COMM_DEVICE		0x80
#define SYNC_COMM_OC			0x40
#define SYNC_COMM_SC_INFO		0x20
#define SYNC_COMM_LOCAL_REG		0x10

/*
#define biDevPV(id)				(commStats[0] >> (5-id) & 0x01)
#define biPV(id)				(commStats[2] >> (7-(id)) & 0x1)
#define biAlarm(id)				(commStats[2] >> (3-(id)) & 0x01)
#define boPV(id)				(commStats[3] >> (7-(id)) & 0x1)
#define adAlarm(id)				(commStats[3] >> 4 & 0x03)
#define adPvId(id)				(commStats[3] >> 1 & 0x07)
*/
#define MAX_COMM_REGS_SZ		96
#define MAX_COMM_STATS_SZ		10
#define MAX_UNIT_REGS_SZ		63
#define MAX_UNIT_STATS_SZ		14

extern unsigned char	commRegs[];
extern unsigned char	unitRegs[][MAX_UNIT_REGS_SZ];
extern unsigned char	commStats[];
extern unsigned char	unitStats[][MAX_UNIT_STATS_SZ];
extern unsigned char	iounitStats[][MAX_UNIT_STATS_SZ];


void slvmUpdateDevStatus(void);
void slvmExtendedData(unsigned char *buf);
void slvmCommonStatus(unsigned char *buf);
void slvmIoStatus(int ioId, unsigned char *buf);
void slvmCredential(unsigned char *buf, int size);
int  slvmDownload(void);
void slvmWriteLocalIoConfig(void);
void slvmWriteReaderConfig(void);
void slvmReset(int param);
void slvmResetMaster(int param);
void slvmWriteCredBuffer(unsigned char *data, int length);
void slvmDoorCommand(int pv, int rte);
void slvmOutputCommand(int boId, unsigned char *buf);
void ioInitIo(void);
int  ioGetPoe(void);

void fpmSetMode(int mode);
void fpmIdentify(void);
void fpmCapture(void);
void fpmResult(unsigned char *buf, int size);


#endif

