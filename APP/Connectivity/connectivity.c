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
extern uint8_t Global_u8CurrentDeviceId;
extern char gArr_DebugMsg[APP_DEBUG_ARRAY_MAX_SIZE];

/*==================== PRIVATE FUNCTION DECLARATIONS ====================*/
/**
 * @brief Sends a full vehicle message over UART
 */
static void CONNECTIVITY_voidSendHandleMessage(void);

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

/**
 * @brief Validate received data length against specs
 * @param Copy_enuMsgType Received msg type
 * @param Copy_u16DataLength Data length
 */
static bool CONNECTIVITY_bValidateDataLength(CONNECTIVITY_tenuVehicleMsgType Copy_enuMsgType, uint16_t Copy_u16DataLength);

/**
 * @brief Sends a resend request for the vehicle's verifier [not requesting from server].
 * @usage When there is error in transmitting data from connectivity to fusion through UART         
 */
static void Connectivity_voidResend(CONNECTIVITY_tenuVehicleMsgType Copy_enuMsgType);

/*==================== PRIVATE FUNCTION IMPLEMENTATIONS ====================*/
CONNECTIVITY_tstructMsgHandle g_receivedMessage;
void UART1_Handler(void) {
    static enum {
        STATE_WAIT_TYPE,
        STATE_WAIT_LENGTH_1,
        STATE_WAIT_LENGTH_2,
        STATE_RECEIVING_DATA
    } uart_state = STATE_WAIT_TYPE;

    static CONNECTIVITY_tenuVehicleMsgType msg_type;
    static uint16_t expected_len = 0;
    static uint16_t received_len = 0;
    static uint8_t error = 0;

    uint32_t status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, status);

    if (status & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(UART1_BASE)) {
            uint8_t recChar = (uint8_t)UARTCharGetNonBlocking(UART1_BASE);

            switch (uart_state) {
                case STATE_WAIT_TYPE:
                    msg_type = (CONNECTIVITY_tenuVehicleMsgType) recChar;
                    uart_state = STATE_WAIT_LENGTH_1;
                    break;

                case STATE_WAIT_LENGTH_1:
                    expected_len = recChar;
                    uart_state = STATE_WAIT_LENGTH_2;
                    break;

                case STATE_WAIT_LENGTH_2:
                    expected_len |= ((uint16_t)recChar << 8);

                    if (expected_len > MAX_BUFFER_SIZE) {
                        error = 1;
                        Connectivity_voidResend(msg_type);
                        uart_state = STATE_WAIT_TYPE;
                        break;
                    }

                    received_len = 0;
                    uart_state = STATE_RECEIVING_DATA;
                    break;

                case STATE_RECEIVING_DATA:
                    g_receivedMessage.data[received_len++] = recChar;

                    if (received_len == expected_len) {
                        g_receivedMessage.msg_type = (CONNECTIVITY_tenuVehicleMsgType)msg_type;
                        g_receivedMessage.data_len = expected_len;

                        // Validate message size
                        if (CONNECTIVITY_bValidateDataLength(g_receivedMessage.msg_type, g_receivedMessage.data_len)) {
                            CONNECTIVITY_voidReceiveHandleMessage(&g_receivedMessage);
                            CONNECTIVITY_voidCallback(&g_receivedMessage);
                        } else {
                            error = 1;
                            Connectivity_voidResend(msg_type);
                        }

                        uart_state = STATE_WAIT_TYPE;
                    }
                    break;

                default:
                    uart_state = STATE_WAIT_TYPE;
                    break;
            }
        }
    }

    if (status & (UART_INT_OE | UART_INT_FE | UART_INT_PE)) {
        UARTIntClear(UART1_BASE, status);
        error = 1;
    }

    if (error) {
        error = 0;
        uart_state = STATE_WAIT_TYPE;
    }
}




/**
 * @brief Validate received data length against specs
 * @param Copy_enuMsgType Received msg type
 * @param Copy_u16DataLength Data length
 * @return true or false
 */
