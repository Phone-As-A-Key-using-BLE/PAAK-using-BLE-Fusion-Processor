/*
 * APP_FSM.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Mohamed Abdel Hamid
 *  
 *  Description: Application Finite State Machine for vehicle access control system
 *               Handles device pairing, passive entry, distance measurements, and fusion algorithms
 */

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "UART/uart.h"
#include "APP/can_msg_types.h"
#include "APP/CAN_Send.h"
#include "APP/CAN_MasterReceive.h"
#include "DeviceRangingTypeManager.h"
#include "APP/CAN_App.h"
#include "APP/APP_FSM.h"
#include "APP/Timer.h"
#include "APP/ErrorHandling.h"
#include "APP/Connectivity/connectivity.h"
#include "ccc_keys.h"
// Fusion algorithms
#include "APP/Fusion/Trilateration.h"
#include "APP/Fusion/Particle.h"

/*==================== GLOBAL VARIABLES ====================*/

/* Debug message buffer for UART communication */
char gArr_DebugMsg[1024];

/* Current FSM state */
APP_tenuStates currentState = STATE_IDLE;

/* Handover attempt counter */
uint8_t Global_u8HandoverTrials = 0;

/* Distance measurement completion flags for each anchor */
uint8_t Global_DistanceMeasurementDone[CAN_ANCHOR_MAX+1] = {0};

/* Distance readings array - [device_id][anchor_id] */
extern double_t Global_f64DistanceReadings[APP_MAX_NO_OF_DEVICES][CAN_ANCHOR_MAX+1];

/* direction flag */
AnchorTraversalDirection_t Global_traversalDirection = DIRECTION_FORWARD;

/* Current active anchor ID */
uint8_t Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;

/* Next anchor ID chosen */
uint8_t Global_u8NextAnchor = CAN_PRIMARY_ANCHOR;

/* Current device NVM index in anchors */
uint8_t Global_u8CurrentNvmIndex = 0;

/* Store Ranging type of each device for quick access */
APP_tenuRangingType Global_u8DevicesRangingType[APP_MAX_NO_OF_DEVICES] = {0};

/* Number of successful distance measurements */
uint8_t Global_u8NoOfDistances = 0;

/* First-time execution flag */
uint8_t Global_u8FirstTime = 1;

/* Passive Entry retry counter */
uint8_t Global_u8PERetryCount = 0;

/* Command synchronization flags */
uint8_t Global_u8SendPE = 0;        /* Send Passive Entry command */
uint8_t Global_u8SendTDM = 0;       /* Send Trigger Distance Measurement command */
uint8_t Global_u8SendHandover = 0;  /* Send Handover command */

/* Current device ID being processed */
uint8_t Global_u8CurrentDeviceId = 0;

/* Reset and PE flag */
uint8_t Global_u8ResetAndPE = 0;

/* Reset needed flag */
uint8_t Global_u8IgnoreResponse = 0;

/* External variables */
extern CAN_tstrDistance s_distanceData;
extern uint8_t isBondingDataReceived;
extern RSSIData_t gRSSIData[CAN_ANCHOR_MAX + 1];

/* Fusion algorithm variables */
RSSIData_t targetData;
Particle particles[NUM_PARTICLES];
uint8_t volatile firstTimeFlag = 1;

/*==================== PRIVATE FUNCTION DECLARATIONS ====================*/

