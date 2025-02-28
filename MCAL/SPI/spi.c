/***********************************************
 * Module Name: SPI
 * Author: MOHAMAD WALEED
 * Purpose: Initialize SPI 1
 ***********************************************/

/***********************************************
 * Includes
 ***********************************************/
#include "spi.h"

/***********************************************
 * Function Name: SPI_Init
 * Inputs: uint32_t base - the base address for SSI (SPI)
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synchronous
 * Description: Initializes the SPI interface by configuring the appropriate SSI base,
 *              enabling peripherals, and setting up the GPIO pins for SPI communication.
 ***********************************************/
void SPI_Init(void) {
    uint32_t peripheral;
    uint32_t gpioPort;
    uint32_t clkPin, fssPin, rxPin, txPin;

/*    // Determine the SSI base and related peripherals*/
    switch (SPI_BASE_ADDRESS) {
        case SSI0_BASE:
            peripheral = SYSCTL_PERIPH_SSI0;
            gpioPort = SYSCTL_PERIPH_GPIOA;
            clkPin = GPIO_PA2_SSI0CLK;
            fssPin = GPIO_PA3_SSI0FSS;
            rxPin = GPIO_PA4_SSI0RX;
            txPin = GPIO_PA5_SSI0TX;
            break;
        case SSI1_BASE:
            peripheral = SYSCTL_PERIPH_SSI1;
            gpioPort = SYSCTL_PERIPH_GPIOD;
            clkPin = GPIO_PD0_SSI1CLK;
            fssPin = GPIO_PD1_SSI1FSS;
            rxPin = GPIO_PD2_SSI1RX;
            txPin = GPIO_PD3_SSI1TX;
            break;
        default:
            break;  /*Unsupported SSI base*/
    }

/*    // Enable SSI and GPIO peripherals*/
    SysCtlPeripheralEnable(peripheral);
    SysCtlPeripheralEnable(gpioPort);
    // Reevaluate the status of the peripherals within the loop
    bool peripheralReady = 0;
    bool gpioPortReady = 0;

    while (1) {
        // Check the status of both peripherals separately
        peripheralReady = SysCtlPeripheralReady(peripheral);
        gpioPortReady = SysCtlPeripheralReady(gpioPort);

        // Exit the loop when both peripherals are ready
        if (peripheralReady && gpioPortReady) {
            break;  // Both peripherals are ready, exit the loop
        }
    }



/*    // Configure GPIO Pins for SPI*/
    GPIOPinConfigure(clkPin);
    GPIOPinConfigure(fssPin);
    GPIOPinConfigure(rxPin);
    GPIOPinConfigure(txPin);
    GPIOPinTypeSSI(
        gpioPort == (uint32_t)SYSCTL_PERIPH_GPIOD ? (uint32_t)GPIO_PORTD_BASE :
        gpioPort == (uint32_t)SYSCTL_PERIPH_GPIOA ? (uint32_t)GPIO_PORTA_BASE :
        gpioPort == (uint32_t)SYSCTL_PERIPH_GPIOB ? (uint32_t)GPIO_PORTB_BASE :
                    (uint32_t)GPIO_PORTQ_BASE,
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

/*    // Set up SSI in slave mode*/
    SSIConfigSetExpClk(SPI_BASE_ADDRESS, SysCtlClockGet(), (uint32_t)SPI_DEFAULT_MODE, (uint32_t)SSI_MODE_SLAVE, (uint32_t)SPI_DEFAULT_BAUD_RATE, (uint32_t)SPI_DEFAULT_BIT_LENGTH);

/*    // Enable SSI*/
    SSIEnable(SPI_BASE_ADDRESS);
}

/***********************************************
 * Function Name: SPI_MasterInit
 * Inputs: N/A
 * Outputs: N/A
 * Reentrancy: Non-Reentrant
 * Synchronous: Synchronous
 * Description: Initializes the SPI1 as a master. Configures the necessary GPIO pins
 *              for SPI communication, sets the SSI1 parameters (mode, baud rate, data length),
 *              and enables the SSI1 interface.
 ***********************************************/
void SPI_MasterInit(void) {
/*    Enable SSI1 and GPIO Port D peripherals for SPI*/
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);

    // Reevaluate the status of the peripherals within the loop
        bool peripheralReady = 0;
        bool gpioPortReady = 0;
    while (1) {
        // Check the status of both peripherals separately
        peripheralReady = SysCtlPeripheralReady(SYSCTL_PERIPH_SSI1);
        gpioPortReady = SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOD);

        // Exit the loop when both peripherals are ready
        if (peripheralReady && gpioPortReady) {
            break;  // Both peripherals are ready, exit the loop
        }
    }


