/*

Author: ABDELRAHMAN

 */


#include "connectivity.h"



/* LPUART handle message */
void CONNECTIVITY_SendHandleMessage(uint8_t *buffer, CONNECTIVITY_Message_t *msg)
{
    uint8_t msgBytes = 0;
    buffer[msgBytes++] = UART_START_BYTE;
    buffer[msgBytes++] = msg->msg_type;
    switch (msg->msg_type)
    {
    case MSG_TYPE_CERTIFICATES:
        //        buffer[msgBytes++] = msg->certificate.issueDate;
        //        buffer[msgBytes++] = msg->certificate.expirationDate;
        //        buffer[msgBytes++] = msg->certificate.modelYear;
        //        buffer[msgBytes++] = msg->certificate.certificateID;
        //        buffer[msgBytes++] = *(msg->certificate.vehicleMake);
        //        buffer[msgBytes++] = *(msg->certificate.vehicleModel);
        //        buffer[msgBytes++] = *(msg->certificate.publicKey);
        //        buffer[msgBytes++] = *(msg->certificate.signature);
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
    uint8_t msgBytes = 0;
    if(buffer[msgBytes++] == UART_START_BYTE)
    {
        switch (msg->msg_type)
        {
        case MSG_TYPE_CERTIFICATES:
            //        buffer[msgBytes++] = msg->certificate.issueDate;
            //        buffer[msgBytes++] = msg->certificate.expirationDate;
            //        buffer[msgBytes++] = msg->certificate.modelYear;
            //        buffer[msgBytes++] = msg->certificate.certificateID;
            //        buffer[msgBytes++] = *(msg->certificate.vehicleMake);
            //        buffer[msgBytes++] = *(msg->certificate.vehicleModel);
            //        buffer[msgBytes++] = *(msg->certificate.publicKey);
            //        buffer[msgBytes++] = *(msg->certificate.signature);
            break;

        case MSG_TYPE_REGISTRATION:
            break;

        case MSG_TYPE_VEHICLE_STATE:
            msg->states.engineState = buffer[msgBytes++];
            msg->states.batteryLevel = buffer[msgBytes++];
            msg->states.doorsLocked = buffer[msgBytes++];
            msg->states.acState = buffer[msgBytes++];
            msg->states.tirePSI = buffer[msgBytes++];
            break;
        case MSG_TYPE_ALERT:
            msg->alert = buffer[msgBytes++];
            break;

        case MSG_TYPE_COMMAND:
            msg->command = (VehicleCommand_t)buffer[msgBytes++];
            break;

        default:
            break;
        }
        buffer[msgBytes] = UART_END_BYTE;
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
