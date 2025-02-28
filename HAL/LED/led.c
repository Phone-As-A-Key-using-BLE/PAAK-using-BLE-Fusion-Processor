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

/***********************************************
 * Function Name: LED_BLINK
 * Inputs: uint8_t led_color (LED_RED, LED_BLUE, LED_GREEN, or LED_WHITE)
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Blinks the specified LED(s) on and off with a delay of 500 milliseconds.
 ***********************************************/
void LED_BLINK(uint8_t led_color) {
    LED_OFF(LED_WHITE);  // Turn off all LEDs first
    LED_ON(led_color);   // Turn on the specified LED
    Timer0_Delay((uint32_t)1);  // 1-second delay
    LED_OFF(led_color);  // Turn off the LED
    Timer0_Delay((uint32_t)1);  // 1-second delay
}

//bool LED_IS_ON(uint32_t led) {
//    return GPIOPinRead(GPIO_PORTF_BASE, led) != 0; // Check if the pin is HIGH
//}

void LED_BLINK_SYSTICK(void) {
    LED_OFF(LED_WHITE);
    static uint32_t lastCounter = 0;  // Stores the last SysTick count
    uint32_t currentCounter = sysCounter;

    // Check if the elapsed time has reached the blink period
    if ((currentCounter - lastCounter) * (uint32_t)1 >= (uint32_t)LED_BLINK_PERIOD_MS) {
        // Toggle the LED
        GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
                     (uint8_t)GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);

        // Update the last counter
        lastCounter = currentCounter;
    }
}

/***********************************************
 * Function Name: LED_SetState
 * Purpose: Set the state of LEDs
 ***********************************************/
void LED_SetState(LED_State state) {
    switch (state) {
        case LED_OFF_STATE:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3, 0);
            break;

        case LEDRED:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1, GPIO_PIN_1);  // Red LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2 | GPIO_PIN_3, 0);
            break;

        case LEDBLUE:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2, GPIO_PIN_2);  // Blue LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_3, 0);
            break;

        case LEDWHITE:
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,
                         GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);  // All LEDs
            break;
    }
}

void LED_BLINK_NEW(uint32_t LedState) {
    static uint32_t lastCounter = 0;  // Stores the last SysTick count
    uint32_t currentCounter = sysCounter;

    // Check if the elapsed time has reached the blink period
    if ((currentCounter - lastCounter) >= 1000) {

        switch(LedState)
        {
        case LED_RED:
            // Toggle the LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1,
            GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1);
            break;

        case LED_GREEN:
            // Toggle the LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,
            GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ^ GPIO_PIN_3);
            break;

        case LED_BLUE:
            // Toggle the LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,
            GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2);
            break;

        case LED_WHITE:
            // Toggle the LED
            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3,(GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ^ GPIO_PIN_3) | (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2) | (GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_1) ^ GPIO_PIN_1));
//            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_2,GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_2) ^ GPIO_PIN_2);
//            GPIOPinWrite(GPIO_PORTF_BASE, GPIO_PIN_3,GPIOPinRead(GPIO_PORTF_BASE, GPIO_PIN_3) ^ GPIO_PIN_3);
            break;

        }

        // Update the last counter
        lastCounter = currentCounter;
    }
}
