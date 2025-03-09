///*
// * Module Name: MASTER ECU1 Application
// * Author: MOHAMAD WALEED & ABDELRAHMAN
// * Purpose: Entry point for the Master ECU. Uses OS Layer for task management.
//*/

#include "OS/os.h"
#include "Connectivity/connectivity.h"
#include "UART/uart.h"

 int main(void)
{
    OS_Init();

    // Main loop to keep checking for incoming messages
    while (1)
    {
        // The receiver will handle interrupts when a message is received.
        // Nothing needs to be done here in the main loop as interrupts will trigger.
    }
}
