/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>          /* For va_list, va_start, etc. */
#include "CAN.h"             // Your Tiva CAN driver header
#include "UART/uart.h"       // For UART debug printing
#include "APP_FSM.h"
#include "can_msg_types.h"
#include "CAN_MasterReceive.h"
#include "CAN_App.h"

#if (CAN_ANCHOR_ID != CAN_MASTER_NODE)
#include "app_conn.h"        // If needed by your application
#include "digital_key_car_anchor_cs.h"
#include "shell_digital_key_car_anchor_cs.h"
#include "shell_print.h"
#endif

/* ---------------------------------------------------------------------------
 * ANSI escape color codes
 * ---------------------------------------------------------------------------*/
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_WHITE   "\x1b[37m"

/* ---------------------------------------------------------------------------
 * Helper function for standardized, colored UART output
 * ---------------------------------------------------------------------------*/
static void UART_SendColoredMessage(const char* color, const char* format, ...)
{
    char buffer[1024];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    // If color is null or empty, just send the raw text without color codes.  
    if ((color == NULL) || (color[0] == '\0'))  
    {  
        UART_SendMessage(buffer);  
        return;  
    }  

    // Otherwise, use ANSI color codes.  
    char coloredBuffer[1024];  
    snprintf(coloredBuffer, sizeof(coloredBuffer),  
            "%s%s%s", color, buffer, ANSI_COLOR_RESET);  

    UART_SendMessage(coloredBuffer);  
}

/* ---------------------------------------------------------------------------
 * Data structures mirroring NXP-based variables
 * ---------------------------------------------------------------------------*/
static CAN_tstrDistance    s_distanceData       = {0};
static CAN_tstrCommandData s_commandData        = {CAN_COMMAND_INVALID, 0};
static add_device          s_addDeviceData      = {0};
static remove_device_t     s_removeDeviceData   = {0};
static uint8_t             s_bondingDataCounter = 0;

/* Global variables ----------------------------------------------------------*/
uint8_t gNextAnchorId = 0;
uint8_t isTDM    = 0;
uint8_t anchorID = 0;

/* Debug print buffer */
char msg[1024];

/* ---------------------------------------------------------------------------
 * Local (static) function prototypes
 * ---------------------------------------------------------------------------*/
static void parseDistanceData(const tCANMsgObject* pRxMsg, const uint8_t* rxData);
static void parseBondingData(const tCANMsgObject* pRxMsg, const uint8_t* rxData);

/* ---------------------------------------------------------------------------
 * CAN0_Handler (Interrupt Service Routine)
 * ---------------------------------------------------------------------------*/
void CAN0_Handler(void)
{
    uint32_t ui32Status = CANIntStatus(CAN0_BASE, CAN_INT_STS_CAUSE);

    /* If ui32Status == 0, it means it's a global status interrupt or error. */
    if (ui32Status == 0)
    {
        uint32_t ctrlStatus = CANStatusGet(CAN0_BASE, CAN_STS_CONTROL);
        CANIntClear(CAN0_BASE, ui32Status);
        (void) ctrlStatus; /* Suppress unused variable warning. */
        return;
    }

    /* Retrieve the message and parse it. */
    tCANMsgObject rxMsg;
    uint8_t rxData[8];

    rxMsg.pui8MsgData = rxData;
    CANMessageGet(CAN0_BASE, MSG_OBJ_RX_1, &rxMsg, true);

    /* Parse the already-read message. */
    CAN_voidParseReceivedFrame(&rxMsg, rxData);

    /* Clear the interrupt for this message object. */
    CANIntClear(CAN0_BASE, ui32Status);
}

/* ---------------------------------------------------------------------------
 * CAN_voidParseReceivedFrame
 * ---------------------------------------------------------------------------*/
void CAN_voidParseReceivedFrame(const tCANMsgObject* pRxMsg, const uint8_t* rxData)
{
    uint32_t messageId = pRxMsg->ui32MsgID;

    switch (messageId)
    {
        case CAN_ID_BONDING_DATA:
            parseBondingData(pRxMsg, rxData);
            break;

        /* Distance messages ---------------------------------------------- */
        case CAN_ID_DISTANCE_TDM_A1:
            isTDM = 1;
            anchorID = 1;
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
            break;

        case CAN_ID_DISTANCE_PE_A1:
            isTDM = 0;
            anchorID = 1;
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
            break;

        case CAN_ID_DISTANCE_TDM_A2:
            isTDM = 1;
            anchorID = 2;
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
            break;

        case CAN_ID_DISTANCE_PE_A2:
            isTDM = 0;
            anchorID = 2;
            /* Original code commented out:
             * APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
             */
            break;

        case CAN_ID_DISTANCE_TDM_A3:
            isTDM = 1;
            anchorID = 3;
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
            break;

        case CAN_ID_DISTANCE_PE_A3:
            isTDM = 0;
            anchorID = 3;
            /* Original code commented out:
             * APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
             */
            break;

        /* PE Status messages --------------------------------------------- */
        case CAN_ID_PE_STATUS_A1:
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_PRIMARY_PE_SUCCESSFUL);
            }
            else
            {
                APP_voidFSMHandler(EVENT_PRIMARY_PE_FAILED);
            }
            break;

        case CAN_ID_PE_STATUS_A2:
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_SECONDARY_PE_SUCCESSFUL);
            }
            else
            {
                APP_voidFSMHandler(EVENT_SECONDARY_PE_FAILED);
            }
            break;

        case CAN_ID_PE_STATUS_A3:
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_SECONDARY_PE_SUCCESSFUL);
            }
            else
            {
                APP_voidFSMHandler(EVENT_SECONDARY_PE_FAILED);
            }
            break;

        /* Wake-up notification messages ---------------------------------- */
        case CAN_ID_WAKEUP_NOTIFICATION_A1:
            //APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED);
            break;

        case CAN_ID_WAKEUP_NOTIFICATION_A2:
            //APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED);
            break;

        case CAN_ID_WAKEUP_NOTIFICATION_A3:
            //APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED);
            break;

        default:
        {
            snprintf(msg, sizeof(msg), "Received unknown message ID: 0x%03X\r\n", (unsigned)messageId);
            UART_SendColoredMessage(ANSI_COLOR_RED, "%s", msg);
            break;
        }
    }

    /* If a known anchor ID was set, parse and log distance data. */
    if (anchorID > 0)
    {
        parseDistanceData(pRxMsg, rxData);
        snprintf(msg, sizeof(msg),
                 "Received Distance Data from Anchor %d, Type: %s\r\n",
                 anchorID, isTDM ? "TDM" : "PE");
        UART_SendColoredMessage(ANSI_COLOR_CYAN, "%s", msg);
    }
}

