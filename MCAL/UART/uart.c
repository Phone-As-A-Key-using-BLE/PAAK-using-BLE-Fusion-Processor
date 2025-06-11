/***********************************************
 * Module Name: UART
 * Author: Waleed
 * Purpose: initialize UART
 ***********************************************/


#include "uart.h"
#include "String.h"
#include "UART/uart_cfg.h"
extern void UART1_Handler(void);
/***********************************************
 * Function Name: UART_INIT
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Initializes the UART0 peripheral with the following settings: system clock set to 50 MHz,
 * UART0 configured for 9600 baud rate, 8 data bits, no parity, and 1 stop bit. It also enables the GPIO pins for UART0 RX (PA0) and TX (PA1) and enables the UART0 peripheral for communication.
 ***********************************************/
void UART_Init(void)
{
    // 1. Set the system clock to 50 MHz (if not already done globally)
    SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);

    // 2. Enable GPIO ports used by UART0 and UART1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  // UART0: PA0, PA1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);  // UART1: PB0, PB1

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOB)) {}

    // 3. Configure GPIO pins for UART0
    GPIOPinConfigure(GPIO_PA0_U0RX);
    GPIOPinConfigure(GPIO_PA1_U0TX);
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 4. Configure GPIO pins for UART1
    GPIOPinConfigure(GPIO_PB0_U1RX);
    GPIOPinConfigure(GPIO_PB1_U1TX);
    GPIOPinTypeUART(GPIO_PORTB_BASE, GPIO_PIN_0 | GPIO_PIN_1);

    // 5. Enable UART modules
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);

    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART0)) {}
    while (!SysCtlPeripheralReady(SYSCTL_PERIPH_UART1)) {}

    // 6. Configure UART modules (assumes UART_BASE_ADDRESS and UART_CONNECTIVITY_BASE_ADDRESS are defined)
    UARTConfigSetExpClk(UART_BASE_ADDRESS, SysCtlClockGet(), UART_DEFAULT_BAUD_RATE, UART_DEFAULT_CONFIG);
    UARTConfigSetExpClk(UART_CONNECTIVITY_BASE_ADDRESS, SysCtlClockGet(), UART_DEFAULT_BAUD_RATE, UART_DEFAULT_CONFIG);

    // 7. Clear any pending UART interrupts before enabling
    UARTIntClear(UART_CONNECTIVITY_BASE_ADDRESS, UARTIntStatus(UART_CONNECTIVITY_BASE_ADDRESS, true));

    // 8. Register UART1 ISR (optional, if using dynamic vector table)
    UARTIntRegister(UART_CONNECTIVITY_BASE_ADDRESS, UART1_Handler);  // only if vector table is dynamic

    // 9. Enable UART1 interrupts
    UARTIntEnable(UART_CONNECTIVITY_BASE_ADDRESS, UART_INT_RX | UART_INT_RT);
    IntEnable(INT_UART1);

    // 10. Enable UART modules
    UARTEnable(UART_BASE_ADDRESS);
    UARTEnable(UART_CONNECTIVITY_BASE_ADDRESS);
}


/***********************************************
 * Function Name: UARTSendMessage
 * Inputs: const char *array (message to send)
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Sends a message (string) through UART0 by transmitting each character in the string sequentially.
 * The transmission continues until the null terminator ('\0') is encountered.
 ***********************************************/
void UART_SendMessage(const char *array)
{
    while (*array)
    {
        UARTCharPut(UART_BASE_ADDRESS, *array); // Transmit each character
        array++;
    }
}

void UART_ConnectivitySendMessage(const char *array)
{
    while (*array)
    {
        UARTCharPut(UART_CONNECTIVITY_BASE_ADDRESS, *array); // Transmit each character
        array++;
    }
}

/***********************************************
 * Function Name: UARTRecieveMessage
 * Inputs: N/A
 * Outputs: char (received character)
 * Reentrancy: Non-Reentrant
 * Synchronous: Synch
 * Description: Waits until a character is available to be received via UART0, then retrieves and returns the received character.
 * This function blocks until data is received.
 ***********************************************/
