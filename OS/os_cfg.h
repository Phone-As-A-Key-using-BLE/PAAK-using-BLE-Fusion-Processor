/***********************************************
 * Configuration File: OS Layer
 * Purpose: OS Configuration Parameters
 ***********************************************/

#ifndef OS_CFG_H
#define OS_CFG_H

/***********************************************
 * Communication Protocol Configuration
 ***********************************************/
#define OS_COMM_PROTOCOL   OS_COMM_CAN  /* Choose CAN or SPI */

/***********************************************
 * UART Configuration
 ***********************************************/
#define OS_UART_ACTIVE     true  /* Enable UART communication */

/***********************************************
 * Peripheral Initialization
 ***********************************************/
#define OS_GPIO_INIT_ENABLE          true
#define OS_LED_INIT_ENABLE           true
#define OS_PUSHBUTTON_INIT_ENABLE    true

/***********************************************
 * Diagnostic Mode
 ***********************************************/
#define OS_DIAGNOSTIC_MODE_ENABLE    true

#endif /* OS_CFG_H */