/* ---------------------------------------------------------------------------
 * Public �get� routines similar to your original code
 * ---------------------------------------------------------------------------*/
CAN_tstrDistance CAN_structGetDistanceData(void)
{
    return s_distanceData;
}

CAN_tstrCommandData CAN_structGetLastCommandData(void)
{
    return s_commandData;
}

add_device CAN_structGetAddDeviceData(void)
{
    return s_addDeviceData;
}

remove_device_t CAN_structGetRemoveDeviceData(void)
{
    return s_removeDeviceData;
}

/* ---------------------------------------------------------------------------
 * parseDistanceData
 * ---------------------------------------------------------------------------*/
static void parseDistanceData(const tCANMsgObject* pRxMsg, const uint8_t* rxData)
{
    /* NXP code: dataByte0..dataByte7 => Tiva: rxData[0..7] */
    s_distanceData.deviceId            = rxData[0];
    s_distanceData.procNo              = (uint16_t)rxData[1] | ((uint16_t)rxData[2] << 8);
    s_distanceData.distanceIntegerPart = rxData[3];
    s_distanceData.distanceDecimalPart = (uint16_t)rxData[4] | ((uint16_t)rxData[5] << 8);

    /* Convert DQI */
    uint16_t dqiRaw = (uint16_t)rxData[6] | ((uint16_t)rxData[7] << 8);
    s_distanceData.dqiPercentage = (float)dqiRaw * 0.01f;


    snprintf(
    msg,
    sizeof(msg),
    "Anchor %d, Dist: %d.%d\r\n",
    anchorID,
    s_distanceData.distanceIntegerPart,
    s_distanceData.distanceDecimalPart
    );
    UART_SendColoredMessage(NULL, "%s", msg);

    /* (Optional) If you also want to log the DQI:
    snprintf(
    msg,
    sizeof(msg),
    "Anchor %d, Dist: %d.%d, DQI: %.2f%%\r\n",
    anchorID,
    s_distanceData.distanceIntegerPart,
    s_distanceData.distanceDecimalPart,
    (double)s_distanceData.dqiPercentage
    );
    UART_SendColoredMessage(ANSI_COLOR_GREEN, "%s", msg);
    */


    (void)pRxMsg; /* Suppress unused parameter warning if not needed otherwise */
}

/* ---------------------------------------------------------------------------
 * parseBondingData
 * ---------------------------------------------------------------------------*/
static void parseBondingData(const tCANMsgObject* pRxMsg, const uint8_t* rxData)
{
    s_bondingDataCounter++;

    switch (s_bondingDataCounter)
    {
        case 1:
            /* Fill s_addDeviceData from first chunk */
            s_addDeviceData.nvmIndex = (rxData[0] & 0x0F);
            s_addDeviceData.bleDeviceAddress_t.idAddressType = (rxData[1] & 0x01);
            s_addDeviceData.gAppOutAuth = ((rxData[1] >> 1) & 0x01);
            s_addDeviceData.gAppOutLeSc = ((rxData[1] >> 2) & 0x01);
            /* Copy address (6 bytes) */
            memcpy(s_addDeviceData.bleDeviceAddress_t.idAddress, &rxData[2], 6);
            break;

        case 2:
            /* Next 8 bytes (aLtk[0..7]) */
            memcpy(&s_addDeviceData.aLtk[0], rxData, 8);
            break;

        case 3:
            /* aLtk[8..15] */
            memcpy(&s_addDeviceData.aLtk[8], rxData, 8);
            break;

        case 4:
            /* aIrk[0..7] */
            memcpy(&s_addDeviceData.aIrk[0], rxData, 8);
            break;

        case 5:
            /* aIrk[8..15] */
            memcpy(&s_addDeviceData.aIrk[8], rxData, 8);
            UART_SendColoredMessage(ANSI_COLOR_BLUE, "Bonding data fully received.\r\n");
            APP_voidFSMHandler(EVENT_BONDING_DATA_RECEIVED);
            s_bondingDataCounter = 0;
            break;

        default:
            /* Out-of-order or extra frames */
            s_bondingDataCounter = 0;
            break;
    }

    (void)pRxMsg; /* Suppress unused parameter warning if not needed otherwise */
}
