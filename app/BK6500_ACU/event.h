#ifndef _EVENT_H_
#define _EVENT_H_


void EventInit(void);
void EventAdd(int ObjectType, int ObjetctID, int EventID, unsigned char *ctm, unsigned char *EventData);
int  SDCardOpenEvtFile(unsigned char *ctm);
void SDCardCloseEvtFile(void);
void SDCardAddEvtFile(unsigned char *ctm, unsigned char *data);
void SDCardInitCamFile(void);
void SDCardExitCamFile(void);
void SDCardRemoveCamFile(void);
void SDCardAddCamFile(unsigned char *ctm, unsigned char *JPEGBuffer, int JPEGSize);


#endif	/* _EVENT_H_ */