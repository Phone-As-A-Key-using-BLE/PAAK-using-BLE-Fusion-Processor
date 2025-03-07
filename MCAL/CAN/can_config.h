#ifndef CAN_CONFIG_H_
#define CAN_CONFIG_H_

#include <stdint.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/can.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

/**
 * Options for CAN Module Selection:
 *      CAN_MODULE_0   // Use CAN0
 *      CAN_MODULE_1   // Use CAN1
 */
#define CAN_MODULE                  CAN_MODULE_0

/**
 * GPIO Port Configuration (depends on the selected CAN module):
 *      CAN0_PORT_GPIOB              // Port B (default for CAN0)
 *      CAN1_PORT_GPIOA              // Port A (default for CAN1)
 */
#define CAN_GPIO_PORT               CAN0_PORT_GPIOB

/**
 * Define GPIO ports, pins, and pin configurations for CAN0
 */
#define CAN0_PORT                   SYSCTL_PERIPH_GPIOB   // GPIO port for CAN0
#define CAN0_RX_PIN                 GPIO_PIN_4           // RX pin for CAN0
#define CAN0_TX_PIN                 GPIO_PIN_5           // TX pin for CAN0
#define CAN0_RX_CFG                 GPIO_PB4_CAN0RX      // Pin configuration for CAN0 RX
#define CAN0_TX_CFG                 GPIO_PB5_CAN0TX      // Pin configuration for CAN0 TX
#define CAN0_PORT_BASE              GPIO_PORTB_BASE

/**
 * Define GPIO ports, pins, and pin configurations for CAN1
 */
#define CAN1_PORT                   SYSCTL_PERIPH_GPIOA   // GPIO port for CAN1
#define CAN1_RX_PIN                 GPIO_PIN_0           // RX pin for CAN1
#define CAN1_TX_PIN                 GPIO_PIN_1           // TX pin for CAN1
#define CAN1_RX_CFG                 GPIO_PA0_CAN1RX      // Pin configuration for CAN1 RX
#define CAN1_TX_CFG                 GPIO_PA1_CAN1TX      // Pin configuration for CAN1 TX
#define CAN1_PORT_BASE              GPIO_PORTA_BASE

/**
 * Automatically select GPIO ports, pins, and CAN base based on the CAN_MODULE
 */
#if CAN_MODULE == CAN_MODULE_0
    #define CAN_SYSCTL_PERIPH       SYSCTL_PERIPH_CAN0    // Peripheral for CAN0
    #define CAN_GPIO_PORT_BASE      CAN0_PORT            // GPIO port base for CAN0
    #define CAN_GPIO_RX_PIN         CAN0_RX_PIN          // RX pin for CAN0
    #define CAN_GPIO_TX_PIN         CAN0_TX_PIN          // TX pin for CAN0
    #define CAN_GPIO_RX_CFG         CAN0_RX_CFG          // RX pin configuration for CAN0
    #define CAN_GPIO_TX_CFG         CAN0_TX_CFG          // TX pin configuration for CAN0
    #define CAN_BASE                CAN0_BASE            // Base address for CAN0
    #define CAN_INT                 INT_CAN0             // Interrupt number for CAN0
    #define CAN_PORT_BASE           CAN0_PORT_BASE
#else
    #define CAN_SYSCTL_PERIPH       SYSCTL_PERIPH_CAN1    // Peripheral for CAN1
    #define CAN_GPIO_PORT_BASE      CAN1_PORT            // GPIO port base for CAN1
    #define CAN_GPIO_RX_PIN         CAN1_RX_PIN          // RX pin for CAN1
    #define CAN_GPIO_TX_PIN         CAN1_TX_PIN          // TX pin for CAN1
    #define CAN_GPIO_RX_CFG         CAN1_RX_CFG          // RX pin configuration for CAN1
    #define CAN_GPIO_TX_CFG         CAN1_TX_CFG          // TX pin configuration for CAN1
    #define CAN_BASE                CAN1_BASE            // Base address for CAN1
    #define CAN_INT                 INT_CAN1             // Interrupt number for CAN1
    #define CAN_PORT_BASE           CAN1_PORT_BASE
#endif

/**
 * Options for Bit Rate (in bits per second):
 *      CAN_BIT_RATE_125K            // 125 Kbps
 *      CAN_BIT_RATE_250K            // 250 Kbps
 *      CAN_BIT_RATE_500K            // 500 Kbps
 *      CAN_BIT_RATE_1M              // 1 Mbps
 */
#define CAN_BIT_RATE               CAN_BIT_RATE_500K 

/**
 * Options for Interrupt Configuration:
 *      CAN_INT_ENABLE               // Enable CAN Interrupts
 *      CAN_INT_DISABLE              // Disable CAN Interrupts
 */
#define CAN_INTERRUPT_MODE          CAN_INT_ENABLE

/**
 * CAN Interrupt Flags:
 *      CAN_INT_MASTER               // Enable master interrupt
 *      CAN_INT_ERROR                // Enable error interrupt
 *      CAN_INT_STATUS               // Enable status interrupt
 */
#define CAN_INT_FLAGS               (CAN_INT_MASTER | CAN_INT_ERROR | CAN_INT_STATUS)

/**
 * Options for Message Filtering:
 *      NO_FILTERING                 // No filtering (receive all messages)
 *      USE_ID_FILTER                // Use ID filtering
 */
#define CAN_FILTERING_MODE           USE_ID_FILTER

/**
 * RX Message ID:
 *      Specify the 11-bit or 29-bit identifier (in hexadecimal).
 *      Example: 0x100
 */
#define CAN_RX_MESSAGE_ID                       0x20           //accept all IDs

#define CAN_RX_UNIDENTIFIED_MESSAGE_ID          0x09

/**
 * TX Message ID:
 *      Specify the 11-bit or 29-bit identifier (in hexadecimal).
 *      Example: 0x880
 */
#define CAN_TX_MESSAGE_ID            0x14

/**
 * RX Message ID Mask:
 *      Specify the ID mask for filtering (in hexadecimal).
 *      Example: 0x700 for filtering the first 3 bits
 *      Set to 0 for no filtering.
 */
#define CAN_RX_MESSAGE_MASK          0x00      //no masking

/**
 * Options for TX Message Object Number:
 *      MSG_OBJ_TX_1                 // Use Message Object 1 for TX
 *      MSG_OBJ_TX_2                 // Use Message Object 2 for TX
 *      MSG_OBJ_TX_3                 // Use Message Object 3 for TX
 */
#define CAN_TX_OBJECT_NUM            MSG_OBJ_TX_1

/**
 * Options for RX Message Object Number:
 *      MSG_OBJ_RX_1                 // Use Message Object 1 for RX
 *      MSG_OBJ_RX_2                 // Use Message Object 2 for RX
 *      MSG_OBJ_RX_3                 // Use Message Object 3 for RX
 */
#define CAN_RX_OBJECT_NUM            MSG_OBJ_RX_1

/**
 * Options for Data Length:
 *      CAN_DATA_LEN_0               // No data
 *      CAN_DATA_LEN_1               // 1 byte
 *      CAN_DATA_LEN_2               // 2 bytes
 *      ...
 *      CAN_DATA_LEN_8               // 8 bytes (maximum)
 */
#define CAN_DATA_LENGTH              CAN_DATA_LEN_8

#endif /* CAN_CONFIG_H_ */
