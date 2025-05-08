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
void HandoverTimeoutHandler(){
    TimerDriver_Stop();
    CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, CAN_ANCHOR_MAX, Global_u8CurrentDeviceId);
    CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_ANCHOR_1, Global_u8CurrentDeviceId);
}
void HandoverTimeoutSecondaryHandler(){
    TimerDriver_Stop();
    CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor-1, Global_u8CurrentDeviceId);
}