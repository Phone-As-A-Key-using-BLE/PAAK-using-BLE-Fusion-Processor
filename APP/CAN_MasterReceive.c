/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
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
#include "sys/cdefs.h"
#include "CAN_Send.h"

extern uint8_t Global_u8CurrentAnchor; // Temp Solution
extern uint8_t Global_u8DevicesRangingType [APP_MAX_NO_OF_DEVICES];
extern uint8_t Global_u8IgnoreResponse;
extern uint8_t Global_u8CurrentNvmIndex;
extern uint8_t g_verifierSendIndex;
extern const uint8_t* g_verifierRawData;
extern bool g_verifierSendingActive;
/* ---------------------------------------------------------------------------
 * Data structures mirroring NXP-based variables
 * ---------------------------------------------------------------------------*/
CAN_tstrDistance    s_distanceData       = {0};
static CAN_tstrCommandData s_commandData        = {CAN_COMMAND_INVALID, 0};
static add_device          s_addDeviceData      = {0};
static remove_device_t     s_removeDeviceData   = {0};
static uint8_t             s_bondingDataCounter = 0;

/* Global variables ----------------------------------------------------------*/
uint8_t gNextAnchorId = 0;
uint8_t anchorID = 0;

extern uint8_t Global_u8SendPE;
extern uint8_t Global_u8SendTDM;
extern uint8_t Global_u8SendHandover;
extern uint8_t Global_u8CurrentDeviceId;

/* Debug print buffer */
extern char gArr_DebugMsg[APP_DEBUG_ARRAY_MAX_SIZE];

extern APP_tenuStates currentState;

volatile RSSIData_t gRSSIData[CAN_ANCHOR_MAX + 1] = {0};
const char* CAN_statusStr[] = {
    "CAN_PE_SUCCESS",
    "CAN_PE_FAILED",
    "CAN_HANDOVER_SUCCESS",
    "CAN_HANDOVER_FAILED"
};
extern uint8_t Global_u8ResetAndPE;

