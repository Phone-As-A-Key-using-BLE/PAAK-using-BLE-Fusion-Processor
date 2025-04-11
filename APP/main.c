///*
// * Module Name: MASTER ECU1 Application
// * Author: MOHAMAD WALEED & ABDELRAHMAN
// * Purpose: Entry point for the Master ECU. Uses OS Layer for task management.
//*/

#include "OS/os.h"
#include "Connectivity/connectivity.h"
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
extern uint8_t lock;
extern uint8_t Loc_u8CurrentDeviceId;
extern uint8_t Global_u8CurrentAnchor;
 int main(void)
{
    OS_Init();

    // Main loop to keep checking for incoming messages
    while (1)
    {
        if(lock){
            lock=0;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Loc_u8CurrentDeviceId);
        }
        __asm("WFE");
    }
}
