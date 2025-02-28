/***********************************************
 * Module Name: TIMERS
 * Author: MOHAMED WALEED
 * Purpose: initialize TIMER 1
 ***********************************************/

#ifndef TIMER_H
#define TIMER_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"
#include "CAN/can.h"
#include "UART/uart.h"

/***********************************************
 * Variables
 ***********************************************/
volatile uint32_t idleTimeoutCounter;
//static uint8_t IdleState = 0x04;
volatile bool Idle_Flag_timer;

/***********************************************
 * Function Prototypes
 ***********************************************/
void Timer1_Init(void);  // Initialize Timer 1
void Timer1IntHandler(void);  // Timer 1 interrupt handler

#endif /* TIMER_H */
