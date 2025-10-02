#include "as3911.h"
#include "as3911_errno.h"
#include "rtc.h"
#include "as3911_interrupt.h"

static volatile unsigned long as3911InterruptMask = 0;
static volatile unsigned long as3911InterruptStatus = 0;


void as3911EnableInterrupts(unsigned long mask)
{
	unsigned char	iregs[4];
	unsigned long	irqMask;

	as3911GetRegs(AS3911_REG_IRQ_MASK_MAIN, iregs, 3);
	irqMask = iregs[0] & 0xfc; irqMask |= iregs[1] << 8; irqMask |= iregs[2] << 16;
	irqMask &= ~mask;
	as3911InterruptMask |= mask;
	iregs[0] = irqMask; iregs[1] = irqMask >> 8; iregs[2] = irqMask >> 16;
	as3911SetRegs(AS3911_REG_IRQ_MASK_MAIN, iregs, 3);
}

void as3911DisableInterrupts(unsigned long mask)
{
	unsigned char	iregs[4];
	unsigned long	irqMask;

	as3911GetRegs(AS3911_REG_IRQ_MASK_MAIN, iregs, 3);
	irqMask = iregs[0] & 0xfc; irqMask |= iregs[1] << 8; irqMask |= iregs[2] << 16;
	irqMask |= mask;
	as3911InterruptMask &= ~mask;
	iregs[0] = irqMask; iregs[1] = irqMask >> 8; iregs[2] = irqMask >> 16;
	as3911SetRegs(AS3911_REG_IRQ_MASK_MAIN, iregs, 3);
}

void as3911ClearInterrupts(unsigned long mask)
{
	unsigned char	iregs[4];
	unsigned long	irqStatus;

	as3911GetRegs(AS3911_REG_IRQ_MAIN, iregs, 3);
	irqStatus = iregs[0] & 0xfc; irqStatus |= iregs[1] << 8; irqStatus |= iregs[2] << 16;
	as3911InterruptStatus |= irqStatus & as3911InterruptMask;
	as3911InterruptStatus &= ~mask;
}

static void _ReadInterrupt(void)
{
	unsigned char	iregs[4];
	unsigned long	irqStatus;
 
	as3911GetRegs(AS3911_REG_IRQ_MAIN, iregs, 3);
	irqStatus = iregs[0] & 0xfc; irqStatus |= iregs[1] << 8; irqStatus |= iregs[2] << 16;
	as3911InterruptStatus |= irqStatus & as3911InterruptMask;
}

unsigned long as3911GetInterrupts(unsigned long mask)
{
	unsigned long	irqStatus;
	
	_ReadInterrupt();
	irqStatus = as3911InterruptStatus & mask;
	as3911InterruptStatus &= ~mask;
	return irqStatus;
}

unsigned long as3911WaitInterrupts(unsigned long mask, int timeout)
{
	unsigned long	timer, irqStatus;
	int		rval, tmout;

	timer = MS_TIMER;
	do {
#ifdef _A20
		tmout = timeout - (MS_TIMER - timer);
		if(tmout < 1) tmout = 1;
		rval = as3911WaitScEvent(tmout);
#else
		usleep(5000);
#endif
		_ReadInterrupt();
		irqStatus = as3911InterruptStatus & mask;
	} while(!irqStatus && (MS_TIMER-timer) < timeout) ;
	if(irqStatus) as3911InterruptStatus &= ~irqStatus;
	return irqStatus;
}

