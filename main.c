#include <stdio.h>
#include "bmi270.h"
#include "timer.h"

int main(int argc, char **argv)
{
    uint32_t i;
    bool success;
    Init_libMPSSE();

    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE channel_info;
    SpiChannelConfig channel_config;
    FT_HANDLE handle;
    float init_timer;
    float start;

    // check how many MPSSE channels are available
    uint32_t channel_count = 0;
    status = SPI_GetNumChannels(&channel_count);
    if (status != FT_OK)
        LOG;

    printf("MPSSE channel(s) available: %ld\n", channel_count);
    for (i = 0; i < channel_count; i++) {
        status = SPI_GetChannelInfo(i, &channel_info);
        if (status != FT_OK)
            LOG;
        puts("");
        printf("Channel number: %ld\n", i);
        printf("Description: %s\n", channel_info.Description);
        printf("Serial Number: %s\n", channel_info.SerialNumber);
    }
    if (!channel_count)
        return 0;
    
    // Open MPSSE chanel 0
    status = SPI_OpenChannel(0, &handle);
    if (status != FT_OK)
        LOG;
    printf("Handle: %p\n", handle);
    // Configure channel 
    channel_config.ClockRate = I2C_CLOCK_STANDARD_MODE;
    channel_config.ConfigOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channel_config.LatencyTimer = 100;
    status = SPI_InitChannel(handle, &channel_config);
    if (status != FT_OK)
        LOG;
    
    // Initialize BMI270
    timer_blocking_sleep_msec(200u);
    init_timer = timer_start_elapsed_timer();
    success = bmi270_spi_init(handle, ODR_25_HZ, ACC_RANGE_2G, ODR_25_HZ, GYRO_RANGE_250DPS);
    if (!success) {
        success = bmi270_spi_init(handle, ODR_25_HZ, ACC_RANGE_2G, ODR_25_HZ, GYRO_RANGE_250DPS);
    }
    printf("bmi270_spi_init: %d after %.1fms\n", success, timer_elapsed_time_msec(init_timer));
    bmi270_spi_flush_fifo();

    while (1) {
        start = timer_start_elapsed_timer();
        
        // Single instantaneous read test
        /*bmi270_spi_read_data();
        printf("read_time: %.2fms\n", timer_elapsed_time_msec(start));
        bmi270_print_latest_converted_data();
        flush_fifo_spi(); // In case FIFO enabled
        // End single instantaneous read test
        */

        bmi270_spi_read_headerless_fifo();
        printf("read_time: %.2fms\n", timer_elapsed_time_msec(start));
        bmi270_print_fifo_converted_data();
        timer_blocking_sleep_remaining_msec(start, 500u);

    }
    Cleanup_libMPSSE();
    return 0;
}