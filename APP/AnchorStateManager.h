/***************************************************************************/
/* File: AnchorStateManager.h                                             */
/* Author: Mohamad Waleed Nassar                                         */
/* Description: Manages anchor states (version, status, capabilities, etc)*/
/***************************************************************************/

#ifndef ANCHOR_STATE_MANAGER_H_
#define ANCHOR_STATE_MANAGER_H_

#include <string.h>
#include <stdint.h>

#include "UART/uart.h"
#include "can.h"
#include "can_msg_types.h"
#include "CAN_MasterReceive.h"
#include "APP_FSM.h"         // For APP_tenuRangingType
#include "can.h"             // For tCANMsgObject
#include "can_msg_types.h"   // For CAN_ANCHOR_MAX
#include "Timer.h"           // For timer functions

/* Constants */
#define KEEP_ALIVE_PERIOD_MS 5  // Periodic keep-alive message interval

/* Enum for Anchor States */
typedef enum {
    ANCHOR_STATE_UNKNOWN = 0,
    ANCHOR_STATE_ADVERTISING,
    ANCHOR_STATE_CONNECTING,
    ANCHOR_STATE_CONNECTED,
    ANCHOR_STATE_DEAD,
    ANCHOR_STATE_IDLE
} Anchor_tenuStates;

/* Structure to store anchor state data */
typedef struct {
    uint8_t anchorId;                  // Anchor ID (1-3)
    uint8_t firmwareVersion;           // Version of burnt code firmware
    Anchor_tenuStates state;           // Current state of the anchor
    uint8_t numDevices;                // Number of devices connected
    APP_tenuRangingType rangingType;   // Ranging capability (CS or RSSI)
    uint32_t lastKeepAliveTimeMs;      // Timestamp of last keep-alive message
} Anchor_tstrState;

/* Global variable to store state for each anchor */
extern Anchor_tstrState g_AnchorStates[CAN_ANCHOR_MAX + 1]; // Array for anchors 0-3

/* Function prototype */
void AnchorStateManager_voidUpdateStateFromCAN(const tCANMsgObject* pRxMsg, const uint8_t* rxData);

#endif /* ANCHOR_STATE_MANAGER_H_ */
