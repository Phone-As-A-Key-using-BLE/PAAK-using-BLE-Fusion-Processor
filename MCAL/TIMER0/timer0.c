/***********************************************
 * Module Name: TIMERS
 * Author: MOHAMED
 * Purpose: initialize TIMER 0
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/
#include "timer0.h"

/***********************************************
 * Function Name: Timer0_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes Timer0 as a 32-bit periodic timer by enabling the peripheral and configuring it for periodic operation.
 ***********************************************/
void Timer0_Init(void) {
    // Enable the Timer0 peripheral
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);

    // Wait for Timer0 to be ready
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0)) {}

    // Configure Timer0 as a 32-bit periodic timer
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
}

/***********************************************
 * Function Name: Timer0_Delay
 * Inputs:
 *      - uint32_t seconds: The number of seconds to delay.
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Delays the execution for the specified number of seconds by using Timer0.
 * The function calculates the load value based on the system clock and the input time, and then waits for the timer to timeout.
 ***********************************************/
void Timer0_Delay(uint32_t seconds) {
    // Get the system clock frequency
    uint32_t systemClock = SysCtlClockGet();

    // Calculate the load value based on the number of seconds and system clock
    uint32_t loadValue = systemClock * seconds;

    // Set the load value to Timer0
    TimerLoadSet(TIMER0_BASE, TIMER_A, loadValue - (uint32_t)1);

    // Clear any pending interrupt flags from Timer0
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);

    // Enable Timer0
    TimerEnable(TIMER0_BASE, TIMER_A);

    // Wait for the timer to timeout (blocking until the timer reaches the specified time)
    while (!(TimerIntStatus(TIMER0_BASE, false) & TIMER_TIMA_TIMEOUT)) {}

    // Disable Timer0 after the delay
    TimerDisable(TIMER0_BASE, TIMER_A);
}
