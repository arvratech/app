#ifndef _UNIT_CFG_H_
#define _UNIT_CFG_H_


/q/ Unit Configuration Data Structure
typedef struct _UNIT_CFG {
	unsigned char	model;	// 0:Virtual  1:K200-4/Local  2:K200-2/Local  3:K500-I/24  4:K700-R/12  255:Null
	unsigned char	reserve[3];
} UNIT_CFG;


void unitSetDefault(UNIT_CFG *cfg, int ID);
int  unitEncode(UNIT_CFG *cfg, void *buf);
int  unitDecode(UNIT_CFG *cfg, void *buf);
int  unitValidate(UNIT_CFG *cfg);

int  unitGerParent(int ID);
int  unitIsNull(int ID);
int  unitIsNonNull(int ID);


#endif

