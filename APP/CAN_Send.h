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

#include "APP/Connectivity/connectivity.h"
#include "APP/APP_FSM.h"
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


#define VERIFIER_TOTAL_SIZE_BYTES     117
#define VERIFIER_FRAME_COUNT          17
#define VERIFIER_FRAME_DATA_SIZE      7
#define VERIFIER_LAST_FRAME_DATA_SIZE 5

/* Function Prototypes */

/**
 * @brief Sends commands over CAN bus, including receiver identifier.
 * 
 * @param Copy_enuCommand Command ID.
 * @param Copy_u8ReceiverId Receiver Identifier.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8Data);

void CAN_voidSendHandoverCommand(uint8_t Copy_u8ReceiverId, uint8_t Copy_u8HandoverTo,uint8_t Copy_u8deviceId);

/**
 * @brief Sends ranging type of devices over CAN bus.
 * 
 * @param Add_enuRangingType Array of ranging types of devices
 */
void CAN_voidSendRangingType(APP_tenuRangingType* Add_enuRangingType);

void CAN_voidSendVerifiers();

void CAN_voidSendPkCertificate();
void CAN_voidSendNextVerifierFrame(void);
#endif /* CAN_SEND_H_ */
