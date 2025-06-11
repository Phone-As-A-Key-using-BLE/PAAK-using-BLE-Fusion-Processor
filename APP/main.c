#include "CAN_App.h"
#include "Connectivity/connectivity.h"
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/APP_FSM.h"
#include "APP_SYS.h"


extern uint8_t isBondingDataReceived;

int main(void)
{
    APP_voidSystemInit();
    isBondingDataReceived=1;
    
    CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);

    while (1)
    {
        APP_voidCommandsHandler();
        __asm(" WFE \n");
    }
}
