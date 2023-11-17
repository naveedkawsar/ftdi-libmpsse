#include <signal.h> // For signal(SIGINT, ...)
#include <stdio.h> // For printf(), fprintf(), fopen(), fclose()
#include <time.h> // For time(), localtime()
#include <unistd.h> // For getopt(), optarg
#include "bmi270.h"
#include "timer.h"

volatile bool continue_run = true;

typedef struct {
    FILE *csv;
    bool logging_en;
    bool disable_debug_print;
    uint32_t interval_time_ms;
    AccRangeG accel_range;
    GyroRangeDps gyro_range;
    OutputDataRateHz odr;
} CommandlineArgs;


void reset_commandline_args(CommandlineArgs *args)
{
    args->logging_en = false;
    args->disable_debug_print = false;
    args->interval_time_ms = 500u;
    args->accel_range = ACC_RANGE_2G;
    args->gyro_range = GYRO_RANGE_250DPS;
    args->odr = ODR_50_HZ;
}

void csv_log_samples(int16_t sample_count, FILE *csv)
{
    int16_t i;
    const RawFifoData *raw_data = bmi270_raw_headerless_fifo_data();
    //const AccelUnits *accel = bmi270_converted_fifo_accel();
    //const GyroUnits *gyro= bmi270_converted_fifo_gyro();

    // For hex representation
    /*
    for (i = 0; i < sample_count; i++) {
        fprintf(csv,"%04X, %04X, %04X, %04X, %04X, %04X\n",
            (uint16_t)raw_data[i].accel.pitch_x, (uint16_t)raw_data[i].gyro.pitch_x,
            (uint16_t)raw_data[i].accel.roll_y, (uint16_t)raw_data[i].gyro.roll_y,
            (uint16_t)raw_data[i].accel.yaw_z, (uint16_t)raw_data[i].gyro.yaw_z);
    }*/
    for (i = 0; i < sample_count; i++) {
        fprintf(csv,"%d, %d, %d, %d, %d, %d\n",
            (int16_t)raw_data[i].accel.pitch_x, (int16_t)raw_data[i].gyro.pitch_x,
            (int16_t)raw_data[i].accel.roll_y, (int16_t)raw_data[i].gyro.roll_y,
            (int16_t)raw_data[i].accel.yaw_z, (int16_t)raw_data[i].gyro.yaw_z);
    }
}

void print_samples(int16_t sample_count)
{
    int16_t i;
    const RawFifoData *raw_data = bmi270_raw_headerless_fifo_data();
    const AccelUnits *accel = bmi270_converted_fifo_accel();
    const GyroUnits *gyro= bmi270_converted_fifo_gyro();

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
        printf("accel/gyro[%d] x: 0x%x | 0x%x y: 0x%x | 0x%x z: 0x%x | 0x%x \n",
            i,
            (uint16_t)raw_data[i].accel.pitch_x, (uint16_t)raw_data[i].gyro.pitch_x,
            (uint16_t)raw_data[i].accel.roll_y, (uint16_t)raw_data[i].gyro.roll_y,
            (uint16_t)raw_data[i].accel.yaw_z, (uint16_t)raw_data[i].gyro.yaw_z);
    }
}

void print_latest_sample(bool disable_debug_print)
{
    if (disable_debug_print) {
        return;
    }
    print_samples(1);
}

void print_fifo_samples(bool disable_debug_print)
{
    int16_t sample_count;
    sample_count = bmi270_fifo_sample_count();
    if (disable_debug_print) {
        return;
    }
    print_samples(sample_count);
}


void update_odr_from_cmdline(CommandlineArgs *args, int odr_hz)
{
    if (odr_hz == 25) {
        args->odr = ODR_25_HZ;
    } else if (odr_hz == 50) {
        args->odr = ODR_50_HZ;
    } else if (odr_hz == 100) {
        args->odr = ODR_100_HZ;
    } else if (odr_hz == 200) {
        args->odr = ODR_200_HZ;
    } else {
        args->odr = ODR_50_HZ; // Default value
    }
}

void update_accel_range_from_cmdline(CommandlineArgs *args, int g_range)
{
    if (g_range == 2) {
        args->accel_range = ACC_RANGE_2G;
    } else if (g_range == 4) {
        args->accel_range = ACC_RANGE_4G;
    } else if (g_range == 8) {
        args->accel_range = ACC_RANGE_8G;
    } else if (g_range == 16) {
        args->accel_range = ACC_RANGE_16G;
    } else {
        args->accel_range = ACC_RANGE_2G; // Default value
    }
}