int32_t UART_RecieveMessage(void) {
    /* Wait for data to be available*/
    while (!UARTCharsAvail(UART_CONNECTIVITY_BASE_ADDRESS)){}

    /* Read and return the received character*/
    return UARTCharGet(UART_CONNECTIVITY_BASE_ADDRESS);
}

void UART_SendNumber(uint32_t number) {
    char buffer[12];  // Buffer to hold the string representation of the number (max 10 digits + sign + null terminator)
    int index = 0;
    int i = 0;
    // Handle special case if number is 0
    if (number == (uint32_t)0) {
        buffer[index++] = '0';
    } else {
        // Convert the number to a string (reverse order)
        while (number > (uint32_t)0) {
            buffer[index++] = (number % (uint32_t)10) + '0';  // Convert the last digit to a character
            number /= (uint32_t)10;  // Remove the last digit
        }
    }

    buffer[index] = '\0';  // Null-terminate the string

    // Reverse the buffer to get the correct order
    for (i = 0; i < index / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[index - 1 - i];
        buffer[index - 1 - i] = temp;
    }

    // Send the string using UART
    UART_SendMessage(buffer);  // Assuming UART_SendMessage is your function to send strings
    UART_SendMessage("\r\n");     // Print�the�unit
}


void ftoa(float number, char *buffer, int decimalPlaces) {
    int integerPart = (int)number;  // Extract integer part
    float fractionalPart = number - (float)integerPart;  // Get the decimal part

    // Convert integer part to string
    sprintf(buffer, "%d.", integerPart);

    // Convert fractional part to string
    fractionalPart *= (float)pow(10, decimalPlaces);  // Shift decimal places
    char fracBuffer[10];
    sprintf(fracBuffer, "%d", (int)fractionalPart);

    // Append fractional part to buffer
    strcat(buffer, fracBuffer);
}

void UART_SendFloat(float number) {
    char buffer[20];  // Buffer to hold the converted float string
    ftoa(number, buffer, 2);  // Convert float to string with 2 decimal places
    UART_SendMessage(buffer);  // Send string over UART
    UART_SendMessage("\r\n");  // New line for readability
}
void UART_SendMessageConnectivity(const char *array, uint16_t stop)
{
    uint16_t index = 0;
    while (*array)
    {
        UART_SendNumberConnectivity(*array);
        array++;
        if(index == stop){
            break;
        }
        index++;
    }
}
void UART_SendNumberConnectivity(uint8_t number) {
    char buffer[12];  // Buffer to hold the string representation of the number (max 10 digits + sign + null terminator)
    int index = 0;
    int i = 0;
    // Handle special case if number is 0
    if (number == (uint8_t)0) {
        buffer[index++] = '0';
    } else {
        // Convert the number to a string (reverse order)
        while (number > (uint8_t)0) {
            buffer[index++] = (number % (uint8_t)10) + '0';  // Convert the last digit to a character
            number /= (uint8_t)10;  // Remove the last digit
        }
    }

    buffer[index] = '\0';  // Null-terminate the string

    // Reverse the buffer to get the correct order
    for (i = 0; i < index / 2; i++) {
        char temp = buffer[i];
        buffer[i] = buffer[index - 1 - i];
        buffer[index - 1 - i] = temp;
    }

    // Send the string using UART
    UART_ConnectivitySendMessage(buffer);  // Assuming UART_SendMessage is your function to send strings
    UART_SendMessage("\r\n");     // Print�the�unit
}
void UART_SendHexConnectivity(uint8_t *dataArray, uint8_t stopIndex) {
    uint8_t i = 0;
    for (; i <= stopIndex; i++) {
        UARTCharPut(UART_CONNECTIVITY_BASE_ADDRESS, dataArray[i]); // Send each byte over UART
    }
}
