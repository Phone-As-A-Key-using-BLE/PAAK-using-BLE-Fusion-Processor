///*
// * Module Name: MASTER ECU1 Application
// * Author: MOHAMAD WALEED & ABDELRAHMAN
// * Purpose: Entry point for the Master ECU. Uses OS Layer for task management.
//*/



#include "OS/os.h"
#include "Connectivity/connectivity.h"


//uint32_t x = 66;
//uint8_t temp;
//uint32_t z[] = {1,2,3,4};
//float y = 6.5;
//CONNECTIVITY_Message_t test = {
//    .command = VEHICLE_SEND_STATUS,
//    .status = {1, 2, 3, 4, 5} // Directly initializing status
//};
//int main(void) {
//    OS_Init();        // Initialize OS and peripherals
//    UART_SendMessage("Sent Message: ");
//    UART_SendNumber(x);
//
//    //OS_RunScheduler(); // Start the OS scheduler
//    CONNECTIVITY_SendData(&test);
//    //UART_SendFloat(y);
//    //temp = UART_RecieveMessage();
//    while(1)
//    {
//        //CONNECTIVITY_SendData(&test);
//        temp = UART_RecieveMessage();
//    }
//    //return 0;
//}



// // Reciever Code
//void CAN0_Handler(void) {
//    uint32_t ui32Status;
//    uint32_t ui32IntStatus;
//    tCANMsgObject msgObject;
//    uint8_t receivedData[8];  // Temporary buffer to store received data
//
//    ui32Status = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL); // read back error bits, do something with them?
//
//
//
//    // Get the received message (for message object 1, you can change this if needed)
//    msgObject.pui8MsgData = receivedData;
//    CANMessageGet(CAN0_BASE, 1, &msgObject, true);  // 'true' means clear interrupt flag
//        LED_ON(LED_BLUE);
//
//
//
//        // Check if new data is available
//        if (msgObject.ui32Flags & MSG_OBJ_NEW_DATA) {
//
//            // Process message based on its ID
//            if (msgObject.ui32MsgID == 0x100) {
//                // Assuming receivedData[0] contains the number to print
//                UART_SendMessage("Received number: ");
//                uint8_t i=0;
//                for(i=0;i<8;i++){
//                    UART_SendNumber(receivedData[i]);  // Print the received number
//                    UART_SendMessage(" ");
//                }
//                UART_SendMessage("\n");
//            } else {
//                // Handle other message IDs or default behavior
//                UART_SendMessage("Received unknown message ID\n");
//            }
//        }
//}

volatile bool rxFlag = 0; // msg recieved flag
volatile bool errFlag = 0; // error flag



#define CAN_INT_INTID_STATUS   0x00008000


 int main(void)
{
    OS_Init();        // Initialize OS and peripherals


    // Print a message to indicate the system is running
    UART_SendMessage("CAN Receiver started!\n\r");

    // Main loop to keep checking for incoming messages
    while (1)
    {
        // The receiver will handle interrupts when a message is received.
        // Nothing needs to be done here in the main loop as interrupts will trigger.
    }

    return 0;
}



 // Sender Code
// Define a simple message data to send


//uint32_t messageData[1] = {0};
//
//int main(void)
//{
//    OS_Init();        // Initialize OS and
//    //LED_ON(LED_RED);
//
//    // Enable global interrupts (required for CAN interrupts)
//    IntMasterEnable();
//
//    // Print a message to indicate the system is running
//    UART_SendMessage("CAN Transmitter started!\n\r");
//
//    // Send a message every second
//    while (1)
//    {
//        //UART_SendMessage("im am alive: ");
//        // Increment the message data (for testing purposes)
//        messageData[0]++;
//
//        // Send the message with ID 0x101 (you can modify this as needed)
//        UART_SendMessage("Sending message...\n");  // Debugging
//        CAN_SendMessage(0x500, 5, (uint8_t *)messageData, 8);
//
//
//
//        //UART_SendMessage("Message sent.\n");  // Debugging
//        LED_ON(LED_GREEN);
//        // Print message sent
//       // UART_SendMessage("Sent Message: ");
//        UART_SendNumber(messageData[0]);
//
//        // Delay for 1 second before sending the next message
//        //SysCtlDelay(SysCtlClockGet() / 3); // Delay ~1 second
//    }
//
//    return 0;
//}
//
//
