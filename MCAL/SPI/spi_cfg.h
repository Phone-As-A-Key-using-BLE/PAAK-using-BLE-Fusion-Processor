/***********************************************
 * Configuration File: SPI_Config.h
 * Author: Fady
 * Purpose: Provide configuration parameters for SPI module
 ***********************************************/

#ifndef SPI_CONFIG_H
#define SPI_CONFIG_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>

/***********************************************
 * SPI Configuration Parameters
 ***********************************************/

// Default SPI settings
// Options for baud rate: Common values include 1000000 (1 MHz), 2000000 (2 MHz), etc.
#define SPI_DEFAULT_BAUD_RATE   1000000U    // 1 MHz baud rate

// Options for SPI mode:
// SSI_FRF_MOTO_MODE_0 - Mode 0: Clock Polarity = 0, Clock Phase = 0
// SSI_FRF_MOTO_MODE_1 - Mode 1: Clock Polarity = 0, Clock Phase = 1
// SSI_FRF_MOTO_MODE_2 - Mode 2: Clock Polarity = 1, Clock Phase = 0
// SSI_FRF_MOTO_MODE_3 - Mode 3: Clock Polarity = 1, Clock Phase = 1
#define SPI_DEFAULT_MODE        SSI_FRF_MOTO_MODE_0

// Options for bit length:
// Values typically range from 4 to 16 bits
#define SPI_DEFAULT_BIT_LENGTH  8          // 8-bit data

// SPI Base Address Configuration
// Options for base address: SSI0_BASE, SSI1_BASE, SSI2_BASE, SSI3_BASE
#define SPI_BASE_ADDRESS        SSI1_BASE

#endif /* SPI_CONFIG_H */
