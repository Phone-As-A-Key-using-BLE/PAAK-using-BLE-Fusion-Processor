/***********************************************
 * Configuration File: UART_Config.h
 * Author: Fady
 * Purpose: Provide configuration parameters for UART module
 ***********************************************/

#ifndef UART_CONFIG_H
#define UART_CONFIG_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>


/***********************************************
 * UART Configuration Parameters
 ***********************************************/

// Default UART settings
// Options for baud rate: Common values include 9600, 19200, 38400, 57600, 115200
#define UART_DEFAULT_BAUD_RATE   9600U      // 9600 baud rate

// Options for configuration:
// UART_CONFIG_WLEN_8 - 8 data bits
// UART_CONFIG_WLEN_7 - 7 data bits
// UART_CONFIG_PAR_NONE - No parity
// UART_CONFIG_PAR_EVEN - Even parity
// UART_CONFIG_PAR_ODD - Odd parity
// UART_CONFIG_STOP_ONE - 1 stop bit
// UART_CONFIG_STOP_TWO - 2 stop bits
#define UART_DEFAULT_CONFIG      (UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE)

// UART Base Address Configuration
// Options for base address: UART0_BASE, UART1_BASE.
#define UART_BASE_ADDRESS        UART0_BASE

#define UART_CONNECTIVITY_BASE_ADDRESS UART1_BASE

#endif /* UART_CONFIG_H */
