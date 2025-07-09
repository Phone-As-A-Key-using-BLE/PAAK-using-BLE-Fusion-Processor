/*
 * DeviceRangingManager.h
 *
 *  Created on: Apr 14, 2025
 *      Author: Mohamed Abdel Hamid
 */

#ifndef DEVICE_RANGING_TYPE_MANAGER_H
#define DEVICE_RANGING_TYPE_MANAGER_H

#include <stdint.h>
#include <stdbool.h>
#include "APP_FSM.h"
#define NUM_DEVICES        APP_MAX_NO_OF_DEVICES
#define EEPROM_BASE_ADDR   0x00

// -----------------------------------------------------------------------------
// Configuration for Owner Public Key storage
// -----------------------------------------------------------------------------
#define OWNER_PUBLIC_KEY_SIZE           65
#define OWNER_PUBLIC_KEY_PADDED_SIZE   ((OWNER_PUBLIC_KEY_SIZE + 3) & ~0x03)  
// Place the key right after your deviceStates block in EEPROM:
#define OWNER_PUBKEY_EEPROM_ADDR       (EEPROM_BASE_ADDR + (((NUM_DEVICES + 3) & ~0x03)))

void DeviceStateManager_Init(void);
void DeviceStateManager_LoadAll(void);
bool DeviceStateManager_Update(uint8_t deviceIndex, uint8_t newState);
uint8_t DeviceStateManager_Load(uint8_t deviceIndex);
bool DeviceStateManager_StoreOwnerPk(const uint8_t *pubKey);
bool DeviceStateManager_LoadOwnerPk(uint8_t *pubKeyOut);

#endif // DEVICE_RANGING_TYPE_MANAGER_H
