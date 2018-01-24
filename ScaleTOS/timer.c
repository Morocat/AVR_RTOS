/*
 * timer.c
 *
 * Created: 12/28/2017 1:06:34 PM
 *  Author: alelop
 */ 
 #include "timer.h"

 volatile uint32_t timerVal, current_ms;
 #define maxTimerVal ((uint32_t)(4294967295 / TIMER_PERIOD)) * TIMER_PERIOD

 uint32_t calc_time_elapsed(Timer *timer);

 void timer_set(Timer *timer, uint32_t milliseconds) {
	timer->interval = milliseconds;
	timer->start = current_ms;
 }

 bool timer_has_expired(Timer *timer) {
	uint32_t elapsedTime = calc_time_elapsed(timer);
	return elapsedTime >= timer->interval;
 }

 uint32_t timer_time_remaining(Timer *timer) {
	 uint32_t elapsedTime = calc_time_elapsed(timer);

	if (timer->interval > elapsedTime)
		return timer->interval - elapsedTime;
	return 0;
 }

 uint32_t calc_time_elapsed(Timer *timer) {
	uint32_t curMs = current_ms;
	uint32_t elapsedTime;

	if (curMs >= timer->start) // timer has not wrapped around
		elapsedTime = curMs - timer->start;
	else // timer has wrapped around
		elapsedTime = maxTimerVal - (timer->start - curMs);

	return elapsedTime;
 }

 void onTimerTick(void) {
	if (++timerVal >= maxTimerVal)
		timerVal = 0;
	current_ms = timerVal * TIMER_MS;
 }