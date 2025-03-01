/********************************************************************/
/* Author   : Mohamed Abdel Hamid                                   */
/* Date     : 26 / 2 / 2025                                     */
/* Email    : mohamedhamiid20@gmail.com                             */
/* Phone    : 01092301921                                           */
/* Brief    : Handling sending data through CAN.                    */
/* Copyright: Sponsored by Ejad                                     */
/********************************************************************/

#ifndef CAN_SEND_H_
#define CAN_SEND_H_

/* Definitions */
typedef enum{
    PE,
    TDM
}CAN_tenuDistanceType;


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
/**
 * @brief Sends distance measurement data over CAN bus.
 * 
 * @param deviceId Device ID.
 * @param procNo Process number.
 * @param result Struct containing localization algorithm results.
 * @param Copy_u8PEorTDM The distance is trigger by Passive Entry or Trigger Distance Measurement
 */
void CAN_voidSendDistance(uint8_t deviceId, uint16_t procNo, localizationAlgoRun_t result, uint8_t Copy_u8PEorTDM);
#endif

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
 * @brief Sends commands over CAN bus, including receiver identifier.
 * 
 * @param Copy_enuCommand Command ID.
 * @param Copy_u8ReceiverId Receiver Identifier.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8DeviceId);
#endif /* CAN_SEND_H_ */