static inline void FSM_voidResetPEState(void);
static uint8_t FSM_u8FindNextAnchor(void);
static void FSM_voidLogAnchorMessage(const char* Copy_pchLevel, const char* Copy_pchMessage, uint8_t Copy_u8Anchor);
static void FSM_voidHandleIdleState(APP_tenuEvents Copy_enuEvent);
static void FSM_voidHandleTriggerOwnerPairing(APP_tenuEvents Copy_enuEvent);
static void FSM_voidHandleWaitingForBondingData(APP_tenuEvents Copy_enuEvent);
static void FSM_voidHandleWaitingForPrimaryWakeup(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleCSPrimaryPE(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleRSSIPrimaryPE(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleStartCsState(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleStartRSSIState(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandlePrimaryTDM(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleWakeupDecisionMaking(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleDistanceMeasurementInit(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleDistanceMeasurementProcessing(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleDistanceMeasurementHandover(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleDistanceMeasurementEvaluation(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleFusionAlgo(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleVehicleLevelDecisionMaking(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);
static void FSM_voidHandleReturningConnectionToPrimaryAnchor(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId);

/*==================== PRIVATE FUNCTION IMPLEMENTATIONS ====================*/

/**
 * @brief Reset Passive Entry state variables
 * @param None
 * @return None
 */
static inline void FSM_voidResetPEState(void) {
    Global_u8NoOfDistances = 0;
    Global_u8FirstTime = 0;
    Global_u8PERetryCount = 0;
    
    /* Clear distance measurement flags for all anchors */
    uint8_t Loc_u8Index;
    for (Loc_u8Index = CAN_PRIMARY_ANCHOR; Loc_u8Index <= CAN_ANCHOR_MAX; Loc_u8Index++) {
        Global_DistanceMeasurementDone[Loc_u8Index] = 0;
    }
}

/**
 * @brief Find the next available anchor for distance measurement
 * @param None
 * @return uint8_t Next available anchor ID, or > CAN_ANCHOR_MAX if none available
 */
static uint8_t FSM_u8FindNextAnchor(void) {
    int8_t Loc_i8NextAnchor = (int8_t)Global_u8CurrentAnchor + (int8_t)Global_traversalDirection;

    while (Loc_i8NextAnchor >= CAN_ANCHOR_1 && Loc_i8NextAnchor <= CAN_ANCHOR_MAX) {
        if (!Global_DistanceMeasurementDone[Loc_i8NextAnchor]) {
            return (uint8_t)Loc_i8NextAnchor;
        }
        Loc_i8NextAnchor += Global_traversalDirection;
    }

    // Return out-of-range value to trigger evaluation
    return CAN_ANCHOR_MAX + 1;
}


/**
 * @brief Log debug message with anchor information
 * @param Copy_pchLevel Log level (INFO, SUCCESS, ERROR)
 * @param Copy_pchMessage Message to log
 * @param Copy_u8Anchor Anchor ID
 * @return None
 */
static void FSM_voidLogAnchorMessage(const char* Copy_pchLevel, const char* Copy_pchMessage, uint8_t Copy_u8Anchor) {
    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), "\n[%s] %s anchor %d\n", 
             Copy_pchLevel, Copy_pchMessage, Copy_u8Anchor);
    UART_SendMessage(gArr_DebugMsg);
}

/**
 * @brief Handle handover timeout scenario
 * @param Copy_u8DeviceId Device ID that timed out
 * @return None
 */
void FSM_voidHandleHandoverTimeout(uint8_t Copy_u8DeviceId) {
    snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
        "\n[ERROR] Timeout reached in handover from anchor %d. Disconnect device from anchor %d ...\n", 
        Global_u8CurrentAnchor, Global_u8NextAnchor);
    UART_SendMessage(gArr_DebugMsg);

    uint8_t Loc_u8DeviceToDisconnect;

    if(Global_u8DevicesRangingType[Copy_u8DeviceId] == APP_CS){
        Loc_u8DeviceToDisconnect = Global_u8NextAnchor;
        /* Set timeout for disconnect operation */
    }
    else if(Global_u8DevicesRangingType[Copy_u8DeviceId] == APP_RSSI){
        Loc_u8DeviceToDisconnect = Global_u8CurrentAnchor;
    }   
    TimerDriver_Start(TIMEOUT_DISCONNECT, ERR_voidDisconnectTimeOutHandler);
    /* Disconnect to prevent stuck CS context */
    CAN_voidSendCommand(CAN_COMMAND_DISCONNECT_FROM_DEVICE, Loc_u8DeviceToDisconnect, Copy_u8DeviceId);
}

/**
 * @brief Handle handover fail scenario
 * @param Copy_u8DeviceId Device ID that failed
 * @return None
 */
void FSM_voidHandleHandoverFail(uint8_t Copy_u8DeviceId) {
    Global_u8HandoverTrials++;
    if (Global_u8HandoverTrials == MAX_HANDOVER_RETRIES) {
        /* Max retries reached - restart PE */
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
        "\n[ERROR] Max handover from anchor %d to anchor %d reached. Starting PE on anchor %d...\n", 
        Global_u8CurrentAnchor, Global_u8NextAnchor, Global_u8NextAnchor);
        UART_SendMessage(gArr_DebugMsg);
        Global_u8IgnoreResponse = Global_u8CurrentAnchor;
        CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0);
        Global_u8HandoverTrials = 0;
        Global_u8SendPE = 1;
    } else {
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
        "\n[ERROR] Handover fail from anchor %d to anchor %d. Try again...\n", 
        Global_u8CurrentAnchor, Global_u8NextAnchor);
        UART_SendMessage(gArr_DebugMsg);
        TimerDriver_Start(TIMEOUT_HANDOVER, ERR_voidHandoverTimeoutHandler);
        /* Retry handover */
        Global_u8SendHandover = 1;
    }
}

/**
 * @brief Handle IDLE state
 * @param Copy_enuEvent FSM event
 * @return None
 */
static void FSM_voidHandleIdleState(APP_tenuEvents Copy_enuEvent) {
    if (Copy_enuEvent == EVENT_OWNER_PAIRING_BUTTON_PRESSED) {
        UART_SendMessage("\n[INFO] Owner pairing button pressed. Factory reset all anchors...\n");
        currentState = STATE_TRIGGER_OWNER_PAIRING;
        CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);
    }
}

/**
 * @brief Handle waiting for bonding data state
 * @param Copy_enuEvent FSM event
 * @return None
 */
static void FSM_voidHandleTriggerOwnerPairing(APP_tenuEvents Copy_enuEvent) {
    if (Copy_enuEvent == EVENT_PRIMARY_WAKEUP_RECEIVED) {
        /* Initialize pairing process */
        Global_u8CurrentAnchor = CAN_PRIMARY_ANCHOR;
        Global_u8NoOfDistances = 0;
        Global_u8FirstTime = 1;
        Global_u8PERetryCount = 0;
        
        currentState = STATE_WAITING_FOR_BONDING_DATA;
        UART_SendMessage("\n[SUCCESS] Primary anchor wakeup received. Trigger owner pairing on primary anchor...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
    }
}

/**
 * @brief Handle waiting for bonding data state
 * @param Copy_enuEvent FSM event
 * @return None
 */
static void FSM_voidHandleWaitingForBondingData(APP_tenuEvents Copy_enuEvent) {
    if (Copy_enuEvent == EVENT_BONDING_DATA_RECEIVED) {
        currentState = STATE_WAITING_FOR_PRIMARY_WAKEUP;
        UART_SendMessage("\n[INFO] Bonding data received, resetting all anchors. Waiting for wake-up signal from primary anchor...\n");
        CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_RESET_ALL, 0);
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR) {
        UART_SendMessage("\n[ERROR] Trigger owner pairing failed. Trigger OP again...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_OWNER_PAIRING, CAN_PRIMARY_ANCHOR, 0);
    }
}

/**
 * @brief Handle waiting for primary wakeup state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleWaitingForPrimaryWakeup(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_PRIMARY_WAKEUP_RECEIVED) {
        currentState = STATE_CS_PRIMARY_PE;
        UART_SendMessage("\n[INFO] Wake-up signal received. Triggering Passive Entry (PE) in CS mode on primary anchor...\n");
        Global_u8DevicesRangingType[Global_u8CurrentNvmIndex] = APP_CS;
        Global_u8CurrentDeviceId = Copy_u8DeviceId;
        Global_u8SendPE = 1;
    }
}

/**
 * @brief Handle CS primary passive entry state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleCSPrimaryPE(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL) {
        DeviceStateManager_Update(Global_u8CurrentNvmIndex, APP_CS);
        currentState = STATE_PRIMARY_TDM;
        UART_SendMessage("\n[SUCCESS] Primary Passive Entry successful, The device supports CS. Proceeding to Trigger Distance Measurement (TDM)...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR) {
        currentState = STATE_CS_PRIMARY_PE;
        UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Trigger PE again...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_PRIMARY_WAKEUP_RECEIVED) {
        UART_SendMessage("\n[ERROR] CS is not supported in mobile. Turning to RSSI...\n");
        Global_u8DevicesRangingType[Global_u8CurrentNvmIndex] = APP_RSSI;
        currentState = STATE_RSSI_PRIMARY_PE;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
}


/**
 * @brief Handle CS primary passive entry state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleRSSIPrimaryPE(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL) {
        DeviceStateManager_Update(Global_u8CurrentNvmIndex, APP_RSSI);
        currentState = STATE_PRIMARY_TDM;
        UART_SendMessage("\n[SUCCESS] Primary Passive Entry successful, RSSI is used!. Proceeding to Trigger Distance Measurement (TDM)...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR) {
        currentState = STATE_RSSI_PRIMARY_PE;
        UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Trigger PE again...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
}

/**
 * @brief Handle CS start state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleStartCsState(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL) {
        currentState = STATE_PRIMARY_TDM;
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                "\n[SUCCESS] Device %d Connected supports CS!. Trigger distance measurement...\n", 
                Global_u8CurrentDeviceId);
        UART_SendMessage(gArr_DebugMsg);
        // Store friend device ranging type
        if(Global_u8DevicesRangingType[Global_u8CurrentNvmIndex] == APP_NOT_DETERMINED){
            DeviceStateManager_Update(Global_u8CurrentNvmIndex, APP_CS);
        }
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR) {
        currentState = STATE_START_CS_STATE;
        UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Trigger PE again...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_PRIMARY_WAKEUP_RECEIVED) {
        currentState = STATE_START_RSSI_STATE;
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
}

/**
 * @brief Handle RSSI start state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleStartRSSIState(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL) {
        currentState = STATE_PRIMARY_TDM;
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                "\n[SUCCESS] Device %d Connected supports RSSI!. Trigger distance measurement...\n", 
                Global_u8CurrentDeviceId);
        UART_SendMessage(gArr_DebugMsg);
        // Store friend device ranging type
        if(Global_u8DevicesRangingType[Global_u8CurrentNvmIndex] == APP_NOT_DETERMINED){
            DeviceStateManager_Update(Global_u8CurrentNvmIndex, APP_RSSI);
        }
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR) {
        currentState = STATE_START_RSSI_STATE;
        UART_SendMessage("\n[ERROR] Primary Passive Entry failed. Trigger PE again...\n");
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_PASSIVE_ENTRY, CAN_PRIMARY_ANCHOR, Copy_u8DeviceId);
    }
}

/**
 * @brief Handle primary trigger distance measurement state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandlePrimaryTDM(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_RECEIVE_DISTANCE) {
        TimerDriver_Stop();
        currentState = STATE_WAKEUP_DECISION_MAKING;
        UART_SendMessage("\n[SUCCESS] Distance measurement completed. Evaluating wake-up decision...\n");

        /* Evaluate distance threshold */
        uint16_t Loc_u16Distance = (uint16_t)Global_f64DistanceReadings[Copy_u8DeviceId][Global_u8CurrentAnchor];
        if (Loc_u16Distance <= APP_DISTANCE_TRIGGER_THRESHOLD) {
            APP_voidFSMHandler(EVENT_DISTANCE_BELOW_THRESHOLD, Copy_u8DeviceId);
        } else {
            APP_voidFSMHandler(EVENT_DISTANCE_ABOVE_THRESHOLD, Copy_u8DeviceId);
        }
    }
    else if(Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_SECONDARY_ANCHOR || Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR){
        UART_SendMessage("\n[ERROR] Device disconnected. Sending PE again...\n");
        Global_u8NextAnchor = Global_u8CurrentAnchor;
        Global_u8SendPE = 1;
    }
    else if(Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL || Copy_enuEvent == EVENT_SECONDARY_PE_SUCCESSFUL){
        UART_SendMessage("\n[SUCCESS] PE success!. Starting TDM...\n");
        Global_u8SendTDM = 1;
    }
}

/**
 * @brief Handle wakeup decision making state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleWakeupDecisionMaking(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_DISTANCE_BELOW_THRESHOLD) {
        /* Device in range - start multi-anchor distance measurements */
        currentState = STATE_DISTANCE_MEASUREMENT_INIT;
        UART_SendMessage("\n[INFO] Device is in range. Starting distance measurements through anchors...\n");
        Global_u8FirstTime = 1;
        APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE, Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_DISTANCE_ABOVE_THRESHOLD) {
        /* Device out of range - retry primary anchor */
        currentState = STATE_PRIMARY_TDM;
        UART_SendMessage("\n[INFO] Distance above threshold. Retrying primary anchor distance measurement...\n");
        Global_u8SendTDM = 1;
    }
}

/**
 * @brief Handle distance measurement initialization state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleDistanceMeasurementInit(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Copy_enuEvent == EVENT_DEVICE_IN_RANGE) {
        /* Initialize distance measurement process */
        FSM_voidResetPEState();
        
        UART_SendMessage("\n[INFO] Initiating distance measurement process, starting with primary anchor\n");
        
        /* Trigger distance measurement on primary anchor */
        CAN_voidSendCommand(CAN_COMMAND_TRIGGER_DISTANCE_MEASURMENT, Global_u8CurrentAnchor, Copy_u8DeviceId);
        currentState = STATE_DISTANCE_MEASUREMENT_PROCESSING;
    }
}

