/*
 * APP_SYS.h
 *
 *  Created on: june 11, 2025
 *      Author: Mohamed Abdel Hamid
 */
#ifndef VEHICLE_H_
#define VEHICLE_H_

#include <stdint.h>
#include "APP_FSM.h"

// Real-time vehicle state structure
typedef struct {
    uint8_t engineState : 1;         // true: ON, false: OFF
    uint8_t doorsLocked : 1;         // true if all doors locked
    uint8_t acState     : 1;         // true: AC ON, false: AC OFF
    uint8_t batteryLevel;            // Battery level percentage
    uint8_t tirePSI;                 // Average tire pressure
} Vehicle_tstructVehicleStates;

typedef struct {
    uint8_t p;
    uint8_t r;
    uint8_t n[2];
    uint8_t salt[16];
    uint8_t w0[32];
    uint8_t L[65];
} Vehicle_tstructVerifiers;

typedef struct {
    const char * vehiclePublicKey;
    const uint8_t * vehiclePrivateKey;
    uint8_t vehiclePublicKeyCeritificate [APP_VEHICLE_PK_CERTIFICATE_SIZE];
    Vehicle_tstructVerifiers verifiers;
    Vehicle_tstructVehicleStates states;
}Vehicle_tstructVehicleInfo;

extern Vehicle_tstructVehicleInfo Global_u8VehicleInfo;

#endif
