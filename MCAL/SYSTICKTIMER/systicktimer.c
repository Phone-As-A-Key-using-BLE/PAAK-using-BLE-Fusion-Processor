/***********************************************
 * Module Name: SYSTICKTIMER
 * Author: ABDULLAH
 * Purpose: initialize SYSTICK TIMER
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/
#include "systicktimer.h"
#include "driverlib/sysctl.h"
#include "driverlib/systick.h"
#include "driverlib/interrupt.h"

/***********************************************
 * Function Name: SysTick_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes the SysTick timer to generate an interrupt every 10 ms (100 Hz). The function sets the SysTick period to match the system clock, enabling the SysTick interrupt and registering the interrupt handler.
 ***********************************************/
void SysTick_Init(void) {
    // Set the clock source for SysTick to the system clock (default)
    SysTickPeriodSet(SysCtlClockGet() /1000 );  // 1 ms interval
    SysTickIntRegister(&SysTick_Handler);        // Register SysTick interrupt handler
    SysTickIntEnable();                         // Enable SysTick interrupt
    SysTickEnable();                            // Enable SysTick timer
}
