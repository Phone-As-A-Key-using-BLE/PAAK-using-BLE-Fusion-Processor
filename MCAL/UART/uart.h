/***********************************************
 * Module Name: UART
 * Author: Waleed
 * Purpose: initialize UART
 ***********************************************/


#ifndef UART_H
#define UART_H

/***********************************************
  Includes
 ***********************************************/
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include <inttypes.h>
#include "inc/hw_ints.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"



#include "uart_cfg.h"

/***********************************************
  Variables
 ***********************************************/
unsigned char data;

/***********************************************
  Function Prototypes
 ***********************************************/
void UART_Init(void);
void UART_SendMessage(const char *array);
int32_t UART_RecieveMessage(void);
void UART_SendNumber(uint32_t number);
void UART_ProcessCommand(const char *command);  // Process received diagnostic commands
void UART_SendFloat(float number);
void ftoa(float number, char *buffer, int decimalPlaces);
void UART_SendMessageConnectivity(const char *array, uint16_t stop);
void UART_SendNumberConnectivity(uint8_t number);
void UART_SendHexConnectivity(uint8_t *dataArray, uint8_t stopIndex);
int my_sprintf(char** buffer, const char* format, ...);
#endif /* UART_H */
