/*
 * kernel.h
 *
 * Created: 1/3/2018 2:59:23 PM
 *  Author: alelop
 */ 


#ifndef KERNEL_H_
#define KERNEL_H_

#include <stdint.h>
#include "timer.h"
#include "config.h"

//#define STACK_SIZE 1024
//#define MAX_PROCESS_COUNT 8
#define MAX_PRIORITY 8 // this value is the lowest priority
#define DEFAULT_BLOCK_DURATION 150

// DONE: task has run to completion
// ACTIVE: task is currently active
// READY: task was just created and is ready to start
// BLOCKED: task is waiting to be run
typedef enum ContextStatus{
	DONE, ACTIVE, READY, BLOCKED
}ContextStatus;

typedef struct Context{
	uint8_t				*stack;
	uint16_t			*stackPointer;
	char				*taskName;
	uint8_t				priority;
	uint16_t			stackSize;
	ContextStatus		status;
	void (*entryPoint)(void);
	Timer				timer;
#if DEBUG_TRACE
	uint16_t			stackTop;
#endif
}Context;

typedef enum SchedulerStatus{
	STOPPED, RUNNING, SUSPENDED
}SchedulerStatus;
typedef struct Scheduler{
	SchedulerStatus status;
}Scheduler;

volatile uint16_t *ptrStackP;
volatile uint16_t defaultStackP;

void kernel_init(void);
Context* kernel_create_task(void (*task)(void), uint8_t priority, const char *taskName, uint16_t stackSize);
void kernel_yield_from_task(uint16_t duration);
void kernel_process_all_tasks(void);
void kernel_no_return(void);
void kernel_enable_scheduler(void);
void kernel_disable_scheduler(void);

#define TASK_YIELD_DEFAULT() kernel_yield_from_task(DEFAULT_BLOCK_DURATION)
#define TASK_YIELD_DURATION(_duration) kernel_yield_from_task((uint16_t)_duration)

#define GET_MACRO(_0, _1, NAME, ...) NAME
#define TASK_YIELD(...) GET_MACRO(_0, ##__VA_ARGS__, TASK_YIELD_DURATION, TASK_YIELD_DEFAULT)(__VA_ARGS__)

#endif /* KERNEL_H_ */