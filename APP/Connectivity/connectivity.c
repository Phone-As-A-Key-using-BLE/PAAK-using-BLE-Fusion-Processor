/*

Author: ABDELRAHMAN

*/


#include "connectivity.h"
#include "string.h"
#include <stdint.h>
#include "APP/can_msg_types.h"
#include "APP/APP_FSM.h"
#include "APP/CAN_Send.h"
#define BUFFER_SIZE 128

volatile char rxBuffer[BUFFER_SIZE];
volatile uint32_t rxIndex = 0;
static uint8_t uart_rx_buffer[MAX_BUFFER_SIZE];
CONNECTIVITY_Message_t g_receivedMessage;
CONNECTIVITY_Message_t g_sendMessage;

extern uint8_t Global_u8CurrentDeviceId;

void UART1_Handler(void)
{
    static uint8_t uart_rx_index = 0;
    uint32_t status = UARTIntStatus(UART1_BASE, true);
    UARTIntClear(UART1_BASE, status);

    // Handle receive interrupts
    if (status & (UART_INT_RX | UART_INT_RT)) {
        while (UARTCharsAvail(UART1_BASE)) {
            uint8_t recChar = (uint8_t)UARTCharGetNonBlocking(UART1_BASE);

            // Store character into buffer if space is available
            if (uart_rx_index < MAX_BUFFER_SIZE) {
                uart_rx_buffer[uart_rx_index++] = recChar;

                // Process message on end byte
                if (recChar == UART_END_BYTE && uart_rx_index >= 2) {
                    CONNECTIVITY_ReceiveHandleMessage(uart_rx_buffer, &g_sendMessage);
                    CONNECTIVITY_callback(&g_sendMessage);
                    uart_rx_index = 0;
                }
            } else {
                uart_rx_index = 0; // Discard current buffer
            }
        }
    }

    // Handle error interrupts (if enabled)
    if (status & (UART_INT_OE | UART_INT_FE | UART_INT_PE)) {
        // Log or handle errors (e.g., reset UART)
        UARTIntClear(UART1_BASE, status); // Ensure errors are cleared
    }

    
}

/* LPUART handle message */
void CONNECTIVITY_SendHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg)
{
    uint8_t index = 0;
    uint8_t msgBytes = 0;
    buffer[msgBytes++] = UART_START_BYTE;
    buffer[msgBytes++] = msg->msg_type;
    switch (msg->msg_type)
    {
    case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
    while (msgBytes <= 65) { //64 byte + start and type -> 65
        buffer[msgBytes++] = msg->data[index++];
    }
        break;

    case MSG_TYPE_REGISTRATION:
        break;

    case MSG_TYPE_VEHICLE_STATE:
        buffer[msgBytes++] = (uint8_t)msg->states.engineState;
        buffer[msgBytes++] = (uint8_t)msg->states.batteryLevel;
        buffer[msgBytes++] = (uint8_t)msg->states.doorsLocked;
        buffer[msgBytes++] = (uint8_t)msg->states.acState;
        buffer[msgBytes++] = (uint8_t)msg->states.tirePSI;
        break;
    case MSG_TYPE_ALERT:
        buffer[msgBytes++] = (uint8_t)msg->alert;
        break;
    case MSG_TYPE_COMMAND:
        buffer[msgBytes++] = (uint8_t)msg->command;
        break;
    default:
        break;
    }
    buffer[msgBytes] = UART_END_BYTE;
}

/* LPUART handle message */
void CONNECTIVITY_ReceiveHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg)
{
    uint8_t index = 0;
    uint8_t msgBytes = 0;
    if(buffer[msgBytes++] == UART_START_BYTE)
    {
        switch (buffer[msgBytes])
        {
        case MSG_TYPE_REGISTRATION:
            break;

        case MSG_TYPE_VEHICLE_STATE:
            msg->msg_type = (VehicleMessageType_t)buffer[msgBytes++];
            msg->states.engineState = buffer[msgBytes++];
            msg->states.batteryLevel = buffer[msgBytes++];
            msg->states.doorsLocked = buffer[msgBytes++];
            msg->states.acState = buffer[msgBytes++];
            msg->states.tirePSI = buffer[msgBytes++];
            break;
        case MSG_TYPE_ALERT:
            msg->msg_type = (VehicleMessageType_t)buffer[msgBytes++];
            msg->alert = (VehicleAlert_t)buffer[msgBytes++];
            break;

        case MSG_TYPE_COMMAND:
            msg->msg_type = (VehicleMessageType_t)buffer[msgBytes++];
            msg->command = (VehicleCommand_t)buffer[msgBytes++];
            break;
        case MSG_TYPE_PK_VEHICLE_CERTIFICATE://Be Modified Later
            while(buffer[msgBytes] != UART_END_BYTE)
            {
                msg->data[index++] = buffer[msgBytes++];
            }
            break;
        default:
            break;
        }
    }
    else
    {
        //Error
    }
}
/* Function to receive data over UART */
CONNECTIVITY_Message_t CONNECTIVITY_ReceiveData()
{
    CONNECTIVITY_Message_t msg;
    uint8_t recvBuffer[MAX_BUFFER_SIZE] = {0U};
    uint32_t receivedBytes = 0;
    while (recvBuffer[receivedBytes-1] != UART_END_BYTE)
    {
        recvBuffer[receivedBytes++] = UART_RecieveMessage();
    }
    CONNECTIVITY_ReceiveHandleMessage(recvBuffer, &msg);
    return msg;
}

/* Function to send data over UART */
void CONNECTIVITY_SendData(CONNECTIVITY_Message_t *message)
{
    uint8_t sendBuffer[MAX_BUFFER_SIZE] = {0U};
    uint8_t i = 0;

    CONNECTIVITY_SendHandleMessage(sendBuffer,message);

    for(;i<MAX_BUFFER_SIZE;)
    {
        if(sendBuffer[i] == UART_END_BYTE)
        {
            break;
        }
        i++;
    }
    /* Set up the transfer data */
    UART_SendHexConnectivity(sendBuffer, i);
    /* Wait for the transmit complete. */
    //    while (!isTransferCompleted)
    //    {
    //    }
}
void CONNECTIVITY_callback(CONNECTIVITY_Message_t *msg)
{
    switch(msg->msg_type)
    {
    case MSG_TYPE_ALERT:
        break;
    case MSG_TYPE_REGISTRATION:
        APP_voidFSMHandler(EVENT_VERIFIERS_RECEIVED, Global_u8CurrentDeviceId);
         break;
    case MSG_TYPE_PK_VEHICLE_CERTIFICATE:
        CAN_voidSendCertificate(Global_u8CurrentDeviceId,VEHICLE_PK_CERTIFICATE, msg->data_len, msg->data);
        break;
    case MSG_TYPE_COMMAND:
        break;
    case MSG_TYPE_VEHICLE_STATE:
        break;
    }
}
