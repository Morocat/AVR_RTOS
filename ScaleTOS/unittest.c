/*
* unittest.c
*
* Created: 12/22/2017 1:57:03 PM
*  Author: alelop
*/
#include "unittest.h"

#include "serial.h"
#include <stdio.h>
#include <string.h>

#include "bios.h"
#include "tick.h"
#include "timer.h"
#include "kernel.h"
#include "common.h"
#include "mem_manage.h"

int testsPassed;

// tests
void test_tick(void);
void test_timer(void);
void test_mem_manager(void);
void test_create_task(void);
void _task_test_func1(void);
void _task_test_func2(void);
void _task_test_func3(void);
void _task_test_post_func(void);

void (*testList[])(void) = {
	test_tick,
	test_timer,
	//test_mem_manager,
	test_create_task
};

void test_init(void);
void test_print_results(void);

#define NUM_TESTS sizeof(testList) / sizeof(*testList)

void do_tests(void) {
	uint8_t i;
	test_init();

	for (i = 0; i < NUM_TESTS; i++)
		testList[i]();

	test_print_results();
}

void test_init(void) {
	//sprintf(strBuf, "sizeof(testList): %d\n\rsizeof(*testList): %d", sizeof(testList), sizeof(*testList));
	//serial_send_string(strBuf);
	serial_send_string("\n\r");
}

void test_mem_manager(void) {
	mem_manager_test();
	mem_manager_test2();
	testsPassed++;
}

volatile bool threadFunctionCalled;
void test_create_task(void) {
	Timer timer;
	Context* c;
	kernel_enable_scheduler();
	c = kernel_create_task(_task_test_func1, 0, "Test Task", 256);
	m_assert(c != NULL);
	kernel_create_task(_task_test_func3, 1, "Low Priority Task", 256);
	//kernel_create_task(_task_test_func2, 0, "Test Task 2", 256);
	while(!threadFunctionCalled);
	timer_set(&timer, 1000);
	while(!timer_has_expired(&timer));
	kernel_create_task(_task_test_post_func, 0, "Post Task", 256);
	testsPassed++;
}

void _task_test_func1(void) {
	Timer timer;
	timer_set(&timer, 1000);

	while(!timer_has_expired(&timer))
		TASK_YIELD();

	kernel_create_task(_task_test_func2, 0, "Test Task 2", 256);

	serial_send_string("Somebody once told me the world was gonna roll me");
	threadFunctionCalled = true;
}

void _task_test_func2(void) {
	Timer timer;
	timer_set(&timer, 1000);

	while(!timer_has_expired(&timer));
	serial_send_string("I aint the sharpest tool in the shed");
}

void _task_test_func3(void) {
	Timer timer;
	timer_set(&timer, 200);

	while(!timer_has_expired(&timer));
	serial_send_string("She was looking kind of dumb with her finger and her thumb");
}

void _task_test_post_func(void) {
	Timer timer;
	timer_set(&timer, 200);

	while(!timer_has_expired(&timer));
	serial_send_string("In the shape of an L on her forehead");
}

void test_timer(void) {
	Timer timer;
	timer_set(&timer, 500);
	while (!timer_has_expired(&timer));
	m_assert(timer_has_expired(&timer));
	testsPassed++;
}

volatile int tick_count;
void test_tick(void) {
	while (tick_count < 5);
	m_assert(tick_count == 5);
	testsPassed++;
}

void test_tick_callback(void) {
	tick_count++;
	(&PORTB)->OUT ^= PIN6_bm;
}

void test_print_results(void) {
	serial_send_string("\n\rTests finished\r\n");
	sprintf(gStrBuf, "Passed: %d\n\rFailed: %d\n\r\n\rResult: %s\n\r", testsPassed, NUM_TESTS - testsPassed, (testsPassed != NUM_TESTS ? "Fail" : "Pass"));
	serial_send_string(gStrBuf);
}