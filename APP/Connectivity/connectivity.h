/*

Author: ABDELRAHMAN

 */

#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H


#include "UART/uart.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

// UART Configuration
#define UART_BAUD_RATE 9600
#define SERIAL_MONITOR_BAUD_RATE 115200
#define UART_TIMEOUT 1000  // Timeout for UART communication in milliseconds

// Message Types
#define MSG_TYPE_VEHICLE_STATE_BYTE 0x01
#define MSG_TYPE_ALERT_BYTE 0x03
#define MSG_TYPE_CERTIFICATES_BYTE 0x04
#define MSG_TYPE_REGISTRATION_BYTE 0x05
#define MSG_TYPE_COMMAND_BYTE 0x06

// UART Start and End Bytes
#define UART_START_BYTE 0x0F
#define UART_END_BYTE 0xFF
#define UART_ACK_BYTE 0xAA
#define MAX_BUFFER_SIZE 256


/***********************************************
  Enums
 ***********************************************/
typedef enum
{
    MSG_TYPE_VEHICLE_STATE = 0x01,  // Vehicle state message
    MSG_TYPE_ALERT = 0x03,          // Alert message
    MSG_TYPE_CERTIFICATES = 0x04,   // Certificates message
    MSG_TYPE_REGISTRATION = 0x05,   // Registration message
    MSG_TYPE_COMMAND = 0x06         // Command message
} VehicleMessageType_t;

typedef enum
{
    CMD_START_ENGINE_BYTE = 0x10,
    CMD_STOP_ENGINE_BYTE = 0x11,

    CMD_ENABLE_AC_BYTE = 0x12,
    CMD_DISABLE_AC_BYTE = 0x13,

    CMD_LOCK_ALL_DOORS_BYTE = 0x14,
    CMD_UNLOCK_ALL_DOORS_BYTE = 0x15,
    CMD_LOCK_FRONT_LEFT_DOOR_BYTE = 0x16,
    CMD_UNLOCK_FRONT_LEFT_DOOR_BYTE = 0x17,
    CMD_LOCK_FRONT_RIGHT_DOOR_BYTE = 0x18,
    CMD_UNLOCK_FRONT_RIGHT_DOOR_BYTE = 0x19,
    CMD_LOCK_REAR_LEFT_DOOR_BYTE = 0x1A,
    CMD_UNLOCK_REAR_LEFT_DOOR_BYTE = 0x1B,
    CMD_LOCK_REAR_RIGHT_DOOR_BYTE = 0x1C,
    CMD_UNLOCK_REAR_RIGHT_DOOR_BYTE = 0x1D,

    CMD_LOCK_BONNET_BYTE = 0x1E,
    CMD_UNLOCK_BONNET_BYTE = 0x1F,
    CMD_LOCK_TRUNK_BYTE = 0x20,
    CMD_UNLOCK_TRUNK_BYTE = 0x21,

    CMD_ENABLE_IMMOBILIZER_BYTE = 0x22,
    CMD_DISABLE_IMMOBILIZER_BYTE = 0x23,

    CMD_TERMINATE_USER_BYTE = 0x30

} VehicleCommand_t;

// Vehicle status structure
typedef struct VehicleStates_t {
    bool engineState;
    uint8_t batteryLevel;
    bool doorsLocked;
    bool acState;
    uint8_t tirePSI;
} VehicleStates_t;


// Vehicle alert enum
typedef enum VehicleAlert_t
{
    ENGINE_OVERHEAT = 0xF0,
    DOOR_UNLOCKED = 0xF1,
    DOOR_LOCKED = 0XF2,
    TIRE_PRESSURE_LOW = 0XF3
} VehicleAlert_t;


//typedef struct {
//    uint32_t issueDate;            // Issue date of the certificate
//    uint32_t expirationDate;       // Expiration date of the certificate
//    uint16_t modelYear;            // Year of manufacture
//    uint8_t certificateID;
//    uint8_t vehicleMake[8];          // Vehicle make (e.g., Tesla, BMW)
//    uint8_t vehicleModel[16];         // Vehicle model (e.g., Model S, X5)
//    uint8_t publicKey[16];        // Public key for encryption
//    uint8_t signature[16];        // Digital signature for certificate verification
//} VehicleCertificate_t;


// UART communication message structure
typedef struct {
    VehicleMessageType_t msg_type;
    VehicleCommand_t command;               // Command type (e.g., send certificate, status, etc.)
    VehicleStates_t states;
    VehicleAlert_t alert;
}CONNECTIVITY_Message_t ;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/***********************************************
  Function Prototypes
 ***********************************************/
void CONNECTIVITY_SendHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg);
CONNECTIVITY_Message_t CONNECTIVITY_ReceiveData();
void CONNECTIVITY_SendData(CONNECTIVITY_Message_t *message);
void CONNECTIVITY_ReceiveHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg);

#endif /* CONNECTIVITY_H */
