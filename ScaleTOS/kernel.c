/*
 * rtk.c
 *
 * Created: 1/3/2018 2:59:09 PM
 *  Author: alelop
 */

 #include "kernel.h"

 #include <string.h>
 //#include <stdlib.h>
 #include "bios.h"
 #include "queue.h"
 #include "common.h"
 #include "tick.h"
 #include "mem_manage.h"
 #include <avr/io.h>

 #if DEBUG_TRACE
 #include "serial.h"
 #include <stdio.h>
 #endif

 Scheduler scheduler;
 Queue contextList[MAX_PRIORITY];
 Context *activeContext;

 void kernel_init_stack(Context *c);
 void kernel_swap_to_task(Context *c);
 void kernel_remove_task(Context *c);
 void kernel_check_stack_overflow(void);
 void kernel_catch_task_return(void);
 Context* kernel_find_highest_priority_ready_task(void);
 void kernel_swap_task(void);

 void kernel_init(void) {
	ptrStackP = &defaultStackP;
 }

 Context* kernel_create_task(void (*task)(void), uint8_t priority, const char *taskName, uint16_t stackSize) {
	Context *c;
	QueueNode *n;
	uint8_t i;

	m_assert(priority < MAX_PRIORITY);
	m_assert(taskName != NULL);
	m_assert(stackSize > 45); // each task requires 45 bytes of overhead on Atmel xmega chips due to the registers

	for (i = 0; i < MAX_PRIORITY; i++) {
		n = contextList[i].head;
		while (n != NULL) {
			c = (Context*) n->data;
			if (c->entryPoint == task)
				return NULL;
			n = n->next;
		}
	}

	c = m_malloc(sizeof(Context));
	m_assert(c != NULL);
	c->stack = (uint8_t*) m_malloc(stackSize);
	m_assert(c->stack != NULL);

	c->taskName = (char*) m_malloc(strlen(taskName) + 1);
	m_assert(c->taskName != NULL);

	c->stackSize = stackSize;
#if DEBUG_TRACE
	c->stackTop = c->stack + stackSize;
#endif
	c->priority = priority;
	c->entryPoint = task;
	c->status = READY;
	strcpy(c->taskName, taskName);
	kernel_init_stack(c);

	queue_add_with_priority(&contextList[priority], c, 0);

	return c;
 }

 void kernel_init_stack(Context *c) {
	uint16_t i, j;
	volatile uint32_t addr = kernel_catch_task_return;
	for (i = j = 0; i < 4; i++)
		c->stack[c->stackSize - j++ - 1] = (addr >> (i * 8)) & 0xff;
	
	addr = c->entryPoint;
	for (i = 0; i < 4; i++)
		c->stack[c->stackSize - j++] = (addr >> (i * 8)) & 0xff;

	c->stack[c->stackSize - j] = 0x80; // global interrupt enable
	c->stackPointer = &c->stack[c->stackSize - j - 36]; // 37 pops in RESTORE_CONTEXT
 }

 void kernel_process_all_tasks(void) {
	uint8_t i, j;
	Context *c;
	QueueNode *n;

	if (scheduler.status != RUNNING)
		return;

	kernel_check_stack_overflow();

	for (i = 0; i < MAX_PRIORITY; i++) {
		n = contextList[i].head;
		for (j = 0; j < contextList[i].size; j++) {
			c = (Context*) n->data;
			if (c->status == BLOCKED) {
				n->priority = timer_time_remaining(&c->timer);
				if (n->priority == 0)
					c->status = READY;
			}
			n = n->next;
		}
	}

	kernel_swap_task();
 }

 void kernel_swap_task(void) {
	int8_t i;
	Context *c;
	bool needContextSwitch = false;
	Queue *q;
	QueueNode *n;

	if (activeContext != NULL) {
		for (i = activeContext->priority - 1; i >= 0; i--) {
			c = (Context*) contextList[i].head->data;
			if (c->status == READY) {
				needContextSwitch = true;
				break;
			}
		}
	}

	if (activeContext == NULL || needContextSwitch || activeContext->status == BLOCKED || timer_has_expired(&activeContext->timer)) {
		if (activeContext != NULL) {
			q = &contextList[activeContext->priority];
			if (activeContext->status != BLOCKED) {
				activeContext->status = BLOCKED;
				timer_set(&activeContext->timer, DEFAULT_BLOCK_DURATION);
			}
			n = queue_find_node(q, (void*)activeContext);
			n->priority = timer_time_remaining(&activeContext->timer);
			queue_sort(q);
		}

		for (i = 0; i < MAX_PRIORITY; i++) {
			c = (Context*) contextList[i].head->data;
			if (c != NULL && c->status == READY) {
				kernel_swap_to_task(c);
				return;
			}
		}
		for (i = 0; i < MAX_PRIORITY; i++) {
			c = (Context*) contextList[i].head->data;
			if (c != NULL) {
				kernel_swap_to_task(c);
				return;
			}
		}
	}
 }

 void kernel_remove_task(Context *c) {
	Queue *q = &contextList[c->priority];
	queue_remove(q, (void*)c);
	free(c->stack);
	free(c->taskName);
	free(c);
 }

 void kernel_swap_to_task(Context *c) {
	sprintf(gStrBuf, "Switching to task %s\n\r", c->taskName);
	serial_send_string(gStrBuf);
	ptrStackP = &c->stackPointer;

	activeContext = c;
	activeContext->status = ACTIVE;
	timer_set(&c->timer, DEFAULT_BLOCK_DURATION);
 }

 void kernel_yield_from_task(uint16_t duration) {
	if (activeContext != NULL) {
		activeContext->status = BLOCKED;
		timer_set(&activeContext->timer, duration);
	}
	//TCC0.CNT = TCC0.PER;
	//tick_trigger_interrupt();
 }

 void kernel_check_stack_overflow(void) {
	if (activeContext != NULL && activeContext->stackPointer < activeContext->stack) {
		#if DEBUG_TRACE
			sprintf(gStrBuf, "Stack overflow detected in task %s!\n\r", activeContext->taskName);
			serial_send_string(gStrBuf);
		#endif
		while(1);
	}
 }

 void kernel_catch_task_return(void) {
	Context *c;
	serial_send_string("\n\rTask caught in kernel catch function\n\r");
	AVR_ENTER_CRITICAL_REGION();
	SPL = defaultStackP & 0xff;
	SPH = (defaultStackP >> 8) & 0xff;
	kernel_remove_task(activeContext);
	AVR_LEAVE_CRITICAL_REGION();

	activeContext = NULL;
	c = kernel_find_highest_priority_ready_task();
	if (c != NULL)
		kernel_swap_to_task(c);
	else
		ptrStackP = &defaultStackP;
	
	portRESTORE_CONTEXT();
	asm volatile ("ret \n\t"); // need to ignore the function's default return pops
 }

 Context* kernel_find_highest_priority_ready_task(void) {
	uint8_t i;
	Context *c;

	for (i = 0; i < MAX_PRIORITY; i++) {
		c = (Context*) contextList[i].head->data;
		if (c != NULL && c->status == READY)
			return c;
	}
	return NULL;
 }

 void kernel_enable_scheduler(void) {
	scheduler.status = RUNNING;
 }

 void kernel_disable_scheduler(void) {
	scheduler.status = STOPPED;
 }

 void kernel_no_return(void) {
	asm volatile ("rjmp .-2 \n\t");
 }

 void kernel_take_mutex(Mutex *m) {
	if (!m->inUse) {
		m->inUse = true;
		m->owner = activeContext;
		return;
	}

	if (m->owner == activeContext)
		return;

	// boost priority of lower priority task if needed
	if (activeContext->priority < m->owner->priority) {
		activeContext->prevPriority = activeContext->priority;
		activeContext->priority = m->owner->priority;
		queue_move_item(&contextList[activeContext->priority], &contextList[activeContext->prevPriority], (void*)activeContext);
		queue_sort(&contextList[activeContext->priority]);
	}

 }