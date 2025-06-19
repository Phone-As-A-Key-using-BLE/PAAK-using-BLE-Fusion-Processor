
/*
 * AnchorStateManager.c
 * Created on: Jun 19, 2025
 * Author: Mohamad Waleed Nassar
 * Description: Implements state management for BLE anchors based on CAN data
 */

#include "AnchorStateManager.h"

/* Global variable definition */
Anchor_tstrState g_AnchorStates[CAN_ANCHOR_MAX + 1] = {0}; // Initialize all fields to 0

/* Debug print buffer */
extern char gArr_DebugMsg[1024];

/* Local (static) function prototypes */
static void AnchorStateManager_voidLogStateUpdate(uint8_t anchorId, const char* message);

/* ---------------------------------------------------------------------------
 * Anchor_voidUpdateStateFromCAN
 * ---------------------------------------------------------------------------*/
void AnchorStateManager_voidUpdateStateFromCAN(const tCANMsgObject* pRxMsg, const uint8_t* rxData) // rxData pointer to the 8-byte data payload of the CAN message.
{
    uint32_t messageId = pRxMsg->ui32MsgID; // Extracts the messageId to determine the message type and anchor.
    uint8_t anchorId = 0;

    /* Determine anchor ID based on message ID
     * updating the state of BLE anchors based on CAN messages */
    switch (messageId) {
        case CAN_ID_STATUS_A1:
        case CAN_ID_KEEP_ALIVE_A1:
            anchorId = CAN_ANCHOR_1;
            break;
        case CAN_ID_STATUS_A2:
        case CAN_ID_KEEP_ALIVE_A2:
            anchorId = CAN_ANCHOR_2;
            break;
        case CAN_ID_STATUS_A3:
        case CAN_ID_KEEP_ALIVE_A3:
            anchorId = CAN_ANCHOR_3;
            break;
        default:
            return; // Ignore unsupported message IDs
    }

    /* Update anchor state */
    Anchor_tstrState* pAnchor = &g_AnchorStates[anchorId];
    pAnchor->anchorId = anchorId;

    if (messageId >= CAN_ID_STATUS_A1 && messageId <= CAN_ID_STATUS_A3) {
        /* Validate and parse status message */
        if (rxData[1] > 255) return;                                    // Basic validation for firmware version
        pAnchor->firmwareVersion = rxData[1];                           // Byte 1: Firmware version
        if (rxData[2] > ANCHOR_STATE_IDLE) return;                      // Validate state
        pAnchor->state = (Anchor_tenuStates)rxData[2];                  // Byte 2: State
        pAnchor->numDevices = rxData[3];                                // Byte 3: Number of devices
        pAnchor->rangingType = (rxData[4] == 1) ? APP_CS : APP_RSSI;    // Byte 4: 1=CS, 0=RSSI
        pAnchor->lastKeepAliveTimeMs = TimerDriver_GetCurrentTime();    // Update timestamp

        AnchorStateManager_voidLogStateUpdate(anchorId, "State updated from status message");
    }
    else if (messageId >= CAN_ID_KEEP_ALIVE_A1 && messageId <= CAN_ID_KEEP_ALIVE_A3) {
        /* Update only keep-alive timestamp */
        pAnchor->lastKeepAliveTimeMs = TimerDriver_GetCurrentTime();
        AnchorStateManager_voidLogStateUpdate(anchorId, "Keep-alive received");
    }
}

/* ---------------------------------------------------------------------------
 * Anchor_voidLogStateUpdate
 * ---------------------------------------------------------------------------*/
static void AnchorStateManager_voidLogStateUpdate(uint8_t anchorId, const char* message)
{
    const char* stateStr[] = {"UNKNOWN", "ADVERTISING", "CONNECTING", "CONNECTED", "IDLE"};
    const char* rangingStr[] = {"RSSI", "CS"};

    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[INFO] Anchor %d - %s: FW v%d, State: %s, Devices: %d, Mode: %s, Last Keep-Alive: %lu ms\n",
             anchorId,
             message,
             g_AnchorStates[anchorId].firmwareVersion,
             stateStr[g_AnchorStates[anchorId].state],
             g_AnchorStates[anchorId].numDevices,
             rangingStr[g_AnchorStates[anchorId].rangingType],
             g_AnchorStates[anchorId].lastKeepAliveTimeMs);
    UART_SendMessage(gArr_DebugMsg);
}
