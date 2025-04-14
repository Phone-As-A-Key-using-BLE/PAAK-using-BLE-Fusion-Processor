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

void DeviceStateManager_Init(void);
void DeviceStateManager_LoadAll(void);
bool DeviceStateManager_Update(uint8_t deviceIndex, uint8_t newState);
uint8_t DeviceStateManager_Load(uint8_t deviceIndex);

#endif // DEVICE_RANGING_TYPE_MANAGER_H
