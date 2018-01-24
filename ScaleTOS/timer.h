/*
 * timer.h
 *
 * Created: 12/28/2017 1:06:48 PM
 *  Author: alelop
 */ 


#ifndef TIMER_H_
#define TIMER_H_

#include <stdint.h>
#include <stdbool.h>

#define TIMER_MS 50
#define TIMER_PERIOD (32768 * TIMER_MS) / 1000

typedef struct Timer{
	uint16_t start;
	uint16_t interval;
}Timer;

void onTimerTick(void);
void timer_set(Timer *timer, uint32_t milliseconds);
bool timer_has_expired(Timer *timer);
uint32_t timer_time_remaining(Timer *timer);

#endif /* TIMER_H_ */