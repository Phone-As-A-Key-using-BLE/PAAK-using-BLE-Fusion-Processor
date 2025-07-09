#include <stdint.h>
#include <stdbool.h>
#include "APP_FSM.h"
#include "can.h"
#include "UART/uart.h"
#include "LED/led.h"
#include "GPIO/gpio.h"
#include "PUSHBUTTON/pushbuttons.h"
#include "DeviceStateManager.h"
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
extern uint8_t Global_u8NextAnchor;
extern APP_tenuRangingType Global_u8DevicesRangingType[APP_MAX_NO_OF_DEVICES];
static void APP_voidInitRangingTypes(){
    uint8_t Loc_u8Devices;
    for (Loc_u8Devices = 0; Loc_u8Devices < APP_MAX_NO_OF_DEVICES; Loc_u8Devices++) {
        int8_t Loc_u8RangingType = DeviceStateManager_Load(Loc_u8Devices);
        if(Loc_u8RangingType == -1)
            Global_u8DevicesRangingType[Loc_u8Devices] = APP_NOT_DETERMINED;
        else
            Global_u8DevicesRangingType[Loc_u8Devices] = (APP_tenuRangingType)Loc_u8RangingType;
    }
}
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
    APP_voidInitRangingTypes();
    currentState = STATE_START_FSM;
}

void APP_voidCommandsHandler(){
    if(Global_u8SendPE == 1){
        __asm(" CPSID I \n");
        Global_u8SendPE = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8NextAnchor, Global_u8CurrentDeviceId);
        __asm(" CPSIE I \n");
    }
    if(Global_u8SendTDM == 1){
        __asm(" CPSID I\n");
        Global_u8SendTDM = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Global_u8CurrentDeviceId);      
        __asm(" CPSIE I \n");
    }
    if(Global_u8SendHandover == 1){
        __asm(" CPSID I \n");
        Global_u8SendHandover = 0;
        CAN_voidSendHandoverCommand(Global_u8CurrentAnchor, Global_u8NextAnchor, Global_u8CurrentDeviceId);
        __asm(" CPSIE I \n");
    }
    if(Global_u8ResetAndPE == 2){
        Global_u8ResetAndPE = 3;
        CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0);
    }
    if(Global_u8ResetAndPE == 1){
        Global_u8ResetAndPE = 0;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, Global_u8NextAnchor, Global_u8CurrentDeviceId);
    }
}
