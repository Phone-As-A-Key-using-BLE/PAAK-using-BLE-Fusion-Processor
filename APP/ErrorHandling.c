#include <string.h>
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "DeviceRangingTypeManager.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"
#include "APP/Timer.h"
#include "APP/ErrorHandling.h"
extern uint8_t Global_u8CurrentDeviceId;
extern uint8_t currentState;
extern uint8_t Global_u8HandoverTo;
extern uint8_t Global_u8CurrentAnchor;
extern uint8_t Global_u8HandoverTo;
extern uint8_t Global_u8isAnchorConnected[CAN_ANCHOR_MAX+1];
extern uint8_t Global_u8isWaitingForTDM[CAN_ANCHOR_MAX+1];
extern uint8_t Global_u8HandoverFailCount[CAN_ANCHOR_MAX + 1];
extern char gArr_DebugMsg[1024];
void HandoverTimeoutHandler(){
    Global_u8HandoverFailCount[Global_u8CurrentAnchor]=0;
    Global_u8isAnchorConnected[Global_u8CurrentAnchor] = 0;
    Global_u8isAnchorConnected[Global_u8HandoverTo] = 1;
    Global_u8CurrentAnchor = Global_u8HandoverTo;
    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
            "\n[INFO] Handover Timeout: start PE on Anchor %d\n", Global_u8CurrentAnchor);
    UART_SendMessage(gArr_DebugMsg);
    CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);

}

void TDMTimeoutHandler(){
    Global_u8isWaitingForTDM[Global_u8CurrentAnchor]=0;
    Global_u8isAnchorConnected[Global_u8CurrentAnchor] = 1;
     snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),
            "\n[INFO] TDM Timeout: start PE on Anchor %d\n", Global_u8CurrentAnchor);
    CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);

}
