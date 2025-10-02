#ifndef _EVENT_H_
#define _EVENT_H_


void EventInit(void);
int  EventRead(unsigned char *evtData, int maxSize);
void EventClear(void);
void EventAdd(int ObjectType, int ObjectID, int EventID, unsigned char *ctm, unsigned char *EventData);
void EventAdd2(unsigned char *buf, int ObjectType, int ObjectID, int EventID, unsigned char *ctm, unsigned char *EventData);


#endif
