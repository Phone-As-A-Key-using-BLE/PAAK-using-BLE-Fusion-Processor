/***********************************************
 * Module Name: led
 * Author: FADY
 * Purpose: initialize leds on master board
 ***********************************************/

#include "led.h"


/***********************************************
 * Function Name: LED_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Configures PF1, PF2, and PF3 pins on the GPIO port as output for controlling LEDs.
 ***********************************************/
void LED_Init(void){
    // Configure PF1, PF2, and PF3 as output for LEDs
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, LED_RED | LED_BLUE | LED_GREEN);
}


/***********************************************
 * Function Name: LED_ON
 * Inputs: uint8_t led_color (LED_RED, LED_BLUE, LED_GREEN, or LED_WHITE)
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Turns on the specified LED(s) by setting the corresponding GPIO pin(s) high.
 ***********************************************/
void LED_ON(uint8_t led_color) {
    switch (led_color) {
        case LED_RED:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, led_color);
            break;
        case LED_BLUE:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color,led_color);
            break;
        case LED_GREEN:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color,led_color);
            break;
        case LED_WHITE:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, led_color);
            break;
        default:
            // Invalid LED color, do nothing
            break;
    }
}

/***********************************************
 * Function Name: LED_OFF
 * Inputs: uint8_t led_color (LED_RED, LED_BLUE, LED_GREEN, or LED_WHITE)
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Turns off the specified LED(s) by setting the corresponding GPIO pin(s) low.
 ***********************************************/
void LED_OFF(uint8_t led_color) {
    switch (led_color) {
        case LED_RED:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, (uint8_t)0);
            break;
        case LED_BLUE:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, (uint8_t)0);
            break;
        case LED_GREEN:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, (uint8_t)0);
            break;
        case LED_WHITE:
            GPIOPinWrite(GPIO_PORTF_BASE, led_color, (uint8_t)0);
            break;
        default:
            // Invalid LED color, do nothing
            break;
    }
}
