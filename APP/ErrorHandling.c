#include <stdint.h>
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
extern uint8_t Global_u8CurrentAnchor;
extern uint8_t Global_u8ResetAndPE;
extern uint8_t isResetNeeded;
void ERR_voidTdmTimeoutHandler(){
    TimerDriver_Stop();
    UART_SendMessage("\nTimer Timeout: TDM\n");
    CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
}
void ERR_voidHandoverTimeoutHandler(){
    TimerDriver_Stop();
    UART_SendMessage("\nTimer Timeout: Handover\n");
    CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
}

void ERR_voidDisconnectTimeOutHandler(){
    TimerDriver_Stop();
    UART_SendMessage("\nTimeout: Disconnect\n");
    Global_u8ResetAndPE = 2;
}

