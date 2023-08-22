#include <stdio.h>
#include "bmi270.h"
#include "timer.h"

void print_samples(int16_t sample_count)
{
    int16_t i;
    const RawFifoData *raw_data = bmi270_raw_headerless_fifo_data();
    const AccelUnits *accel = bmi270_converted_fifo_accel();
    const GyroUnits *gyro= bmi270_converted_fifo_gyro();

    printf("samples: %d\n", sample_count);

    for (i = 0; i < sample_count; i++) {
        printf("accel[%*d] (g-force | m/s%c) x: %5.2f | %5.2f y: %5.2f | %5.2f z: %5.2f |%5.2f\n",
            2, i, '\xFD',
            accel[i].g_force.pitch_x, accel[i].m_per_sec_squared.pitch_x,
            accel[i].g_force.roll_y, accel[i].m_per_sec_squared.roll_y,
            accel[i].g_force.yaw_z, accel[i].m_per_sec_squared.yaw_z);
        printf("gyro[%*d] (%c/sec | rad/sec) x: %5.2f | %5.2f y: %5.2f | %5.2f z: %5.2f |%5.2f\n",
            2, i, '\xF8',
            gyro[i].deg_per_sec.pitch_x, gyro[i].rad_per_sec.pitch_x,
            gyro[i].deg_per_sec.roll_y, gyro[i].rad_per_sec.roll_y,
            gyro[i].deg_per_sec.yaw_z, gyro[i].rad_per_sec.yaw_z);
        printf("accel/raw[%d] x: 0x%x | 0x%x y: 0x%x | 0x%x z: 0x%x | 0x%x \n",
            i,
            (uint16_t)raw_data[i].accel.pitch_x, (uint16_t)raw_data[i].gyro.pitch_x,
            (uint16_t)raw_data[i].accel.roll_y, (uint16_t)raw_data[i].gyro.roll_y,
            (uint16_t)raw_data[i].accel.yaw_z, (uint16_t)raw_data[i].gyro.yaw_z);
    }
}

void print_latest_sample(void)
{
    print_samples(1);
}

void print_fifo_samples(void)
{
    int16_t sample_count;
    sample_count = bmi270_fifo_sample_count();
    print_samples(sample_count);
}

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
        printf("Type: 0x%lx\n", channel_info.Type);
        printf("ID: 0x%lx\n", channel_info.ID);
        printf("Location Id: 0x%lx\n", channel_info.LocId);
    }
    if (!channel_count)
        return 0;
    
    // Open MPSSE chanel 0
    status = SPI_OpenChannel(0, &handle);
    if (status != FT_OK)
        LOG;
    printf("Channel handle: 0x%p\n", handle);
    puts("");
    // Configure channel 
    channel_config.ClockRate = I2C_CLOCK_STANDARD_MODE;
    channel_config.ConfigOptions = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channel_config.LatencyTimer = 100;
    status = SPI_InitChannel(handle, &channel_config);
    if (status != FT_OK)
        LOG;
    
    // Initialize BMI270
    init_timer = timer_start_elapsed_timer();
    success = bmi270_spi_init(handle, ODR_50_HZ, ACC_RANGE_2G, ODR_50_HZ, GYRO_RANGE_250DPS);
    if (!success) {
        success = bmi270_spi_init(handle, ODR_50_HZ, ACC_RANGE_2G, ODR_50_HZ, GYRO_RANGE_250DPS);
    }
    printf("bmi270_spi_init: %d after %.1fms\n", success, timer_elapsed_time_msec(init_timer));
    bmi270_spi_flush_fifo();

    while (1) {
        start = timer_start_elapsed_timer();

        // Single instantaneous read test
        /*bmi270_spi_read_data();
        printf("read_time: %.2fms\n", timer_elapsed_time_msec(start));
        print_latest_sample();
        bmi270_spi_flush_fifo(); // In case FIFO enabled
        // End single instantaneous read test
        */

        // Bulk FIFO read test
        bmi270_spi_read_headerless_fifo();
        printf("read_time: %.2fms\n", timer_elapsed_time_msec(start));
        print_fifo_samples();
        timer_blocking_sleep_remaining_msec(start, 500u);

    }
    Cleanup_libMPSSE();
    return 0;
}