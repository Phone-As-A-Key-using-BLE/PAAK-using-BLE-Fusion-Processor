/***********************************************
 * Module Name: SPI
 * Author: MOHAMAD WALEED
 * Purpose: Initialize spi 1
 ***********************************************/

#ifndef SPI_H
#define SPI_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"  // For interrupt management
#include "TIMER0/timer0.h"
#include "UART/uart.h"

#include "spi_cfg.h"
/***********************************************
 * Definitions and Macros
 ***********************************************/
#define ACK_CMD 0x04U  // Define ACK command (a simple byte for acknowledgment)
#define UNDEFINED_STATE 0x05U

volatile bool UndefinedState;
/***********************************************
 * Type Declarations (enums, structs and unions)
 ***********************************************/
typedef enum {
    COMM_FAILURE,
    COMM_SUCCESS,
    COMM_UNDEFINED
} COMMStatus;

typedef enum {
    SPI_FAILURE,
    SPI_SUCCESS,
    SPI_UNDEFINED
} SPIStatus;

typedef enum {
    STATE_RED_INITIAL = 0x01,
    STATE_GREEN       = 0x02,
    STATE_BLUE        = 0x03
} LEDState;
/***********************************************
 * Functions Prototypes
 ***********************************************/
void SPI_Init(void);
COMMStatus SPI_Receive(uint32_t base, uint32_t *local_data);
COMMStatus sendStateToSlave(LEDState state);
void SPI_MasterInit(void);
//void SSI1_Handler(void);
#endif /* SPI_H */
