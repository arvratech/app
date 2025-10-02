#ifndef _UNIT_H_
#define _UNIT_H_


// Unit Configuration structure
typedef struct _UNIT_CFG {
	unsigned char	model;	// 0:Virtual  1:K200-4/Local  2:K200-2/Local  3:K500-I/24  4:K700-R/12  255:Null
	unsigned char	reserve[3];
} UNIT_CFG;

typedef struct _UNIT {
	unsigned char	cls;
	unsigned char	id;
	unsigned char	reliability;
	unsigned char	reserve[1];
	UNIT_CFG		 *cfg;
} UNIT;


void unitsInit(void);
void *unitsGet(int id);
int  unitsMaxSize(void);

void _UnitSetDefault(UNIT_CFG *cfg);
int  _UnitEncode(UNIT_CFG *cfg, void *buf);
int  _unitDecode(UNIT_CFG *cfg, void *buf);
int  _UnitValidate(UNIT_CFG *cfg);

int  unitId(void *self);
BOOL unitIsNonNull(void *self);
int  unitModel(void *self);
void unitCreate(void *self, int model);
void unitDelete(void *self);
int  unitReliability(void *self);
int  unitGetAvailableModels(void *self, int *models);
int  unitMaxInputsSize(void *self);
int  unitMaxOutputsSize(void *self);


#endif

