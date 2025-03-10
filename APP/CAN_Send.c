/********************************************************************/
/* Author   : Mohamed Abdel Hamid                                   */
/* Date     : 26 / 2 / 2025                                         */
/* Email    : mohamedhamiid20@gmail.com                             */
/* Phone    : 01092301921                                           */
/* Brief    : Handling sending data through CAN.                    */
/* Copyright: Sponsored by Ejad                                     */
/********************************************************************/

#include "CAN_App.h"
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

// Flag to determine the distance being sent is from passive entry or from trigger distance measurement command
#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)
uint8_t Global_u8PEorTDM;
#endif

char loc_cPrintBuffer[256];     // Buffer for formatted print output

#if (CAN_ENABLE_HANDOVER)
// Global variable to track handover message sending count
uint8_t Global_u8SendHandover;
#endif


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

    // Print formatted bonding data transmission details after all data is sent
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "CAN Bonding Data Transmission        \n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "[INFO] Bonding Data Sent:\n  - NVM_ID        : %d\n  - Authenticated : %s\n  - LE Secure Conn: %s\n  - Address       : ", 
             Copy_u8NvmId, gAppOutAuth ? "Yes" : "No", gAppOutLeSc ? "Yes" : "No");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);

    for (i = 0; i < gcBleDeviceAddressSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aAddress[i]);
        SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    }

    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n  - LTK           : ");
    for (i = 0; i < gcSmpMaxLtkSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aLtk[i]);
        SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    }

    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n  - IRK           : ");
    for (i = 0; i < gcSmpIrkSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aIrk[i]);
        SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    }

    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n\n");

#else
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Bonding Data Transmission        \n");
    UART_SendMessage("============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "[INFO] Bonding Data Sent:\n  - NVM_ID        : %d\n  - Authenticated : %s\n  - LE Secure Conn: %s\n  - Address       : ", 
             Copy_u8NvmId, gAppOutAuth ? "Yes" : "No", gAppOutLeSc ? "Yes" : "No");
    UART_SendMessage(loc_cPrintBuffer);

    for (i = 0; i < gcBleDeviceAddressSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aAddress[i]);
        UART_SendMessage(loc_cPrintBuffer);
    }

    UART_SendMessage("\n  - LTK           : ");
    for (i = 0; i < gcSmpMaxLtkSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aLtk[i]);
        UART_SendMessage(loc_cPrintBuffer);
    }

    UART_SendMessage("\n  - IRK           : ");
    for (i = 0; i < gcSmpIrkSize_c; i++){
        snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "%02X ", Add_structKeys.aIrk[i]);
        UART_SendMessage(loc_cPrintBuffer);
    }

    UART_SendMessage("\n============================\n\n");
#endif
}

#if (CAN_ENABLE_HANDOVER)
/* CAN_voidHandover
 * @brief Sends a handover message with the next anchor point for the device.
 *
 * @param Copy_u8DeviceId ID of the device undergoing handover.
 */
void CAN_voidHandover(uint8_t Copy_u8DeviceId){
  uint8_t loc_u8CanData[8] = {0};
  loc_u8CanData[0] = Copy_u8DeviceId; // Store device ID
  loc_u8CanData[1] = CAN_GET_NEXT_ANCHOR(CAN_ANCHOR_ID); // Fetch next anchor ID

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE) && defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
  // Print title in shell
  shell_write("\n================= \nCAN send Handover \n=================\n");
#else
  UART_SendMessage("\n================= \nCAN send Handover \n=================\n");
#endif

  // Send handover message
  CAN_voidSendMsg(CAN_ID_HANDOVER, loc_u8CanData);
}
#endif

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)

/* CAN_voidSendDistance
 * @brief Sends distance measurement data over CAN bus.
 *
 * @param Copy_u8DeviceId Device ID.
 * @param Copy_u16ProcNo Process number.
 * @param Copy_structResults Struct containing localization algorithm results.
 * @param Copy_u8PEorTDM flag to determine either the distance is from Passive Entry or Trigger Distance Measurement command
 */
void CAN_voidSendDistance(uint8_t Copy_u8DeviceId, uint16_t Copy_u16ProcNo, localizationAlgoRun_t Copy_structResults, uint8_t Copy_u8PEorTDM){
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data

    loc_u8CanData[0] = Copy_u8DeviceId;
    loc_u8CanData[1] = Copy_u16ProcNo & 0xff;
    loc_u8CanData[2] = (Copy_u16ProcNo >> 8) & 0xff;
    loc_u8CanData[3] = Copy_structResults.distanceIntegerPart;
    loc_u8CanData[4] = (Copy_structResults.distanceDecimalPart) & 0xff;
    loc_u8CanData[5] = (Copy_structResults.distanceDecimalPart >> 8) & 0xff;
    loc_u8CanData[6] = (uint8_t)(Copy_structResults.dqiPercentage * 100) & 0xff;
    loc_u8CanData[7] = ((uint8_t)(Copy_structResults.dqiPercentage * 100) >> 8) & 0xff;

    if(Copy_u8PEorTDM == PE)
        CAN_voidSendMsg(CAN_ID_DISTANCE_PE, loc_u8CanData);
    else
        CAN_voidSendMsg(CAN_ID_DISTANCE_TDM, loc_u8CanData);

#if (CAN_ENABLE_HANDOVER)
    // Track the number of handover messages sent
    Global_u8SendHandover++;
    if (Global_u8SendHandover == gCsProcRepeatMaxNumProcedures_c){
        CAN_voidHandover(Copy_u8DeviceId);
        Global_u8SendHandover = 0;
    }
#endif

    // Print formatted distance transmission details after all data is sent
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "CAN Distance Transmission            \n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), 
             "[INFO] Distance Data Sent:\n  - Device ID   : %d\n  - Process No  : %d\n  - Distance    : %d.%02d meters\n  - DQI         : %d%%\n  - Trigger Type: %s\n", 
             Copy_u8DeviceId, Copy_u16ProcNo, Copy_structResults.distanceIntegerPart, 
             Copy_structResults.distanceDecimalPart, (uint8_t)(Copy_structResults.dqiPercentage * 100),
             (Copy_u8PEorTDM == PE) ? "Passive Entry" : "Trigger Distance Measurement");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n\n");

