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
void HandoverTimeoutHandler(){
    CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);
}
