/*
 * File: connectivity.h
 * Author: Mohamed Abdel Hamid
           Ahmed Abdelrahman
           Ahmed Helal
           Abdelrahman Hassan
 * Description: Header file for vehicle connectivity communication over UART.
 */

#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H

#include <stdint.h>
#include <stdbool.h>
#include "UART/uart.h"  // UART driver

/******************************************************************************/
/*                               Configuration Macros                         */
/******************************************************************************/

#define UART_BAUD_RATE              9600       // UART baud rate
#define UART_TIMEOUT                1000       // Timeout in milliseconds

#define CONNECTIVITY_START_BYTE     0x0F       // Start byte for UART framing
#define CONNECTIVITY_END_BYTE       0xFF       // End byte for UART framing
#define CONNECTIVITY_ACK_BYTE       0xAA       // Acknowledgement byte
#define MAX_BUFFER_SIZE             190

/******************************************************************************/
/*                                  Enums                                     */
/******************************************************************************/

// Message types for UART communication
typedef enum {
    MSG_TYPE_REQUEST_VERIFIERS,         // Request Verifiers
    MSG_TYPE_PK_VEHICLE_CERTIFICATE,    // Certificate/public key exchange
    MSG_TYPE_RESEND_VERIFIERS,
    MSG_TYPE_RESEND_CERTIFICATE,
    MSG_TYPE_VEHICLE_STATE,             // State update from vehicle
    MSG_TYPE_ALERT,                     // Alert notification
    MSG_TYPE_COMMAND                    // Command execution
} CONNECTIVITY_tenuVehicleMsgType;

// Vehicle alert types
typedef enum {
    ENGINE_OVERHEAT = 0xF0,
    DOOR_UNLOCKED,
    DOOR_LOCKED,
    TIRE_PRESSURE_LOW
} CONNECTIVITY_tenuVehicleAlert;

// Command identifiers for vehicle control
typedef enum {
    CMD_START_ENGINE_BYTE = 0x10,
    CMD_STOP_ENGINE_BYTE,

    CMD_ENABLE_AC_BYTE,
    CMD_DISABLE_AC_BYTE,

    CMD_LOCK_ALL_DOORS_BYTE,
    CMD_UNLOCK_ALL_DOORS_BYTE,
    CMD_LOCK_FRONT_LEFT_DOOR_BYTE,
    CMD_UNLOCK_FRONT_LEFT_DOOR_BYTE,
    CMD_LOCK_FRONT_RIGHT_DOOR_BYTE,
    CMD_UNLOCK_FRONT_RIGHT_DOOR_BYTE,
    CMD_LOCK_REAR_LEFT_DOOR_BYTE,
    CMD_UNLOCK_REAR_LEFT_DOOR_BYTE,
    CMD_LOCK_REAR_RIGHT_DOOR_BYTE,
    CMD_UNLOCK_REAR_RIGHT_DOOR_BYTE,

    CMD_LOCK_BONNET_BYTE,
    CMD_UNLOCK_BONNET_BYTE,
    CMD_LOCK_TRUNK_BYTE,
    CMD_UNLOCK_TRUNK_BYTE,

    CMD_ENABLE_IMMOBILIZER_BYTE,
    CMD_DISABLE_IMMOBILIZER_BYTE,

    CMD_TERMINATE_USER_BYTE
} CONNECTIVITY_tenuVehicleCommand;

/******************************************************************************/
/*                                  Structs                                   */
/******************************************************************************/

// Main UART message structure
typedef struct {
    CONNECTIVITY_tenuVehicleMsgType msg_type;    // Type of message
    uint8_t data[MAX_BUFFER_SIZE];               // Optional data buffer
    uint16_t data_len;                           // Length of valid data
} CONNECTIVITY_tstructMsgHandle;

/******************************************************************************/
/*                              Function Prototypes                           */
/******************************************************************************/
void Connectivity_voidRequestVerifiers(void);
void Connectivity_voidRequestCertificate(void);
#endif /* CONNECTIVITY_H */
