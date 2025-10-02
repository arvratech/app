#ifndef _SLVM_LIB_H_
#define _SLVM_LIB_H_


#define _ESP32		1

#include "inputlib.h"

#ifdef _ESP32

#include "spimlib.h"

#define slvmOpen()								spimOpen();
#define slvmClose(fd)							spimClose(fd);
#define slvmReadInterrupt(fd, buf)				spimReadInterrupt(fd, buf)
#define slvmWriteRegister(fd, addr, buf, size)	spimWriteRegister(fd, 0, addr, buf, size)
#define slvmReadRegister(fd, addr, buf, size)	spimReadRegister(fd, 0, addr, buf, size)
#define slvmWriteCommand(fd, addr, buf, size)	spimWriteCommand(fd, 0, addr, buf, size)
#define slvmReadStatus(fd, addr, buf, size)		spimReadStatus(fd, 0, addr, buf, size)
#define slvmWriteBuffer(fd, addr, buf, size)	spimWriteBuffer(fd, 0, addr, buf, size)
#define slvmReadBuffer(fd, addr, buf, size)		spimReadBuffer(fd, 0, addr, buf, size)

#define slvmEventOpen()				blemEventOpen()
#define slvmEventClose(fd)			blemEventClose(fd)
#define slvmEventReadEvent(fd)		blemEventReadEvent(fd)
#define slvmEventRead(fd)			blemEventRead(fd)

#else

#include "i2cmlib.h"

#define slvmOpen()								i2cmOpen();
#define slvmClose(fd)							i2cmClose(fd);
#define slvmReadInterrupt(fd, buf)				i2cmReadInterrupt(fd, buf)
#define slvmWriteRegister(fd, addr, buf, size)	i2cmWriteRegister(fd, addr, buf, size)
#define slvmReadRegister(fd, addr, buf, size)	i2cmReadRegister(fd, addr, buf, size)
#define slvmWriteCommand(fd, addr, buf, size)	i2cmWriteCommand(fd, addr, buf, size)
#define slvmReadStatus(fd, addr, buf, size)		i2cmReadStatus(fd, addr, buf, size)
#define slvmWriteBuffer(fd, addr, buf, size)	i2cmWriteBuffer(fd, addr, buf, size)
#define slvmReadBuffer(fd, addr, buf, size)		i2cmReadBuffer(fd, addr, buf, size)

#define slvmEventOpen()				iowmEventOpen()
#define slvmEventClose(fd)			iowmEventClose(fd)
#define slvmEventReadEvent(fd)		iowmEventReadEvent(fd)
#define slvmEventRead(fd)			iowmEventRead(fd)

#endif
	

#endif

