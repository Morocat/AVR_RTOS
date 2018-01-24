/*
* tick.c
*
* Created: 12/21/2017 10:36:34 AM
*  Author: alelop
*/
#include <avr/io.h>

#include "tick.h"
#include "config.h"
#include <avr/interrupt.h>
#include "timer.h"

#if TEST_MODULES
#include "unittest.h"
#endif

void tick_init(void) {
	// configure timer0
	TCC0.PER = TICK_PERIOD;
	TCC0.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC0.INTCTRLA = TC_OVFINTLVL_LO_gc;

	// configure timer1
	TCC1.PER = TIMER_PERIOD;
	TCC1.CTRLA = TC_CLKSEL_DIV1024_gc;
	TCC1.INTCTRLA = TC_OVFINTLVL_LO_gc;
}

void tick_trigger_interrupt(void) {
	TCC0_OVF_vect ();
}

void TCC0_OVF_vect (void) {
	portSAVE_CONTEXT();
	TCC0.CNT = 0;
	kernel_process_all_tasks();
	portRESTORE_CONTEXT();
	reti();
}

void TCC1_OVF_vect (void) __attribute__ ((signal, __INTR_ATTRS));
void TCC1_OVF_vect (void) {
	onTimerTick();
#if TEST_MODULES
	test_tick_callback();
#endif
}