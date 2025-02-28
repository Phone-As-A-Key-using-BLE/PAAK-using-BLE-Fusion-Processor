/***********************************************
 * Module Name: NVM Driver
 * Purpose: Provide APIs for Non-Volatile Memory Management
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/
#include "nvm.h"
/***********************************************
 * Local Variables
 ***********************************************/
uint32_t ui32EEPROMInit;
static uint8_t dtcCount = 0;  // Number of stored DTCs
static uint8_t index = 0;

/***********************************************
 * Function Definitions
 ***********************************************/

/***********************************************
 * Function Name: NVM_Init
 * Purpose: Initialize NVM module
 ***********************************************/
void NVM_Init(void) {
    // Initialize EEPROM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)){}
    //
    // Wait for the EEPROM Initialization to complete
    //
    ui32EEPROMInit = EEPROMInit();
    //
    // Check if the EEPROM Initialization returned an error
    // and inform the application
    //
    if(ui32EEPROMInit != EEPROM_INIT_OK)
    {
    while(1)
    {
        LED_ON(LED_BLUE);
    }
    }
}

/***********************************************
 * Function Name: NVM_SaveDTC
 * Purpose: Save a Diagnostic Trouble Code
 ***********************************************/
bool NVM_SaveDTC( uint32_t *dtc,uint32_t address) {
    if (dtcCount >= NVM_MAX_DTC_COUNT) {
        return false;  // Memory is full
    }
    //uint32_t address = NVM_FLASH_START + (dtcCount * 8);  // Each DTC is 32 bytes
    EEPROMProgram((uint32_t *)dtc, address, 8);  // Save string with null terminator
    dtcCount++;
    return true;
}

/***********************************************
 * Function Name: NVM_RetrieveDTC
 * Purpose: Retrieve a stored DTC by index
 ***********************************************/
bool NVM_RetrieveDTC(uint32_t *buffer,uint32_t address) {
    if (index >= dtcCount) {
        return false;  // Invalid index
    }
    //uint32_t address = NVM_FLASH_START + (index * 8);
    EEPROMRead((uint32_t *)buffer, address, 8);  // Read up to 32 bytes
    return true;
}

/***********************************************
 * Function Name: NVM_ClearAllDTCs
 * Purpose: Clear all stored DTCs
 ***********************************************/
bool NVM_ClearAllDTCs(void) {
    EEPROMMassErase();  // Erase all EEPROM data
    dtcCount = 0;
    return true;
}