/**
 * @brief Handle distance measurement processing state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleDistanceMeasurementProcessing(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    TimerDriver_Stop();
    switch (Copy_enuEvent) {
        case EVENT_RECEIVE_DISTANCE:
            /* Record successful distance reading */
            Global_u8NoOfDistances++;
            Global_DistanceMeasurementDone[Global_u8CurrentAnchor] = 1;
            Global_u8PERetryCount = 0;
            
            FSM_voidLogAnchorMessage("SUCCESS", "Distance received from", Global_u8CurrentAnchor);
            
            /* Move to handover state */
            currentState = STATE_DISTANCE_MEASUREMENT_HANDOVER;
            APP_voidFSMHandler(EVENT_RECEIVE_DISTANCE, Copy_u8DeviceId);
            break;

        case EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR:
        case EVENT_DEVICE_DISCONNECTED_FROM_SECONDARY_ANCHOR:
            UART_SendMessage("\n[SUCCESS] Device Disconnected.\n");
            
            /* Schedule reset and PE restart */
            Global_u8CurrentDeviceId = Copy_u8DeviceId;
            UART_SendMessage("\n[INFO] Reset anchor and start PE again...\n");
            // Global_u8IgnoreResponse = Global_u8CurrentAnchor;
            // CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0); 
            Global_u8CurrentAnchor = Global_u8NextAnchor;
            Global_u8ResetAndPE = 2;
            break;

        case EVENT_HANDOVER_SUCCESS:
            Global_u8IgnoreResponse = Global_u8CurrentAnchor;
            FSM_voidLogAnchorMessage("SUCCESS", "Handover is done. Reset previous anchor and trigger distance measurement on ", Global_u8NextAnchor);

            /* Reset previous anchor and trigger distance measurement */
            Global_u8CurrentDeviceId = Copy_u8DeviceId;

            CAN_voidSendCommand(CAN_COMMAND_RESET, Global_u8CurrentAnchor, 0);
            Global_u8CurrentAnchor = Global_u8NextAnchor;

            Global_u8SendTDM = 1;
            break;

        case EVENT_HANDOVER_FAILED:
            FSM_voidHandleHandoverFail(Copy_u8DeviceId);
            break;

        case EVENT_PRIMARY_WAKEUP_RECEIVED:
        case EVENT_SECONDARY_WAKEUP_RECEIVED:
            FSM_voidHandleHandoverTimeout(Copy_u8DeviceId);
            break;
        
        case EVENT_PRIMARY_PE_SUCCESSFUL:
        case EVENT_SECONDARY_PE_SUCCESSFUL:
            snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                    "\n[SUCCESS] Passive entry is done on anchor %d. Trigger distance measurement on this anchor...\n", 
                    Global_u8CurrentAnchor);
            UART_SendMessage(gArr_DebugMsg);
            
            Global_u8CurrentAnchor = Global_u8NextAnchor;
            /* Trigger distance measurement */
            Global_u8CurrentDeviceId = Copy_u8DeviceId;
            Global_u8SendTDM = 1;
            break;
            
        default:
            /* Unhandled event - no action required */
            break;
    }
}