#else
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Distance Transmission            \n");
    UART_SendMessage("============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), 
             "[INFO] Distance Data Sent:\n  - Device ID   : %d\n  - Process No  : %d\n  - Distance    : %d.%02d meters\n  - DQI         : %d%%\n  - Trigger Type: %s\n", 
             Copy_u8DeviceId, Copy_u16ProcNo, Copy_structResults.distanceIntegerPart, 
             Copy_structResults.distanceDecimalPart, (uint8_t)(Copy_structResults.dqiPercentage * 100),
             (Copy_u8PEorTDM == PE) ? "Passive Entry" : "Trigger Distance Measurement");
    UART_SendMessage(loc_cPrintBuffer);
    UART_SendMessage("============================\n\n");
#endif
}


/** CAN_voidSendPeResponse
 * @brief Sends passive entry response over CAN bus.
 *
 * @param Copy_enuResponse Respone either success or fail.
 * @param Copy_u8ReceiverId Receiver Identifier.
 * @param Copy_u8DeviceId Device ID.
 */
void CAN_voidSendPeResponse(CAN_tenumPeResponse Copy_enuResponse, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8DeviceId) {
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data

    loc_u8CanData[0] = Copy_enuResponse;  // Store response
    loc_u8CanData[1] = Copy_u8ReceiverId; // Store receiver ID
    loc_u8CanData[2] = Copy_u8DeviceId;   // Store device ID

    // Send PE response message
    CAN_voidSendMsg(CAN_ID_PE_STATUS, loc_u8CanData);

    // Print formatted PE response details after all data is sent
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "CAN PE Response Transmission         \n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer),
             "[INFO] PE Response Sent:\n  - Response Type : %d\n  - Receiver ID   : %d\n  - Device ID     : %d\n",
             Copy_enuResponse, Copy_u8ReceiverId, Copy_u8DeviceId);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n\n");

#else
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN PE Response Transmission         \n");
    UART_SendMessage("============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer),
             "[INFO] PE Response Sent:\n  - Response Type : %d\n  - Receiver ID   : %d\n  - Device ID     : %d\n",
             Copy_enuResponse, Copy_u8ReceiverId, Copy_u8DeviceId);
    UART_SendMessage(loc_cPrintBuffer);
    UART_SendMessage("============================\n\n");
#endif
}


/** CAN_voidNotifyWakeup
 * @brief Sends wakeup notification over CAN bus.
 *
 */
void CAN_voidNotifyWakeup() {
    uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data
    
    loc_u8CanData[0] = 1; // Indicate wakeup notification

    // Send wakeup notification message
    CAN_voidSendMsg(CAN_ID_WAKEUP_NOTIFICATION, loc_u8CanData);

    // Print formatted wakeup notification details after data is sent
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "CAN Wakeup Notification             \n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "[INFO] Wakeup Notification Sent Successfully\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n\n");

#else
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Wakeup Notification             \n");
    UART_SendMessage("============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer), "[INFO] Wakeup Notification Sent Successfully\n");
    UART_SendMessage(loc_cPrintBuffer);
    UART_SendMessage("============================\n\n");
#endif
}

#else

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

    // Send command message before printing the output
    CAN_voidSendMsg(CAN_ID_COMMANDS, loc_u8CanData);

    // Print formatted command transmission details after data is sent
#if defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "\n============================\n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "CAN Command Transmission           \n");
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer),
             "[INFO] Command Sent:\n  - Command ID  : %d\n  - Receiver ID : %d\n  - Data        : %d\n",
             Copy_enuCommand, Copy_u8ReceiverId, Copy_u8Data);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, loc_cPrintBuffer);
    SHELL_PrintfSynchronization((shell_handle_t)g_shellHandle, "============================\n\n");

#else
    UART_SendMessage("\n============================\n");
    UART_SendMessage("CAN Command Transmission           \n");
    UART_SendMessage("============================\n");

    snprintf(loc_cPrintBuffer, sizeof(loc_cPrintBuffer),
             "[INFO] Command Sent:\n  - Command ID  : %d\n  - Receiver ID : %d\n  - Data        : %d\n",
             Copy_enuCommand, Copy_u8ReceiverId, Copy_u8Data);
    UART_SendMessage(loc_cPrintBuffer);
    UART_SendMessage("\n============================\n\n");
#endif
}


#endif

