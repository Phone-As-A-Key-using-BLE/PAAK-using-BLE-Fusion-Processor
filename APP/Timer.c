// timer_driver.c
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "APP/Timer.h"
#include "driverlib/timer.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/timer.h"

static TimerCallback_t TimerCallback = 0;

void Timer0AIntHandler(void) {
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerDriver_Stop();
    
    if (TimerCallback) {
        TimerCallback();
    }
}

void TimerDriver_Init(void) {
    // Enable Timer0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    
    TimerConfigure(TIMER0_BASE, TIMER_CFG_ONE_SHOT);
    
    // Enable Timer interrupt
    TimerIntRegister(TIMER0_BASE, TIMER_A, Timer0AIntHandler);
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}

void TimerDriver_Start(uint32_t milliseconds, TimerCallback_t callback) {
    uint32_t loadValue = (SysCtlClockGet() / 1000) * milliseconds;
    
    TimerCallback = callback;
    
    TimerLoadSet(TIMER0_BASE, TIMER_A, loadValue - 1);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void TimerDriver_Stop(void) {
    TimerDisable(TIMER0_BASE, TIMER_A);
}