/**
 * @brief Handle distance measurement handover state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleDistanceMeasurementHandover(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    /* Only process relevant events */
    if (Copy_enuEvent != EVENT_RECEIVE_DISTANCE && Copy_enuEvent != EVENT_HANDOVER_FAILED) {
        return;
    }
    
    Global_u8NextAnchor = FSM_u8FindNextAnchor();
    
    if (Global_u8NextAnchor <= CAN_ANCHOR_MAX) {
        /* More anchors available - initiate handover */
        Global_u8CurrentDeviceId = Copy_u8DeviceId;
        Global_u8SendHandover = 1;
        
        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                "\n[INFO] Sending handover command from anchor %d to anchor %d...\n", 
                Global_u8CurrentAnchor, Global_u8NextAnchor);
        UART_SendMessage(gArr_DebugMsg);
        
        currentState = STATE_DISTANCE_MEASUREMENT_PROCESSING;
        TimerDriver_Start(TIMEOUT_HANDOVER, ERR_voidHandoverTimeoutHandler);
    } else {
        /* No more anchors - evaluate results */
        Global_u8NextAnchor = CAN_ANCHOR_MAX - 1;
        currentState = STATE_DISTANCE_MEASUREMENT_EVALUATION;
        APP_voidFSMHandler(Copy_enuEvent, Copy_u8DeviceId);
    }
}