static bool CONNECTIVITY_bValidateDataLength(CONNECTIVITY_tenuVehicleMsgType Copy_enuMsgType, uint16_t Copy_u16DataLength){
    bool Loc_bValidLength;
    switch (Copy_enuMsgType) {
        case MSG_TYPE_REQUEST_VERIFIERS:
            Loc_bValidLength = (Copy_u16DataLength == APP_VEHICLE_VERIFIERS_SIZE) ? 1 : 0 ;
            break;
        case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
            Loc_bValidLength = (Copy_u16DataLength == APP_VEHICLE_PK_CERTIFICATE_SIZE) ? 1 : 0 ;
            break;
        default:
            break;
    }
    return Loc_bValidLength;
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
            memcpy((uint8_t*)(&Global_u8VehicleInfo.verifiers), (Add_u8MsgReceived->data), Add_u8MsgReceived->data_len);
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


CONNECTIVITY_tstructMsgHandle g_sendMessage;

/**
 * @brief Constructs and sends a structured UART message based on the message type.
 * 
 * @param msg Pointer to the message handle structure containing the data to send.
 */
static void CONNECTIVITY_voidSendHandleMessage(void) {
    // Shift the existing payload to make room for the header
    // Safe because MAX_BUFFER_SIZE is large enough
    memmove(&g_sendMessage.data[3], g_sendMessage.data, g_sendMessage.data_len);

    // Insert header
    g_sendMessage.data[0] = (uint8_t)g_sendMessage.msg_type;
    g_sendMessage.data[1] = (uint8_t)(g_sendMessage.data_len & 0xFF);       // Length LSB
    g_sendMessage.data[2] = (uint8_t)((g_sendMessage.data_len >> 8) & 0xFF); // Length MSB

    // Send full message: 3 (header) + data_len
    uint16_t total_len = g_sendMessage.data_len + 3;
    UART_SendHexConnectivity(g_sendMessage.data, total_len);

    // Reset structure for next use
    g_sendMessage.msg_type = (CONNECTIVITY_tenuVehicleMsgType) 0;
    g_sendMessage.data_len = 0;
    memset(g_sendMessage.data, 0, MAX_BUFFER_SIZE);
}

/**
 * @brief Sends a resend request for the vehicle's verifier [not requesting from server].
 * @usage When there is error in transmitting data from connectivity to fusion through UART         
 */
static void Connectivity_voidResend(CONNECTIVITY_tenuVehicleMsgType Copy_enuMsgType) {
    CONNECTIVITY_tenuVehicleMsgType Loc_enuMsgType;
    switch (Copy_enuMsgType) {
        case MSG_TYPE_REQUEST_VERIFIERS:
            Loc_enuMsgType =  MSG_TYPE_RESEND_VERIFIERS;
            break;
        case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
            Loc_enuMsgType = MSG_TYPE_RESEND_CERTIFICATE ;
            break;
        default:
            break;
    }
    
    g_sendMessage.msg_type = Loc_enuMsgType;

    CONNECTIVITY_voidSendHandleMessage();
}

/**
 * @brief Sends a message over UART using structured formatting.
 * 
 * @param message Pointer to the message structure to send.
 */
static void CONNECTIVITY_voidSendData(CONNECTIVITY_tstructMsgHandle *message) {
    CONNECTIVITY_voidSendHandleMessage();
}

/*==================== PUBLIC FUNCTION IMPLEMENTATIONS ====================*/
/**
 * @brief Sends a request message for the vehicle's verifiers.
 */
void Connectivity_voidRequestVerifiers(void) {
    g_sendMessage.msg_type = MSG_TYPE_REQUEST_VERIFIERS;
    g_sendMessage.data_len = 0;
    CONNECTIVITY_voidSendHandleMessage();
}

/**
 * @brief Sends a request message to the server for the vehicle's certificate.
 */

void Connectivity_voidRequestCertificate(void) {
    uint16_t len = strlen((const char*)Global_u8VehicleInfo.vehiclePublicKey);

    g_sendMessage.msg_type = MSG_TYPE_PK_VEHICLE_CERTIFICATE;
    g_sendMessage.data_len = len;

    // Copy payload into the start of g_sendMessage.data (we'll shift it in the send function)
    memcpy(g_sendMessage.data, Global_u8VehicleInfo.vehiclePublicKey, len);

    // Send the message using the updated send logic
    CONNECTIVITY_voidSendHandleMessage();
}


