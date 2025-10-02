#ifndef	_SINNET_H__
#define	_SINNET_H__


int  sinReadNet(int s, unsigned short *pDestinationAddress, NET *net);
int  sinReadNetCompleted(int s, NET *net);
int  sinTxBufferFull(int s);
int  sinWriteNet(int s,  unsigned short SourceAddress, NET *net);
int  sinWriteNetCompleted(int s);


#endif	/* _SINNET_H_ */

