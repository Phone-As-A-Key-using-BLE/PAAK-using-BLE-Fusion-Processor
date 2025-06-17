#include "pushbuttons.h"
#include "APP/APP_FSM.h"
#include <stdint.h>

volatile bool buttonPressed = false;
extern APP_tenuStates currentState;
extern uint8_t isBondingDataReceived;

void PushButtons_Init(void) {
    // Unlock PF0 (SW2)
    HWREG(BUTTON_PORT + GPIO_O_LOCK) = GPIO_LOCK_KEY;
    HWREG(BUTTON_PORT + GPIO_O_CR) |= BUTTON_SW1 | BUTTON_SW2;  // Enable both SW1 (PF4) and SW2 (PF0)
    HWREG(BUTTON_PORT + GPIO_O_LOCK) = 0;

    // Set SW1 (PF4) and SW2 (PF0) as input pins
    GPIOPinTypeGPIOInput(BUTTON_PORT, BUTTON_SW1 | BUTTON_SW2);

    // Enable internal pull-up resistors
    GPIOPadConfigSet(BUTTON_PORT, BUTTON_SW1 | BUTTON_SW2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPU);

    // Set interrupt types for both buttons to falling edge
    GPIOIntTypeSet(GPIO_PORTF_BASE, BUTTON_SW1 | BUTTON_SW2, GPIO_FALLING_EDGE);

    // Register the interrupt handler
    GPIOIntRegister(GPIO_PORTF_BASE, PushButtonHandler);

    // Enable interrupts for both SW1 and SW2
    GPIOIntEnable(GPIO_PORTF_BASE, BUTTON_SW1 | BUTTON_SW2);

    // Enable GPIO interrupt in the NVIC
    IntEnable(INT_GPIOF);

    // Enable global interrupts
    IntMasterEnable();
}

// Unified interrupt handler for both buttons
void PushButtonHandler(void) {
    uint32_t status = GPIOIntStatus(GPIO_PORTF_BASE, true);

    // Clear all handled interrupts
    GPIOIntClear(GPIO_PORTF_BASE, status);

    if (status & BUTTON_SW1) {
        // SW1 (PF4) pressed
        LED_ON(LED_GREEN);
        isBondingDataReceived = 0;
        currentState = STATE_IDLE;
        APP_voidFSMHandler(EVENT_OWNER_PAIRING_BUTTON_PRESSED, 0);
        buttonPressed = true;
    }

    if (status & BUTTON_SW2) {
        // SW2 (PF0) pressed
        LED_ON(LED_RED);
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_ADV, CAN_PRIMARY_ANCHOR, 0);
        buttonPressed = true;
    }
}

// Reads if a specific button is pressed
bool PushButton_Read(uint8_t buttonPin) {
    return GPIOPinRead(BUTTON_PORT, buttonPin) == 0;
}
