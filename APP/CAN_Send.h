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



/*==============================*
 *     Function Declarations    *
 *==============================*/

/**
 * @brief Sends bonding data over CAN bus, including device info and security keys.
 *
 * @param Copy_u8NvmId Non-volatile memory ID of the bonded device.
 * @param gAppOutAuth Authentication flag.
 * @param gAppOutLeSc LE Secure Connections flag.
 * @param Add_structKeys Structure containing security keys.
 */
void CAN_voidSendBondingData(uint8_t Copy_u8NvmId, bool gAppOutAuth, bool gAppOutLeSc, gapSmpKeys_t Add_structKeys);

/**
 * @brief Sends command messages over CAN to a receiver.
 *
 * @param Copy_enuCommand Command type to send.
 * @param Copy_u8ReceiverId ID of the receiving device.
 * @param Copy_u8Data Device or anchor ID.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8Data);

/**
 * @brief Sends a handover command specifying source and destination anchors.
 *
 * @param Copy_u8ReceiverId Anchor currently in control.
 * @param Copy_u8HandoverTo Anchor to hand over to.
 * @param Copy_u8deviceId ID of the device being handed over.
 */
void CAN_voidSendHandoverCommand(uint8_t Copy_u8ReceiverId, uint8_t Copy_u8HandoverTo, uint8_t Copy_u8deviceId);

/**
 * @brief Sends a certificate over CAN in header + data frame format.
 *
 * @param Copy_u8Device Target device ID.
 * @param Copy_u8CertificateType Type of certificate being sent.
 * @param Copy_u16Size Size of certificate data in bytes.
 * @param Add_u8Certificate Pointer to certificate data.
 */
void CAN_voidSendCertificate(uint8_t Copy_u8Device, uint8_t Copy_u8CertificateType, uint16_t Copy_u16Size, uint8_t * Add_u8Certificate);

/**
 * @brief Starts sending verifier values over CAN using indexed frames.
 */
void CAN_voidSendVerifiers(void);

/**
 * @brief Sends the next frame of verifier data.
 */
void CAN_voidSendNextVerifierFrame(void);

/**
 * @brief Sends the public key certificate in 16 indexed CAN frames.
 */
void CAN_voidSendPkCertificate(void);

/**
 * @brief Starts sending the 65-byte Owner Public Key over CAN in 9 frames.
 */
void CAN_voidStartFriendSharing(void);
#endif /* CAN_SEND_H_ */