void update_gyro_range_from_cmdline(CommandlineArgs *args, int dps_range)
{
    if (dps_range == 125) {
        args->gyro_range = GYRO_RANGE_125DPS;
    } else if (dps_range == 250) {
        args->gyro_range = GYRO_RANGE_250DPS;
    } else if (dps_range == 500) {
        args->gyro_range = GYRO_RANGE_500DPS;
    } else if (dps_range == 1000) {
        args->gyro_range = GYRO_RANGE_1000DPS;
    } else if (dps_range == 2000) {
        args->gyro_range = GYRO_RANGE_2000DPS;
    } else {
        args->gyro_range = GYRO_RANGE_250DPS; // Default value
    }
}

void print_help_msg(void)
{
    printf("\n");
}

void datetime_for_filename(char *str, int str_len)
{
    time_t t = time(NULL);
    struct tm *current = localtime(&t);
    strftime(str, str_len, "%Y-%m-%d-%H_%M_%S", current);
    strcat(str,".csv");
}

void handle_cmdline_args(int argc, char **argv, CommandlineArgs *args)
{
    int val;
    int opt;
    while ((opt = getopt(argc, argv, "hidalgo:")) != -1) {
        switch (opt) {
        case 'h':
            // Intentional fallthrough
        case '?':
            print_help_msg();
            break;
        case 'l':
            args->logging_en = true;
            break;
        case 'd':
            args->disable_debug_print = true;
            break;
        case 'i':
            val = atoi(optarg);
            if ((val > 0) && (val <= 1000)) { // Limit to 1sec loop
                args->interval_time_ms = (uint32_t)val;
            } else {
                args->interval_time_ms = 500u;
            }
            break;
        case 'a':
            val = atoi(optarg);
            update_accel_range_from_cmdline(args, val);
            break;
        case 'g':
            val = atoi(optarg);
            update_gyro_range_from_cmdline(args, val);
            break;
        case 'o':
            val = atol(optarg);
            update_odr_from_cmdline(args, val);
            break;
        default: /* '?' */
            print_help_msg();
            break;
        }
    }
}

void sigint_handler(int sig)
{
    continue_run = false;
}


int main(int argc, char **argv)
{
    uint32_t i;
    bool success;
    int16_t sample_count;
    Init_libMPSSE();

    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE channel_info;
    SpiChannelConfig channel_config;
    FT_HANDLE handle;
    float init_timer;
    float start;
    CommandlineArgs args;
    char filename[64] = {0};

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
    
    continue_run = true;
    reset_commandline_args(&args);
    if (argc > 1) // Arguments supplied
    {
        handle_cmdline_args(argc, argv, &args);
    }
    
    continue_run = true;

    if (args.logging_en) {
        datetime_for_filename(filename, sizeof(filename));
        printf("%s\n", filename);
        args.csv = fopen(filename, "w+");
        fprintf(args.csv, "accel_x_hex, gyro_x, accel_y, gyro_y, accel_z, gyro_z\n");
    }

    // Set up to detect Ctrl + C
    signal(SIGINT, sigint_handler);

    // Initialize BMI270
    init_timer = timer_start_elapsed_timer();
    success = bmi270_spi_init(handle, args.odr, args.accel_range, args.odr, args.gyro_range);
    if (!success) {
        printf("Retry bmi270 init\n");
        success = bmi270_spi_init(handle, args.odr, args.accel_range, args.odr, args.gyro_range);
    }
    printf("bmi270_spi_init: %d after %.1fms\n", success, timer_elapsed_time_msec(init_timer));
    bmi270_spi_flush_fifo();

    while (continue_run) {
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
        printf("\nread_time: %.2fms\n", timer_elapsed_time_msec(start));
        sample_count = bmi270_fifo_sample_count();
        printf("samples: %d\n", sample_count);
        if (!args.disable_debug_print)
        {
            print_samples(sample_count);
        }
        if (args.logging_en) 
        {
            csv_log_samples(sample_count, args.csv);
        }
        timer_blocking_sleep_remaining_msec(start, 500u);

    }

    printf("Ctrl + C detected. Cleaning up.\n");
    if (args.logging_en) // Close CSV
    {
        fclose(args.csv);
    }
    Cleanup_libMPSSE();

    return 0;
}
