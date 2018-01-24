/*
 * tick.h
 *
 * Created: 12/21/2017 10:36:41 AM
 *  Author: alelop
 */ 


#ifndef TICK_H_
#define TICK_H_

#include "kernel.h"

#define TICK_MS 20
#define TICK_PERIOD (32768 * TICK_MS) / 1000

/* 
 * Macro to save all the general purpose registers, the save the stack pointer
 * into the TCB.  
 * 
 * The first thing we do is save the flags then disable interrupts.  This is to 
 * guard our stack against having a context switch interrupt after we have already 
 * pushed the registers onto the stack - causing the 32 registers to be on the 
 * stack twice. 
 * 
 * The interrupts will have been disabled during the call to portSAVE_CONTEXT()
 * so we need not worry about reading/writing to the stack pointer. 
 */

#define portSAVE_CONTEXT()									\
	asm volatile (	"push	r0						\n\t"	\
					"in		r0, __SREG__			\n\t"	\
					"cli							\n\t"	\
					"push	r0						\n\t"	\
					"in		r0, 0x38				\n\t"	\
					"push	r0						\n\t"	\
					"in		r0, 0x39				\n\t"	\
					"push	r0						\n\t"	\
					"in		r0, 0x3A				\n\t"	\
					"push	r0						\n\t"	\
					"in		r0, 0x3B				\n\t"	\
					"push	r0						\n\t"	\
					"push	r1						\n\t"	\
					"clr	r1						\n\t"	\
					"push	r2						\n\t"	\
					"push	r3						\n\t"	\
					"push	r4						\n\t"	\
					"push	r5						\n\t"	\
					"push	r6						\n\t"	\
					"push	r7						\n\t"	\
					"push	r8						\n\t"	\
					"push	r9						\n\t"	\
					"push	r10						\n\t"	\
					"push	r11						\n\t"	\
					"push	r12						\n\t"	\
					"push	r13						\n\t"	\
					"push	r14						\n\t"	\
					"push	r15						\n\t"	\
					"push	r16						\n\t"	\
					"push	r17						\n\t"	\
					"push	r18						\n\t"	\
					"push	r19						\n\t"	\
					"push	r20						\n\t"	\
					"push	r21						\n\t"	\
					"push	r22						\n\t"	\
					"push	r23						\n\t"	\
					"push	r24						\n\t"	\
					"push	r25						\n\t"	\
					"push	r26						\n\t"	\
					"push	r27						\n\t"	\
					"push	r28						\n\t"	\
					"push	r29						\n\t"	\
					"push	r30						\n\t"	\
					"push	r31						\n\t"	\
					"lds	r26, ptrStackP			\n\t"	\
					"lds	r27, ptrStackP + 1		\n\t"	\
					"in		r0, __SP_L__			\n\t"	\
					"st		x+, r0					\n\t"	\
					"in		r0, __SP_H__			\n\t"	\
					"st		x+, r0					\n\t"	\
				);

/* 
 * Opposite to portSAVE_CONTEXT().  Interrupts will have been disabled during
 * the context save so we can write to the stack pointer. 
 */

#define portRESTORE_CONTEXT()								\
	asm volatile (	"lds	r26, ptrStackP			\n\t"	\
					"lds	r27, ptrStackP + 1		\n\t"	\
					"ld		r0, X+					\n\t"	\
					"ld		r1,	X					\n\t"	\
					"out	__SP_L__, r0			\n\t"	\
					"out	__SP_H__, r1			\n\t"	\
					"pop	r31						\n\t"	\
					"pop	r30						\n\t"	\
					"pop	r29						\n\t"	\
					"pop	r28						\n\t"	\
					"pop	r27						\n\t"	\
					"pop	r26						\n\t"	\
					"pop	r25						\n\t"	\
					"pop	r24						\n\t"	\
					"pop	r23						\n\t"	\
					"pop	r22						\n\t"	\
					"pop	r21						\n\t"	\
					"pop	r20						\n\t"	\
					"pop	r19						\n\t"	\
					"pop	r18						\n\t"	\
					"pop	r17						\n\t"	\
					"pop	r16						\n\t"	\
					"pop	r15						\n\t"	\
					"pop	r14						\n\t"	\
					"pop	r13						\n\t"	\
					"pop	r12						\n\t"	\
					"pop	r11						\n\t"	\
					"pop	r10						\n\t"	\
					"pop	r9						\n\t"	\
					"pop	r8						\n\t"	\
					"pop	r7						\n\t"	\
					"pop	r6						\n\t"	\
					"pop	r5						\n\t"	\
					"pop	r4						\n\t"	\
					"pop	r3						\n\t"	\
					"pop	r2						\n\t"	\
					"pop	r1						\n\t"	\
					"pop	r0						\n\t"	\
					"out	0x3B, r0				\n\t"	\
					"pop	r0						\n\t"	\
					"out	0x3A, r0				\n\t"	\
					"pop	r0						\n\t"	\
					"out	0x39, r0				\n\t"	\
					"pop	r0						\n\t"	\
					"out	0x38, r0				\n\t"	\
					"pop	r0						\n\t"	\
					"out	__SREG__, r0			\n\t"	\
					"pop	r0						\n\t"	\
				);

void tick_init(void);
void tick_trigger_interrupt(void);
void TCC0_OVF_vect (void) __attribute__ ((signal, naked, __INTR_ATTRS));

#endif /* TICK_H_ */