/**
 * @brief Handle distance measurement evaluation state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleDistanceMeasurementEvaluation(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if (Global_u8NoOfDistances >= APP_MINUMUM_DISTANCE_READINGS) {
        TimerDriver_Stop();
        /* Sufficient readings collected */
        FSM_voidResetPEState();
        UART_SendMessage("\n[INFO] Minimum distance readings met. Handover the connection to the primary anchor...\n");
        
        currentState = STATE_FUSION_ALGO;

        APP_voidFSMHandler(EVENT_START_FUSION, Copy_u8DeviceId);
    } else {
        /* Insufficient readings - restart process */
        FSM_voidResetPEState();
        Global_u8FirstTime = 1;
        
        UART_SendMessage("\n[INFO] Loop again through anchors...\n");
        
        currentState = STATE_DISTANCE_MEASUREMENT_INIT;
        APP_voidFSMHandler(EVENT_DEVICE_IN_RANGE, Copy_u8DeviceId);
    }
}


/**
 * @brief Handle fusion algorithm state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleFusionAlgo(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    if(Copy_enuEvent == EVENT_START_FUSION){
        object_list Loc_astrTestObj[CAN_ANCHOR_MAX];
        double_t Loc_af64EstimateFinal[2];
        Measurement_Type Loc_strMeasureA;

        /* Prepare distance data for trilateration */
        Loc_astrTestObj[0] = (object_list){Global_f64DistanceReadings[Copy_u8DeviceId][1], 1};
        Loc_astrTestObj[1] = (object_list){Global_f64DistanceReadings[Copy_u8DeviceId][2], 2};
        Loc_astrTestObj[2] = (object_list){Global_f64DistanceReadings[Copy_u8DeviceId][3], 3};

        /* Initialize estimate array */
        Loc_af64EstimateFinal[0] = 0;
        Loc_af64EstimateFinal[1] = 0;

        /* Execute trilateration algorithm */
        Loc_strMeasureA = Master_trilaterate_position(Loc_astrTestObj);

        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                "\n[INFO] Location from Trilateration: (x = %.2f , y = %.2f)\n", 
                Loc_strMeasureA.x, Loc_strMeasureA.y);
        UART_SendMessage(gArr_DebugMsg);

        /* Particle filter implementation (commented out for performance) */
        /*
        if (firstTimeFlag) {
            firstTimeFlag = 0;
            Master_initialize_particles(particles, Loc_strMeasureA.x, Loc_strMeasureA.y, 1.00);
        }

        Master_prediction(particles);
        Master_update_particles(particles, Loc_strMeasureA);
        Master_resample(particles);
        Master_estimate(particles, Loc_af64EstimateFinal);

        snprintf(gArr_DebugMsg, sizeof(gArr_DebugMsg), 
                "\n[INFO] Location from Particle Filter: (x = %.2f , y = %.2f)\n", 
                Loc_af64EstimateFinal[0], Loc_af64EstimateFinal[1]);
        UART_SendMessage(gArr_DebugMsg);
        */

        UART_SendMessage("\n[INFO] Fusion Algorithm is done, go to vehicle decision-making...\n");
        
        /* Reset secondary anchors and return to primary TDM */
        currentState = STATE_VEHICLE_LEVEL_DECISION_MAKING;
        APP_voidFSMHandler(EVENT_TAKE_DECISION, Copy_u8DeviceId);
    }
}

