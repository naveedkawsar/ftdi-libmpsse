//#include <stdint.h>
#include <stdio.h>
//#include "bmi270.h"
#include "ftd2xx.h"
//#include "libmpsse_spi.h"
#include "ftdi_libMPSSE.h"

// Re-define typedefs from WinTypes to get rid of compiler warnings
//typedef unsigned long int uint32_t;

#define LOG (printf("%s | func: %s() line: %d\n", __FILE__, __func__, __LINE__))

#define BMI270_SDO_MISO_PIN    (26u) // I2C address LSB
#define BMI270_SDA_MOSI_PIN    (27u) // I2C data line
#define BMI270_SCLK_PIN        (28u)
#define BMI270_CSEL_PIN        (29u) // Set high for I2C

#define SPI_READ_MASK       (uint8_t)(0x80) // (1 << 7)

#define CHIP_ID_REG         (uint8_t)(0)
#define INTERNAL_STATUS_REG (uint8_t)(0x21)
#define FIFO_LENGTH_0_REG   (uint8_t)(0x24)
#define FIFO_DATA_REG       (uint8_t)(0x26)
#define ACC_X_LSB_REG       (uint8_t)(0x0C)
#define ACC_CONF_REG        (uint8_t)(0x40)
#define ACC_RANGE_REG       (uint8_t)(0x41)
#define GYRO_CONF_REG       (uint8_t)(0x42)
#define GYRO_RANGE_REG      (uint8_t)(0x43)
#define FIFO_CONFIG_1_REG   (uint8_t)(0x49)

// 11 bits ->  2 ^ 11 = 2048 max value
#define INIT_ADDR_0_REG     (uint8_t)(0x5B) // Bits 0 - 3 of init data base addr
#define INIT_ADDR_1_REG     (uint8_t)(0x5C) // Bits 4 - 11 of init data base addr

#define INIT_CTRL_REG       (uint8_t)(0x59)
#define INIT_DATA_REG       (uint8_t)(0x5E)
#define PWR_CONF_REG        (uint8_t)(0x7C)
#define PWR_CTRL_REG        (uint8_t)(0x7D)
#define CMD_REG             (uint8_t)(0x7E)


#define BMI270_CHIP_ID      (uint8_t)(0x24)

#define FIFO_CONFIG1_GYRO_EN    (uint8_t)(1 << 7)
#define FIFO_CONFIG1_ACCEL_EN   (uint8_t)(1 << 6)
#define FIFO_CONFIG1_AUX_EN     (uint8_t)(1 << 5)
#define FIFO_CONFIG1_HEADER_EN  (uint8_t)(1 << 4)

// Per Table 4
#define TEMP_SENSITIVITY_LSB_PER_K (512u)
#define LSB_0_TEMP_C        (23u)

// Conversion factors
#define DEG_TO_RAD                  (0.01745329f)
#define MILLIDEG_TO_RAD             ((float)(1.745329e-5))
#define G_TO_M_PER_S_SQUARED        (9.80665f)
#define MILLI_G_TO_M_PER_S_SQUARED   (0.00980665f)

#define FIELD_SIZEOF(t, f) (sizeof(((t*)0)->f))

// Indexed by AccRangeG.  Per Table 2
const int16_t accel_sensitivity_lsb_per_g[] =
{
    16384, // ACC_RANGE_2G // 0
    8192, // ACC_RANGE_4G,
    4096, // ACC_RANGE_8G,
    2048 // ACC_RANGE_16G // 0x03
};

// Indexed by GyroRangeDps.  Per Table 3
const float gyro_sensitivity_lsb_per_deg_per_sec[] =
{
    16.384f, // GYRO_RANGE_2000DPS // 0
    32.768f, // GYRO_RANGE_1000DPS
    65.536f, // GYRO_RANGE_500DPS
    131.072f, // GYRO_RANGE_250DPS
    262.144f // GYRO_RANGE_125DPS // 0x04
};

void spi_shift_bytes(FT_HANDLE handle, const uint8_t *tx, uint32_t tx_len, uint8_t *rx, uint32_t rx_len)
{
    // For some reason, SPI_ReadWrite() does not shift any data out, might be
    // an issue with the mingw version of the libmpsse.a file packaged with v0.6
    uint32_t options;
    uint32_t bytes_transferred;
    FT_STATUS status;
    options = SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES | SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE;
    status = SPI_Write(handle, (uint8_t *)tx, tx_len, &bytes_transferred, options);
    if (status != FT_OK)
        LOG;
    if (bytes_transferred != tx_len)
        LOG;
    options |= SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE;
    status = SPI_Read(handle, rx, rx_len, &bytes_transferred, options);
    if (status != FT_OK)
        LOG;
    if (bytes_transferred != rx_len)
        LOG;
}

bool bmi270_chip_id_correct(uint8_t chip_id)
{
    return (chip_id == BMI270_CHIP_ID);
}

uint8_t bmi270_spi_get_id(FT_HANDLE handle)
{
    const uint8_t tx = (CHIP_ID_REG) | (SPI_READ_MASK);
    uint16_t rx = 0xAAAA; // 2 bytes, 1 dummy byte
    uint8_t chip_id;

    spi_shift_bytes(handle, &tx, sizeof(tx), (uint8_t *)&rx, sizeof(rx));
    
    chip_id = ((rx >> 8) & 0xFF); // 1st byte is dummy

    return chip_id;
}

int main(int argc, char **argv)
{
    uint32_t i;
    bool success;
    uint8_t device_id;
    Init_libMPSSE();

    FT_STATUS status;
    FT_DEVICE_LIST_INFO_NODE channel_info;
    SpiChannelConfig channel_config;
    FT_HANDLE handle;

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
    // Configure channel 
    channel_config.ClockRate = I2C_CLOCK_STANDARD_MODE;
    channel_config.config_options = SPI_CONFIG_OPTION_MODE0 | SPI_CONFIG_OPTION_CS_DBUS3 | SPI_CONFIG_OPTION_CS_ACTIVELOW;
    channel_config.LatencyTimer = 100;
    status = SPI_InitChannel(handle, &channel_config);
    if (status != FT_OK)
        LOG;

    while (1) {
        device_id = bmi270_spi_get_id(handle);
        success = bmi270_chip_id_correct(device_id);
        if (success) {
            printf("chip_id: 0x%x\n", device_id);
        }
        for (i = 0; i < 100000000; i++) {
            if (!(i % 2))
                i += 2;
            else
                i -= 2;
        }
    }
    Cleanup_libMPSSE();
    return 0;
}