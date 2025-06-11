/***********************************************
 * Module Name: OS Layer
 * Purpose: Task Scheduler and Application Manager
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/

#include "os.h"
#include "driverlib/interrupt.h"


volatile SystemState currState = STATE_NORMAL;

//volatile uint32_t sysCounter = 0;  // Define the SysTick counter


// Global Flags
volatile bool flag_heartbeat = false;          // Send heartbeat
volatile bool flag_receiveTemperature = false; // Receive temperature

uint8_t heartbeatBuffer[1] = {0xAA};  // Heartbeat value
uint8_t temperatureData = 0;          // Temperature data buffer

uint8_t overHeat = 0x01;
uint8_t normalState = 0x02;
uint8_t faultState = 0x03;
uint8_t BlinkState = 0x07;

uint32_t BlueCounter = 0;
uint32_t RedCounter = 0;
uint32_t systemTimeMs = 0;
uint32_t CheckOverheatCounter = 0;
uint32_t whiteCounter = 0;

volatile bool flag_BlueBlink = false;
volatile bool flag_WhiteBlink = false;
volatile bool flag_RedBlink = false;
volatile bool ADC_HealthGOOD = false;



/* Global Variables */
bool last_sw1_state = false;
bool last_sw2_state = false;
volatile uint32_t OVcounter = 0;
volatile uint32_t FScounter = 0;
volatile uint32_t CLcounter = 0;
bool dtcRecorded = false;
const uint32_t dtcCode = 1; // Example DTC Code
//static uint32_t ctrBuffer;
static uint32_t dtcBuffer;

uint8_t receivedData[8];
uint32_t overheatDTC_counter = 0;
uint32_t FaultStateDTC_counter = 0;
uint32_t CommLoss_Counter = 0;

// Initialize system and restore state from EEPROM
void InitializeSystem(void) {

    uint32_t savedOVCounter;
    uint32_t savedFSCounter;
    uint32_t savedCLCounter;
    // uint32_t savedFlag = 0;

    // Read the current button press counter from EEPROM
    EEPROMRead(&savedOVCounter, OVERHEAT_COUNTER_ADDR, sizeof(savedOVCounter));
    OVcounter = savedOVCounter;
    EEPROMRead(&savedFSCounter, FaultState_COUNTER_ADDR, sizeof(savedFSCounter));
    FScounter = savedFSCounter;
    EEPROMRead(&savedCLCounter, COMMLOSS_COUNTER_ADDR, sizeof(savedCLCounter));
    CLcounter = savedCLCounter;
    // Restore counter


    // Restore dtcRecorded flag
    if (OVcounter == 3 || FScounter == 1 || CLcounter == 3) {
        dtcRecorded = true;
        NVM_SaveDTC((uint32_t *)&dtcCode, DTC_CODE_ADDR); // Save the DTC code
    } else {
        dtcRecorded = false;
    }

    // Indicate startup state
    if (dtcRecorded) {
        UART_SendMessage("DTC detected on startup.\r\n");
        LED_ON(LED_RED);
    }
}

// Display the tester mode menu
void DisplayMenu(void) {
    UART_SendMessage("\n--- Tester Mode Menu ---\r\n");
    UART_SendMessage("1. Clear All DTCs\r\n");
    UART_SendMessage("2. Display Recorded DTCs\r\n");
    UART_SendMessage("3. Print History logs\r\n");
    UART_SendMessage("4. Blink Led\r\n");
    UART_SendMessage("5. Display Tempreture\r\n");
    UART_SendMessage("6. Blink Led on ECU2\r\n");

    UART_SendMessage("9. Exit Tester Mode\r\n");
    UART_SendMessage("Enter your choice: ");
}

// Execute options from the tester mode menu
void ExecuteOption(char option) {
    static uint32_t counter;
    switch (option) {
    case '1': { // Clear all DTCs
        NVM_ClearAllDTCs();
        overheatDTC_counter = 0;
        FaultStateDTC_counter = 0;
        CommLoss_Counter = 0;
        dtcRecorded = false;
        NVM_SaveDTC(&overheatDTC_counter, OVERHEAT_COUNTER_ADDR);       // Save counter
        NVM_SaveDTC(&FaultStateDTC_counter, FaultState_COUNTER_ADDR);       // Save counter
        NVM_SaveDTC(&CommLoss_Counter, COMMLOSS_COUNTER_ADDR);       // Save counter
        LED_OFF(LED_WHITE);
        UART_SendMessage("All DTCs Cleared.\r\n");
        CANEnable(CAN0_BASE);
        break;
    }
    case '2': { // Display recorded DTCs
        if (NVM_RetrieveDTC(&dtcBuffer, DTC_CODE_ADDR)) {
            UART_SendMessage("DTC: ");
            UART_SendNumber(dtcBuffer);
            UART_SendMessage("\n");
        } else {
            UART_SendMessage("No DTCs recorded.\r\n");
        }
        break;
    }
    case '3':{ // display error history
        EEPROMRead(&counter, OVERHEAT_COUNTER_ADDR, 8);
        if (counter > 0) {
            UART_SendMessage("COUNTER: ");
            UART_SendNumber(counter);
            UART_SendMessage("\n");
        } else {
            UART_SendMessage("No History recorded.\r\n");
        }
        break;
    }
    case '4':{ // Manipulate GPIO
        LED_ON(LED_BLUE);
        break;
    }
    case '5':{ // Display last tempreture
        UART_SendNumber(receivedData[0]);
        break;
    }
    case '6':{ // Blink Led on ECU2
        //UART_SendNumber(receivedData[0]);
        CAN_SendMessage(0x600,6,&BlinkState,8);
        break;
    }
    case '9': // Exit tester mode
        LED_OFF(LED_WHITE);
        break;
    default:
        UART_SendMessage("Invalid option! Try again.\r\n");
        break;
    }
}


