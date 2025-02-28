/***********************************************
 * Configuration File: NVM Driver
 * Purpose: Configure NVM parameters
 ***********************************************/

#ifndef NVM_CFG_H
#define NVM_CFG_H

/***********************************************
 * NVM Storage Configuration
 ***********************************************/
#define NVM_DTC_SIZE         32     // Maximum size of a single DTC in bytes
#define NVM_FLASH_START_ADDR 0x20000  // Start address of EEPROM storage
#define NVM_MAX_DTC          10     // Maximum number of stored DTCs

#endif /* NVM_CFG_H */
