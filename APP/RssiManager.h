#ifndef EEPROM_DRIVER_H
#define EEPROM_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "APP/CAN_MasterReceive.h"

#define RSSI_EEPROM_START_ADDR    0x1000  // Starting from 4KB offset
#define RSSI_EEPROM_SIZE          0x100   // 256 bytes for RSSI data
#define EEPROM_BLOCK_SIZE         16      // Size in bytes for each anchor's data
#define MAX_ANCHOR_NUMBER         3

// Data offset definitions within each block
#define OFFSET_RSSI           0
#define OFFSET_CONFIDENCE     1
#define OFFSET_ACCURACY       2
#define OFFSET_DISTANCE      3
#define OFFSET_ANCHOR_ID     5

#define RSSI_EEPROM_END_ADDR (RSSI_EEPROM_START_ADDR + RSSI_EEPROM_SIZE)
// Error codes
typedef enum {
    EEPROM_OK = 0,
    EEPROM_INIT_ERROR,
    EEPROM_WRITE_ERROR,
    EEPROM_READ_ERROR,
    EEPROM_INVALID_ADDR,
    EEPROM_OUT_OF_BOUNDS
} EEPROM_Status_t;


// Function prototypes
EEPROM_Status_t RSSI_EEPROM_Init(void);
EEPROM_Status_t RSSI_EEPROM_WriteRSSIData(uint8_t anchorNumber, RSSIData_t* data);
EEPROM_Status_t RSSI_EEPROM_ReadRSSIData(uint8_t anchorNumber, RSSIData_t* data);
EEPROM_Status_t RSSI_EEPROM_EraseAll(void);
bool RSSI_EEPROM_IsBusy(void);
bool isValidAddress(uint32_t address, uint32_t size);
static bool isValidAnchorNumber(uint8_t anchorNumber);
uint32_t calculateAddress(uint8_t anchorNumber, uint32_t offset);

#endif /* EEPROM_DRIVER_H */
