/*!
 * \file libMPSSE_i2c.h
 *
 * \author FTDI
 * \date 20110505
 *
 * Copyright © 2000-2014 Future Technology Devices International Limited
 *
 *
 * THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY DEVICES INTERNATIONAL LIMITED
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Project: libMPSSE
 * Module: I2C
 *
 * Rivision History:
 * 0.1 - initial version
 * 0.2 - 20110708 - added FT_ReadGPIO, FT_WriteGPIO & 3-phase clocking
 * 0.3 - 20111025 - modified for supporting 64bit linux
 *                    added I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE
 * 0.5 - 20140912 - modified for compilation issues with either C application/C++ application
 *
 * Project: libMPSSE
 * Module: SPI
 *
 * Rivision History:
 * 0.1  - initial version
 * 0.2  - 20110708 - added FT_ReadGPIO, FT_WriteGPIO & SPI_ChangeCS
 * 0.3  - 20111025 - modified for supporting 64bit linux
 * 0.41 - 20140903 - modified for compilation issues with either C application/C++ application
 */

#ifndef FTDI_LIBMPSSE_H
#define FTDI_LIBMPSSE_H

//#include <stdbool.h>
//#include <stdint.h>
#include "ftd2xx.h"


/******************************************************************************/
/*                                Macro defines                                  */
/******************************************************************************/

#ifdef __cplusplus
#define FTDI_API extern "C"
#else
#define FTDI_API
#endif

/* Options to I2C_DeviceWrite & I2C_DeviceRead */
/*Generate start condition before transmitting */
#define I2C_TRANSFER_OPTIONS_START_BIT        0x00000001

/*Generate stop condition before transmitting */
#define I2C_TRANSFER_OPTIONS_STOP_BIT        0x00000002

/*Continue transmitting data in bulk without caring about Ack or nAck from device if this bit is
not set. If this bit is set then stop transitting the data in the buffer when the device nAcks*/
#define I2C_TRANSFER_OPTIONS_BREAK_ON_NACK    0x00000004

/* libMPSSE-I2C generates an ACKs for every byte read. Some I2C slaves require the I2C
master to generate a nACK for the last data byte read. Setting this bit enables working with such
I2C slaves */
#define I2C_TRANSFER_OPTIONS_NACK_LAST_BYTE    0x00000008

/* no address phase, no USB interframe delays */
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BYTES    0x00000010
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER_BITS    0x00000020
#define I2C_TRANSFER_OPTIONS_FAST_TRANSFER        0x00000030

/* if I2C_TRANSFER_OPTION_FAST_TRANSFER is set then setting this bit would mean that the
address field should be ignored. The address is either a part of the data or this is a special I2C
frame that doesn't require an address*/
#define I2C_TRANSFER_OPTIONS_NO_ADDRESS        0x00000040

#define I2C_CMD_GETDEVICEID_RD    0xF9
#define I2C_CMD_GETDEVICEID_WR    0xF8

#define I2C_GIVE_ACK    1
#define I2C_GIVE_NACK    0

/* 3-phase clocking is enabled by default. Setting this bit in config_options will disable it */
#define I2C_DISABLE_3PHASE_CLOCKING    0x0001

/* The I2C master should actually drive the SDA line only when the output is LOW. It should be
tristate the SDA line when the output should be high. This tristating the SDA line during output
HIGH is supported only in FT232H chip. This feature is called DriveOnlyZero feature and is
enabled when the following bit is set in the options parameter in function I2C_Init */
#define I2C_ENABLE_DRIVE_ONLY_ZERO    0x0002

/* Bit defination of the transfer_options parameter in SPI_Read, SPI_Write & SPI_Transfer  */

/* transfer_options-Bit0: If this bit is 0 then it means that the transfer size provided is in bytes */
#define    SPI_TRANSFER_OPTIONS_SIZE_IN_BYTES            0x00000000
/* transfer_options-Bit0: If this bit is 1 then it means that the transfer size provided is in bytes */
#define    SPI_TRANSFER_OPTIONS_SIZE_IN_BITS            0x00000001
/* transfer_options-Bit1: if BIT1 is 1 then CHIP_SELECT line will be enabled at start of transfer */
#define    SPI_TRANSFER_OPTIONS_CHIPSELECT_ENABLE        0x00000002
/* transfer_options-Bit2: if BIT2 is 1 then CHIP_SELECT line will be disabled at end of transfer */
#define SPI_TRANSFER_OPTIONS_CHIPSELECT_DISABLE        0x00000004


// SPI
/* Bit defination of the Options member of config_options structure*/
#define SPI_CONFIG_OPTION_MODE_MASK        0x00000003
#define SPI_CONFIG_OPTION_MODE0            0x00000000
#define SPI_CONFIG_OPTION_MODE1            0x00000001
#define SPI_CONFIG_OPTION_MODE2            0x00000002
#define SPI_CONFIG_OPTION_MODE3            0x00000003

#define SPI_CONFIG_OPTION_CS_MASK           0x0000001C        /*111 00*/
#define SPI_CONFIG_OPTION_CS_DBUS3        0x00000000        /*000 00*/
#define SPI_CONFIG_OPTION_CS_DBUS4        0x00000004        /*001 00*/
#define SPI_CONFIG_OPTION_CS_DBUS5        0x00000008        /*010 00*/
#define SPI_CONFIG_OPTION_CS_DBUS6        0x0000000C        /*011 00*/
#define SPI_CONFIG_OPTION_CS_DBUS7        0x00000010        /*100 00*/

#define SPI_CONFIG_OPTION_CS_ACTIVELOW    0x00000020

/******************************************************************************/
/*                                Type defines                                  */
/******************************************************************************/

typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned long long  uint64_t;

typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed long long    int64_t;

#ifndef __cplusplus
typedef unsigned char       bool;
#endif

#ifdef __x86_64__
/*20111025: 64bit linux doesn't work is uint32 is unsigned long*/
    typedef unsigned int    uint32_t;
    typedef signed int      int32_t;
#else
    typedef unsigned long   uint32_t;
    typedef signed long     int32_t;
#endif

typedef enum I2C_ClockRate_t{
    I2C_CLOCK_STANDARD_MODE = 100000,                            // 100kb/sec
    I2C_CLOCK_FAST_MODE = 400000,                                // 400kb/sec
    I2C_CLOCK_FAST_MODE_PLUS = 1000000,                         // 1000kb/sec
    I2C_CLOCK_HIGH_SPEED_MODE = 3400000                         // 3.4Mb/sec
} I2C_CLOCKRATE;


/* Channel configuration information */
typedef struct
{
    I2C_CLOCKRATE   ClockRate;
    uint8_t         LatencyTimer;
    uint32_t        Options;
} I2cChannelConfig;

typedef struct
{
    uint32_t    ClockRate;

    uint8_t     LatencyTimer;

    uint32_t    config_options;  /*This member provides a way to enable/disable features
    specific to the protocol that are implemented in the chip
    BIT1-0=CPOL-CPHA:    00 - MODE0 - data captured on rising edge, propagated on falling
                         01 - MODE1 - data captured on falling edge, propagated on rising
                         10 - MODE2 - data captured on falling edge, propagated on rising
                         11 - MODE3 - data captured on rising edge, propagated on falling
    BIT4-BIT2: 000 - A/B/C/D_DBUS3=ChipSelect
             : 001 - A/B/C/D_DBUS4=ChipSelect
              : 010 - A/B/C/D_DBUS5=ChipSelect
              : 011 - A/B/C/D_DBUS6=ChipSelect
              : 100 - A/B/C/D_DBUS7=ChipSelect
     BIT5: ChipSelect is active high if this bit is 0
    BIT6 -BIT31        : Reserved
    */

    uint32_t    Pin;/*BIT7   -BIT0:   Initial direction of the pins    */
                    /*BIT15 -BIT8:   Initial values of the pins        */
                    /*BIT23 -BIT16: Final direction of the pins        */
                    /*BIT31 -BIT24: Final values of the pins        */
    uint16_t      reserved;
} SpiChannelConfig;

/******************************************************************************/
/*                                External variables                              */
/******************************************************************************/





/******************************************************************************/
/*                                Function declarations                          */
/******************************************************************************/
FTDI_API FT_STATUS I2C_GetNumChannels(uint32_t *num_channels);
FTDI_API FT_STATUS I2C_GetChannelInfo(uint32_t index,
    FT_DEVICE_LIST_INFO_NODE *chan_info);
FTDI_API FT_STATUS I2C_OpenChannel(uint32_t index, FT_HANDLE *handle);
FTDI_API FT_STATUS I2C_InitChannel(FT_HANDLE handle, I2cChannelConfig *config);
FTDI_API FT_STATUS I2C_CloseChannel(FT_HANDLE handle);
FTDI_API FT_STATUS I2C_DeviceRead(FT_HANDLE handle, uint32_t device_address,
    uint32_t size_to_transfer, uint8_t *buffer, uint32_t *size_transferred, uint32_t options);
FTDI_API FT_STATUS I2C_DeviceWrite(FT_HANDLE handle, uint32_t device_address,
    uint32_t size_to_transfer, uint8_t *buffer, uint32_t *size_transferred, uint32_t options);
FTDI_API void Init_libMPSSE(void);
FTDI_API void Cleanup_libMPSSE(void);
FTDI_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, uint8_t dir, uint8_t value);
FTDI_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle,uint8_t *value);

FTDI_API FT_STATUS SPI_GetNumChannels(uint32_t *num_channels);
FTDI_API FT_STATUS SPI_GetChannelInfo(uint32_t index,
    FT_DEVICE_LIST_INFO_NODE *chan_info);
FTDI_API FT_STATUS SPI_OpenChannel(uint32_t index, FT_HANDLE *handle);
FTDI_API FT_STATUS SPI_InitChannel(FT_HANDLE handle, SpiChannelConfig *config);
FTDI_API FT_STATUS SPI_CloseChannel(FT_HANDLE handle);
FTDI_API FT_STATUS SPI_Read(FT_HANDLE handle, uint8_t *buffer,
    uint32_t size_to_transfer, uint32_t *size_transferred, uint32_t options);
FTDI_API FT_STATUS SPI_Write(FT_HANDLE handle, uint8_t *buffer,
    uint32_t size_to_transfer, uint32_t *size_transferred, uint32_t options);
FTDI_API FT_STATUS SPI_ReadWrite(FT_HANDLE handle, uint8_t *in_buffer,
    uint8_t *out_buffer, uint32_t size_to_transfer, uint32_t *size_transferred,
    uint32_t transfer_options);
FTDI_API FT_STATUS SPI_IsBusy(FT_HANDLE handle, bool *state);

FTDI_API FT_STATUS SPI_ChangeCS(FT_HANDLE handle, uint32_t config_options);
FTDI_API FT_STATUS FT_WriteGPIO(FT_HANDLE handle, uint8_t dir, uint8_t value);
FTDI_API FT_STATUS FT_ReadGPIO(FT_HANDLE handle,uint8_t *value);
FTDI_API FT_STATUS SPI_ToggleCS(FT_HANDLE handle, bool state);


/******************************************************************************/


#endif // FTDI_LIBMPSSE_H

