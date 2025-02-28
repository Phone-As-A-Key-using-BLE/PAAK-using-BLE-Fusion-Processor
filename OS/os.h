/***********************************************
 * Module Name: OS Layer
 * Purpose: Task Scheduler and Application Manager
 ***********************************************/

#ifndef OS_H
#define OS_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>
#include "MCAL/CAN/can.h"
#include "MCAL/SYSTICKTIMER/systicktimer.h"
#include "MCAL/UART/uart.h"
#include "MCAL/NVM/nvm.h"
#include "MCAL/ADC/adc.h"
#include "HAL/LED/led.h"
#include "MCAL/GPIO/gpio.h"
#include "HAL/PUSHBUTTON/pushbuttons.h"

volatile uint32_t sysCounter;

typedef enum {
    STATE_NORMAL,
    STATE_OVERHEAT,
    STATE_COMMUNICATION_LOST,
    STATE_FAULT
} SystemState;

// Event Flags
extern volatile bool flag_heartbeat;         // Send heartbeat
extern volatile bool flag_receiveTemperature;// Receive temperature

/***********************************************
 * EEPROM Address Definitions
 ***********************************************/
#define DTC_CODE_ADDR            0x400
#define OVERHEAT_COUNTER_ADDR    0x800
#define FaultState_COUNTER_ADDR  0x600
#define COMMLOSS_COUNTER_ADDR    0x500
#define FLAG_ADDR                0x804
/***********************************************
 * Function Prototypes
 ***********************************************/

// OS Core Functions
void OS_Init(void);                  // Initialize the OS and peripherals
void OS_RunScheduler(void);          // Main loop to execute tasks
void SysTick_Handler(void);          // SysTick ISR for periodic interrupts

// Tester Mode Functions
void EnterTesterMode(void);          // Enter tester mode for diagnostics
void DisplayMenu(void);              // Display tester mode menu
void ExecuteOption(char option);     // Execute user option in tester mode
void HandleSW2Press(void);           // Handle button press to enter tester mode
void TesterMode(void);               // Run Tester Mode logic

// Task Prototypes
void Task_SendHeartbeat(void);       // Send heartbeat to ECU2
void Task_ReceiveTemperature(void);  // Handle received temperature data
void OS_voidCANHandleReceivedMessages(void); // Process received CAN messages
void CheckOverHeat(void);            // Check for overheat conditions
bool Task_HealthCheckADC(void);      // Perform ADC health check
void FaultState(void);               // Handle fault state logic
void CommLoss(void);                 // Handle communication loss logic
void OS_voidCheckTXOK(void);         // Check CAN TX status and errors


#endif /* OS_H */
