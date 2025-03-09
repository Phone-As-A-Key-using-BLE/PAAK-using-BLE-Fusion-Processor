/***********************************************
 * Module Name: pushbutton / Switches
 * Author: FADY
 * Purpose: initialize pushbottons on master board
 ***********************************************/

#include "pushbuttons.h"
#include "APP/APP_FSM.h"


volatile bool buttonPressed = false;  // Flag to indicate if button is pressed
extern APP_tenuStates currentState;
/***********************************************
 * Function Name: PushButtons_Init
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes the push buttons on Port F. Unlocks PF0 (SW2) and configures PF0 (SW2) and PF4 (SW1) as input pins with internal pull-up resistors enabled.
 ***********************************************/
void PushButtons_Init(void) {

    // Unlock PF0 (SW2) to be used as GPIO
    HWREG(BUTTON_PORT + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(BUTTON_PORT + GPIO_O_CR) |= BUTTON_SW1;  // SW1 is PF4
    HWREG(BUTTON_PORT + GPIO_O_LOCK) = (uint32_t)0;

    // Set SW1 (PF4) and SW2 (PF0) as input pins
    GPIOPinTypeGPIOInput(BUTTON_PORT, BUTTON_SW1);

    // Enable internal pull-up resistors on SW1 and SW2
    GPIOPadConfigSet(BUTTON_PORT, BUTTON_SW1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Enable interrupts for the push button (SW1)
    GPIOIntEnable(GPIO_PORTF_BASE, BUTTON_SW1);  // Enable interrupt for SW1 (PF4)
    GPIOIntTypeSet(GPIO_PORTF_BASE, BUTTON_SW1, GPIO_FALLING_EDGE);  // Trigger on falling edge (button press)
    GPIOIntRegister(GPIO_PORTF_BASE, PushButtonHandler);  // Register the custom handler function

    // Enable GPIO interrupt in the NVIC
    IntEnable(INT_GPIOF);  // Enable GPIO interrupt for PF4

    // Enable global interrupts
    IntMasterEnable();
}

// Interrupt service routine for push button press
void PushButtonHandler(void) {
    // Clear the interrupt flag
    GPIOIntClear(GPIO_PORTF_BASE, BUTTON_SW1);

    LED_ON(LED_GREEN);
    currentState = STATE_IDLE;
    APP_voidFSMHandler(EVENT_OWNER_PAIRING_BUTTON_PRESSED);
    // When SW1 is pressed, turn on the Red LED and stop the application
    buttonPressed = true;
}

/***********************************************
 * Function Name: PushButton_Read
 * Inputs: uint8_t buttonPin - The specific button pin to check (SW1 or SW2)
 * Outputs: bool - Returns true if the button is pressed (active low), otherwise false
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Reads the state of a specific button and returns true if it is pressed (active low), otherwise returns false.
 ***********************************************/
bool PushButton_Read(uint8_t buttonPin) {
    // Check if the button pin is low (pressed)
    return GPIOPinRead(BUTTON_PORT, buttonPin) == 0;
}
