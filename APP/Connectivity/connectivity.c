/*
 * File: connectivity.c
 * Authors: Mohamed Abdel Hamid, Ahmed Abdelrahman, Ahmed Helal, Abdelrahman Hassan
 * Description: Source file for vehicle connectivity communication over UART.
 */

#include "connectivity.h"
#include <string.h>
#include "APP/can_msg_types.h"
#include "APP/APP_FSM.h"
#include "APP/CAN_Send.h"
#include "vehicle.h"
/*==================== GLOBAL VARIABLES ====================*/

CONNECTIVITY_tstructMsgHandle g_receivedMessage;
extern uint8_t Global_u8CurrentDeviceId;
extern char gArr_DebugMsg[1024];

/*==================== PRIVATE FUNCTION DECLARATIONS ====================*/
/**
 * @brief Sends a full vehicle message over UART
 * @param msg Pointer to the message structure to send
 */
static void CONNECTIVITY_voidSendHandleMessage(CONNECTIVITY_tstructMsgHandle *msg);

/**
 * @brief Sends raw UART data from a message structure
 * @param message Pointer to message structure
 */
static void CONNECTIVITY_voidSendData(CONNECTIVITY_tstructMsgHandle *message);

/**
 * @brief Handles a received message and takes appropriate action
 * @param msg Pointer to the received message
 */
static void CONNECTIVITY_voidReceiveHandleMessage(CONNECTIVITY_tstructMsgHandle *msg);

/**
 * @brief User-defined callback to process a message
 * @param msg Pointer to received message
 */
static void CONNECTIVITY_voidCallback(CONNECTIVITY_tstructMsgHandle *msg);

/*==================== PRIVATE FUNCTION IMPLEMENTATIONS ====================*/

void UART1_Handler(void) {
    static uint16_t uart_rx_index = 0;
    static bool startDetected = false;
    uint8_t error = 0;
    uint32_t status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, status);

    if (status & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(UART1_BASE)) {
            uint8_t recChar = (uint8_t)UARTCharGetNonBlocking(UART1_BASE);

            if (!startDetected) {
                if (recChar == CONNECTIVITY_START_BYTE) {
                    startDetected = true;
                    uart_rx_index = 0;
                }
                continue;  // Skip everything until start byte is seen
            }

            if (recChar == CONNECTIVITY_END_BYTE) {
                g_receivedMessage.data_len = uart_rx_index - 1; // Exclude msg type
                if(g_receivedMessage.data[0] == MSG_TYPE_REQUEST_VERIFIERS){
                    if(g_receivedMessage.data_len == APP_VEHICLE_VERIFIERS_SIZE){
                        CONNECTIVITY_voidReceiveHandleMessage(&g_receivedMessage);
                        CONNECTIVITY_voidCallback(&g_receivedMessage);
                        error = 0
;                    }
                    else {
                       error = 1;
                    }
                }
                else{
                    CONNECTIVITY_voidReceiveHandleMessage(&g_receivedMessage);
                    CONNECTIVITY_voidCallback(&g_receivedMessage);
                }
                uart_rx_index = 0;
                startDetected = false;
                continue;  // Don't store end byte
            }

            if (uart_rx_index < MAX_BUFFER_SIZE) {
                // snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                // "\nReceived value: %c...\n", 
                // (char)recChar);
                // UART_SendMessage(gArr_DebugMsg);
                g_receivedMessage.data[uart_rx_index++] = recChar;
            } else {
                // Overflow: reset
                uart_rx_index = 0;
                startDetected = false;
            }
        }
    }

    if (status & (UART_INT_OE | UART_INT_FE | UART_INT_PE)) {
        UARTIntClear(UART1_BASE, status);
    }
    if (error) {
        Connectivity_voidRequestVerifiers();
    }
    
}

/**
 * @brief Parses a received UART message into a structured format.
 * 
 * This function extracts the message type and related payload fields
 * from the raw UART data buffer into a structured message.
 *
 * @param msg Pointer to the message structure to populate.
 */
