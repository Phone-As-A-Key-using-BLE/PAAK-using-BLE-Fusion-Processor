#include <stdint.h>
#include <stdbool.h>
#include "can.h"
#include "UART/uart.h"
#include "LED/led.h"
#include "GPIO/gpio.h"
#include "PUSHBUTTON/pushbuttons.h"
#include "DeviceRangingTypeManager.h"
#include "Timer.h"
#include "can_msg_types.h"
#include "CAN_Send.h"
#include "driverlib/interrupt.h"

extern uint8_t Global_u8SendPE;
extern uint8_t Global_u8SendTDM;
extern uint8_t Global_u8SendHandover;
extern uint8_t Global_u8CurrentDeviceId;
extern uint8_t Global_u8CurrentAnchor;
extern APP_tenuStates currentState;
extern uint8_t isBondingDataReceived;
extern uint8_t Global_u8ResetAndPE;

void APP_voidSystemInit(){ 

    // Initialize system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // Initialize peripherals
    GPIO_InitPort(GPIO_PORTF_BASE);
    LED_Init();
    PushButtons_Init();
    CAN_Init();
    CAN_ReceiveInit();
    IntMasterEnable();
    UART_Init();
    TimerDriver_Init();
    DeviceStateManager_Init();

}

void APP_voidCommandsHandler(){
    if(Global_u8SendPE){
        __asm(" CPSID I \n");
        Global_u8SendPE = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
        __asm(" CPSIE I \n");
    }
    if(Global_u8SendTDM){
        __asm(" CPSID I\n");
        Global_u8SendTDM = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);      
        __asm(" CPSIE I \n");
    }
    if(Global_u8SendHandover){
        __asm(" CPSID I \n");
        Global_u8SendHandover = 0;
        if(Global_u8CurrentAnchor > CAN_ANCHOR_MAX){
            CAN_voidSendHandoverCommand(CAN_ANCHOR_MAX, CAN_ANCHOR_1, Global_u8CurrentDeviceId);
        }
        else
            CAN_voidSendHandoverCommand(Global_u8CurrentAnchor - 1, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
        __asm(" CPSIE I \n");
    }
    if(Global_u8ResetAndPE == 2){
        Global_u8ResetAndPE = 3;
        CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0);
    }
    if(Global_u8ResetAndPE == 1){
        Global_u8ResetAndPE = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);
    }
}
