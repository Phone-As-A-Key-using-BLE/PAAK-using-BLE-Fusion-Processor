/***********************************************
 * Module Name: led
 * Author: FADY
 * Purpose: initialize leds on master board
 ***********************************************/

#ifndef LED_H
#define LED_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

/***********************************************
 * Definitions and Macros
 ***********************************************/

#define LED_RED      GPIO_PIN_1
#define LED_BLUE     GPIO_PIN_2
#define LED_GREEN    GPIO_PIN_3
#define LED_WHITE   (GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)

#define SYSTICK_PERIOD_MS 50  // SysTick handler period
#define LED_BLINK_PERIOD_MS 1000  // LED blink delay (1 second)

extern volatile uint32_t sysCounter;


/***********************************************
 * LED States
 ***********************************************/
typedef enum {
    LED_OFF_STATE,
    LEDRED,
    LEDBLUE,
    LEDWHITE
} LED_State;

/***********************************************
 * Function Prototypes
 ***********************************************/
void LED_Init(void);
void LED_ON(uint8_t led_color);
void LED_OFF(uint8_t led_color);
void LED_BLINK(uint8_t led_color);
bool LED_IS_ON(uint32_t led);
void LED_BLINK_SYSTICK(void);
void LED_SetState(LED_State state); // Set LED state
void LED_BLINK_NEW(uint32_t LedState);

#endif /* LED_H */
