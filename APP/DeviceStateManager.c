#include "DeviceStateManager.h"
#include "driverlib/eeprom.h"
#include "driverlib/sysctl.h"

static uint8_t deviceStates[NUM_DEVICES];

void DeviceStateManager_Init(void) {
    SysCtlPeripheralEnable(SYSCTL_PERIPH_EEPROM0);
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_EEPROM0)){}
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

// -----------------------------------------------------------------------------
// Write the 65-byte public key to EEPROM.
// Returns true on success.
// -----------------------------------------------------------------------------
bool DeviceStateManager_StoreOwnerPk(const uint8_t *pubKey)
{
    uint8_t buffer[OWNER_PUBLIC_KEY_PADDED_SIZE] = {0};

    // Copy the 65 bytes; the rest remain 0
    memcpy(buffer, pubKey, OWNER_PUBLIC_KEY_SIZE);

    // Program the padded block (must be word‑aligned and a multiple of 4 bytes)
    EEPROMProgram((uint32_t*)buffer, OWNER_PUBKEY_EEPROM_ADDR, OWNER_PUBLIC_KEY_PADDED_SIZE);

    // You could read back and verify here if you want extra safety.
    return true;
}

// -----------------------------------------------------------------------------
// Read the 65-byte public key from EEPROM into pubKeyOut.
// pubKeyOut must have room for at least 65 bytes.
// -----------------------------------------------------------------------------
bool DeviceStateManager_LoadOwnerPk(uint8_t *pubKeyOut)
{
    uint8_t buffer[OWNER_PUBLIC_KEY_PADDED_SIZE];

    // Read the entire padded block
    EEPROMRead((uint32_t*)buffer, OWNER_PUBKEY_EEPROM_ADDR, OWNER_PUBLIC_KEY_PADDED_SIZE);

    // Copy out only the original 65 bytes
    memcpy(pubKeyOut, buffer, OWNER_PUBLIC_KEY_SIZE);

    return true;
}
