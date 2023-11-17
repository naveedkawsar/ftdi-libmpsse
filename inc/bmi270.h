#ifndef BMI270_H_
#define BMI270_H_

//#include <stdbool.h>
//#include <stdint.h>
#include "ftdi_libMPSSE.h"

// Per section 5.2.42
typedef enum {
    ACC_RANGE_2G = 0,
    ACC_RANGE_4G,
    ACC_RANGE_8G,
    ACC_RANGE_16G // 0x03
} AccRangeG;

// Per section 5.2.44
typedef enum {
    GYRO_RANGE_2000DPS = 0,
    GYRO_RANGE_1000DPS,
    GYRO_RANGE_500DPS,
    GYRO_RANGE_250DPS,
    GYRO_RANGE_125DPS // 0x04
} GyroRangeDps;

// Per section 5.2.41, 43, 45
typedef enum {
    ODR_0_78_HZ = 1, // 25/32
    ODR_1_5_HZ, // 25/16
    ODR_3_1_HZ, // 25/8
    ODR_6_25_HZ, // 25/4
    ODR_12_5_HZ, // 25/2
    ODR_25_HZ, // 25
    ODR_50_HZ, // 50
    ODR_100_HZ, // 100
    ODR_200_HZ, // 200
    ODR_400_HZ, // 400
    ODR_800_HZ, // 800, 0x0b
    ODR_1600_HZ, // Reserved
    ODR_3200_HZ, // Reserved
    ODR_6400_HZ, // Reserved
    ODR_12800_HZ // Reserved
} OutputDataRateHz;

typedef struct {
    int16_t pitch_x;
    int16_t roll_y;
    int16_t yaw_z;
} ThreeAxisInt;

typedef struct {
    ThreeAxisInt accel; // Linear acceleration
    ThreeAxisInt gyro; // Angular rate
} RawMotionData;

typedef struct {
    ThreeAxisInt gyro; // Angular rate
    ThreeAxisInt accel; // Linear acceleration
} RawFifoData;

typedef struct {
    float pitch_x;
    float roll_y;
    float yaw_z;
} ThreeAxisFloat;

typedef struct {
    ThreeAxisFloat accel; // Linear acceleration
    ThreeAxisFloat gyro; // Angular rate
} ConvertedMotionData;

typedef struct
{
    ThreeAxisFloat g_force;
    ThreeAxisFloat m_per_sec_squared;
} AccelUnits;

typedef struct
{
    ThreeAxisFloat deg_per_sec;
    ThreeAxisFloat rad_per_sec;
} GyroUnits;


bool bmi270_spi_init(FT_HANDLE handle, OutputDataRateHz accel_data_rate,
                AccRangeG accel_range,
                OutputDataRateHz gyro_data_rate, GyroRangeDps gyro_range);
void bmi270_spi_read_data(void);
void bmi270_spi_read_raw_data(uint8_t *rx, uint32_t max_rx_len);
void bmi270_spi_flush_fifo(void);
void bmi270_spi_read_headerless_fifo(void);
// Call bmi270_spi_read_headerless_fifo() before using the following functions
int16_t bmi270_fifo_sample_count(void);
const RawFifoData * const bmi270_raw_headerless_fifo_data(void);
const AccelUnits * const bmi270_converted_fifo_accel(void);
const GyroUnits * const bmi270_converted_fifo_gyro(void);

#endif // BMI270_H_
