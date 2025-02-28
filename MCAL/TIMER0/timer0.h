/***********************************************
 * Module Name: TIMERS
 * Author: MOHAMED
 * Purpose: initialize TIMER 0
 ***********************************************/


#ifndef TIMER0_H
#define TIMER0_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"   // Include Timer library
#include "driverlib/interrupt.h" // For clearing interrupt flags, though we won't use interrupts here
#include "inc/hw_memmap.h"

/***********************************************
  * Defines
 ***********************************************/

//#define TIMER0_BASE 0x40030000  // Base address for Timer 0 (depends on your microcontroller)
/***********************************************
  * Function Prototypes
 ***********************************************/
void Timer0_Init(void);      // Initializes Timer0
void Timer0_Delay(uint32_t seconds);  // Delay function using Timer0

#endif /* TIMER0_H */
