/*

Author: ABDELRAHMAN

*/

#ifndef CONNECTIVITY_H
#define CONNECTIVITY_H


#include "UART/uart.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define UART_START_BYTE      0x0F
#define UART_END_BYTE        0xFF
#define UART_ACK             0xAA
#define BUFFER_SIZE          256


/***********************************************
  Enums
 ***********************************************/

typedef enum
{
    VEHICLE_SEND_CERTIFICATE      = 0x11, // Vehicle sends its certificate
    VEHICLE_REQUEST_CERTIFICATE   = 0x12, // Vehicle requests a certificate
    VEHICLE_RECEIVE_CERTIFICATE   = 0x13, // Vehicle receives a certificate

    VEHICLE_SEND_STATUS           = 0x22, // Vehicle sends its current status
    VEHICLE_REQUEST_STATUS        = 0x23, // Vehicle requests status from another system
    VEHICLE_RECEIVE_STATUS        = 0x24, // Vehicle receives a status update

    VEHICLE_SEND_COMMAND          = 0x31, // Vehicle sends a control command
    VEHICLE_RECEIVE_COMMAND       = 0x32, // Vehicle receives a control command

    VEHICLE_SEND_DATA             = 0x41, // Vehicle sends general data (e.g., logs, diagnostics)
    VEHICLE_REQUEST_DATA          = 0x42, // Vehicle requests specific data
    VEHICLE_RECEIVE_DATA          = 0x43, // Vehicle receives general data

    VEHICLE_INITIATE_COMMUNICATION = 0x51, // Vehicle starts communication
    VEHICLE_TERMINATE_COMMUNICATION = 0x52 // Vehicle ends communication
} VehicleCommand_t;


// Vehicle status structure
typedef struct {
    uint8_t battery_level;
    int8_t  temperatrue;
    uint8_t lock;
    uint8_t active_users;
    uint8_t tire_pressure;
}VehicleStatus_t ;


typedef struct {
    uint32_t issueDate;            // Issue date of the certificate
    uint32_t expirationDate;       // Expiration date of the certificate
    uint16_t modelYear;            // Year of manufacture
    uint8_t certificateID;
    uint8_t vehicleMake[8];          // Vehicle make (e.g., Tesla, BMW)
    uint8_t vehicleModel[16];         // Vehicle model (e.g., Model S, X5)
    uint8_t publicKey[16];        // Public key for encryption
    uint8_t signature[16];        // Digital signature for certificate verification
} VehicleCertificate_t;

// UART communication message structure
typedef struct {
    VehicleCertificate_t certificate;
    VehicleCommand_t command;               // Command type (e.g., send certificate, status, etc.)
    VehicleStatus_t status;
}CONNECTIVITY_Message_t ;

/*******************************************************************************
 * Variables
 ******************************************************************************/

/***********************************************
  Function Prototypes
 ***********************************************/
void CONNECTIVITY_SendHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg);
void CONNECTIVITY_ReceiveData();
void CONNECTIVITY_SendData(CONNECTIVITY_Message_t *message);

#endif /* CONNECTIVITY_H */
