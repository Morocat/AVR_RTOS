/*
 * bios.h
 *
 * Created: 12/20/2017 4:49:03 PM
 *  Author: alelop
 */ 


#ifndef BIOS_H_
#define BIOS_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdint.h>
#include <stdbool.h>

volatile uint8_t nestedCalls;
volatile uint8_t _sreg;

#define AVR_ENTER_CRITICAL_REGION( ) uint8_t volatile saved_sreg = SREG; \
									 cli();

#define AVR_LEAVE_CRITICAL_REGION( ) SREG = saved_sreg;

#define CRITICAL_SYNC_START()	if (nestedCalls++ == 0) {				\
									_sreg = SREG;						\
									cli();								\
								}

#define CRITICAL_SYNC_END()		if(--nestedCalls == 0) {				\
									SREG = _sreg;						\
								}
								
 void bios_init(void);

#endif /* BIOS_H_ */