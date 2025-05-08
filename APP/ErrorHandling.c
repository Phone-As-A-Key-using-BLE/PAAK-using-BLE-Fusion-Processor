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
extern uint8_t isPeRunning;
void HandoverTimeoutHandler(){
    TimerDriver_Stop();
    UART_SendMessage("\nTimeout: TDM\n");
    CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
}
void HandoverTimeoutSecondaryHandler(){
    TimerDriver_Stop();
    UART_SendMessage("\nTimeout: Handover\n");
    CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor-1, Global_u8CurrentDeviceId);
}
