/********************************************************************/
/* Author   : Mohamed Abdel Hamid                                   */
/* Date     : 26 / 2 / 2025                                         */
/* Email    : mohamedhamiid20@gmail.com                             */
/* Phone    : 01092301921                                           */
/* Brief    : Handling sending data through CAN.                    */
/* Copyright: Sponsored by Ejad                                     */
/********************************************************************/

#include "DeviceRangingTypeManager.h"
#include "CAN_App.h"
#include "APP_FSM.h"
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "APP/Connectivity/connectivity.h"

#include "can.h"
#include "can_config.h"

#include "can_msg_types.h"
#include "CAN_Send.h"

extern char gArr_DebugMsg[1024];     // Buffer for formatted print output
const char* CAN_CommandStrings[] = {
    "CAN_COMMAND_TRIGGER_OWNER_PAIRING",
    "CAN_COMMAND_TRIGGER_PASSIVE_ENTRY",
    "CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT",
    "CAN_COMMAND_STOP_DISTANCE_MEASURMENT",
    "CAN_COMMAND_RESET",
    "CAN_COMMAND_INVALID",
    "CAN_COMMMAND_DISCONNECT_FROM_DEVICE",
    "CAN_COMMAND_HANDOVER",
    "CAN_COMMAND_FACTORY_RESET"
};

/* CAN_voidSendBondingData
 * @brief Sends bonding data over CAN bus, including device information and security keys.
 *
 * @param Copy_u8NvmId Non-volatile memory ID of the bonded device.
 * @param gAppOutAuth Authentication flag.
 * @param gAppOutLeSc LE Secure Connections flag.
 * @param Add_structKeys Structure containing security keys.
 */
void CAN_voidSendBondingData(uint8_t Copy_u8NvmId, bool gAppOutAuth, bool gAppOutLeSc, gapSmpKeys_t Add_structKeys){
    uint8_t i=0, j=0;
    uint8_t loc_u8CanData[8] = {0}; // Local array to store CAN message data
    
    loc_u8CanData[0] = Copy_u8NvmId; // Store device ID in NVM

    // Store authentication and security properties
    loc_u8CanData[1] = Add_structKeys.addressType & 0x01;
    loc_u8CanData[1] |= (uint8_t)gAppOutAuth << 1;
    loc_u8CanData[1] |= (uint8_t)gAppOutLeSc << 2;

    // Store device address
    for (i = 0; i < gcBleDeviceAddressSize_c; i++)
        loc_u8CanData[i+2] = Add_structKeys.aAddress[i];

    // Send bonding data CAN message
    CAN_voidSendMsg(CAN_ID_BONDING_DATA, loc_u8CanData);

    // Send Long Term Key (LTK) in chunks of 8 bytes
    for (i = 0; i < gcSmpMaxLtkSize_c / 8; i++){
        for (j = 0; j < 8; j++){
            loc_u8CanData[j] = Add_structKeys.aLtk[i * 8 + j];
        }
        CAN_voidSendMsg(CAN_ID_BONDING_DATA, loc_u8CanData);
    }

    // Send Identity Resolving Key (IRK) in chunks of 8 bytes
    for (i = 0; i < gcSmpIrkSize_c / 8; i++){
        for (j = 0; j < 8; j++){
            loc_u8CanData[j] = Add_structKeys.aIrk[i * 8 + j];
        }
        CAN_voidSendMsg(CAN_ID_BONDING_DATA, loc_u8CanData);
    }
 
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Bonding Data Transmission        \n");
    UART_SendMessage("============================\n");

    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "[INFO] Bonding Data Sent:\n  - NVM_ID        : %d\n  - Authenticated : %s\n  - LE Secure Conn: %s\n  - Address       : ", 
             Copy_u8NvmId, gAppOutAuth ? "Yes" : "No", gAppOutLeSc ? "Yes" : "No");
    UART_SendMessage(gArr_DebugMsg);

    for (i = 0; i < gcBleDeviceAddressSize_c; i++){
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "%02X ", Add_structKeys.aAddress[i]);
        UART_SendMessage(gArr_DebugMsg);
    }

    UART_SendMessage("\n  - LTK           : ");
    for (i = 0; i < gcSmpMaxLtkSize_c; i++){
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "%02X ", Add_structKeys.aLtk[i]);
        UART_SendMessage(gArr_DebugMsg);
    }

    UART_SendMessage("\n  - IRK           : ");
    for (i = 0; i < gcSmpIrkSize_c; i++){
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "%02X ", Add_structKeys.aIrk[i]);
        UART_SendMessage(gArr_DebugMsg);
    }

    UART_SendMessage("\n============================\n\n");

}