/**
 * @brief Handle vehicle level decision making state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleVehicleLevelDecisionMaking(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
        
        // Decisions based on location


        // Reverse direction for the next cycle
        Global_traversalDirection = (Global_traversalDirection == DIRECTION_FORWARD) 
                                    ? DIRECTION_BACKWARD 
                                    : DIRECTION_FORWARD;

        // Set next anchor in the new direction
        Global_u8CurrentAnchor = (Global_traversalDirection == DIRECTION_FORWARD) 
                                ? CAN_ANCHOR_1 
                                : CAN_ANCHOR_MAX;
        
        if(Global_u8CurrentAnchor != CAN_PRIMARY_ANCHOR){
            Global_u8IgnoreResponse = CAN_PRIMARY_ANCHOR;
            CAN_voidSendCommand(CAN_COMMAND_RESET, CAN_PRIMARY_ANCHOR, 0);
        }
        currentState = STATE_PRIMARY_TDM;
        Global_u8SendTDM = 1;
}


/**
 * @brief Handle handover connection to primary anchor state
 * @param Copy_enuEvent FSM event
 * @param Copy_u8DeviceId Device ID
 * @return None
 */
static void FSM_voidHandleReturningConnectionToPrimaryAnchor(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    TimerDriver_Stop();
    
    if (Copy_enuEvent == EVENT_HANDOVER_SUCCESS || Copy_enuEvent == EVENT_PRIMARY_PE_SUCCESSFUL) {
        UART_SendMessage("\n[SUCCESS] Connection now is on the primary anchor. Returning to distance measurement on primary anchor...\n");
        Global_u8HandoverTrials = 0;
        Global_u8CurrentAnchor = CAN_ANCHOR_1;
        uint8_t Loc_u8Index;
        
        currentState = STATE_PRIMARY_TDM;
        for (Loc_u8Index = CAN_PRIMARY_ANCHOR + 1; Loc_u8Index <= CAN_ANCHOR_MAX; Loc_u8Index++) {
            CAN_voidSendCommand(CAN_COMMAND_RESET, Loc_u8Index, 0);
        }
        
        Global_u8SendTDM = 1;
    }
    else if (Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_PRIMARY_ANCHOR || 
             Copy_enuEvent == EVENT_DEVICE_DISCONNECTED_FROM_SECONDARY_ANCHOR) {
        /* Device disconnected - reset and restart PE */
        Global_u8CurrentAnchor = CAN_ANCHOR_1;
        Global_u8CurrentDeviceId = Copy_u8DeviceId;
        Global_u8ResetAndPE = 2;
    }
    else if (Copy_enuEvent == EVENT_HANDOVER_FAILED) {
        Global_u8CurrentAnchor = CAN_ANCHOR_1;
        FSM_voidHandleHandoverFail(Copy_u8DeviceId);
    }
    else if (Copy_enuEvent == EVENT_SECONDARY_WAKEUP_RECEIVED) {
        /* Handle timeout scenario */
        Global_u8CurrentAnchor = CAN_ANCHOR_1;
        Global_u8CurrentDeviceId = Copy_u8DeviceId;
        FSM_voidHandleHandoverTimeout(Copy_u8DeviceId);
    }

}

