/*
* bios.c
*
* Created: 12/20/2017 4:49:14 PM
*  Author: alelop
*/

#include "bios.h"

#include "clksys_driver.h"
#include "port_driver.h"
#include "tick.h"
#include "serial.h"
#include "typeDef.h"
#include "kernel.h"

void bios_clock_normal( void );
void bios_io_ports_init(void);
void bios_timer_init(void);
void bios_interrupt_init(void);

void bios_init(void) {
	bios_io_ports_init();
	bios_clock_normal();
	tick_init();
	bios_interrupt_init();
	serial_init();
}

void bios_interrupt_init(void) {
	PMIC.CTRL = PMIC_LOLVLEX_bm;
	sei();
}

void bios_io_ports_init(void) {
	PORT_ConfigurePins( &PORTB,
	PIN6_bm,
	false,							// no slew rate control
	false,							// no inversion
	PORT_OPC_PULLUP_gc,
	PORT_ISC_BOTHEDGES_gc );
	PORT_SetDirection( &PORTB, PIN6_bm);
	(&PORTB)->OUT |= PIN6_bm;

	PORT_ConfigurePins( &PORTC,
	PIN3_bm,	// configure pin0, pin1, pin3, pin4, pin5, pin7.
	false,							// no slew rate control
	false,							// no inversion
	PORT_OPC_TOTEM_gc,				// Totempole
	PORT_ISC_BOTHEDGES_gc );
	PORT_SetDirection( &PORTC, PIN3_bm);

	PORT_ConfigurePins( &PORTF,
	PIN2_bm,
	false,							// no slew rate control
	false,							// no inversion
	PORT_OPC_PULLUP_gc,
	PORT_ISC_BOTHEDGES_gc );
	PORT_SetDirection( &PORTF, PIN2_bm);
	(&PORTF)->OUT |= PIN2_bm;
}

void bios_clock_normal( void ) {
	CLKSYS_Enable( OSC_RC32MEN_bm | OSC_RC32KEN_bm);						// enabled RC32KHz internal oscillator as source for 32MHz calibrated source.
	CLKSYS_Prescalers_Config( CLK_PSADIV_1_gc, CLK_PSBCDIV_1_1_gc );		//The output cpu and peripheral clocks are the same as the original source.
	do {} while ( CLKSYS_IsReady( OSC_RC32KRDY_bm ) == 0 );					// Wait for RC 32KHz stabilized
	do {} while ( CLKSYS_IsReady( OSC_RC32MRDY_bm ) == 0 );
	CLKSYS_Main_ClockSource_Select( CLK_SCLKSEL_RC32M_gc );
	CLKSYS_AutoCalibration_Enable( OSC_RC32MCREF_bm, 0 );				// Enabled CLKSYS auto calibration in order to work with Rev.I ATxmega192D3 chip.
}
