///*
// * Module Name: MASTER ECU1 Application
// * Author: MOHAMAD WALEED & ABDELRAHMAN
// * Purpose: Entry point for the Master ECU. Uses OS Layer for task management.
//*/

#include "CAN_App.h"
#include "OS/os.h"
#include "Connectivity/connectivity.h"
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/APP_FSM.h"
extern uint8_t lock;
extern uint8_t lock1;
extern uint8_t Loc_u8CurrentDeviceId;
extern uint8_t Global_u8CurrentAnchor;
extern APP_tenuStates currentState;
extern uint8_t isBondingDataReceived;
 int main(void)
{
    OS_Init();
    isBondingDataReceived=1;
    
    CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);

    while (1)
    {
        if(lock){
            lock=0;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Loc_u8CurrentDeviceId);
        }
        if(lock1){
            lock1=0;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Loc_u8CurrentDeviceId);        
        }
        __asm("WFE");
    }
}
