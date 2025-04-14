/* Includes ------------------------------------------------------------------*/
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>          
#include "CAN.h"             
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
extern uint8_t Global_u8CurrentAnchor; // Temp Solution
extern uint8_t Global_u8DevicesRangingType [APP_MAX_NO_OF_DEVICES];


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

extern APP_tenuStates currentState;


RSSIData_t gRSSIData = {0};


/* ---------------------------------------------------------------------------
 * Local (static) function prototypes
 * ---------------------------------------------------------------------------*/
static void parseDistanceData(const tCANMsgObject* pRxMsg, const uint8_t* rxData);
static void parseBondingData(const tCANMsgObject* pRxMsg, const uint8_t* rxData);
static void parseRssiData(uint32_t messageId, const uint8_t* rxData);

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

    /* Clear the interrupt for this message object. */
    CANIntClear(CAN0_BASE, ui32Status);

    /* Retrieve the message and parse it. */
    tCANMsgObject rxMsg;
    uint8_t rxData[8];

    rxMsg.pui8MsgData = rxData;
    CANMessageGet(CAN0_BASE, MSG_OBJ_RX_1, &rxMsg, true);

    /* Parse the already-read message. */
    CAN_voidParseReceivedFrame(&rxMsg, rxData);

    // if(lock){
        //Reset the RX object to clear everything and be ready for next one
        rxMsg.ui32Flags = MSG_OBJ_RX_INT_ENABLE | MSG_OBJ_USE_ID_FILTER;
        rxMsg.ui32MsgID = 0x100;
        rxMsg.ui32MsgIDMask = 0x700;
        rxMsg.ui32MsgLen = 8;
        rxMsg.pui8MsgData = 0;

        CANMessageSet(CAN0_BASE, MSG_OBJ_RX_1, &rxMsg, MSG_OBJ_TYPE_RX);
    // }
}
uint8_t counter=0;
uint8_t isBondingDataReceived=0;
/* ---------------------------------------------------------------------------
 * CAN_voidParseReceivedFrame
 * ---------------------------------------------------------------------------*/
void CAN_voidParseReceivedFrame(const tCANMsgObject* pRxMsg, const uint8_t* rxData)
{
    uint32_t messageId = pRxMsg->ui32MsgID;

    switch (messageId)
    {
        case CAN_ID_BONDING_DATA:
            if(!isBondingDataReceived){
                parseBondingData(pRxMsg, rxData);
            }
            break;

        /* Distance messages ---------------------------------------------- */
        case CAN_ID_DISTANCE_TDM_A1:
            isTDM = 1;
            anchorID = 1;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        case CAN_ID_DISTANCE_PE_A1:
            isTDM = 0;
            anchorID = 1;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        case CAN_ID_DISTANCE_TDM_A2:
            isTDM = 1;
            anchorID = 2;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        case CAN_ID_DISTANCE_PE_A2:
            isTDM = 0;
            anchorID = 2;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        case CAN_ID_DISTANCE_TDM_A3:
            isTDM = 1;
            anchorID = 3;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        case CAN_ID_DISTANCE_PE_A3:
            isTDM = 0;
            anchorID = 3;
            counter++;
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
                counter = 0;
            }
            break;

        /* PE Status messages --------------------------------------------- */
        case CAN_ID_PE_STATUS_A1:
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_PRIMARY_PE_SUCCESSFUL);
            }
            else
            {
                if(Global_u8CurrentAnchor == rxData[2] && (currentState==STATE_PRIMARY_PE || currentState==STATE_SECONDARY_PE))
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
                if(Global_u8CurrentAnchor == rxData[2] && (currentState==STATE_PRIMARY_PE || currentState==STATE_SECONDARY_PE))
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
                if(Global_u8CurrentAnchor == rxData[2] && (currentState==STATE_PRIMARY_PE || currentState==STATE_SECONDARY_PE))
                    APP_voidFSMHandler(EVENT_SECONDARY_PE_FAILED);
            }
            break;

        /* Wake-up notification messages ---------------------------------- */
        case CAN_ID_WAKEUP_NOTIFICATION_A1:
            //CAN_voidSendRangingType(Global_u8DevicesRangingType);
            APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED);
            break;

        case CAN_ID_WAKEUP_NOTIFICATION_A2:
            //CAN_voidSendRangingType(Global_u8DevicesRangingType);
            APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED);
            break;

        case CAN_ID_WAKEUP_NOTIFICATION_A3:
            //CAN_voidSendRangingType(Global_u8DevicesRangingType);
            APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED);
            break;

        /* RSSI messages -------------------------------------------------- */
        case CAN_ID_RSSI_A1:
            // if (Global_u8CurrentAnchor == CAN_ANCHOR_1) {
                parseRssiData(messageId, rxData);
            // }
            break;
        case CAN_ID_RSSI_A2:
            // if (Global_u8CurrentAnchor == CAN_ANCHOR_2) {
                parseRssiData(messageId, rxData);
            // }
            break;
        case CAN_ID_RSSI_A3:
            // if (Global_u8CurrentAnchor == CAN_ANCHOR_3) {
                parseRssiData(messageId, rxData);
            // }
            break;
        case CAN_ID_RANGING_TYPE_A1:
            //Global_u8DevicesRangingType[rxData[0]] = rxData[1];
            break;


        default:
        {
            snprintf(msg, sizeof(msg), "Received unknown message ID: 0x%03X\r\n", (unsigned)messageId);
            UART_SendMessage(msg);
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
        UART_SendMessage (msg);
        anchorID=0;
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
    UART_SendMessage (msg);

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
    UART_SendMessage (ANSI_COLOR_GREEN, "%s", msg);
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
            UART_SendMessage ("Bonding data fully received.\r\n");
            APP_voidFSMHandler(EVENT_BONDING_DATA_RECEIVED);
            s_bondingDataCounter = 0;
            isBondingDataReceived=1;
            break;

        default:
            /* Out-of-order or extra frames */
            s_bondingDataCounter = 0;
            break;
    }

    (void)pRxMsg; /* Suppress unused parameter warning if not needed otherwise */
}

/* ---------------------------------------------------------------------------
 * parseRssiData
 * ---------------------------------------------------------------------------*/
static void parseRssiData(uint32_t messageId, const uint8_t* rxData)
{
    RSSIData_t* targetData = NULL;

    /* Determine the target structure based on the message ID */
    switch (messageId)
    {
        case CAN_ID_RSSI_A1:
            targetData = &gRSSIData;
            targetData->anchorId = 1;
            break;
        case CAN_ID_RSSI_A2:
            targetData = &gRSSIData;
            targetData->anchorId = 2;
            break;
        case CAN_ID_RSSI_A3:
            targetData = &gRSSIData;
            targetData->anchorId = 3;
            break;
        default:
            return; // Unknown message ID
    }

    /* Parse RSSI data */
    targetData->averageRssi = rxData[0];
    targetData->confidence = rxData[1];
    targetData->accuracy = rxData[2];
    targetData->distance = ((uint16_t)rxData[3] << 8) | rxData[4];

    /* Log the parsed data */
    snprintf(msg, sizeof(msg),
             "RSSI Data - Anchor: %d, Avg: %d, Conf: %d, Acc: %d, Dist: %d\r\n",
             targetData->anchorId, targetData->averageRssi, targetData->confidence,
             targetData->accuracy, targetData->distance);
    UART_SendMessage(msg);

    /* Trigger an event if needed */
    APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE);
}
