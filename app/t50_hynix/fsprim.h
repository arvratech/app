#ifndef _FS_PRIM_H_
#define _FS_PRIM_H_


int  fsReadRecord(FILE *fp, void *buf, int recordSize, int recordCount);
int  fsReadRecordAll(FILE *fp, void *buf, int recordSize, int keyOffset, int keySize);
int  fsGetRecordCount(FILE *fp, int recordSize);
int  fsFindRecord(FILE *fp, void *buf, int recordSize, unsigned char *key, int keyOffset, int keySize);
int  fsFindNullRecord(FILE *fp, int recordSize);
int  fsAddRecord(FILE *fp, void *buf, int recordSize, int keySize);
int  fsAddRecordNull(FILE *fp, void *buf, int recordSize);
int  fsDeleteRecord(FILE *fp, int recordSize, unsigned char *key, int keySize);


#endif

