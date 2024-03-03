#include "nrf_delay.h"
#include "timer.h"

void nrf_delay_ms(uint32_t ms_time)
{
    timer_blocking_sleep_msec(ms_time);
}