#ifndef TIMER_H_
#define TIMER_H_

#include "ftdi_libMPSSE.h" // For fixed-width types

float timer_start_elapsed_timer(void);
float timer_elapsed_time_msec(float start); // Call timer_start_elapsed_timer() first
void timer_blocking_sleep_sec(uint32_t num_sec);
void timer_blocking_sleep_msec(uint32_t num_msec);
void timer_blocking_sleep_remaining_msec(float start, uint32_t num_msec); // Call timer_start_elapsed_timer() first

#endif // TIMER_H_