double_t Global_f64DistanceReadings[APP_MAX_NO_OF_DEVICES][CAN_ANCHOR_MAX+1] = {0};

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
            if(currentState == STATE_WAITING_FOR_BONDING_DATA)
                parseBondingData(pRxMsg, rxData);
        
            break;

        /* Distance messages ---------------------------------------------- */
        case CAN_ID_DISTANCE_TDM_A1:
            anchorID = 1;
            counter++;
            parseDistanceData(pRxMsg, rxData);
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                counter = 0;
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE, s_distanceData.deviceId);
            }
            break;

        case CAN_ID_DISTANCE_TDM_A2:
            anchorID = 2;
            counter++;
            parseDistanceData(pRxMsg, rxData);
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                counter = 0;
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE, s_distanceData.deviceId);
            }
            break;


        case CAN_ID_DISTANCE_TDM_A3:
            anchorID = 3;
            counter++;
            parseDistanceData(pRxMsg, rxData);
            if(counter == APP_CS_NO_OF_MEASURING_DISTANCE){
                counter = 0;
                APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE, s_distanceData.deviceId);
            }
            break;

        /* PE Status messages --------------------------------------------- */
        case CAN_ID_STATUS_A1:
        {
            if(rxData[0] != CAN_VERIFIERS_OK && rxData[0] != CAN_CERTIFICATES_OK){
                snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Received Status: %s\r\n", CAN_statusStr[rxData[0]]);
                UART_SendMessage(gArr_DebugMsg);
            }
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                Global_u8CurrentNvmIndex = rxData[3];
                Global_u8CurrentDeviceId = rxData[2];
                APP_voidFSMHandler(EVENT_PRIMARY_PE_SUCCESSFUL, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_PE_DISCONNECTED)
            {
                if (Global_u8IgnoreResponse != CAN_ANCHOR_1) {
                    APP_voidFSMHandler(EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR, Global_u8CurrentDeviceId);
                }
                break;
            }
            else if(rxData[0] == CAN_HANDOVER_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_SUCCESS, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_HANDOVER_FAILED)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_FAILED, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_RSSI_MODE){
                Global_u8CurrentNvmIndex = rxData[3];
                break;
            }
            else if(rxData[0] == CAN_VERIFIERS_OK){
                if (g_verifierSendingActive && g_verifierSendIndex < VERIFIER_FRAME_COUNT) {
                    g_verifierSendIndex++;
                    CAN_voidSendNextVerifierFrame();
                }

                // Done?
                if (g_verifierSendIndex >= VERIFIER_FRAME_COUNT) {
                    g_verifierSendingActive = false;
                    APP_voidFSMHandler(EVENT_VERIFIERS_SENT_TO_PRIMARY_ANCHOR, Global_u8CurrentDeviceId);
                }
                break;
            }

            break;
        }
        case CAN_ID_STATUS_A2:
        {
            snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\nReceived Status: %s\r\n", CAN_statusStr[rxData[0]]);
            UART_SendMessage(gArr_DebugMsg);
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                Global_u8CurrentDeviceId = rxData[2];
                APP_voidFSMHandler(EVENT_SECONDARY_PE_SUCCESSFUL, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_PE_DISCONNECTED)
            {
                if (Global_u8IgnoreResponse != CAN_ANCHOR_2) {
                    APP_voidFSMHandler(EVENT_DEVICE_DISCONNECTED_FROM_SECONDARY_ANCHOR, Global_u8CurrentDeviceId);
                }
                break;

            }
            else if(rxData[0] == CAN_HANDOVER_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_SUCCESS, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_HANDOVER_FAILED)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_FAILED, Global_u8CurrentDeviceId);
                break;
            }
            break;
        }
        case CAN_ID_STATUS_A3:
        {
            snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\nReceived Status: %s\r\n", CAN_statusStr[rxData[0]]);
            UART_SendMessage(gArr_DebugMsg);
            if (rxData[0] == CAN_PE_SUCCESS)
            {
                Global_u8CurrentDeviceId = rxData[2];
                APP_voidFSMHandler(EVENT_SECONDARY_PE_SUCCESSFUL, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_PE_DISCONNECTED)
            {
                if (Global_u8IgnoreResponse != CAN_ANCHOR_3) {
                    APP_voidFSMHandler(EVENT_DEVICE_DISCONNECTED_FROM_SECONDARY_ANCHOR, Global_u8CurrentDeviceId);
                }
                break;

            }
            else if(rxData[0] == CAN_HANDOVER_SUCCESS)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_SUCCESS, Global_u8CurrentDeviceId);
                break;
            }
            else if(rxData[0] == CAN_HANDOVER_FAILED)
            {
                APP_voidFSMHandler(EVENT_HANDOVER_FAILED, Global_u8CurrentDeviceId);
                break;
            }
            break;
        }
        /* Wake-up notification messages ---------------------------------- */
        case CAN_ID_WAKEUP_NOTIFICATION_A1:
        if(Global_u8ResetAndPE){
            Global_u8ResetAndPE = 1;
            break;
        }
        if(Global_u8IgnoreResponse != CAN_ANCHOR_1){
            Global_u8IgnoreResponse = 0;
            if(currentState == STATE_IDLE){
                currentState = STATE_START_CS_STATE;
                Global_u8SendPE = 1;
                break;
            }
            APP_voidFSMHandler(EVENT_PRIMARY_WAKEUP_RECEIVED, Global_u8CurrentDeviceId);
        }
         else{
               Global_u8IgnoreResponse = 0;
         }
            break;
        case CAN_ID_WAKEUP_NOTIFICATION_A2:
        if(Global_u8ResetAndPE){
            Global_u8ResetAndPE = 1;
            break;
        }
         if(Global_u8IgnoreResponse != CAN_ANCHOR_2){
            Global_u8IgnoreResponse = 0;
            APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED, Global_u8CurrentDeviceId);
         }
         else{
               Global_u8IgnoreResponse = 0;
         }
            break;

        case CAN_ID_WAKEUP_NOTIFICATION_A3:
        if(Global_u8ResetAndPE){
            Global_u8ResetAndPE = 1;
            break;
        }
         if(Global_u8IgnoreResponse != CAN_ANCHOR_3){
            Global_u8IgnoreResponse = 0;
            APP_voidFSMHandler(EVENT_SECONDARY_WAKEUP_RECEIVED, Global_u8CurrentDeviceId);
         }
         else{
            Global_u8IgnoreResponse = 0;
         }
            break;

        /* RSSI messages -------------------------------------------------- */
        case CAN_ID_RSSI_A1:
                parseRssiData(messageId, rxData);
            break;

        case CAN_ID_RSSI_A2:
                parseRssiData(messageId, rxData);
            break;

        case CAN_ID_RSSI_A3:
                parseRssiData(messageId, rxData);
            break;


        default:
        {
            break;
        }
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
    double Loc_f64Distance = s_distanceData.distanceIntegerPart + (s_distanceData.distanceDecimalPart/100.0);
    Global_f64DistanceReadings[s_distanceData.deviceId][Global_u8CurrentAnchor] = Loc_f64Distance;

    sprintf(gArr_DebugMsg,"[INFO] Received %s distance from anchor %d = %.2f meter\r\n", "CS", Global_u8CurrentAnchor, Loc_f64Distance);
    UART_SendMessage(gArr_DebugMsg);

    if(currentState == STATE_DISTANCE_MEASUREMENT_PROCESSING){
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg),"Anchor %d Distance %s: %.2f meter\r\n", Global_u8CurrentAnchor, "CS", Loc_f64Distance);
        UART_SendMessage (gArr_DebugMsg);
    }

    (void)pRxMsg; 
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
            APP_voidFSMHandler(EVENT_BONDING_DATA_RECEIVED, s_addDeviceData.nvmIndex);
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
    /* Determine the target structure based on the message ID */
    switch (messageId)
    {
        case CAN_ID_RSSI_A1:
            gRSSIData[CAN_ANCHOR_1].averageRssi = rxData[0];
            gRSSIData[CAN_ANCHOR_1].confidence = rxData[1];
            gRSSIData[CAN_ANCHOR_1].accuracy = rxData[2];
            gRSSIData[CAN_ANCHOR_1].distance = ((uint16_t)rxData[3] << 8) | rxData[4];
            gRSSIData[CAN_ANCHOR_1].anchorId = 1;
            break;
        case CAN_ID_RSSI_A2:
            gRSSIData[CAN_ANCHOR_2].averageRssi = rxData[0];
            gRSSIData[CAN_ANCHOR_2].confidence = rxData[1];
            gRSSIData[CAN_ANCHOR_2].accuracy = rxData[2];
            gRSSIData[CAN_ANCHOR_2].distance = ((uint16_t)rxData[3] << 8) | rxData[4];
            gRSSIData[CAN_ANCHOR_2].anchorId = 2;
            break;
        case CAN_ID_RSSI_A3:
            gRSSIData[CAN_ANCHOR_3].averageRssi = rxData[0];
            gRSSIData[CAN_ANCHOR_3].confidence = rxData[1];
            gRSSIData[CAN_ANCHOR_3].accuracy = rxData[2];
            gRSSIData[CAN_ANCHOR_3].distance = ((uint16_t)rxData[3] << 8) | rxData[4];
            gRSSIData[CAN_ANCHOR_3].anchorId = 3;
            break;
        default:
            return; // Unknown message ID
    }

    /* Log the parsed data */
    uint8_t Loc_u8DeviceId = rxData[5];
    double Loc_f64Distance = (gRSSIData[Global_u8CurrentAnchor].distance)/100.0;
    Global_f64DistanceReadings [Loc_u8DeviceId][Global_u8CurrentAnchor] = Loc_f64Distance;
    
    sprintf(gArr_DebugMsg,"[INFO] Received %s distance from anchor %d = %.2f meter\r\n", "RSSI", Global_u8CurrentAnchor, Loc_f64Distance);
    UART_SendMessage(gArr_DebugMsg);

    if(currentState == STATE_DISTANCE_MEASUREMENT_PROCESSING){
        sprintf(gArr_DebugMsg,"Anchor %d Distance %s: %.2f meter\r\n", Global_u8CurrentAnchor, "RSSI", Loc_f64Distance);
        UART_SendMessage(gArr_DebugMsg);
    }
    /* Trigger an event if needed */
    APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE, Loc_u8DeviceId);
}
