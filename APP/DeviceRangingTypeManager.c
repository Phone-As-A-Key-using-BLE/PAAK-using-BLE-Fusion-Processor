#include "DeviceRangingTypeManager.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"

static uint8_t deviceStates[NUM_DEVICES];

void DeviceStateManager_Init(void) {
    EEPROMInit();
    DeviceStateManager_LoadAll();
}

void DeviceStateManager_LoadAll(void) {
    uint8_t paddedSize = (NUM_DEVICES + 3) & ~0x03;  // Round up to multiple of 4
    EEPROMRead((uint32_t*)deviceStates, EEPROM_BASE_ADDR, paddedSize);
}

uint8_t DeviceStateManager_Load(uint8_t deviceIndex) {
    if (deviceIndex >= NUM_DEVICES) return 0xFF;

    uint32_t word;
    uint32_t wordOffset = deviceIndex / 4;
    uint32_t wordAddr = EEPROM_BASE_ADDR + (wordOffset * 4);

    EEPROMRead(&word, wordAddr, 4);
    return ((uint8_t*)&word)[deviceIndex % 4];
}

bool DeviceStateManager_Update(uint8_t deviceIndex, uint8_t newState) {
    if (deviceIndex >= NUM_DEVICES) return false;

    if (deviceStates[deviceIndex] == newState) {
        return true;  // No need to write if same
    }

    deviceStates[deviceIndex] = newState;

    uint32_t wordOffset = deviceIndex / 4;
    uint32_t wordAddr = EEPROM_BASE_ADDR + (wordOffset * 4);
    uint32_t word;

    EEPROMRead(&word, wordAddr, 4);
    ((uint8_t*)&word)[deviceIndex % 4] = newState;
    EEPROMProgram(&word, wordAddr, 4);

    return true;
}
