#include "CAN_App.h"
#include "OS/os.h"
#include "Connectivity/connectivity.h"
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/APP_FSM.h"
extern uint8_t Global_u8SendPE;
extern uint8_t Global_u8SendTDM;
extern uint8_t Global_u8SendHandover;
extern uint8_t Global_u8CurrentDeviceId;
extern uint8_t Global_u8CurrentAnchor;
extern APP_tenuStates currentState;
extern uint8_t isBondingDataReceived;
extern uint8_t Global_u8HandoverTo;
extern uint8_t Global_u8isWaitingForTDM[CAN_ANCHOR_MAX + 1];
extern uint8_t Global_u8Event;
 int main(void)
{
    OS_Init();
    isBondingDataReceived=1;
    
    CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);

    while (1)
    {
        if(Global_u8SendPE){
            Global_u8SendPE = 0;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
        }
        if(Global_u8SendTDM){
            Global_u8SendTDM = 0;
            Global_u8isWaitingForTDM[Global_u8CurrentAnchor] = 1;
            CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);        
        }
        if(Global_u8SendHandover){
            Global_u8SendHandover = 0;
            CAN_voidSendHandoverCommand(Global_u8CurrentAnchor, Global_u8HandoverTo, Global_u8CurrentDeviceId);
        }
        if(Global_u8Event!=0){
            Global_u8Event = 0;
            APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD);
        }
        __asm("WFE");
    }
}