/** CAN_voidSendCommand
 * @brief Sends commands over CAN bus, including receiver identifier.
 *
 * @param Copy_enuCommand Command ID.
 * @param Copy_u8ReceiverId Receiver Identifier.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8Data) {
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data

    loc_u8CanData[0] = Copy_enuCommand;   // Store command ID
    loc_u8CanData[1] = Copy_u8ReceiverId; // Store receiver ID
    loc_u8CanData[2] = Copy_u8Data;       // Store Data either device ID or Anchor ID
    
    if(Copy_enuCommand == CAN_COMMAND_TRIGGER_PASSIVE_ENTRY || Copy_enuCommand == CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT)
    {
        loc_u8CanData[3] = DeviceStateManager_Load(Copy_u8Data);
        if(loc_u8CanData[3] != APP_RSSI)
            loc_u8CanData[3] = APP_CS;

    }

    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Command Transmission           \n");
    UART_SendMessage("============================\n");

    if(Copy_enuCommand == CAN_COMMAND_TRIGGER_PASSIVE_ENTRY || Copy_enuCommand == CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT)
    {
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
             "[INFO] Command Sent :\n  - Command ID : %s\n  - Receiver ID : %d\n  - Device ID : %d\n - Ranging Type : %d\n",
             CAN_CommandStrings[Copy_enuCommand], Copy_u8ReceiverId, Copy_u8Data,loc_u8CanData[3]);

    }
    else{
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
             "[INFO] Command Sent :\n  - Command ID : %s\n  - Receiver ID : %d\n  - Device ID : %d\n",
             CAN_CommandStrings[Copy_enuCommand], Copy_u8ReceiverId, Copy_u8Data);
    }
    UART_SendMessage(gArr_DebugMsg);
    UART_SendMessage("============================\n\n");

    // Send command message before printing the output
    CAN_voidSendMsg(CAN_ID_COMMANDS, loc_u8CanData);
}

void CAN_voidSendHandoverCommand(uint8_t Copy_u8ReceiverId, uint8_t Copy_u8HandoverTo,uint8_t Copy_u8deviceId){
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data

    loc_u8CanData[0] = CAN_COMMAND_HANDOVER;
    loc_u8CanData[1] = Copy_u8ReceiverId;
    loc_u8CanData[2] = Copy_u8deviceId ;
    loc_u8CanData[3] = Copy_u8HandoverTo;
    loc_u8CanData[4] = DeviceStateManager_Load(Copy_u8deviceId);
    if(loc_u8CanData[4] != APP_RSSI)
        loc_u8CanData[4] = APP_CS;

    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Handover Command Transmission\n");
    UART_SendMessage("============================\n");

    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
            "[INFO] Command Sent :\n - Command Name: CAN_COMMAND_HANDOVER\n  - Handover from : Anchor %d\n  - Handover to : Anchor %d\n - Device Id: %d\n - Ranging Type: %d\n", Copy_u8ReceiverId, Copy_u8HandoverTo,
            Copy_u8deviceId,loc_u8CanData[4]);

    UART_SendMessage(gArr_DebugMsg);
    UART_SendMessage("============================\n\n");

    // Send command message before printing the output
    CAN_voidSendMsg(CAN_ID_COMMANDS, loc_u8CanData);
}

void CAN_voidSendCertificate(uint8_t Copy_u8Device,uint8_t Copy_u8CertificateType, uint16_t Copy_u16Size, uint8_t * Add_u8Certificate){        
    uint8_t loc_u8CanData[8];
    uint16_t currentIndex = 0;

    // ---------- Header Frame ----------
    loc_u8CanData[0] = Copy_u8Device;
    loc_u8CanData[1] = Copy_u8CertificateType;
    loc_u8CanData[2] = (uint8_t)(Copy_u16Size & 0xFF);       // Data Length LSB
    loc_u8CanData[3] = (uint8_t)((Copy_u16Size >> 8) & 0xFF); // Data Length MSB
    loc_u8CanData[4] = 0x00;
    loc_u8CanData[5] = 0x00;
    loc_u8CanData[6] = 0x00;
    loc_u8CanData[7] = 0x00;

    CAN_voidSendMsg(CAN_ID_CERTIFICATE, loc_u8CanData);
    uint8_t i = 0;
    // ---------- Data Frames (pure data) ----------
    while (currentIndex < Copy_u16Size) {
        for (i = 0; i < 8; i++) {
            if (currentIndex < Copy_u16Size) {
                loc_u8CanData[i] = Add_u8Certificate[currentIndex++];
            } else {
                loc_u8CanData[i] = 0x00; // Padding
            }
        }
        CAN_voidSendMsg(CAN_ID_CERTIFICATE, loc_u8CanData);
    }
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Certificate\n");
    UART_SendMessage("==============================\n");
}

void CAN_voidSendVerifiers(uint8_t Copy_u8Device, VehicleVerifiers_t* Add_structVerifiers) {
    uint8_t* rawData = (uint8_t*)Add_structVerifiers;
    uint8_t frameData[8];
    uint8_t i;
    for (i = 0; i < 15; i++) {
        frameData[0] = i; // frame index
        memcpy(&frameData[1], &rawData[i * 7], 7);
        CAN_voidSendMsg(CAN_ID_VERIFIERS, frameData);
    }

    // Final frame (frame 15): remaining 6 bytes
    frameData[0] = 15;
    memcpy(&frameData[1], &rawData[15 * 7], 6); // last 6 bytes
    CAN_voidSendMsg(CAN_ID_VERIFIERS, frameData);
}


