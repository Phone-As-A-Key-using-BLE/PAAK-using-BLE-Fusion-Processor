/***********************************************
 * Module Name: GPIO
 * Author: ABDULLAH
 * Purpose: initialize PORTF as GPIO
 ***********************************************/

#include "gpio.h"

/***********************************************
 * Function Name: PortF_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes Ports by enabling its GPIO peripheral and waiting for the peripheral to be ready.
 ***********************************************/
// General function to enable a GPIO port
void GPIO_InitPort(uint32_t portBase)
{

    // Enable the GPIO peripheral for the specified port
    if (portBase == GPIO_PORTA_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA); // Enable GPIO Port A
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    }
    else if (portBase == GPIO_PORTB_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); // Enable GPIO Port B
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}
    }
    else if (portBase == GPIO_PORTC_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOC); // Enable GPIO Port C
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOC)) {}
    }
    else if (portBase == GPIO_PORTD_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD); // Enable GPIO Port D
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD)) {}
    }
    else if (portBase == GPIO_PORTE_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE); // Enable GPIO Port E
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOE)) {}
    }
    else if (portBase == GPIO_PORTF_BASE) {
        SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF); // Enable GPIO Port F
        // Wait for the GPIO module to be ready
                while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF)) {}
    }
    else {
            // Optional: Add a warning or error here if an invalid port is passed.
            // For example, you could print a message or handle the error in another way.
            // No action is needed here, but this ensures compliance with MISRA-C.
        }
    // Add more GPIO ports as necessary


}