/*     Configure Port D pins for SPI (SSI1)*/
    GPIOPinConfigure(GPIO_PD0_SSI1CLK);
    GPIOPinConfigure(GPIO_PD1_SSI1FSS);
    GPIOPinConfigure(GPIO_PD2_SSI1RX);
    GPIOPinConfigure(GPIO_PD3_SSI1TX);
    GPIOPinTypeSSI(GPIO_PORTD_BASE, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3);

/*     Set up SSI1 as SPI master, mode 0, 1 MHz, 8-bit data*/
    SSIConfigSetExpClk(SPI_BASE_ADDRESS, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, (uint32_t)1000000, (uint32_t)8);
    SSIEnable(SPI_BASE_ADDRESS);

  /* UNLOCK these if you will use the Handler of Spi
    // Step 6: Register the Interrupt Handler for SPI1 (SSI1)
    IntRegister(INT_SSI1, SSI1_Handler);  // Register the custom interrupt handler

    // Step 7: Enable SSI1 Interrupts
    SSIIntEnable(SSI1_BASE, SSI_RXTO | SSI_RXOR | SSI_RXFF);  // Enable timeout, overrun, and RX FIFO full interrupts

    // Enable the global interrupt
    IntMasterEnable();  // Enable global interrupts
    IntEnable(INT_SSI1); // Enable the SSI1 interrupt specifically
    */
}

/***********************************************
 * Function Name: sendStateToSlave
 * Inputs: LEDState state - the state to be sent to the slave device
 * Outputs: SPIStatus - result of the communication attempt (SPI_SUCCESS, SPI_UNDEFINED, SPI_FAILURE)
 * Reentrancy: Non-Reentrant
 * Synchronous: Synchronous
 * Description: Transmits the given state to the slave device via SPI.
 *              Waits for the transmission to complete, checks for an acknowledgment,
 *              and returns the appropriate status based on the received data.
 ***********************************************/
COMMStatus sendStateToSlave(LEDState state) {
    uint32_t local_data = 0;
    UndefinedState = false;
    COMMStatus COMM_state;
    COMM_state = COMM_SUCCESS;
        SSIDataPut(SPI_BASE_ADDRESS, (uint32_t)state);  /* Transmit state to slave*/
        while (SSIBusy(SPI_BASE_ADDRESS)) {}  /* Wait until transmission is complete*/

        if (SSIDataGetNonBlocking(SSI1_BASE, &local_data)) {
            if (local_data == ACK_CMD) {
                COMM_state = COMM_SUCCESS;  /* Acknowledgment received*/
            }
            else if (local_data == UNDEFINED_STATE) {
                 UndefinedState = true;
                 COMM_state = COMM_UNDEFINED;  /* Error: Undefined state received*/
            } else {
                COMM_state = COMM_FAILURE;  /* Unknown response*/
            }
        }

    return COMM_state;  /*// Default to success*/
}

/***********************************************
 * Function Name: SPI_Receive
 * Inputs: uint32_t base - the base address for SSI (SPI), uint32_t *data - pointer to store received data
 * Outputs: SPIStatus - result of the receive attempt (SPI_SUCCESS, SPI_FAILURE)
 * Reentrancy: Non-Reentrant
 * Synchronous: Synchronous
 * Description: Attempts to read data from the SPI receive FIFO. If data is available,
 *              it stores the data in the provided pointer and returns SPI_SUCCESS.
 *              Otherwise, it returns SPI_FAILURE.
 ***********************************************/
COMMStatus SPI_Receive(uint32_t base, uint32_t *local_data) {

    COMMStatus COMM_state;

/*     Check if data is available in the receive FIFO*/
    if (SSIDataGetNonBlocking(base, local_data)) {
        COMM_state = COMM_SUCCESS;
    } else {
        COMM_state = COMM_FAILURE; /* No data available*/
    }

    return COMM_state;
}
