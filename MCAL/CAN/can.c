#include "CAN.h"
#include "CAN_config.h"
//#include <MCAL/Timers/TIMER0/timer0.h>
volatile bool isMessageSent = false;

// Function to initialize the CAN peripheral
void CAN_Init(void)
{
    // Enable the CAN module and the GPIO port based on the selected CAN module
    SysCtlPeripheralEnable(CAN_SYSCTL_PERIPH);
    SysCtlPeripheralEnable(CAN_GPIO_PORT_BASE);

    // Wait until the peripherals are ready
    while (!SysCtlPeripheralReady(CAN_SYSCTL_PERIPH))
    {
    }
    while (!SysCtlPeripheralReady(CAN_GPIO_PORT_BASE))
    {
    }

    // Configure GPIO pins for CAN RX and TX
    GPIOPinConfigure(CAN_GPIO_RX_CFG);
    GPIOPinConfigure(CAN_GPIO_TX_CFG);
    GPIOPinTypeCAN(CAN_PORT_BASE, CAN_GPIO_RX_PIN | CAN_GPIO_TX_PIN);

    // Initialize the CAN controller
    CANInit(CAN_BASE);

    // Set the bit rate for the CAN bus
    CANBitRateSet(CAN_BASE, SysCtlClockGet(), CAN_BIT_RATE);



    // Enable CAN interrupts if configured
#if CAN_INTERRUPT_MODE == CAN_INT_ENABLE
    CANIntRegister(CAN_BASE, CAN0_Handler); // use dynamic vector table allocation
    CANIntEnable(CAN_BASE, CAN_INT_FLAGS);
    IntEnable(CAN_INT);
#endif

    // Enable the CAN controller
    CANEnable(CAN_BASE);
}

// Function to send data via CAN
void CAN_SendMessage(uint32_t messageID, uint32_t msgObjectID, uint8_t *data,
                     uint8_t dataLength)
{
    tCANMsgObject messageObject;

    // Ensure the length is valid
    if (dataLength > 8)
    {
        dataLength = 8; // CAN data payload cannot exceed 8 bytes
    }

    // Set up the message object for transmission
    messageObject.ui32MsgID = messageID;
    messageObject.ui32MsgIDMask = 0; // No filtering for transmission & No mask
    messageObject.ui32Flags = MSG_OBJ_TX_INT_ENABLE; // Enable TX interrupt
    messageObject.ui32MsgLen = dataLength;
    messageObject.pui8MsgData = data;

    // Send the message
    CANMessageSet(CAN_BASE, msgObjectID, &messageObject, MSG_OBJ_TYPE_TX);

    uint32_t status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL); // read back error bits, do something with them?
    status = 0;
}

// Function to initialize CAN for receiving messages
void CAN_ReceiveInit(void)
{
    tCANMsgObject messageObject;

    // Set up the message object for reception
    messageObject.ui32MsgID = CAN_RX_MESSAGE_ID;
    messageObject.ui32MsgIDMask = CAN_RX_MESSAGE_MASK;
    messageObject.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    messageObject.ui32MsgLen = CAN_DATA_LENGTH;

    // Configure the CAN to receive the message
    CANMessageSet(CAN_BASE, MSG_OBJ_TX_1, &messageObject, MSG_OBJ_TYPE_RX);
}

void CAN_ConfigureReceiveObjects(void)
{
    tCANMsgObject msgObject;

//    // Message Object 1: Keep-alive messages (ID 0x101)
//    msgObject.ui32MsgID = CAN_KEEP_ALIVE_ID;
//    msgObject.ui32MsgIDMask = CAN_RX_MESSAGE_MASK; // Match exact ID
//    msgObject.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
//    msgObject.ui32MsgLen = CAN_DATA_LENGTH; // Maximum CAN data length
//    CANMessageSet(CAN_BASE, CAN_KEEP_ALIVE_OBJ, &msgObject, MSG_OBJ_TYPE_RX);

    // Message Object 2: Temperature data (ID 0x102)
    msgObject.ui32MsgID = CAN_TEMP_ID;
    msgObject.ui32MsgIDMask = CAN_RX_MESSAGE_MASK; // Match exact ID
    msgObject.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
    msgObject.ui32MsgLen = CAN_DATA_LENGTH;
    CANMessageSet(CAN_BASE, 5, &msgObject, MSG_OBJ_TYPE_RX);
}

void CAN_voidSendMsg(uint16_t Copy_u16MsgId, uint8_t * Add_u8Data) {

    CAN_SendMessage(Copy_u16MsgId, MSG_OBJ_TX_1, Add_u8Data, 8);

}



