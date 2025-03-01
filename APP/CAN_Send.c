/********************************************************************/
/* Author   : Mohamed Abdel Hamid                                   */
/* Date     : 26 / 2 / 2025                                     */
/* Email    : mohamedhamiid20@gmail.com                             */
/* Phone    : 01092301921                                           */
/* Brief    : Handling sending data through CAN.                    */
/* Copyright: Sponsored by Ejad                                     */
/********************************************************************/

#include "CAN_App.h"
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

  uint8_t i=0,j=0;
  uint8_t loc_u8CanData[8] = {0}; // Local array to store CAN message data

  loc_u8CanData[0] = Copy_u8NvmId; // Store device id in NVM

  // Store authentication and security properties
  loc_u8CanData[1] = Add_structKeys.addressType & 0x01;
  loc_u8CanData[1] |= (uint8_t)gAppOutAuth << 1;
  loc_u8CanData[1] |= (uint8_t)gAppOutLeSc << 2;

  // Store device address
  for (i = 0; i < gcBleDeviceAddressSize_c; i++)
    loc_u8CanData[i+2] = Add_structKeys.aAddress[i];

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE) && defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
  // Print title in shell
  shell_write("\n================= \nCAN send Bonding Data \n=================\n");
#else
  UART_SendMessage("\n================= \nCAN send Bonding Data \n=================\n");
#endif

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
 * @param deviceId Device ID.
 * @param procNo Process number.
 * @param result Struct containing localization algorithm results.
 */
void CAN_voidSendDistance(uint8_t deviceId, uint16_t procNo, localizationAlgoRun_t result, uint8_t Copy_u8PEorTDM){

  uint8_t loc_u8CanData[8] = {0}; // Local array for CAN message data

  loc_u8CanData[0] = deviceId;
  loc_u8CanData[1] = procNo & 0xff;
  loc_u8CanData[2] = (procNo >> 8) & 0xff;
  loc_u8CanData[3] = result.distanceIntegerPart;
  loc_u8CanData[4] = (result.distanceDecimalPart) & 0xff;
  loc_u8CanData[5] = (result.distanceDecimalPart >> 8) & 0xff;
  loc_u8CanData[6] = (uint8_t)(result.dqiPercentage * 100) & 0xff;
  loc_u8CanData[7] = ((uint8_t)(result.dqiPercentage * 100) >> 8) & 0xff;

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE) && defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
  // Print title in shell
  shell_write("\n================= \nCAN send Distance \n=================");
#else
  UART_SendMessage("\n================= \nCAN send Handover \n=================\n");
#endif

  if(Copy_u8PEorTDM == PE)
    // Send distance measurement triggered by passive entry over CAN
    CAN_voidSendMsg(CAN_ID_DISTANCE_PE, loc_u8CanData);
  else
    // Send distance measurement triggered by trigger distance measurement command over CAN
    CAN_voidSendMsg(CAN_ID_DISTANCE_TDM, loc_u8CanData);

#if (CAN_ENABLE_HANDOVER)
  // Track the number of handover messages sent
  Global_u8SendHandover++;
  if (Global_u8SendHandover == gCsProcRepeatMaxNumProcedures_c){
    CAN_voidHandover(deviceId);
    Global_u8SendHandover = 0;
  }
#endif
}
#endif
/**
 * @brief Sends commands over CAN bus, including receiver identifier.
 *
 * @param Copy_enuCommand Command ID.
 * @param Copy_u8ReceiverId Receiver Identifier.
 */
void CAN_voidSendCommand(CAN_tenumCommands Copy_enuCommand, uint8_t Copy_u8ReceiverId, uint8_t Copy_u8DeviceId){
  uint8_t loc_u8CanData[8] = {0};

  loc_u8CanData[0] = Copy_enuCommand;   // Store command ID
  loc_u8CanData[1] = Copy_u8ReceiverId; // Store receiver ID
  loc_u8CanData[2] = Copy_u8DeviceId;   // Store device ID

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE) && defined(gAppUseShellInApplication_d) && (gAppUseShellInApplication_d == 1)
   // Print title in shell
  shell_write("\n================= \nCAN send Command \n=================\n");
#else
  UART_SendMessage("\n================= \nCAN send Command \n=================\n");
#endif

  // Send command message
  CAN_voidSendMsg(CAN_ID_COMMANDS, loc_u8CanData);
}
