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
#include <string.h>
#include <stdio.h>
/* Include are customized either to anchor which is NXP KW45
   or Master which is tiva C */
#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)
#include "app_localization.h"
#include "flexcan_interrupt_transfer.h"
#include "gap_types.h"
#include "app_localization.h"
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
#include "shell_print.h"
#endif
#else
#include "can.h"
#include "can_config.h"
#endif
#include "can_msg_types.h"
#include "CAN_Send.h"

extern char gArr_DebugMsg[512];     // Buffer for formatted print output
const char* CAN_CommandStrings[] = {
    "CAN_COMMAND_TRIGGER_OWNER_PAIRING",
    "CAN_COMMAND_TRIGGER_PASSIVE_ENTRY",
    "CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT",
    "CAN_COMMAND_STOP_DISTANCE_MEASURMENT",
    "CAN_COMMAND_RESET",
    "CAN_COMMAND_INVALID",
    "CAN_COMMMAND_DISCONNECT_FROM_DEVICE"
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
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Handover Command Transmission\n");
    UART_SendMessage("============================\n");

    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
            "[INFO] Command Sent :\n - Command Name: CAN_COMMAND_HANDOVER\n  - Handover from : Anchor %d\n  - Handover to : Anchor %d\n - Device Id: %d\n", Copy_u8ReceiverId, Copy_u8HandoverTo,
            Copy_u8deviceId);

    UART_SendMessage(gArr_DebugMsg);
    UART_SendMessage("============================\n\n");

    // Send command message before printing the output
    CAN_voidSendMsg(CAN_ID_COMMANDS, loc_u8CanData);
}

/**
 * @brief Sends ranging type of device over CAN bus.
 * 
 * @param Copy_u8DeviceId Device ID
 * @param Copy_enuRangingType Ranging Type RSSI or CS.
 */
void CAN_voidSendRangingType(APP_tenuRangingType* Copy_enuRangingType){
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data
    uint8_t i=0;
    for (; i<APP_MAX_NO_OF_DEVICES; i++) {
        loc_u8CanData[i] = (uint8_t)Copy_enuRangingType[i];
    }

    // Send wakeup notification message
    CAN_voidSendMsg(CAN_ID_RANGING_TYPE, loc_u8CanData);

    char* Loc_u8RangingTypeNameForDebug [] ={"Not Determined","RSSI","CS"};
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Ranging Type              \n");
    UART_SendMessage("============================\n");
    for (i=0;i<1;i++) {
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "[INFO] Sending ranging type for device %d which is %s\n" ,i,Loc_u8RangingTypeNameForDebug[Copy_enuRangingType[i]]);
    }
    UART_SendMessage(gArr_DebugMsg);
    UART_SendMessage("============================\n\n");
}