/*==================== PUBLIC FUNCTION IMPLEMENTATIONS ====================*/

/**
 * @brief Main FSM handler function
 * @param Copy_enuEvent Event to process
 * @param Copy_u8DeviceId Device to handle
 * @return None
 * 
 * This function implements the main state machine logic for the vehicle access control system.
 * It handles various states including device pairing, passive entry, distance measurements,
 * and sensor fusion algorithms.
 */
void APP_voidFSMHandler(APP_tenuEvents Copy_enuEvent, uint8_t Copy_u8DeviceId) {
    uint8_t Loc_u8DeviceId = Copy_u8DeviceId;

    switch (currentState) {
        case STATE_IDLE:
            FSM_voidHandleIdleState(Copy_enuEvent);
            break;

        case STATE_TRIGGER_OWNER_PAIRING:
            FSM_voidHandleTriggerOwnerPairing(Copy_enuEvent);
            break;

        case STATE_WAITING_FOR_BONDING_DATA:
            FSM_voidHandleWaitingForBondingData(Copy_enuEvent);
            break;

        case STATE_WAITING_FOR_PRIMARY_WAKEUP:
            FSM_voidHandleWaitingForPrimaryWakeup(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_CS_PRIMARY_PE:
            FSM_voidHandleCSPrimaryPE(Copy_enuEvent, Loc_u8DeviceId);
            break;
       
        case STATE_RSSI_PRIMARY_PE:
            FSM_voidHandleRSSIPrimaryPE(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_START_CS_STATE:
            FSM_voidHandleStartCsState(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_START_RSSI_STATE:
            FSM_voidHandleStartRSSIState(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_PRIMARY_TDM:
            FSM_voidHandlePrimaryTDM(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_WAKEUP_DECISION_MAKING:
            FSM_voidHandleWakeupDecisionMaking(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_DISTANCE_MEASUREMENT_INIT:
            FSM_voidHandleDistanceMeasurementInit(Copy_enuEvent, Loc_u8DeviceId);
            break;
            
        case STATE_DISTANCE_MEASUREMENT_PROCESSING:
            FSM_voidHandleDistanceMeasurementProcessing(Copy_enuEvent, Loc_u8DeviceId);
            break;
            
        case STATE_DISTANCE_MEASUREMENT_HANDOVER:
            FSM_voidHandleDistanceMeasurementHandover(Copy_enuEvent, Loc_u8DeviceId);
            break;
            
        case STATE_DISTANCE_MEASUREMENT_EVALUATION:
            FSM_voidHandleDistanceMeasurementEvaluation(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_FUSION_ALGO:
            FSM_voidHandleFusionAlgo(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_VEHICLE_LEVEL_DECISION_MAKING:
            FSM_voidHandleVehicleLevelDecisionMaking(Copy_enuEvent, Loc_u8DeviceId);
            break;

        case STATE_HANDOVER_CONNECTION_TO_PRIMARY_ANCHOR:
            FSM_voidHandleReturningConnectionToPrimaryAnchor(Copy_enuEvent, Loc_u8DeviceId);
            break;

        default:
            UART_SendMessage("[ERROR] Unknown system state encountered.\n");
            /* Reset to idle state on unknown state */
            currentState = STATE_IDLE;
            break;
    }
}