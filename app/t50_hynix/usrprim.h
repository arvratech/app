#ifndef _USR_PRIM_H_
#define _USR_PRIM_H_


void GetUserAccessMode(int accessMode, char *buf);
void GetUserCardData(unsigned char *cardData, char *buf);
void GetUserFpData(int fpExist, char *buf);
void GetUserTitleCardData(unsigned char *cardData, char *buf);
void GetUserAccessRights(unsigned char accessRights, char *buf);
void GetUserDate(unsigned char *ctm, char *buf);


#endif
