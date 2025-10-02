#ifndef _GMENU_AUTH_H_
#define _GMENU_AUTH_H_


void ApIdentifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr);
void ApVerifyCredential(ACCESS_POINT *ap, CREDENTIAL_READER *cr);
void CrProcessCommand(CREDENTIAL_READER *cr, unsigned char *buf, int size);
void ApCaptureCredential(CREDENTIAL_READER *cr);


#endif	/* _GMENU_AUTH_H_ */