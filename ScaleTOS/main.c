/*
 * ScaleTOS.c
 *
 * Created: 12/20/2017 4:10:38 PM
 * Author : alelop
 */ 

#include <avr/io.h>
#include "config.h"
#include "timer.h"
#include "kernel.h"
#include "bios.h"

#if TEST_MODULES
#include "unittest.h"
#endif

int main(void)
{
	bios_init();
	kernel_init();
	#if TEST_MODULES
		do_tests();
	#endif
	kernel_no_return();
}