// Enter tester mode to handle DTC-related operations
void EnterTesterMode(void) {
    char option;
    while (1) {
        DisplayMenu();
        option = UART_RecieveMessage();
        ExecuteOption(option);
        if (option == '9') { // Exit tester mode
            UART_SendMessage("Exiting Tester Mode.\r\n");
            break;
        }
    }
}

/***********************************************
 * Function Definitions
 ***********************************************/

/*********************************
 * Function: HandleSW2Press
 * Purpose: Handle user pressing SW2 to enter Tester Mode
 *********************************/

// Handle SW2 Press
void HandleSW2Press(void) {
    UART_SendMessage("Entering Tester Mode\r\n");
    LED_BLINK_NEW(LED_WHITE);
    EnterTesterMode(); // Enter tester mode
}

/*********************************
 * Function: TesterMode
 * Purpose: Handle Tester Mode logic
 *********************************/
void TesterMode(void) {
    /**************Tester MODE*********************/
    bool sw1 = PushButton_Read(BUTTON_SW1); // Read SW1
    bool sw2 = PushButton_Read(BUTTON_SW2); // Read SW2

    // Handle SW2 Press
    if ((PushButton_Read(BUTTON_SW1) && PushButton_Read(BUTTON_SW2))) {
        HandleSW2Press();
    }

    // Indicate DTC state if recorded
    if (dtcRecorded) {
        UART_SendMessage("DTC Detected! Hold SW1 and SW2 to Enter Tester Mode\r\n");
        LED_ON(LED_RED);
        while (!(PushButton_Read(BUTTON_SW1) && PushButton_Read(BUTTON_SW2)));
        HandleSW2Press();
    }

    // Update last button states
    last_sw1_state = sw1;
    last_sw2_state = sw2;

}

/*********************************
 * Function: OS_voidCANHandleReceivedMessages
 * Purpose: Process received CAN messages
 *********************************/
void OS_voidCANHandleReceivedMessages(void) {

    tCANMsgObject msgObject;

    // Temporary storage for message data
    msgObject.pui8MsgData = receivedData;

    // Check Message Object
    CANMessageGet(CAN_BASE, 5, &msgObject, true);


    if (msgObject.ui32Flags & MSG_OBJ_NEW_DATA) {

        if (msgObject.ui32MsgID == 0x200)
        {
            if (receivedData[0] > 30)
            {
                if(flag_WhiteBlink){
                    CheckOverheatCounter = 0;
                }

                if (CheckOverheatCounter >= 3000)
                {
                    if(ADC_HealthGOOD){
                        UART_SendMessage("OverHeat \r\n");
                        overheatDTC_counter++;
                        NVM_SaveDTC(&overheatDTC_counter, OVERHEAT_COUNTER_ADDR);       // Save counter
                        if (overheatDTC_counter >= 3) {    // Trigger DTC logic
                            dtcRecorded = true;
                            NVM_SaveDTC(&overheatDTC_counter, OVERHEAT_COUNTER_ADDR);       // Save counter
                            NVM_SaveDTC((uint32_t *)&dtcCode, DTC_CODE_ADDR); // Save the DTC code
                            UART_SendMessage("OverHeat DTC Recorded\r\n");
                        }
                        flag_WhiteBlink = true;
                        CAN_SendMessage(0x600,6,&overHeat,8);
                    }
                    else{
                        //fault state led red blink
                        //can disable
                        FaultStateDTC_counter = 1;
                        if (FaultStateDTC_counter >= 1) {    // Trigger DTC logic
                            dtcRecorded = true;
                            NVM_SaveDTC(&FaultStateDTC_counter, FaultState_COUNTER_ADDR);       // Save counter
                            NVM_SaveDTC((uint32_t *)&dtcCode, DTC_CODE_ADDR); // Save the DTC code
                            UART_SendMessage("FAULT STATE DTC Recorded\r\n");

                            CANDisable(CAN0_BASE);

                        }
                        CAN_SendMessage(0x600,6,&faultState,8);
                        flag_RedBlink = true;
                    }
                }
            }
            else{

                CAN_SendMessage(0x600,6,&normalState,8);
                CheckOverheatCounter = 0;
            }
        }

        if(msgObject.ui32MsgID == 0x500) //Known Voltage
        {
            if(receivedData[0] < 2)
            {
                ADC_HealthGOOD = false;
            }
            else{
                ADC_HealthGOOD = true;
            }
        }

    }
}



