/***********************************************
 * Module Name: TIMERS
 * Author: MOHAMED
 * Purpose: initialize TIMER 1
 ***********************************************/

#include "timer1.h"


/***********************************************
 * Function Name: Timer1_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes Timer 1 in periodic mode to generate an interrupt every second.
 * It enables the peripheral, configures the timer, and starts the timer with a 1-second interval. The function also registers an interrupt handler for Timer 1 and enables its interrupt.
 ***********************************************/
void Timer1_Init(void) {
    // Enable and configure Timer 1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER1)) {
        // Wait for Timer 1 to be ready
    }

    // Configure Timer 1 in periodic mode
    TimerConfigure(TIMER1_BASE, TIMER_CFG_A_PERIODIC);
    TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());  // 1-second interval
    //TimerIntRegister(TIMER1_BASE, TIMER_A, Timer1IntHandler);  // Register interrupt handler
    TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);  // Enable interrupt for Timer 1
    TimerEnable(TIMER1_BASE, TIMER_A);  // Start Timer 1 (it will count every second)
}

/***********************************************
 * Function Name: Timer1IntHandler
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Interrupt handler for Timer 1, which increments an idle timeout counter
 *              and sends a warning via UART if no command is received for 10 seconds.
 ***********************************************/

// Timer 1 interrupt handler for the 10-second timeout
void Timer1IntHandler(void) {
    // Clear the interrupt flag for Timer 1
    TimerIntClear(TIMER1_BASE, TIMER_TIMA_TIMEOUT);

    // Increment the idle timeout counter
    idleTimeoutCounter++;

    if (idleTimeoutCounter >= (uint32_t)11) {  // 10 seconds elapsed without a command
        Idle_Flag_timer=true;
//        UART_SendMessage("Warning Be careful you didn't send any state for 10seconds.\r\n");
        idleTimeoutCounter = (uint32_t)0;
    }
}
