#include <time.h>
#include "timer.h"

#define CLOCKS_PER_MSEC ((float)(CLOCKS_PER_SEC / 1000))

float timer_start_elapsed_timer(void)
{
    return (float)clock();
}

float timer_elapsed_time_msec(float start)
{
    float duration_ms;
    duration_ms = (float)clock() - start;
    duration_ms /= CLOCKS_PER_MSEC;
    return duration_ms;
}

void timer_blocking_sleep_sec(uint32_t num_sec)
{
    int response __attribute__((unused));
    struct timespec request, remaining;
    request.tv_sec = 1u * num_sec;
    request.tv_nsec = 0;
    response = nanosleep(&request, &remaining);
}

void timer_blocking_sleep_msec(uint32_t num_msec)
{
    float start;
    float current;
    float duration = (float)num_msec / CLOCKS_PER_MSEC;
    
    start = timer_start_elapsed_timer();
    do {
        current = timer_elapsed_time_msec(start);
    } while (current < duration);
}

void timer_blocking_sleep_remaining_msec(float start, uint32_t num_msec)
{
    float current;
    float end = (float)num_msec / CLOCKS_PER_MSEC;

    do {
        current = timer_elapsed_time_msec(start);
    } while (current < end);
}