/***********************************************
 * Module Name: NVM Driver
 * Purpose: Provide APIs for Non-Volatile Memory Management
 ***********************************************/

#ifndef NVM_H
#define NVM_H

/***********************************************
 * Includes
 ***********************************************/
#include <stdint.h>
#include <stdbool.h>

#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include <string.h>
#include "HAL/LED/led.h"
/***********************************************
 * DTC Storage Configuration
 ***********************************************/
#define NVM_MAX_DTC_COUNT  10  // Maximum number of stored DTCs
#define NVM_FLASH_START    0x20000  // Starting address for DTC storage in Flash

/***********************************************
 * Function Prototypes
 ***********************************************/
void NVM_Init(void);                   // Initialize NVM module
bool NVM_SaveDTC( uint32_t *dtc,uint32_t address );     // Save a Diagnostic Trouble Code
bool NVM_RetrieveDTC(uint32_t *buffer,uint32_t address ); // Retrieve a stored DTC by index
bool NVM_ClearAllDTCs(void);           // Clear all stored DTCs

#endif /* NVM_H */