static void CONNECTIVITY_voidReceiveHandleMessage(CONNECTIVITY_tstructMsgHandle * Add_u8MsgReceived) {
    uint8_t index = 0;

    Add_u8MsgReceived->msg_type = (CONNECTIVITY_tenuVehicleMsgType)Add_u8MsgReceived->data[index++];

    switch (Add_u8MsgReceived->msg_type) {
        case MSG_TYPE_VEHICLE_STATE:
            Global_u8VehicleInfo.states.engineState = Add_u8MsgReceived->data[index++];
            Global_u8VehicleInfo.states.batteryLevel = Add_u8MsgReceived->data[index++];
            Global_u8VehicleInfo.states.doorsLocked = Add_u8MsgReceived->data[index++];
            Global_u8VehicleInfo.states.acState = Add_u8MsgReceived->data[index++];
            Global_u8VehicleInfo.states.tirePSI = Add_u8MsgReceived->data[index++];
            break;

        case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
            memcpy(Global_u8VehicleInfo.vehiclePublicKeyCeritificate, &(Add_u8MsgReceived->data[1]), APP_VEHICLE_PK_CERTIFICATE_SIZE);
            break;

        case MSG_TYPE_REQUEST_VERIFIERS:
            if(Add_u8MsgReceived->data_len == APP_VEHICLE_VERIFIERS_SIZE){
                memcpy((uint8_t*)(&Global_u8VehicleInfo.verifiers), &(Add_u8MsgReceived->data[1]), Add_u8MsgReceived->data_len);
            }
            else {
                Connectivity_voidRequestVerifiers();
            }
            break;
    

        default:
            break;
    }
}

/**
 * @brief Executes the appropriate application-level response based on the received message type.
 * 
 * @param msg Pointer to the received message structure.
 */
static void CONNECTIVITY_voidCallback(CONNECTIVITY_tstructMsgHandle *msg) {
    switch (msg->msg_type) {
        case MSG_TYPE_REQUEST_VERIFIERS:
            APP_voidFSMHandler(EVENT_VERIFIERS_RECEIVED, Global_u8CurrentDeviceId);
            break;

        case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
            APP_voidFSMHandler(EVENT_CERTIFICATE_RECEIVED, Global_u8CurrentDeviceId);            
            break;
    }
}

/**
 * @brief Constructs and sends a structured UART message based on the message type.
 * 
 * @param msg Pointer to the message handle structure containing the data to send.
 */
static void CONNECTIVITY_voidSendHandleMessage(CONNECTIVITY_tstructMsgHandle *msg) {
    uint8_t buffer[200];
    uint8_t index = 0;
    uint16_t len = APP_VEHICLE_PK_CERTIFICATE_SIZE;

    buffer[index++] = CONNECTIVITY_START_BYTE;
    buffer[index++] = msg->msg_type;

    switch (msg->msg_type) {
        case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
            memcpy(&buffer[index], msg->data, APP_VEHICLE_PK_CERTIFICATE_SIZE);
            index += len;
            break;

        case MSG_TYPE_VEHICLE_STATE:
            buffer[index++] = Global_u8VehicleInfo.states.engineState;
            buffer[index++] = Global_u8VehicleInfo.states.batteryLevel;
            buffer[index++] = Global_u8VehicleInfo.states.doorsLocked;
            buffer[index++] = Global_u8VehicleInfo.states.acState;
            buffer[index++] = Global_u8VehicleInfo.states.tirePSI;
            break;

        default:
            break;
    }

    buffer[index] = CONNECTIVITY_END_BYTE;
    UART_SendHexConnectivity(buffer, index);
}

/**
 * @brief Sends a message over UART using structured formatting.
 * 
 * @param message Pointer to the message structure to send.
 */
static void CONNECTIVITY_voidSendData(CONNECTIVITY_tstructMsgHandle *message) {
    CONNECTIVITY_voidSendHandleMessage(message);
}

/*==================== PUBLIC FUNCTION IMPLEMENTATIONS ====================*/
/**
 * @brief Sends a request message for the vehicle's certificate.
 */
void Connectivity_voidRequestVerifiers(void) {
    CONNECTIVITY_tstructMsgHandle Loc_structRequestVerifierConfig = {
        .msg_type = MSG_TYPE_REQUEST_VERIFIERS,
    };
    CONNECTIVITY_voidSendHandleMessage(&Loc_structRequestVerifierConfig);
}

/**
 * @brief Sends a request message to the server for the vehicle's certificate.
 */
void Connectivity_voidRequestCertificate(void) {
    CONNECTIVITY_tstructMsgHandle Loc_structRequestCertificateConfig = {
        .msg_type = MSG_TYPE_PK_VEHICLE_CERTIFICATE,
        .data_len = strlen((const char*)Global_u8VehicleInfo.vehiclePublicKey)
    };
    uint16_t len = Loc_structRequestCertificateConfig.data_len;
    memcpy(Loc_structRequestCertificateConfig.data, Global_u8VehicleInfo.vehiclePublicKey, len);
    CONNECTIVITY_voidSendHandleMessage(&Loc_structRequestCertificateConfig);
}
