/********************************************************************/
/* Author   : Mohamed Abdel Hamid                                   */
/* Date     : 26 / 2 / 2025                                         */
/* Email    : mohamedhamiid20@gmail.com                             */
/* Phone    : 01092301921                                           */
/* Brief    : Handling sending data through CAN.                    */
/* Copyright: Sponsored by Ejad                                     */
/********************************************************************/

#ifndef CAN_SEND_H_
#define CAN_SEND_H_

/* Helper Macros */

/** 
 * @brief Gets the next anchor ID in a cyclic manner.
 * 
 * @param current The current anchor ID.
 * @return The next anchor ID.
 */
#define CAN_GET_NEXT_ANCHOR(current)    ((current + 1) % CAN_ANCHOR_MAX)

/** 
 * @brief Gets the previous anchor ID in a cyclic manner.
 * 
 * @param current The current anchor ID.
 * @return The previous anchor ID.
 */
#define CAN_GET_PREV_ANCHOR(current)    ((current + CAN_NO_OF_ANCHORS - 1) % CAN_ANCHOR_MAX)

/** 
 * @brief Checks if the given anchor ID is valid.
 * 
 * @param id The anchor ID to check.
 * @return Boolean indicating whether the ID is valid.
 */
#define CAN_IS_VALID_ANCHOR(id)         ((id) < CAN_ANCHOR_MAX)


/* Function Prototypes */


#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)

#include "app_localization.h"

/* CAN_voidSendDistance
 * @brief Sends distance measurement data over CAN bus.
 *
 * @param Copy_u8DeviceId Device ID.
 * @param Copy_u16ProcNo Process number.
 * @param Copy_structResults Struct containing localization algorithm results.
 * @param Copy_u8PEorTDM flag to determine either the distance is from Passive Entry or Trigger Distance Measurement command
 */
void CAN_voidSendDistance(uint8_t Copy_u8DeviceId, uint16_t Copy_u16ProcNo, localizationAlgoRun_t Copy_structResults, uint8_t Copy_u8PEorTDM);

/* CAN_voidNotifyWakeup
 * @brief Sends wake up notification over CAN bus.
 */
void CAN_voidNotifyWakeup();

#if (CAN_ENABLE_HANDOVER)
/**
 * @brief Sends a handover message with the next anchor point for the device.
 * 
 * @param Copy_u8DeviceId ID of the device undergoing handover.
 */
void CAN_voidHandover(uint8_t Copy_u8DeviceId);
#endif

/**
 * @brief Sends bonding data over CAN bus, including device information and security keys.
 * 
 * @param Copy_u8NvmId Non-volatile memory ID of the bonded device.
 * @param gAppOutAuth Authentication flag.
 * @param gAppOutLeSc LE Secure Connections flag.
 * @param Add_structKeys Structure containing security keys.
 */
void CAN_voidSendBondingData(uint8_t Copy_u8NvmId, bool gAppOutAuth, bool gAppOutLeSc, gapSmpKeys_t Add_structKeys);

/**
 * @brief Sends passive entry response over CAN bus.
 *
 * @param Copy_enuResponse Respone either success or fail.
 * @param Copy_u8ReceiverId Receiver Identifier.
 * @param Copy_u8DeviceId Device ID.
 */
void CAN_voidSendPeResponse(CAN_tenumPeResponse Copy_enuResponse, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8DeviceId);

#else

/**
 * @brief Sends commands over CAN bus, including receiver identifier.
 * 
 * @param Copy_enuCommand Command ID.
 * @param Copy_u8ReceiverId Receiver Identifier.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8Data);

#endif

#endif /* CAN_SEND_H_ */