/*********************************
 * SysTick Timer (Increments systemTime)
 *********************************/
// SysTick ISR
void SysTick_Handler(void) {
    //static uint32_t heartbeatCounter = 0;
    sysCounter++;
    CheckOverheatCounter++;
    whiteCounter++;
    systemTimeMs++;
    RedCounter++;
    BlueCounter++;
}

/*********************************
 * Function: Task_SendHeartbeat
 * Purpose: Send periodic heartbeat message
 *********************************/

// Task: Send Heartbeat
void Task_SendHeartbeat(void) {

    if(sysCounter % 100 == 0){
        uint8_t heartbeat[] = {0x01};
        CAN_SendMessage(0x100,1,heartbeat,8);
    }
}

/*********************************
 * Function: CheckOverHeat
 * Purpose: Manage overheat LED blinking
 *********************************/
void CheckOverHeat(void){
    if(flag_WhiteBlink){
        if(whiteCounter>= 20000){
            flag_WhiteBlink= false;
        }
        LED_BLINK_NEW(LED_WHITE);
    }
    else{
        //LED_OFF(LED_WHITE);
        whiteCounter = 0;
    }
}

/*********************************
 * Function: FaultState
 * Purpose: Manage fault state LED blinking
 *********************************/
void FaultState(void){
    if(flag_RedBlink && !flag_WhiteBlink){
        if(RedCounter>= 20000){
            flag_RedBlink= false;
        }
        LED_OFF(LED_WHITE);
        LED_BLINK_NEW(LED_RED);
    }
    else{
        //LED_OFF(LED_WHITE);
        RedCounter = 0;
    }
}

/*********************************
 * Function: CommLoss
 * Purpose: Handle communication loss blinking
 *********************************/
void CommLoss(void){
    if(flag_BlueBlink && !flag_WhiteBlink){
        if(BlueCounter>= 20000){
            flag_BlueBlink= false;
        }

        LED_BLINK_NEW(LED_BLUE);
    }
    else{
        //LED_OFF(LED_WHITE);
        RedCounter = 0;
    }
}

/*********************************
 * Function: OS_voidCheckTXOK
 * Purpose: Check CAN transmission status
 *********************************/
void OS_voidCheckTXOK(void)
{
    uint32_t status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
    if (status & CAN_STATUS_TXOK) {
        LED_ON(LED_GREEN);
    }
    else if (status & CAN_STATUS_LEC_ACK) {
        flag_BlueBlink = true;
        CommLoss_Counter++;
        NVM_SaveDTC(&CommLoss_Counter, COMMLOSS_COUNTER_ADDR);       // Save counter
        if (CommLoss_Counter >= 3) {    // Trigger DTC logic
            dtcRecorded = true;
            NVM_SaveDTC(&CommLoss_Counter, COMMLOSS_COUNTER_ADDR);       // Save counter
            NVM_SaveDTC((uint32_t *)&dtcCode, DTC_CODE_ADDR); // Save the DTC code
            UART_SendMessage("Communication Loss DTC Recorded\r\n");
        }

    }
}

/*********************************
 * Function: OS_Init
 * Purpose: Initialize system and peripherals
 *********************************/
void OS_Init(void) {
    //systemTime = 0;
    IntMasterDisable();
    // Initialize system clock
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);
    // Initialize SysTick
    SysTick_Init();
    // Initialize peripherals
    GPIO_InitPort(GPIO_PORTF_BASE);
    LED_Init();
    PushButtons_Init();
    NVM_Init();
    TimerDriver_Init();
    DeviceStateManager_Init();
    CAN_Init();
    CAN_ReceiveInit();
    UART_Init();
    IntMasterEnable();


    //CAN_ConfigureReceiveObjects();

    //InitializeSystem();
}

/*********************************
 * Function: OS_RunScheduler
 * Purpose: Main OS loop to run tasks
 *********************************/
// OS Scheduler
void OS_RunScheduler(void) {
    while (1) {

        Task_SendHeartbeat();
        OS_voidCANHandleReceivedMessages();
        CheckOverHeat();
        FaultState();
        CommLoss();
        TesterMode();
        OS_voidCheckTXOK();
    }
}



