/*
 * can_msg_types.h
 *
 *  Created on: Mar 26, 2024
 *      Author: Ali and Hamid
 */

#ifndef CAN_MSG_TYPES_H_

#define CAN_MSG_TYPES_H_

/*! *********************************************************************************
*************************************************************************************
* Include
*************************************************************************************
********************************************************************************** */
#include "CAN_App.h"

#if (CAN_ANCHOR_ID == CAN_MASTER_NODE)
/*! Bluetooth Identity Address - array of 6 bytes. */
typedef struct
{
    uint8_t idAddressType; /*!< Public or Random (static). */
    uint8_t idAddress[6];  /*!< 6-byte address. */
} bleIdentityAddress_t;

/*! Structure containing the SMP information exchanged during pairing. */
typedef struct
{
    uint8_t cLtkSize; /*!< Encryption Key Size. If aLtk is NULL, this is ignored. */
    uint8_t *aLtk;    /*!< Long Term (Encryption) Key. NULL if LTK is not distributed, else size is given by cLtkSize.*/

    uint8_t *aIrk;  /*!< Identity Resolving Key. NULL if aIrk is not distributed. */
    uint8_t *aCsrk; /*!< Connection Signature Resolving Key. NULL if aCsrk is not distributed. */

    uint8_t cRandSize; /*!< Size of RAND; usually equal to gcMaxRandSize_d. If aLtk is NULL, this is ignored. */
    uint8_t *aRand;    /*!< RAND value used to identify the LTK. If aLtk is NULL, this is ignored. */
    uint16_t ediv;     /*!< EDIV value used to identify the LTK. If aLtk is NULL, this is ignored. */

    uint8_t addressType; /*!< Public or Random address. If aAddress is NULL, this is ignored. */
    uint8_t *aAddress;   /*!< Device Address. NULL if address is not distributed. If aIrk is NULL, this is ignored. */
} gapSmpKeys_t;

/*! Identity Resolving Key size in bytes */
#define gcSmpIrkSize_c (16U)
/*! Maximum Long Term Key size in bytes */
#define gcSmpMaxLtkSize_c (16U)
/*! Size of a BLE Device Address */
#define gcBleDeviceAddressSize_c (6U)

#else
#include "ble_general.h"
#include "gap_types.h"

extern volatile bool txComplete;
extern volatile bool rxComplete;
extern volatile bool wakenUp;

extern bool isAdvertising;

#endif

/*! *********************************************************************************
*************************************************************************************
* Public type definitions
*************************************************************************************
********************************************************************************** */

typedef enum
{
    CAN_COMMAND_TRIGGER_OWNER_PAIRING,
    CAN_COMMAND_TRIGGER_PASSIVE_ENTRY,
    CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT,
    CAN_COMMAND_STOP_DISTANCE_MEASURMENT,
    CAN_COMMAND_RESET,
    CAN_COMMAND_INVALID
} CAN_tenumCommands;


typedef enum
{
    CAN_PE_SUCCESS,
    CAN_PE_FAILED
} CAN_tenumPeResponse;


typedef struct object_list
{
    uint8_t deviceId : 4;   /* Maximum 16 devices to be registered for one car */
    uint32_t cycleId : 25;  /* Maximum cycle Id is 33554432 then to be reset */
    uint8_t angle;          /* Assumption that the angle will be Natural number in degrees (45�)*/
    uint16_t distance : 11; /* Maximum 2048 cm ~ 20 m that the BLE can reach (to be checked again in the open space)*/
    uint8_t accuracy : 2;   /* Maximum ( + | -) 4 meter  error of accuracy */
    uint8_t confidence : 4; /* Range of confidence from 0 (Low Confidence) -> 16 (High Confidence) */
    uint16_t mesgId;
} object_list;

typedef struct add_device
{
    uint8_t nvmIndex : 4; /* nvmIndex is the deviceId */
    bleIdentityAddress_t bleDeviceAddress_t;
    uint8_t aLtk[16];
    uint8_t aIrk[16];
    bool gAppOutLeSc : 1;
    bool gAppOutAuth : 1;
} add_device;

typedef struct remove_device
{
    bleIdentityAddress_t bleDeviceAddress_t;
    uint8_t nvmIndex : 4; /* nvmIndex is the deviceId */
} remove_device_t;

#endif /* CAN_MSG_TYPES_H_ */
