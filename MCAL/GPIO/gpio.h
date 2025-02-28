/***********************************************
 * Module Name: GPIO
 * Author: ABDULLAH
 * Purpose: initialize PORTF as GPIO
 ***********************************************/

#ifndef GPIO_H
#define GPIO_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "inc/hw_gpio.h"

#include "gpio_cfg.h"

/***********************************************
 * Function Prototypes
 ***********************************************/
void GPIO_InitPort(uint32_t portBase);

#endif /* GPIO_H */
