#include "aPP/RssiManager.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "APP/CAN_App.h"

// Storage management structure
static struct {
    uint32_t currentAddress[MAX_ANCHOR_NUMBER];
    bool isInitialized;
} storageManager = {
    .currentAddress = {0},
    .isInitialized = false
};

/**
 * @brief Initialize storage manager
 */
static void initStorageManager(void)
{
    // Initialize addresses for each anchor
    uint8_t i;
    for( i = 0; i < CAN_ANCHOR_MAX; i++)
    {
        storageManager.currentAddress[i] = RSSI_EEPROM_START_ADDR + (i * EEPROM_BLOCK_SIZE);
    }
    storageManager.isInitialized = true;
}

/**
 * @brief Initialize EEPROM for RSSI data
 * @return EEPROM_Status_t Status of initialization
 */

/**
 * @brief Write RSSI data for a specific anchor to EEPROM
 * @param anchorNumber Anchor number (1-3)
 * @param data Pointer to RSSI data structure
 * @return EEPROM_Status_t Status of write operation
 */
EEPROM_Status_t RSSI_EEPROM_WriteRSSIData(uint8_t anchorNumber, RSSIData_t* data)
{
    if(!isValidAnchorNumber(anchorNumber) || data == NULL)
    {
        return EEPROM_INVALID_ADDR;
    }
    
    // Get the current write address for this anchor
    uint32_t writeAddr = storageManager.currentAddress[anchorNumber - 1];
    
    // Verify address is within our allocated region
    if(!isValidAddress(writeAddr, EEPROM_BLOCK_SIZE))
    {
        return EEPROM_OUT_OF_BOUNDS;
    }
    
    // Write each field to EEPROM
    if(EEPROMProgram((uint32_t*)&data->averageRssi, 
                     writeAddr + OFFSET_RSSI, 1) != 0)
    {
        return EEPROM_WRITE_ERROR;
    }
    
    if(EEPROMProgram((uint32_t*)&data->confidence, 
                     writeAddr + OFFSET_CONFIDENCE, 1) != 0)
    {
        return EEPROM_WRITE_ERROR;
    }
    
    if(EEPROMProgram((uint32_t*)&data->accuracy, 
                     writeAddr + OFFSET_ACCURACY, 1) != 0)
    {
        return EEPROM_WRITE_ERROR;
    }
    
    if(EEPROMProgram((uint32_t*)&data->distance, 
                     writeAddr + OFFSET_DISTANCE, 2) != 0)
    {
        return EEPROM_WRITE_ERROR;
    }
    
    if(EEPROMProgram((uint32_t*)&data->anchorId, 
                     writeAddr + OFFSET_ANCHOR_ID, 1) != 0)
    {
        return EEPROM_WRITE_ERROR;
    }
    
    return EEPROM_OK;
}

/**
 * @brief Read RSSI data for a specific anchor from EEPROM
 * @param anchorNumber Anchor number (1-3)
 * @param data Pointer to store RSSI data
 * @return EEPROM_Status_t Status of read operation
 */
EEPROM_Status_t RSSI_EEPROM_ReadRSSIData(uint8_t anchorNumber, RSSIData_t* data)
{
    if(!isValidAnchorNumber(anchorNumber) || data == NULL)
    {
        return EEPROM_INVALID_ADDR;
    }
    
    // Get the current read address for this anchor
    uint32_t readAddr = storageManager.currentAddress[anchorNumber - 1];
    
    if(!isValidAddress(readAddr, EEPROM_BLOCK_SIZE))
    {
        return EEPROM_OUT_OF_BOUNDS;
    }
    
    // Read each field from EEPROM
    EEPROMRead((uint32_t*)&data->averageRssi, 
               readAddr + OFFSET_RSSI, 1);
    
    EEPROMRead((uint32_t*)&data->confidence, 
               readAddr + OFFSET_CONFIDENCE, 1);
    
    EEPROMRead((uint32_t*)&data->accuracy, 
               readAddr + OFFSET_ACCURACY, 1);
    
    EEPROMRead((uint32_t*)&data->distance, 
               readAddr + OFFSET_DISTANCE, 2);
    
    EEPROMRead((uint32_t*)&data->anchorId, 
               readAddr + OFFSET_ANCHOR_ID, 1);
    
    return EEPROM_OK;
}

/**
 * @brief Erase all RSSI data from EEPROM
 * @return EEPROM_Status_t Status of erase operation
 */
EEPROM_Status_t RSSI_EEPROM_EraseAll(void)
{
    // Calculate number of words to erase
    uint32_t wordCount = RSSI_EEPROM_SIZE / 4;
    uint32_t zeroData = 0;
    
    // Erase each word in our region
    uint32_t i;
    for(i = 0; i < wordCount; i++)
    {
        uint32_t address = RSSI_EEPROM_START_ADDR + (i * 4);
        if(EEPROMProgram(&zeroData, address, 1) != 0)
        {
            return EEPROM_WRITE_ERROR;
        }
    }
    
    // Reset storage manager
    initStorageManager();
    
    return EEPROM_OK;
}

/**
 * @brief Check if EEPROM is busy
 * @return bool True if busy, false if ready
 */
bool RSSI_EEPROM_IsBusy(void)
{
    return !SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0);
}

/**
 * @brief Check if address is within valid EEPROM range
 * @param address Address to check
 * @param size Size of data to be written/read
 * @return bool True if valid, false if out of bounds
 */
bool isValidAddress(uint32_t address, uint32_t size)
{
    // Check if address is within our allocated region
    if(address < RSSI_EEPROM_START_ADDR)
        return false;
        
    // Check if the end of the data would exceed our allocated region
    if((address + size) > RSSI_EEPROM_END_ADDR)
        return false;
        
    // Check if address is word-aligned
    if(address & 0x03)
        return false;
        
    return true;
}

/**
 * @brief Validate anchor number
 * @param anchorNumber Anchor number to validate
 * @return bool True if valid, false if invalid
 */
static bool isValidAnchorNumber(uint8_t anchorNumber)
{
    return (anchorNumber >= 1 && anchorNumber <= MAX_ANCHOR_NUMBER);
}