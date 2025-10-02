/*
 *****************************************************************************
 * Copyright @ 2009 by austriamicrosystems AG                                *
 * All rights are reserved.                                                  *
 *                                                                           *
 * Reproduction in whole or in part is prohibited without the written consent*
 * of the copyright owner. Austriamicrosystems reserves the right to make    *
 * changes without notice at any time. The software is provided as is and    *
 * Austriamicrosystems makes no warranty, expressed, implied or statutory,   *
 * including but not limited to any implied warranty of merchantability or   *
 * fitness for any particular purpose, or that the use will not infringe any *
 * third party patent, copyright or trademark. Austriamicrosystems should    *
 * not be liable for any loss or damage arising from its use.                *
 *****************************************************************************
 */
 
/*
 * PROJECT: AS3911 firmware
 * $Revision: $
 * LANGUAGE: ANSI C
 */
 
/*! \file as3911_interrupt.h
 *
 * \author Oliver Regenfelder
 *
 * \brief AS3911 interrupt handling.
 */

/*! \defgroup as3911IrqHandling AS3911 Interrupt Handling
 * \ingroup as3911
 *
 * \brief This part of the AS3911 module abstracts AS3911 interrupt handling.
 *
 * The AS3911 interrupt logic consists of three byte registers to mask or
 * unmask interupt
 * sources and three additional registers to indicate which interrupts are
 * pending. The content of this pending interrupt registers is automatically
 * cleared on a register read. Additionally a single interrupt line is used
 * to signal any interrupt pending interrupt condition to the microcontroller.
 *
 * This module abstracts this so the user no longer needs to know to which
 * interrupt register an interrupt source belongs to. To achive this a flat
 * hirarchy of interrupt masks is provided. This module also serves any
 * interrupt requests from the AS3911 and accumulates the pending interrupt
 * requests until the user reads out the interrupt status (as3911GetInterrupts(),
 * as3911WaitForInterruptTimed()).
 *
 * \section sec_1 Enabling and disabling processing of interrupt request from the AS3911
 *
 * The macros AS3911_IRQ_ON() and AS3911_IRQ_OFF() enable or disable processing
 * of AS3911 interrupt requests by the PIC controller (and thus by this module).
 * If interrupt processing is disabled via AS3911_IRQ_OFF, then no interrupt
 * request will reach the software even if the interrupt source is enabled via
 * as3911EnableInterrupts().
 *
 * Add startup processing of interrupts by the microcontroller is enabled and
 * therefore AS3911_IRQ_ON() needs to called at least once to enable AS3911
 * interrupt processing.
 *
 * \section sec_2 Enabling and disabling interrupts.
 *
 * The functions as3911EnableInterrupts() and as3911DisableInterrupts()
 * can be used to enable or disable specific AS3911 interrupt sources.
 * 
 * \section sec_3 Retreiving and reseting the interrupt status.
 *
 * The function as3911GetInterrupts() can be used to retreive the interrupt
 * status of any combination of AS3911 interrupt source(s). If an interrupt
 * from a source is pending \em and this interrupt is read out via
 * as3911GetInterrupts() then the penging interrupt is automatically
 * cleared. So a subsequent call to
 * as3911GetInterrupt() will mark that source as not pending (if no additional
 * interrupt happened in between).
 *
 * The function as3911ClearInterrupts() can be used to clear the interrupt
 * status of an interrupt source.
 *
 * The function as3911WaitForInterruptTimed can be used to wait for any
 * interrupt out of a set of interrupts to occure. Additionally a timeout can
 * be specified. This function blocks until at least one of the selected
 * interrupts occure or the timeout expires. This function does not enable
 * or disable interrupts. So any interrupt source to wait for needs to be
 * enabled prior to the call via as3911EnableInterrupts.
 */

#ifndef AS3911_INTERRUPT_H
#define AS3911_INTERRUPT_H


// Interrupt masks

//Main interrupt register
#define AS3911_IRQ_MASK_ALL			U32_C(0xFFFFFF)	// All AS3911 interrupt sources
#define AS3911_IRQ_MASK_NONE		U32_C(0)		// No AS3911 interrupt source
#define AS3911_IRQ_MASK_OSC			U32_C(0x80)		// oscillator stable interrupt
#define AS3911_IRQ_MASK_WL			U32_C(0x40)		// FIFO water level inerrupt
#define AS3911_IRQ_MASK_RXS			U32_C(0x20)		// start of receive interrupt
#define AS3911_IRQ_MASK_RXE			U32_C(0x10)		// end of receive interrupt
#define AS3911_IRQ_MASK_TXE			U32_C(0x08)		// end of transmission interrupt
#define AS3911_IRQ_MASK_COL			U32_C(0x04)		// bit collision interrupt

// Timer and NFC interrupt register
#define AS3911_IRQ_MASK_DCT			U32_C(0x8000)	// termination of direct command interrupt
#define AS3911_IRQ_MASK_NRE			U32_C(0x4000)	// no-response timer expired interrupt
#define AS3911_IRQ_MASK_GPE			U32_C(0x2000)	// general purpose timer expired interrupt
#define AS3911_IRQ_MASK_EON			U32_C(0x1000)	// external field on interrupt
#define AS3911_IRQ_MASK_EOF			U32_C(0x0800)	// external field off interrupt
#define AS3911_IRQ_MASK_CAC			U32_C(0x0400)	// collision during RF collision avoidance interrupt
#define AS3911_IRQ_MASK_CAT			U32_C(0x0200)	// minimum guard time expired interrupt
#define AS3911_IRQ_MASK_NFCT		U32_C(0x0100)	// initiator bit rate recognized interrupt

// Error and wake-up interrupt register
#define AS3911_IRQ_MASK_CRC			U32_C(0x800000)	// CRC error interrupt
#define AS3911_IRQ_MASK_PAR			U32_C(0x400000)	// parity error interrupt
#define AS3911_IRQ_MASK_ERR2		U32_C(0x200000)	// soft framing error interrupt
#define AS3911_IRQ_MASK_ERR1		U32_C(0x100000)	// hard framing error interrupt
#define AS3911_IRQ_MASK_WT			U32_C(0x080000)	// wake-up interrupt
#define AS3911_IRQ_MASK_WAM			U32_C(0x040000)	// wake-up due to amplitude interrupt
#define AS3911_IRQ_MASK_WPH			U32_C(0x020000)	// wake-up due to phase interrupt
#define AS3911_IRQ_MASK_WCAP		U32_C(0x010000)	// wake-up due to capacitance measurement

#define AS3911_IRQ_MASK_TIMEOUT		0x01000000		// no interrupt with specified time: appended by KDK 		


void as3911EnableInterrupts(unsigned long mask);
void as3911DisableInterrupts(unsigned long mask);
void as3911ClearInterrupts(unsigned long mask);
unsigned long as3911GetInterrupts(unsigned long mask);
unsigned long as3911WaitInterrupts(unsigned long mask, int timeout);


#endif

