/*

Author: ABDELRAHMAN

*/


#include "connectivity.h"



/* LPUART handle message */
void CONNECTIVITY_SendHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg)
{
    uint8_t msgBytes = 0;
    buffer[msgBytes++] = UART_START_BYTE;
    buffer[msgBytes++] = msg->command;
    switch (msg->command)
    {
    case VEHICLE_SEND_CERTIFICATE:
        buffer[msgBytes++] = msg->certificate.issueDate;
        buffer[msgBytes++] = msg->certificate.expirationDate;
        buffer[msgBytes++] = msg->certificate.modelYear;
        buffer[msgBytes++] = msg->certificate.certificateID;
        buffer[msgBytes++] = *(msg->certificate.vehicleMake);
        buffer[msgBytes++] = *(msg->certificate.vehicleModel);
        buffer[msgBytes++] = *(msg->certificate.publicKey);
        buffer[msgBytes++] = *(msg->certificate.signature);
        break;

    case VEHICLE_REQUEST_CERTIFICATE:
        break;

    case VEHICLE_SEND_STATUS:
        buffer[msgBytes++] = (uint8_t)msg->status.battery_level;
        buffer[msgBytes++] = (uint8_t)msg->status.temperatrue;
        buffer[msgBytes++] = (uint8_t)msg->status.lock;
        buffer[msgBytes++] = (uint8_t)msg->status.active_users;
        buffer[msgBytes++] = (uint8_t)msg->status.tire_pressure;
        break;
    case VEHICLE_REQUEST_STATUS:
        break;

    case VEHICLE_SEND_DATA:
        break;

    case VEHICLE_RECEIVE_DATA:
        break;

//    case VEHICLE_SEND_ERROR_REPORT:
//        PRINTF("Handling: VEHICLE_SEND_ERROR_REPORT\n");
//        break;
//
//    case VEHICLE_REQUEST_DIAGNOSTICS:
//        PRINTF("Handling: VEHICLE_REQUEST_DIAGNOSTICS\n");
//        break;

    default:
        break;
    }
    buffer[msgBytes] = UART_END_BYTE;
}

/* Function to receive data over UART */
void CONNECTIVITY_ReceiveData()
{
    uint8_t recvBuffer[BUFFER_SIZE] = {0U};
    uint32_t receivedBytes = 0;
    while (recvBuffer[receivedBytes-1] != UART_END_BYTE)
    {
        recvBuffer[receivedBytes++] = UART_RecieveMessage();
    }
    //CONNECTIVITY_HandleMessage(recvBuffer);
}

/* Function to send data over UART */
void CONNECTIVITY_SendData(CONNECTIVITY_Message_t *message)
{
    uint8_t sendBuffer[BUFFER_SIZE] = {0U};
    uint8_t i = 0;

    CONNECTIVITY_SendHandleMessage(sendBuffer,message);

    for(;i<BUFFER_SIZE;)
    {
        if(sendBuffer[i] == UART_END_BYTE)
        {
            break;
        }
        i++;
    }
    /* Set up the transfer data */
    UART_SendHexConnectivity((char*)sendBuffer, i);
    /* Wait for the transmit complete. */
//    while (!isTransferCompleted)
//    {
//    }
}
