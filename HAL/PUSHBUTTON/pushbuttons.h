/***********************************************
 * Module Name: pushbutton / Switches
 * Author: FADY
 * Purpose: initialize pushbottons on master board
 ***********************************************/


#ifndef PUSHBUTTONS_H
#define PUSHBUTTONS_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"
#include "HAL/LED/led.h"

/***********************************************
 * Definitions and Macros
 ***********************************************/
#define BUTTON_SW1 GPIO_PIN_4  // SW1 is connected to PF4
#define BUTTON_SW2 GPIO_PIN_0  // SW2 is connected to PF0
#define BUTTON_PORT GPIO_PORTF_BASE  // Button port is Port F


/***********************************************
 * Function Prototypes
 ***********************************************/
void PushButtons_Init(void);
bool PushButton_Read(uint8_t buttonPin);
void PushButtonHandler(void);
#endif /* PUSHBUTTONS_H */
